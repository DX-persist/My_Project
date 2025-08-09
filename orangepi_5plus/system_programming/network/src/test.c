#include "socket.h"

int main(void)
{
	int sockfd = -1;

	if(sockfd = socket(AF_INET, SOCK_DGRAM, 0) < 0){
		perror("create sockfd error");
		exit(EXIT_FAILURE);
	}

	int opt;
	socklen_t len = sizeof(opt);
	getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &opt, &len);
	printf("opt = %d\n",opt);
}
