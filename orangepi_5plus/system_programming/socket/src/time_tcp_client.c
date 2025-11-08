#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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
	char read_buffer[48];
	char write_buffer[48];

	memset(read_buffer, '\0', sizeof(read_buffer));
	memset(write_buffer, '\0', sizeof(write_buffer));

	if(read(sockfd, read_buffer, 
				sizeof(read_buffer)) < 0){
		perror("read error");
		return -1;
	}else{
		 write(STDOUT_FILENO, read_buffer, sizeof(read_buffer));
	}

	strncpy(write_buffer, 
			"Hello, server.This is reply from client...", 
			sizeof(write_buffer));
	if(write(sockfd, write_buffer, sizeof(write_buffer)) != sizeof(write_buffer)){
		perror("write error");
		return -1;
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
