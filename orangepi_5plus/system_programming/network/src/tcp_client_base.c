#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int initSocket(char *IP, char *port)
{
    //创建socket套接字用于和主机进行通信
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        perror("sockfd error");
        exit(EXIT_FAILURE);
    }

    //使用connect函数连接到主机
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(port));
    if(inet_pton(AF_INET, IP, &serveraddr.sin_addr.s_addr) < 0)
    {
        perror("inet_pton error");
        return -1;
    }
    if(connect(sockfd, (struct sockaddr *)&serveraddr, 
                        sizeof(serveraddr)) < 0)
    {
        perror("connect error");
        return -1;
    }
}
    