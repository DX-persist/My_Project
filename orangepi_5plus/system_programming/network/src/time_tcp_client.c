#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

void commun_func(int sockfd)
{
	char readbuf[20];
	char writebuf[64];
	memset(readbuf, 0, sizeof(readbuf));
	memset(writebuf, 0, sizeof(writebuf));

	if(read(sockfd, readbuf, sizeof(readbuf)) < 0)
	{
		perror("read error");
		exit(EXIT_FAILURE);
	}
	printf("%s\n",readbuf);

	strcpy(writebuf, " client receive your message,this is reply:[Hello,Server]");
	if(write(sockfd, writebuf, sizeof(writebuf)) != sizeof(writebuf))
	{
		perror("write error");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		fprintf(stderr, "Usage:%s host's ip host's port\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	//step1:创建套接字，指定协议族为IPV4和传输协议为TCP
	int sockfd;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	//step2:将客户端和服务器进行连接，需要指定协议族、主机的IP地址和端口
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(atoi(argv[2]));
	if(inet_pton(AF_INET, argv[1], &serveraddr.sin_addr.s_addr) < 0)
	{
		perror("inet_pton error");
		exit(EXIT_FAILURE);
	}
	
	if(connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
	{
		perror("connect error");
		exit(EXIT_FAILURE);
	}

	//step3:和服务器进行通信
	commun_func(sockfd);

	//step4:关闭套接字描述符
	close(sockfd);
	return 0;
}
