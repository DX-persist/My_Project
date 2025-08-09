#include "socket.h"
#include "msg.h"

char *getAddress(char *cmd)
{
    //使用管道执行指令然后获取指令执行的结果
    FILE *fp = popen(cmd, "r");
    if(fp == NULL){
        perror("popen error");
        exit(EXIT_FAILURE);
    }

    //在堆空间上开辟空间，然后将读取到的结果放到IP中
    char *IP = (char *)malloc(sizeof(char) * 16);
    if(IP == NULL){
        perror("malloc error");
        return NULL;
    }

    if(fgets(IP, 16, fp) < 0)
    {
        perror("fgets error");
        return NULL;
    }
    size_t size = strcspn(IP, "\n");
	IP[size] = '\0';

	pclose(fp);

    return IP;
}

void out_addr(struct sockaddr_in *clientaddr)
{
    int port = ntohs(clientaddr->sin_port);
	char client_addr[16];
	memset(client_addr, 0, sizeof(client_addr));

	if(inet_ntop(AF_INET, &clientaddr->sin_addr.s_addr,
							client_addr, sizeof(client_addr)) == NULL)
	{
		perror("inet_ntop error");
		exit(EXIT_FAILURE);
	}
	printf("receive a client connect,IP:%s port:%d\n",client_addr,port);
}

void finishCom(char *IP)
{
    assert(IP != NULL);

    free(IP);
}
