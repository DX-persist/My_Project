#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc,char **argv)
{
	if(argc < 3)
	{
		printf("parameter is not enough,please enter IP and port\n");
		exit(EXIT_FAILURE);
	}

	int sock_fd;
	int ret;
	int new_sockfd;
	int len;
	int n_read;

	char readbuf[128] = {'\0'};
	char dest[128] = {'\0'};
	char *writebuf = "I got your message";

	struct sockaddr_in saddr;
	struct sockaddr_in caddr;
	memset(&saddr,'\0',sizeof(struct sockaddr_in));
	memset(&caddr,'\0',sizeof(struct sockaddr_in));

	if((sock_fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		printf("%s|%s|%d failed to socket\n",__FILE__,__func__,__LINE__);
		perror("socket");
		exit(EXIT_FAILURE);
	}

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(atoi(argv[2]));	//先将字符串转换为整型，然后再将无符号的短整型数值转换为网络字节序
	//int inet_pton(int af, const char *restrict src, void *restrict dst);
	if(inet_pton(AF_INET,argv[1],&(saddr.sin_addr)) == 0)	//将点分十进制转换为网络字节序
	{
		printf("converted failed\n");
		exit(EXIT_FAILURE);
	}

	if(ret = bind(sock_fd,(struct sockaddr *)&saddr,sizeof(struct sockaddr_in)) == -1)
	{
		printf("bind failed\n");
		perror("bind");
		exit(EXIT_FAILURE);
	}

	listen(sock_fd,10);

	len = sizeof(struct sockaddr_in);
	if ((new_sockfd = accept(sock_fd, (struct sockaddr *)&caddr, &len)) == -1)
	{
		printf("accept failed\n");
		perror("accept");
		exit(EXIT_FAILURE);
	}
	inet_ntop(AF_INET,&(caddr.sin_addr),dest,sizeof(dest));
	printf("receive a connect signal and the IP is %s, port is %d\n",dest,ntohs(caddr.sin_port));

	n_read = read(new_sockfd,readbuf,sizeof(readbuf));
	printf("%d\n",n_read);
	if(n_read > 0)
	{
		printf("read %d bytes and the content is %s\n",n_read,readbuf);
	}

	write(new_sockfd,writebuf,strlen(writebuf) + 1);

	return 0;
}
