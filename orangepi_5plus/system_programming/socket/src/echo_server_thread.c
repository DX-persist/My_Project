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
#include <arpa/inet.h>
#include <pthread.h>
#include "msg.h"

typedef struct{
	char ip[16];				//存储IP地址字符串
	int port;					//端口号
	int sockfd;					//套接字文件描述符
	struct sockaddr_in addr;	//socket地址结构
	socklen_t addrlen;			//地址结构长度
	int retval;					//函数返回值，用于错误检查
}socket_config_t;

typedef struct{
	int sockfd;
	unsigned long int main_tid;
}temp_t;

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

void signal_handler(int signum)
{
	//收到Ctrl+C信号关闭套接字描述符
	char *sigint_msg = "Received a signal is SIGINT, server closed and clean up resources...\n";
	if(signum == SIGINT){
		write(STDOUT_FILENO, sigint_msg, strlen(sigint_msg));
		close(server.sockfd);
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

void out_client_msg(int sockfd)
{
	int port;
	char ip[16];

	memset(ip, '\0', sizeof(ip));

	//调用getpeername函数通过传入的套接字描述符获取客户端的IP地址和端口
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	getpeername(sockfd, (struct sockaddr *)&addr, &addrlen);

	port = ntohs(addr.sin_port);
	inet_ntop(AF_INET, &addr.sin_addr.s_addr,ip, sizeof(ip));
	
	fprintf(stdout, "Receive a connection: IP:[%s] Port:[%d] connected....\n",ip, port);
}

void commun_func(int new_fd)
{
	char buffer[512];
	ssize_t size;

	while(1){
		memset(buffer, '\0', sizeof(buffer));
		//size < 0表示read读取出错,可以通过perror函数查看errno
		if((size = read_msg(new_fd, buffer, sizeof(buffer))) < 0){
			perror("read error");
			break;
		//size = 0表示客户端关闭,对方可能调用了close或者shutdown	
		}else if(size == 0){
			fprintf(stderr, "peer closed the connection\n");
			break;
		//size > 0表示正常读取到了数据
		}else{
			printf("Server receive TID:[0x%lx] client's message is : %s\n",
							(unsigned long)pthread_self(), buffer);	
			//若向已经断开连接的客户端发送消息,则会收到SIGPIPE信号,并将errno设置为EPIPE
			if(write_msg(new_fd, buffer, sizeof(buffer)) < 0){
				if(errno == EPIPE){
					fprintf(stderr, "peer closed the connection\n");
					break;
				}
				perror("write error");
			}
		}
	}
}

void *thread_func(void *arg)
{
	temp_t *temp = (temp_t *)arg;

	out_client_msg(temp->sockfd);
	printf("create new thread, thread id:[0x%lx] main thread id:[0x%lx]\n",
					pthread_self(),temp->main_tid);
	commun_func(temp->sockfd);
	
	close(temp->sockfd);
	free(temp);

	pthread_exit(NULL);
}

void accept_func(socket_config_t server, socket_config_t *client)
{
	client->addrlen = sizeof(client->addr);
	

	while(1){
		int new_fd = accept(server.sockfd, 
				(struct sockaddr *)&client->addr, 
				&client->addrlen);
		if(new_fd < 0){
			perror("accept error");
			continue;
		}

		temp_t *temp = (temp_t *)malloc(sizeof(temp_t));
		//获取主线程的线程ID
		temp->sockfd = new_fd;
		temp->main_tid = pthread_self();

		//初始化线程属性
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		//设置线程属性为分离线程
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);	
		
		//创建线程与客户端进行通信
		pthread_t tid;
		if(pthread_create(&tid, &attr, thread_func, (void *)temp) != 0){
			perror("pthread_create error");
			continue;
		}	

		//销毁线程属性
		pthread_attr_destroy(&attr);	
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


	//4.允许客户端连接主机
	accept_func(server, &client);

	//关闭套接字描述符防止资源泄漏
	close(server.sockfd);

	return 0;
}
