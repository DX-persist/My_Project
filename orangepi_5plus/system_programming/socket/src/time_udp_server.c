#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

typedef struct{
	int sockfd;
	int retval;
	char ip[16];
	int port;
	struct sockaddr_in addr;
	socklen_t addrlen;
}socket_udp_config_t;

#define CHECK_ERROR(condition, errmsg)		\
	do{										\
		if(condition){						\
			perror(errmsg);					\
			exit(1);						\
		}									\
	}while(0)								

socket_udp_config_t server;

void signal_handler(int signum)
{
	if(signum == SIGINT){
		printf("Received a signal is SIGINT, program exit and release the resource...\n");
		close(server.sockfd);
		exit(0);
	}
}

int create_socket(int *sockfd)
{
	//通过SOCK_DGRAM参数指定创建数据报套接字
	*sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(*sockfd < 0){
		perror("socket error");
		return -1;
	}
	return 0;
}

int get_hostaddr(char *cmd, char *ip)
{
	//使用popen创建管道来执行指令
	FILE *fp = popen(cmd, "r");
	if(fp == NULL){
		perror("popen error");
		return -1;
	}
	
	//使用fgets函数来将指令执行的结果放入用户自定义的缓存中
	if(fgets(ip, 16, fp) == NULL){
		perror("fgets error");
		pclose(fp);
		return -1;
	}
	//若ip中含有换行符,将换行符替换为\0
	ip[(strcspn(ip, "\n"))] = '\0';

	pclose(fp);

	return 0;
}

int bind_func(socket_udp_config_t *server, char *param)
{
	//初始化结构体成员
	server->addr.sin_family = AF_INET;
	server->addr.sin_port = htons(atoi(param));
	inet_pton(AF_INET, server->ip, &server->addr.sin_addr.s_addr);
	server->addrlen = sizeof(server->addr);

	if((server->retval = bind(server->sockfd, 
					(struct sockaddr *)&server->addr, 
						server->addrlen)) < 0){
		perror("bind error");
		return -1;
	}
	return 0;
}

void out_client_msg(struct sockaddr_in *client)
{
	int port;
	char ip[16];

	memset(ip, '\0', sizeof(ip));
	port = ntohs(client->sin_port);
	inet_ntop(AF_INET, &client->sin_addr.s_addr, ip, sizeof(ip));
	
	printf("Client IP:[%s] Port:[%d]\n",ip, port);
}

int commun_func(int sockfd)
{
	char buffer[32];
	struct sockaddr_in client_addr;
	socklen_t addrlen = sizeof(client_addr);

	memset(buffer, '\0', sizeof(buffer));
	memset(&client_addr, '\0', sizeof(client_addr));

	printf("waiting for message from client...\n");
	//读取来自客户端的消息
	if(recvfrom(sockfd, buffer, sizeof(buffer), 0, 
				(struct sockaddr *)&client_addr, 
					&addrlen) < 0){
			perror("recvfrom error");
			return -1;
	}else{
		//如果检测到客户端发送了"clear"指令,就调用shell指令clear达到清屏的效果
		if(!(strcmp(buffer, "clear"))){
			system("clear");
			printf("Server received clear command.\n");
		}else{
			printf("Server received message:%s\n",buffer);
			out_client_msg(&client_addr);
		}
	}

	//发送消息给客户端
	time_t now = time(NULL);
	char time_buffer[32];
	strncpy(time_buffer, ctime(&now), sizeof(time_buffer));
	time_buffer[(strcspn(time_buffer, "\n"))] = '\0';

	if(sendto(sockfd, time_buffer, sizeof(time_buffer), 0, 
				(struct sockaddr *)&client_addr, 
					sizeof(client_addr)) < 0){
		perror("send error");
		return -1;
	}else{
		return 0;
	}
	
}

int main(int argc, char **argv)
{
	//从命令行获取参数(主机端口号)
	if(argc < 2){
		fprintf(stderr, "Usage: [%s] [Port]\\n",argv[0]);
		return -1;
	}

	if(signal(SIGINT, signal_handler) == SIG_ERR){
		perror("signal error");
		return -1;
	}

	//清空结构体内容
	memset(&server, '\0', sizeof(server));

	//1.创建数据报套接字
	CHECK_ERROR(create_socket(&server.sockfd) < 0, "Failed to create socket error");

	//允许端口复用(防止服务器异常退出导致端口暂时不可用)
	int optval = 1;
	if(setsockopt(server.sockfd, SOL_SOCKET, 
		SO_REUSEADDR, &optval, sizeof(optval)) < 0){
		perror("setsockopt reuseaddr error");
		close(server.sockfd);
	}

	//获取本地IP地址
	char *cmd = "hostname -I | awk '{print $1}'";
	CHECK_ERROR(get_hostaddr(cmd, server.ip) < 0, "Failed to get host address");

	//2.将套接字与地址族、IP地址和端口号进行绑定
	CHECK_ERROR(bind_func(&server, argv[1]) < 0, "Failed to bind with server");

	//3.与客户端进行通信
	while(1){
		CHECK_ERROR(commun_func(server.sockfd) < 0, "Failed to communicate with client");
	}
	

	//4.关闭连接
	close(server.sockfd);

	return 0;
}
