#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct{
	int sockfd;
	char ip[16];
	int port;
	int retval;
	struct sockaddr_in saddr;
}ServerInfo_t;

typedef struct{
	int port;
	char ip[16];
	struct sockaddr_in caddr;
	socklen_t clientaddr_len;
}ClientInfo_t;

ServerInfo_t server;
ClientInfo_t client;

void signal_handler(int signum)
{
	if(signum == SIGINT){
		fprintf(stdout, "receive a signal is SIGINT. Program Exit\n");
		close(server.sockfd);
		exit(EXIT_SUCCESS);
	}
}

void out_addr(ClientInfo_t *cli)
{
	assert(cli != NULL);

	cli->port = ntohs(cli->caddr.sin_port);
	inet_ntop(AF_INET, &cli->caddr.sin_addr.s_addr, cli->ip, sizeof(cli->ip));
	fprintf(stdout, "Client IP:[%s] Port:[%d]\n",cli->ip, cli->port);
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
	if(argc != 3){
		fprintf(stderr, "Usage: [%s] [IPADDR] [PORT]\n",argv[0]);
		return -1;
	}

	memset(&server, '\0', sizeof(ServerInfo_t));
	memset(&client, '\0', sizeof(ClientInfo_t));

	if(signal(SIGINT, signal_handler) == SIG_ERR){
		perror("SIGINT Error!");
		return -1;
	}
	
	server.sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(server.sockfd == -1){
		fprintf(stderr, "create socket error\n");
		perror("socket");
		return -1;
	}

	server.saddr.sin_family = AF_INET;
	server.saddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &server.saddr.sin_addr.s_addr);

	server.retval = bind(server.sockfd, 
							(struct sockaddr *)&server.saddr, 
								sizeof(server.saddr));
	if(server.retval == -1){
		fprintf(stderr, "bind error\n");
		perror("bind");
		return -1;
	}

	server.retval = listen(server.sockfd, 10);
	if(server.retval == -1){
		fprintf(stderr, "listen error\n");
		perror("listen");
		return -1;
	}
	client.clientaddr_len = sizeof(client.caddr);
	while(1)
	{
		int fd = accept(server.sockfd, 
							(struct sockaddr *)&client.caddr, 
								&client.clientaddr_len);
		if(fd < 0){
			perror("accept");
			continue;
		}	

		out_addr(&client);
		commun_func(fd);

		close(fd);
	}

	return 0;
}
