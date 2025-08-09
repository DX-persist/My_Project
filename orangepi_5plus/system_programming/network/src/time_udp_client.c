#include "socket.h"

int sockfd = -1;

void signal_handler(int signum)
{
	if(signum == SIGINT)
	{
		printf("client closed....\n");
		close(sockfd);
		exit(EXIT_SUCCESS);
	}
}

int main(int argc, char **argv)
{
	if(argc < 2){
		fprintf(stderr,"Usage:[%s] [server's IP] [server's Port]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	if(signal(SIGINT, signal_handler) == SIG_ERR){
		perror("signal error");
		exit(EXIT_FAILURE);
	}


	//step1:创建socket套接字
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0){
		perror("create socket error");
		exit(EXIT_FAILURE);
	}

	//step2:设置socket套接字的选项
	int opt = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
		perror("setsockopt error");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	socklen_t len = sizeof(server_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &server_addr.sin_addr.s_addr);

	//step3:获取从终端输入的信息发送给服务器
	
	while(1)
	{
		char buffer[1024];
		memset(buffer, 0, sizeof(buffer));	
		read(STDIN_FILENO, buffer, sizeof(buffer));
		size_t size = strcspn(buffer, "\n");
		buffer[size] = '\0';

		if(sendto(sockfd, buffer, sizeof(buffer), 0,
				   (struct sockaddr*)&server_addr, len) < 0){
			perror("sendto error");
			close(sockfd);
			continue;
		}else{
			char readbuf[1024];
			memset(readbuf, 0, sizeof(readbuf));
			if(recv(sockfd, readbuf, sizeof(readbuf), 0) < 0){
				perror("recv errror");
				close(sockfd);
				continue;
			}else{
				printf("recv message from the host and the context is :%s\n",readbuf);
			}
		}
	}


	return 0;
}
