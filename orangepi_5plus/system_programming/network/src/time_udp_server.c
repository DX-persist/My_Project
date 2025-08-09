#include "socket.h"

int sockfd = -1;

void signal_handler(int signum)
{
	if(signum == SIGINT){
		printf("server close....\n");
		close(sockfd);
		exit(EXIT_SUCCESS);
	}
}

int main(int argc, char **argv)
{
	if(argc < 3){
		fprintf(stderr,"Usage:[%s] [IP] [Port]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	if(signal(SIGINT, signal_handler) == SIG_ERR){
		perror("signal sigint error");
		exit(EXIT_FAILURE);
	}

	//step1:创建socket套接字
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0){
		perror("create socket error");
		exit(EXIT_FAILURE);
	}

	//step2:设置服务器地址信息
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &server_addr.sin_addr.s_addr);

	//step3:绑定套接字到地址
	if(bind(sockfd, (struct sockaddr *)&server_addr,
				sizeof(server_addr)) < 0){
		perror("bind error");
		exit(EXIT_FAILURE);
	}

	char buffer[1024];
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	socklen_t len = sizeof(client_addr);

	while(1)
	{
		memset(buffer, 0, sizeof(buffer));
		if(recvfrom(sockfd, buffer, sizeof(buffer), 
					0, (struct sockaddr *)&client_addr, &len) < 0){
			perror("recv error");
			continue;
		}else{
			int client_port = ntohs(client_addr.sin_port);
			char client_ip[16];
			memset(client_ip, 0, sizeof(client_ip));
			inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip));

			printf("IP:%s Port:%d\n",client_ip,client_port);

			printf("%s\n",buffer);
			time_t tim = time(NULL);
			char *ti = ctime(&tim);
			size_t size = strcspn(ti, "\n");
			ti[size] = '\0';
			if(sendto(sockfd, ti, strlen(ti), 0, 
						(struct sockaddr *)&client_addr, len) < 0){
				perror("send error");
				continue;
			}
		}
	}

	return 0;
}
