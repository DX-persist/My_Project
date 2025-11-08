#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "dht22_config.h"


typedef struct{
	int port;
	char ip[16];
	struct sockaddr_in addr;
	socklen_t addrlen;
	int retval;
	int sockfd;
	int accept_fd;
}socket_config_t;

#define CHECK_TCP_ERR(cond, failure_msg, retval)	\
	do{												\
		if(cond == -1){								\
			fprintf(stderr, "%s\n",failure_msg);	\
			perror(failure_msg);					\
			return retval;							\
		}											\
	}while(0)										\

socket_config_t server;
socket_config_t client;


void signal_handler(int signum)
{
	if(signum == SIGINT){
		fprintf(stdout, "Received a signal is SIGINT...\n");
		fprintf(stdout, "server closed...\n");
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

	if(fgets(server.ip, sizeof(server.ip), fp) == NULL){
		perror("fgets error");
		pclose(fp);
		return;
	}

	server.ip[strcspn(server.ip, "\n")] = '\0';

	pclose(fp);
}

void out_addr(socket_config_t *client_info)
{
	assert(client_info != NULL);

	client_info->port = ntohs(client_info->addr.sin_port);
	inet_ntop(AF_INET, &client_info->addr.sin_addr.s_addr, client_info->ip, sizeof(client_info->ip));
	printf("Receive a connection: IP:[%s] Port:[%d] connected....\n",client_info->ip, client_info->port);
}

void commun_func(int fd)
{
	time_t t = time(NULL);
	char buffer[32];
	char readbuffer[32];


	memset(buffer, '\0', sizeof(buffer));
	memset(readbuffer, '\0', sizeof(readbuffer));
	strncpy(buffer, ctime(&t), sizeof(buffer));

	if(write(fd, buffer, strlen(buffer)) != strlen(buffer)){
		perror("write error");
		return;
	}
	
	if(read(fd, readbuffer, sizeof(readbuffer)) < 0){
		perror("read error");
		return;
	}
	printf("Server receive client's message is : %s\n",readbuffer);
}

void get_msg(char *send_msg, size_t size)
{
	//1.获取时间
	char time_buffer[32];

	memset(time_buffer, '\0', sizeof(time_buffer));
	memset(send_msg, '\0', size);

	time_t t = time(NULL);
	strncpy(time_buffer, ctime(&t), sizeof(time_buffer));
	time_buffer[strcspn(time_buffer, "\n")] = '\0';
	
	//2.获取温湿度
	DHT22_Data_t Data = {0};

	if(DHT22_ReadData(&Data) != 0){
		fprintf(stderr, "read dht22 error");
		return;
	}else{
		snprintf(send_msg, size, "time:%s\t temperature:%.2f°C\t humidity:%.2f%%RH\n", 
												time_buffer, Data.temperature, Data.humidity);	
	}
	
}

void handler_dht22(socket_config_t *client)
{
	assert(client != NULL);
	
	char send_msg[128];
	
	get_msg(send_msg, sizeof(send_msg));
	
	if(write(client->accept_fd, send_msg, strlen(send_msg)) != strlen(send_msg)){
		perror("write error");
	}
}

void *handler_func(void *arg)
{
	//1.输出客户端的地址
	
	assert(arg != NULL);
	socket_config_t *client = (socket_config_t *)arg;

	out_addr(client);

	//2.获取温湿度与时间
	handler_dht22(client);

	//3.与客户端进行通信
	//commun_func(client->accept_fd);

	close(client->accept_fd);

	pthread_exit(NULL);
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

	//初始化wiringPi库并配置为输出模式
	if(wiringPiSetup() == -1){
		fprintf(stderr, "failed to load lib wiringPi\n");
	}

	memset(&server, '\0', sizeof(socket_config_t));
	memset(&client, '\0', sizeof(socket_config_t));

	char *cmd = "ifconfig | grep \"inet 192\" | awk '{print $2}'";
	get_Hostaddress(cmd, server.ip);
	
	server.sockfd = socket(AF_INET, SOCK_STREAM, 0);
	CHECK_TCP_ERR(server.sockfd, "socket error", -1);

	server.addr.sin_family = AF_INET;
	server.addr.sin_port = htons(atoi(argv[1]));
	inet_pton(AF_INET, server.ip, &server.addr.sin_addr.s_addr);
	
	//设置SO_REUSEADDR选项，表示允许端口关闭后立即重新绑定
	int optval = 1;
	server.retval = setsockopt(server.sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	CHECK_TCP_ERR(server.retval, "setsockopt error", -1);

	server.retval = bind(server.sockfd, (struct sockaddr *)&server.addr, sizeof(server.addr));
	CHECK_TCP_ERR(server.retval, "bind error", -1);
	
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
		client.accept_fd = fd;
		pthread_t tid;
		pthread_attr_t attr;

		//设置线程属性为分离线程
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		if(pthread_create(&tid, &attr, handler_func, (void *)&client) != 0){
			perror("pthead_create error");
			continue;
		}

		//销毁线程属性
		pthread_attr_destroy(&attr);
	}


	return 0;
}
