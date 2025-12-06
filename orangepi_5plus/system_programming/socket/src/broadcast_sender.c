#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
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
	if(argc < 3){
		fprintf(stderr,"Usage: [%s] [Broadcast addr] [Port]\n",argv[0]);
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

	//2.设置套接字选项,启用广播功能
	int opt = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) < 0){
		perror("setsockopt error");
		return -1;
	}

	//3.从命令行获取信息发送到广播地址
	struct sockaddr_in addr;
	memset(&addr, '\0', sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &addr.sin_addr.s_addr);

	char *prompt = ">";
	char send_buff[64];

	while(1){
		if(write(STDOUT_FILENO, prompt, strlen(prompt)) != strlen(prompt)){
			perror("write error");
			return -1;
		}
		memset(send_buff, '\0', sizeof(send_buff));
		if(read(STDIN_FILENO, send_buff, sizeof(send_buff)) < 0){
			perror("read error");
			return -1;
		}
		send_buff[strlen(send_buff)-1] = '\0';

		if(sendto(sockfd, send_buff, strlen(send_buff), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0){
			perror("sendto error");
			return -1;
		}else{
			printf("broadcast success\n");
		}
	}
	//4.关闭socket套接字
	close(sockfd);

	return 0;
}
