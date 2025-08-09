#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

int sockfd;

void signal_handler(int signum)
{
	if(signum == SIGINT)
	{
		printf("server close\n");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
}

char *get_address(char *cmd)
{
	//使用管道获取指令执行的结果，然后将其结果使用fread函数读取到指定的字符串中去
	FILE *fp = popen(cmd, "r");
	if(fp == NULL)
	{
		perror("popen error");
		exit(EXIT_FAILURE);
	}

	char *buffer = (char *)malloc(sizeof(char) * 16);

	if(fread(buffer, 16, 1, fp) < 0)
	{
		perror("fread error");
		exit(EXIT_FAILURE);
	}
	size_t len = strcspn(buffer, "\n");
	buffer[len] = '\0';

	pclose(fp);
	
	return buffer;
}

void print_client(struct sockaddr_in *clientaddr)
{
	int clientport = ntohs(clientaddr->sin_port);
	char clientip[16];

	if(inet_ntop(AF_INET, &clientaddr->sin_addr.s_addr, clientip, sizeof(clientip)) == NULL)
	{
		perror("inet_ntop errpr");
		exit(EXIT_FAILURE);
	}
	printf("receive a [client] connection request, IP is [%s] port is [%d]\n",clientip,clientport);
}

void commun_func(int fd)
{
	time_t T = time(NULL);
	char *timep = ctime(&T);
	char readbuf[64];
	memset(readbuf, 0,sizeof(readbuf));

	if(write(fd, timep, strlen(timep)) != strlen(timep))
	{
		perror("write error");
		exit(EXIT_FAILURE);
	}
	if(read(fd, readbuf, sizeof(readbuf)) < 0)
	{
		perror("read error");
		exit(EXIT_FAILURE);
	}
	printf("host receive client message is:%s\n",readbuf);
}

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr, "Usage:%s host's port\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	//step1:获取本地主机以192开头的IP地址
	char *cmd = "ifconfig | grep \"inet 192\" | awk '{print $2}'";
	char *ip = get_address(cmd);

	//step2:创建套接字用于服务器和客户端之间的通信
	//指定Internet协议族为AF_INET,使用的传输协议是TCP协议
	
	//向内核注册信号和信号处理函数，若产生此信号就去执行相应的信号处理函数，关闭套接字描述符
	if(signal(SIGINT, signal_handler) == SIG_ERR)
	{
		perror("signal error");
		exit(EXIT_FAILURE);
	}

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	//将套接字和本地主机地址与端口号进行绑定
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;
	//将主机端口从本地字节序转换为网络字节序
	serveraddr.sin_port = htons(atoi(argv[1]));
	//将主机ip地址从本地字节序转换为网络字节序
	if((inet_pton(AF_INET, ip, &serveraddr.sin_addr.s_addr)) < 0)
	{
		perror("inet_pton error");
		exit(EXIT_FAILURE);
	}
	
	//将因特网专用的结构体强转为通用的结构体
	if(bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
	{
		perror("bind error");
		exit(EXIT_FAILURE);
	}

	//step3:监听客户端的连接
	listen(sockfd, 10);

	printf("[host] ip:[%s] port:[%d] waiting for connecting.....\n",ip,atoi(argv[1]));
	//step4:接受客户端的连接

	sleep(10);
	while(1)
	{
		int fd;
		struct sockaddr_in clientaddr;
		memset(&clientaddr, 0, sizeof(clientaddr));
		socklen_t len = sizeof(clientaddr);

		//如果队列中没有客户端的连接，此函数会阻塞直到有客户端连接上来
		//若成功执行返回新的文件描述符用于和客户端进行通信，有几个客户端
		//就会创建几个文件描述符
		if((fd = accept(sockfd, (struct sockaddr *)&clientaddr, &len)) < 0)
		{
			perror("accept error");
			continue;
		}

		//step5:和客户端进行通信，使用accept函数返回的文件描述符进行读写操作
		
		print_client(&clientaddr);
		commun_func(fd);
		//step6:关闭accept函数返回的用于和客户端进行通信的文件描述符
		close(fd);
	}

	free(ip);

	return 0;
}
