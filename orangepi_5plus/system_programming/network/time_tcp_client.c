#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>

void commun_func(int sockfd)
{
	char buffer[128];
	memset(buffer, 0, sizeof(buffer));
	ssize_t size;

	if((size = read(sockfd, buffer, sizeof(buffer))) < 0)
	{
		perror("client read error");
		exit(EXIT_FAILURE);
	}	
	printf("client receive a message:%s\n",buffer);

	char reply[128];
	memset(reply, 0, sizeof(reply));

	strcpy(reply,"hello, server");

	if(write(sockfd, reply, strlen(reply)) != strlen(reply))
	{
		perror("write error");
		exit(EXIT_FAILURE);
	}
	
}

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		fprintf(stderr, "Usage:%s server's ip server's port\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int sockfd;

	//step1:客户端创建套接字，执行Internet协议族为IPV4
	//传输协议采用TCP协议
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	//step2:连接到服务器（指定服务器的ip地址和端口号）
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;			//指定Internet协议族为IPV4
	serveraddr.sin_port = htons(atoi(argv[2]));			//指定端口号（网络字节序）
	if(inet_pton(AF_INET, argv[1], &serveraddr.sin_addr.s_addr) < 0)		//将点分十进制转换为网络字节序
	{
		perror("inet_pton error");				
		exit(EXIT_FAILURE);
	}	

	if(connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
	{
		perror("connect error");
		exit(EXIT_FAILURE);
	}

	//step3:使用IO进行读写
	commun_func(sockfd);

	//step4:关闭描述符
	close(sockfd);
	return 0;
}
