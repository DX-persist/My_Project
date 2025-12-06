#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include "vessel_fd.h"

typedef struct{
	char ip[16];				//存储IP地址字符串
	int port;					//端口号
	int sockfd;					//套接字文件描述符
	struct sockaddr_in addr;	//socket地址结构
	socklen_t addrlen;			//地址结构长度
	int retval;					//函数返回值，用于错误检查
}socket_config_t;

#define CHECK_ERROR(condition, errmsg)	\
	do{									\
		if(condition){					\
			perror(errmsg);				\
			return -1;					\
		}								\
	}while(0)	

#define CHECK_ERROR_CLEANUP(condition, errmsg, cleanup)	\
	do{													\
		if(condition){									\
			fprintf(stderr, "%s\n",errmsg);				\
			cleanup;									\
			exit(1);									\
		}												\
	}while(0)

socket_config_t server;
socket_config_t client;
vessel_fd_t *ves = NULL;

void signal_handler(int signum)
{
	//收到Ctrl+C信号关闭套接字描述符
	char *sigint_msg = "Received a signal is SIGINT, server closed and clean up resources...\n";
	if(signum == SIGINT){
		write(STDOUT_FILENO, sigint_msg, strlen(sigint_msg));
		close(server.sockfd);
		//销毁创建的动态数组
		destroy_vessel(ves);
		exit(EXIT_SUCCESS);
	}
}

int create_socket(int *sockfd)
{
	*sockfd = socket(AF_INET, SOCK_STREAM, 0);
	CHECK_ERROR(*sockfd < 0, "socket error");

	return 0;
}

int get_hostaddress(char *cmd, socket_config_t *server)
{
	FILE *fp = popen(cmd, "r");
	if(fp == NULL){
		perror("popen error");
		return -1;
	}

	if(fgets(server->ip, sizeof(server->ip), fp) == NULL){
		perror("fgets error");
		pclose(fp);
		return -1;
	}
	//由于分配16字节大小，实际的ip地址为192.168.0.40加换行符占13字节，但是最后不需要
	//换行符，将从换行符开始全部填充为'\0'
	server->ip[(strcspn(server->ip, "\n"))] = '\0';

	pclose(fp);

	//验证IP地址是否有效,若没有写入到server->ip中,则为空,strlen(server->ip)的结果为0
	if(strlen(server->ip) == 0){
		fprintf(stderr, "Failed to get vaild IP address\n");
		return -1;
	}

	return 0;
}

int bind_func(socket_config_t *server, char *para)
{
	//设置SO_REUSEADDR选项，允许端口快速重用，避免"Address already in use"错误
	int optval = 1;
	CHECK_ERROR(setsockopt(server->sockfd, SOL_SOCKET, 
				SO_REUSEADDR, &optval, sizeof(optval)) < 0, 
				"setsockopt error");

	//采用IPv4
	server->addr.sin_family = AF_INET;
	//获取端口号
	server->port = atoi(para);
	server->addr.sin_port = htons(atoi(para));
	//将主机字节序转换为网络字节序
	CHECK_ERROR(inet_pton(AF_INET, server->ip, &server->addr.sin_addr) <= 0, 
				"inet_pton error");

	//将套接字与地址和端口号进行绑定
	server->retval = bind(server->sockfd, 
						(struct sockaddr *)&server->addr, 
							sizeof(server->addr));
	CHECK_ERROR(server->retval < 0, "bind error");

	return 0;
}

int listen_func(socket_config_t *server)
{
	CHECK_ERROR(listen(server->sockfd, 10) < 0, "listen error");

	fprintf(stdout, "Host IP:[%s] Port:[%d] is waiting for connection...\n",
					server->ip,server->port);

	return 0;
}

void out_client_msg(socket_config_t *client)
{
	int port;
	char ip[16];

	memset(ip, '\0', sizeof(ip));
	
	//输出连接上来的客户端信息
	assert(client != NULL);
	inet_ntop(AF_INET, &client->addr.sin_addr.s_addr, ip, sizeof(ip));
	port = ntohs(client->addr.sin_port);

	printf("client [%s](%d) connected...\n",ip, port);
}

void commun_func(int new_fd)
{
	char buffer[512];
	ssize_t size;

	memset(buffer, '\0', sizeof(buffer));
	//size < 0表示read读取出错,可以通过perror函数查看errno
	if((size = read(new_fd, buffer, sizeof(buffer))) < 0){
		if(errno == EAGAIN || errno == EWOULDBLOCK){
			return;
		}else{
			perror("read error");
			remove_vessel_fd(ves, new_fd);
			close(new_fd);
		}
	//size = 0表示客户端关闭,对方可能调用了close或者shutdown	
	}else if(size == 0){
		char info[] = "client closed...\n";
		write(STDOUT_FILENO, info, sizeof(info));
		remove_vessel_fd(ves, new_fd);
		close(new_fd);
	//size > 0表示正常读取到了数据
	}else{
		printf("Server receive TID:[0x%lx] client's message is : %s\n",
						(unsigned long)pthread_self(), buffer);	
		//若向已经断开连接的客户端发送消息,则会收到SIGPIPE信号,并将errno设置为EPIPE
		if(write(new_fd, buffer, size) != size){
			if(errno == EPIPE){
				char info[] = "client closed...\n";
			write(STDOUT_FILENO, info, sizeof(info));
			remove_vessel_fd(ves, new_fd);
			close(new_fd);
			}
		}
	}
}

int get_readfd(fd_set *readfds)
{
	int i;
	int max_fd = -1;

	FD_ZERO(readfds);		//清空集合
	for(i = 0; i < ves->counter; i++){
		if(max_fd < ves->fd[i])	max_fd = ves->fd[i];
		FD_SET(get_vessel_fd(ves, i), readfds);	
	}
	return max_fd;
}

void *thread_func(void *arg)
{
	int ret = -1;
	int max_fd = 0;
	int i;
	fd_set readfds;
	struct timeval tv;
	
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	max_fd = get_readfd(&readfds);

	while((ret = select(max_fd + 1, &readfds, NULL, NULL, &tv)) >= 0){
		if(ret > 0){
			int i;

			for(i = 0; i < ves->counter; i++){
				if(FD_ISSET(get_vessel_fd(ves, i), &readfds)){
					commun_func(ves->fd[i]);
				}
			}
		}
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		max_fd = get_readfd(&readfds);
	}
	
	pthread_exit(NULL);
}

void accept_func(socket_config_t server, socket_config_t *client)
{
	/*
	* 主线程使用accept()接受来自客户端的连接
	* 并将新的套接字描述符放到动态数组中
	*
	* 子线程使用select()函数委托内核检查套接字描述符集是否就绪
	* 若就绪则调用read/write与客户端进行通信
	* (在调用select后的套接字描述符自动被设置为非阻塞状态)
	*
	*/
	client->addrlen = sizeof(client->addr);
	
	while(1){
		int new_fd = accept(server.sockfd, 
				(struct sockaddr *)&client->addr, 
				&client->addrlen);
		if(new_fd < 0){
			perror("accept error");
			continue;
		}
		out_client_msg(client);

		/* 将 new_fd 置为非阻塞 */
        int flags = fcntl(new_fd, F_GETFL, 0);
        if(flags >= 0){
            fcntl(new_fd, F_SETFL, flags | O_NONBLOCK);
        }

		//将设置好的套接字描述符放到动态数组中
		add_vessel_fd(ves, new_fd);
	}
}

int main(int argc, char **argv)
{
	//从命令行获取参数(获取主机的端口号)
	if(argc < 2){
		fprintf(stderr, "Usage: [%s] [Port]\n",argv[0]);
		return -1;
	}

	//向内核注册信号处理函数，当接收到对应的信号就转而去执行信号处理函数
	CHECK_ERROR(signal(SIGINT, signal_handler) == SIG_ERR, 
			"signal  sigint error");

	//清空结构体内容
	memset(&server, '\0', sizeof(server));
	memset(&client, '\0', sizeof(client));

	//1.创建流式套接字(TCP协议)
	if(create_socket(&server.sockfd) < 0){
		exit(1);	
	}

	//获取主机IP地址
	char *cmd = "ifconfig | awk '/192/' | awk '{print $2}'";
	CHECK_ERROR_CLEANUP(get_hostaddress(cmd, &server) < 0, 
			"Failed to get host address", 
			close(server.sockfd));

	//2.绑定协议族、IP地址和端口号
	CHECK_ERROR_CLEANUP(bind_func(&server, argv[1]) < 0, 
			"Failed to bind socket", 
			close(server.sockfd));

	//3.监听客户端的连接
	CHECK_ERROR_CLEANUP(listen_func(&server) < 0, 
			"Failed to listen the request", 
			close(server.sockfd));

	//创建动态数组用来存放和客户端连接的socket套接字
	ves = create_vessel();

	//初始化线程属性
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	//设置线程属性为分离线程
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);	
	
	//创建线程与客户端进行通信
	pthread_t tid;
	if(pthread_create(&tid, &attr, thread_func, NULL) != 0){
		perror("pthread_create error");
	}	

	//销毁线程属性
	pthread_attr_destroy(&attr);	

	//4.允许客户端连接主机
	accept_func(server, &client);

	//关闭套接字描述符防止资源泄漏
	close(server.sockfd);

	return 0;
}
