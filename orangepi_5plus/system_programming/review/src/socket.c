#include "socket.h"
#include "msg.h"

char *getAddress(char *cmd)
{
	FILE *fp = popen(cmd, "r");
	if(fp == NULL){
		perror("popen error");
		exit(EXIT_FAILURE);
	}   

	char *IP = (char *)malloc(sizeof(char) * 14);
	if(IP == NULL){
		perror("malloc error");
		exit(EXIT_FAILURE);
	}   

	if(fgets(IP, 14, fp) == NULL){
		perror("fgets error");
		exit(EXIT_FAILURE);
	}   
	pclose(fp);

	return IP; 
}

int initSocket(int sockfd, char *ip, char *port)
{
	//step1:创建套接字用于和客户端进行通信
    //指定Internet协议族为IPV4协议族,传输协议使用TCP传输控制协议
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if(sockfd < 0){ 
        perror("socket error");
        exit(EXIT_FAILURE);
    }   

    //step2:将套接字和对应的IP地址与端口号进行绑定,使用因特网专用的结构体
    //然后在传参的时候强转成通用地址结构体
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    if(inet_pton(AF_INET, ip, &server_addr.sin_addr.s_addr) < 0){ 
        perror("inet_pton error");
        exit(EXIT_FAILURE);
    }   

	if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("bind error");
        exit(EXIT_FAILURE);
    }

	printf("server IP:[%s] Port:[%d] waiting for connecting...\n",ip,atoi(port));
	
    //step3:监听客户端的连接,listen函数的第二个参数指定等待队列的大小
    listen(sockfd, 10);

	return sockfd;
}

void out_addr(struct sockaddr_in *client_addr)
{
	int client_port = ntohs(client_addr->sin_port);
	char clientaddr[16];
	memset(clientaddr, 0, sizeof(clientaddr));

	if(inet_ntop(AF_INET, &client_addr->sin_addr.s_addr, 
						clientaddr, sizeof(clientaddr)) == NULL){
		perror("inet_ntop error");
	}

	printf("receive a client IP:[%s] Port:[%d] connecting....\n",clientaddr,client_port);
}

void do_service(int sockfd)
{
	char buffer[512];
	size_t size;

	printf("start read and write...\n");
	while(1)
	{
		memset(buffer, 0, sizeof(buffer));
		if((size = readMsg(sockfd, buffer, sizeof(buffer))) < 0){
			perror("protocol error");
			break;
		}else if(size == 0){
			break;
		}else{
			printf("%s\n",buffer);
			if(writeMsg(sockfd, buffer, sizeof(buffer)) < 0){
				if(errno == EPIPE) break;
				perror("protocol error");
			}	
		}
	}
}

void finish_commun(char *ip)
{
	free(ip);
}
