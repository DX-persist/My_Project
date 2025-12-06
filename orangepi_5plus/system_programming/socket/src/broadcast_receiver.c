#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int sockfd = -1;

void signal_handler(int signum)
{
	if(signum == SIGINT){
		printf("Received a signal is SIGINT, release the resource and exit...\n");
		close(sockfd);
		exit(0);
	}
}

int main(int argc, char **argv)
{
	if(argc < 2){
		fprintf(stderr, "Usage: [%s] [Port]\n",argv[0]);
		return -1;
	}

	if(signal(SIGINT, signal_handler) == SIG_ERR){
		perror("signal error");
		return -1;
	}

	//1.创建数据报套接字
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0){
		perror("socket error");
		return -1;
	}

	//2.绑定套接字
	struct sockaddr_in addr;
	
	memset(&addr, '\0', sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[1]));
	addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
		perror("bind error");
		return -1;
	}

	//3.接收来自广播的消息
	char recv_buff[64];
	struct sockaddr_in client_addr;
	socklen_t addrlen = sizeof(client_addr);
	
	printf("Receiving a message from the broadcast...\n");
	while(1){
		memset(recv_buff, '\0', sizeof(recv_buff));
		memset(&client_addr, '\0', addrlen);

		if(recvfrom(sockfd, recv_buff, sizeof(recv_buff), 0, 
					(struct sockaddr *)&client_addr, &addrlen) < 0){
			perror("recvfrom error");
			return -1;
		}else{
			char ip[16];
			int port = ntohs(client_addr.sin_port);
			inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip, sizeof(ip));
			printf("[%s](%d) received [%s]\n",ip, port, recv_buff);
		}
	}


	return 0;
}
