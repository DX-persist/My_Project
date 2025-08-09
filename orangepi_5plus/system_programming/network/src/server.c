#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "msg.h"

int sockfd = -1;

void signal_handler(int signum)
{
	if(signum == SIGINT)
	{
		printf("server close\n");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	if(signum == SIGCHLD)
	{
		printf("client close\n");
		wait(NULL);
	}
}

void out_client(struct sockaddr_in *clientaddr)
{
	int port = ntohs(clientaddr->sin_port);
	char client_addr[16];
	memset(client_addr, 0, sizeof(client_addr));

	if(inet_ntop(AF_INET, &clientaddr->sin_addr.s_addr,
							client_addr, sizeof(client_addr)) == NULL)
	{
		perror("inet_ntop error");
		exit(EXIT_FAILURE);
	}
	printf("receive a client connect,ip:%s port:%d\n",client_addr,port);
}

void commun_func(int fd)
{
	/*和客户端进行读写操作（双向通信）*/
	char buffer[512];

	printf("start read and write\n");
	while(1)
	{
		memset(buffer, '\0', sizeof(buffer));
		size_t size;

		if((size = readMsg(fd, buffer, sizeof(buffer))) < 0){
			perror("protocol error");
			break;
		}else if(size == 0){
			printf("read ended\n");
			break;
		}else{
			printf("%s\n",buffer);
			if(writeMsg(fd, buffer, sizeof(buffer)) < 0){
				if(errno == EPIPE){
					break;
				}
				perror("protocol error");
			}
		}
	}
}

char *get_address(char *cmd)
{
	//使用ifconfig指令获取ip地址的相关信息,然后将ip地址过滤出来,例如192.168.43.212
	//使用管道获取指令执行的结果然后放到对应的地址中去
	FILE *fp = popen(cmd, "r");
	char *ip = (char*)malloc(sizeof(char) * 16);
	if(ip == NULL)
	{
		perror("malloc error");
		exit(EXIT_FAILURE);
	}

	if(fp == NULL)
	{
		perror("popen error");
		exit(EXIT_FAILURE);
	}
	
	//将指令执行的结果(ip地址)放到开辟出来的空间中去
	if(fgets(ip, 16, fp) < 0)
	{
		perror("fgets error");
		pclose(fp);
		exit(EXIT_FAILURE);
	}
	pclose(fp);

	return ip;
}

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr,"Usage:%s port\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	char *cmd = "ifconfig | grep \"inet 192\" | awk '{print $2}'";
	
	char *ip = get_address(cmd);

	//向内核注册信号和信号处理函数,若收到此信号就去执行相应的信号处理函数
	if(signal(SIGINT, signal_handler) == SIG_ERR)
	{
		perror("signal sigint error");
		exit(EXIT_FAILURE);
	}

	if(signal(SIGCHLD, signal_handler) == SIG_ERR)
	{
		perror("signal sigchld error");
		exit(EXIT_FAILURE);
	}

	//step1:创建套接字,指定Internet协议族为IPV4,传输协议为TCP传输协议
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)	
	{
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	//step2:将套接字与服务器的ip地址和端口进行绑定
	//指定协议族为IPV4
	//要将端口和ip地址从主机字节序转换为网络字节序
	struct sockaddr_in serveraddr;

	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(atoi(argv[1]));
	if((inet_pton(AF_INET, ip, 
							&serveraddr.sin_addr.s_addr)) < 0)
	{
		perror("inet_pton error");
		exit(EXIT_FAILURE);
	}
	if(bind(sockfd, (struct sockaddr*)&serveraddr, 
								sizeof(serveraddr)) < 0)
	{
		perror("bind error");
		exit(EXIT_FAILURE);
	}

	//step3:监听来自客户端的连接,将客户端的请求放到一个队列中
	//设置队列的大小为10,意味着最多接受10个客户端的连接
	printf("server ip:%sport:%d waiting for client connect...\n",ip,atoi(argv[1]));
	listen(sockfd, 10);

	//step4:接受来自客户端的连接,并返回一个描述符用于和客户端
	//进行通信
	struct sockaddr_in clientaddr;

	memset(&clientaddr, 0, sizeof(clientaddr));
	socklen_t len = sizeof(clientaddr);

	while(1)
	{
		int fd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
		if(fd < 0)
		{
			perror("accept error");
			continue;
		}
		//step5:服务器和客户端进行通信,使用accept函数返回的描述符进行read,write
		pid_t pid;
		if((pid = fork()) < 0){
			continue;
		}else if(pid == 0){
			out_client(&clientaddr);
			commun_func(fd);		
			close(fd);
			break;
		}else{
			close(fd);
		}
	}

	free(ip);

	return 0;
}
