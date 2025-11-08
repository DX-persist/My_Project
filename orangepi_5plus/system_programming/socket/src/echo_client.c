#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "msg.h"

typedef struct{
	char ip[16];
	int port;
	int sockfd;
	struct sockaddr_in addr;
	socklen_t addrlen;
	int retval;
}socket_config_t;

socket_config_t server;

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

int create_socket(int *sockfd)
{
	*sockfd = socket(AF_INET, SOCK_STREAM, 0);
	CHECK_ERROR(*sockfd < 0, "socket error");

	return 0;
}

int connect_func(socket_config_t *server, char *param1, char *param2)
{
	//从命令行获取服务器的IP地址和端口号共connect函数使用
	server->addr.sin_family = AF_INET;
	server->addr.sin_port = htons(atoi(param2));
	CHECK_ERROR(inet_pton(AF_INET, param1, 
				&server->addr.sin_addr) < 0, 
				"inet_pton error");

	server->addrlen = sizeof(server->addr);

	server->retval = connect(server->sockfd, 
						(struct sockaddr *)&server->addr, 
						server->addrlen);
	CHECK_ERROR(server->retval < 0, "connect error");

	return 0;
}

int commun_func(int sockfd)
{
	char buffer[512];
	char *prompt = ">";
	ssize_t size;

	while(1){
		write(STDOUT_FILENO, prompt, 1);	
		memset(buffer, '\0', sizeof(buffer));
		
		//从标准输入中获取数据发送给服务器
		if((size = read(STDIN_FILENO, buffer, sizeof(buffer))) < 0){
			perror("read error");
			continue;
		}
		buffer[size - 1] = '\0';

		//如果收到exit的退出指令的时候就关闭套接字描述符并退出程序
		if(!(strcmp(buffer, "exit"))){
			printf("Received exit cmd,program exit...\n");
			close(sockfd);
			exit(0);
		}

		//将数据通过套接字描述符发送给服务器
		if(write_msg(sockfd, buffer, sizeof(buffer)) < 0){
			perror("write_msg error");
			continue;
		}else{
			//然后再通过套接字描述符读取数据并打印
			if(read_msg(sockfd, buffer, sizeof(buffer)) < 0){
				perror("read_msg error");
			}else{
				printf("%s\n",buffer);
			}
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	if(argc < 3){
		fprintf(stderr, "Usage: [%s] [IP] [Port]\n",argv[0]);
		return -1;
	}

	memset(&server, '\0', sizeof(server));

	//1.创建流式套接字(TCP协议)
	create_socket(&server.sockfd);
	//2.连接主机
	CHECK_ERROR_CLEANUP(connect_func(&server, argv[1], argv[2]), 
			"Failed to connected the server...\n", 
			close(server.sockfd));
	//3.和主机进行通信
	CHECK_ERROR_CLEANUP(commun_func(server.sockfd) < 0, 
			"Failed to communicate with server...\n", 
			close(server.sockfd));
	//4.关闭套接字描述符
	close(server.sockfd);


	return 0;
}
