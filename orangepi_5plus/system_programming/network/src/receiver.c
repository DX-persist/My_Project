#include "socket.h"

int sockfd = -1;

void signal_handler(int signum)
{
    if(signum == SIGINT){
        printf("server close....\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    if(argc < 3){
        fprintf(stderr,"Usage:[%s] [IP] [Port]\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    if(signal(SIGINT, signal_handler) == SIG_ERR){
        perror("signal error");
        exit(EXIT_FAILURE);
    }

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("create socket error");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &server_addr.sin_addr.s_addr);

	if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		perror("bind error");
		exit(EXIT_FAILURE);
	}

	char buffer[1024];
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);

	while(1)
	{
		memset(buffer, 0, sizeof(buffer));
		memset(&client_addr, 0, len);

		if(recvfrom(sockfd, buffer, sizeof(buffer), 0,
					(struct sockaddr*)&client_addr, &len) < 0){
			perror("recvfrom error");
			exit(EXIT_FAILURE);
		}else{
			char ip[16];
			memset(ip, 0, sizeof(ip));
			inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip, sizeof(ip));
			int port = ntohs(client_addr.sin_port);

			printf("receive IP:[%s] Port:[%d] context:[%s]\n",ip, port, buffer);
		}
	}

    return 0;
}
