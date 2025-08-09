#include "socket.h"
#include "msg.h"

int sockfd = -1;

void signal_handler(int signum)
{
	if(signum == SIGINT)
	{
		printf("client closed....\n");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char **argv)
{
	if(argc < 2){
		fprintf(stderr, "%s|%s|%d|Usage:[%s] [Port]\n",
							__FILE__,__func__,__LINE__,argv[0]);
		exit(EXIT_FAILURE);
	}

	if(signal(SIGINT, signal_handler) == SIG_ERR)
	{
		perror("signal sigint error");
		exit(EXIT_FAILURE);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		perror("create socket error");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[1]));
	if(inet_pton(AF_INET, IPADDR, &server_addr.sin_addr.s_addr) < 0){
		perror("inet_pton error");
		exit(EXIT_FAILURE);
	}
	if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		perror("connect error");
		exit(EXIT_FAILURE);
	}
	char buffer[512];
	char prompt = '>';
	size_t size;

	while(1)
	{
		memset(buffer, 0, sizeof(buffer));
		write(STDOUT_FILENO, &prompt, sizeof(prompt));
		size = read(STDIN_FILENO, buffer, sizeof(buffer));
		if(size < 0){
			perror("read error");
			continue;
		}		
		buffer[size-1] = '\0';
		
		if(writeMsg(sockfd, buffer, sizeof(buffer)) < 0)
		{
			perror("write error");
			continue;
		}
		else
		{
			if((size = readMsg(sockfd, buffer, sizeof(buffer))) < 0)
			{
				perror("read error");
				continue;
			}	
			else
			{
				printf("%s\n",buffer);
			}
		}
	}

	close(sockfd);

	return 0;
}
