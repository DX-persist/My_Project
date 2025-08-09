#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include "msg.h"

char *getAddress(char *cmd)
{
    //在堆空间上开辟空间，然后在调用结束后再释放ip地址
    char *ip = (char *)malloc(sizeof(char) * 16);
    if(ip == NULL)
    {
        perror("malloc error");
        return NULL;
    }

    //使用管道获取指令执行的结果，然后将执行的结果放到
    //对应的空间中
    FILE *fp = popen(cmd, "r");
    if(fp == NULL)
    {
        perror("popen error");
        return NULL;
    }

    if(fgets(ip, 16, fp) == NULL)
    {
        perror("fgets error");
        return NULL;
    }
	size_t len = strcspn(ip, "\n");
	ip[len] = '\0';

	pclose(fp);     

    return ip;
}

int initSocket(char *port, char *ip)
{
    //step1:创建套接字用于和客户端通信
    //指定Internet地址协议族为IPV4，通讯协议为TCP协议
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        perror("socket error");
        return -1;
    }
    printf("base: %d\n",sockfd);
    //step2:将套接字和指定的IP地址与端口进行绑定
    //Internet协议族：IPV4 IP地址：网络字节序 Port：网络字节序
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;            //指定Internet协议族为IPV4
    //将外部传参数据转换为网络字节序
    serveraddr.sin_port = htons(atoi(port));   
    if(inet_pton(AF_INET, ip, &serveraddr.sin_addr.s_addr) < 0)
    {
        perror("inet_pton error");
        return -1;
    }
    //将因特网专用的结构体转换为通用的结构体
    if(bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        perror("bind error");
        return -1;
    }

    //step3:监听客户端的连接，listen函数的第二个参数指定已经完成
    //三次握手的客户端请求的队列的大小
    listen(sockfd, 10);

    return sockfd;
}

void out_addr(struct sockaddr_in *clientaddr)
{
    int port = ntohs(clientaddr->sin_port);
    char IP[16] = {'\0'};

    if(inet_ntop(AF_INET, &clientaddr->sin_addr.s_addr, 
                        IP, sizeof(IP)) == NULL)
    {
        perror("inet_ntop error");
    }
    printf("receive a client request.IP:[%s] Port:[%d]\n",IP,port);
}

void do_service(int sockfd)
{
    char buffer[512];
    size_t size;

    while(1)
    {
        memset(buffer, 0, sizeof(buffer));
        if((size = readMsg(sockfd, buffer, sizeof(buffer))) < 0){
            perror("protocol error");
            break;
        }else if(size == 0){
            perror("no message in socket");
            break;
        }else{
            printf("%s\n",buffer);
            if(writeMsg(sockfd, buffer, sizeof(buffer)) < 0){
                if(errno == EPIPE)
                    break;
            }
        }

    }
}
