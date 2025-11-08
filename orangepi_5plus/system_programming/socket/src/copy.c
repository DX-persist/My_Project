#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/**封装结构体**/
typedef struct{
	char ip[16];
	int port;
	struct sockaddr_in addr;
	socklen_t addrlen;
	int retval;
	int sockfd;
}socket_config_t;

socket_config_t server;
socket_config_t client;


/**获取本地IP地址**/
void get_Hostaddress(char *cmd, char *ip)
{
	//使用popen来获取shell指令执行的结果
	FILE *fp = popen(cmd, "r");
	if(fp == NULL){
		perror("popen error");
		return;
	}
	
	//将指令执行的结果写入到结构体中
	if(fgets(ip, sizeof(server.ip), fp) == NULL){
		perror("fgets error");
		pclose(fp);
		return;
	}
	
	//size_t strcspn(const char *s1, const char *s2);
	//strcspn函数解析s1字符串中是否含有s2,若有则返回
	//从字符串开头到第一个出现s2的位置,用于去除换行符
	server.ip[strcspn(server.ip, "\n")] = '\0';

	pclose(fp);
}

void out_addr(struct sockaddr_in *client_addr)
{
	client.port = ntohs(client_addr->sin_port);
	inet_ntop(AF_INET, &client_addr->sin_addr.s_addr, 
					client.ip, sizeof(client.ip));

	fprintf(stdout, "Client IP:[%s] Port:[%d] connected...\n",
					client.ip, client.port);
}

void commun_func(int fd)
{
	char readbuffer[48];
	time_t now = time(NULL);
	char *time_buffer = ctime(&now);

	write(fd, time_buffer, strlen(time_buffer));

	memset(readbuffer, '\0', sizeof(readbuffer));
	read(fd, readbuffer, sizeof(readbuffer));
	printf("%s\n",readbuffer);
}

int main(int argc, char **argv)
{
	if(argc < 2){
		fprintf(stderr, "Usage: %s [Port]\n",argv[1]);
		return -1;
	}

	memset(&server, '\0', sizeof(server));
	memset(&client, '\0', sizeof(client));

	//1.创建流式套接字(使用TCP协议)
	server.sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(server.sockfd < 0){
		perror("socket error");
		return -1;
	}

	char *cmd = "ifconfig | awk \'/192/\' | awk \'{print $2}\'";
	get_Hostaddress(cmd, server.ip);
	
	//2.绑定主机的地址族、IP地址与端口号
	server.addr.sin_family = AF_INET;
	//先将从命令行传入的参数转换为整数后将主机字节序转换为网络字节序
	server.addr.sin_port = htons(atoi(argv[1]));
	//将主机的IP地址转换为网络字节序
	inet_pton(AF_INET, server.ip, &server.addr.sin_addr.s_addr);

	server.retval = bind(server.sockfd, (struct sockaddr *)&server.addr, 
						sizeof(struct sockaddr));
	if(server.retval < 0){
		perror("bind error");
		return -1;
	}

	//3.监听客户端的连接
	server.retval = listen(server.sockfd, 10);
	if(server.retval < 0){
		perror("listen error");
		return -1;
	}
	server.port = atoi(argv[1]);
	fprintf(stdout, "Host IP:[%s] Port:[%d] is waiting for connection...\n",
					server.ip,server.port);

	//4.允许客户端连接到服务器
	client.addrlen = sizeof(client.addr);
	int new_fd = -1;

	while(1){
		new_fd = accept(server.sockfd, (struct sockaddr *)&client.addr, 
						&client.addrlen);
		if(new_fd < 0){
			perror("accept error");
			continue;
		}
		
		//5.输出客户端的IP地址和端口并进行通信
		out_addr(&client.addr);
		commun_func(new_fd);
	}
	return 0;
}
