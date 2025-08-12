#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct{
	int port;
	char ip[16];
	struct sockaddr_in addr;
	socklen_t addrlen;
	int retval;
	int sockfd;
}TcpInfo_t;

#define CHECK_TCP_ERR(cond, failure_msg, retval)	\
	do{												\
		if(cond == -1){								\
			fprintf(stderr, "%s\n",failure_msg);	\
			perror(failure_msg);					\
			return retval;							\
		}											\
	}while(0)										\

TcpInfo_t server;
TcpInfo_t client;


void signal_handler(int signum)
{
	if(signum == SIGINT){
		fprintf(stdout, "Received a signal is SIGINT...\n");
		close(server.sockfd);
		exit(EXIT_SUCCESS);
	}
}

void get_Hostaddress(char *cmd, char *ip)
{
	FILE *fp = popen(cmd, "r");
	if(fp == NULL){
		perror("popen error");
		return; 
	}

	if(fread(ip, 13, 1, fp) < 0){
		perror("fread error");
		return;
	}
	pclose(fp);
}

void out_addr(TcpInfo_t *client_info)
{
	assert(client_info != NULL);

	client_info->port = ntohs(client_info->addr.sin_port);
	inet_ntop(AF_INET, &client_info->addr.sin_addr.s_addr, client_info->ip, sizeof(client_info->ip));
	printf("IP:[%s] Port:[%d] connected....\n",client_info->ip, client_info->port);
}

void commun_func(int fd)
{
	time_t t = time(NULL);
	char buffer[32];

	memset(buffer, '\0', sizeof(buffer));
	strncpy(buffer, ctime(&t), sizeof(buffer));

	write(fd, buffer, sizeof(buffer));
}

int main(int argc, char **argv)
{
	if(argc < 2){
		fprintf(stderr, "Usage: [%s] [Port]\n",argv[0]);
		return -1;
	}

	if(signal(SIGINT, signal_handler) == SIG_ERR){
		perror("signal SIGINT error");
		return -1;
	}

	memset(&server, '\0', sizeof(TcpInfo_t));
	memset(&client, '\0', sizeof(TcpInfo_t));

	char *cmd = "ifconfig | grep \"inet 192\" | awk '{print $2}'";
	get_Hostaddress(cmd, server.ip);
	
	server.sockfd = socket(AF_INET, SOCK_STREAM, 0);
	CHECK_TCP_ERR(server.sockfd, "socket error", -1);
/*
	if(server.sockfd < 0){
		fprintf(stderr,"socket error");
		perror("socket error");
		return -1;
	}
*/
	server.addr.sin_family = AF_INET;
	server.addr.sin_port = htons(atoi(argv[1]));
	inet_pton(AF_INET, server.ip, &server.addr.sin_addr.s_addr);
	
	server.retval = bind(server.sockfd, (struct sockaddr *)&server.addr, sizeof(server.addr));
	CHECK_TCP_ERR(server.retval, "bind error", -1);

	//设置SO_REUSEADDR选项，表示允许端口关闭后立即重新绑定
	int optval = 1;
	server.retval = setsockopt(server.sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	CHECK_TCP_ERR(server.retval, "setsockopt error", -1);

	server.retval = listen(server.sockfd, 10);
	CHECK_TCP_ERR(server.retval, "listen error", -1);
	
	server.port = atoi(argv[1]);
	fprintf(stdout, "Host IP:[%s] Port:[%d] is waiting for connecting\n", server.ip, server.port);	
	//允许客户端连接到服务器
	
	client.addrlen = sizeof(client.addr);
	while(1){
		int fd = accept(server.sockfd, (struct sockaddr *)&client.addr, &client.addrlen);
		if(fd == -1){
			fprintf(stderr, "accept error");
			perror("accept error");
			continue;
		}

		out_addr(&client);
		commun_func(fd);

		close(fd);
	}


	return 0;
}
