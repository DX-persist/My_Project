#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define CHECK_ERROR(condition, errmsg)      \
    do{                                     \
        if(condition){                      \
            perror(errmsg);                 \
            exit(1);                        \
        }                                   \
    }while(0)

int sockfd = -1;

void signal_handler(int signum)
{
    if(signum == SIGINT){
        printf("Received a signal is sigint, client exit...\n");
        close(sockfd);
        exit(0);
    }
}

int create_socket(int *sockfd)
{
    *sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    CHECK_ERROR(*sockfd < 0, "socket error");

    return 0;
}

void commun_func(int sockfd, char *param1, char *param2)
{
    char buffer[64];
    char send_buffer[64];
    char *prompt = ">";
    struct sockaddr_in server_addr;
    socklen_t addrlen = sizeof(server_addr);

    //获取服务器IP地址、端口号等信息
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(param2));
    inet_pton(AF_INET, param1, &server_addr.sin_addr.s_addr);

    while(1){
        memset(buffer, '\0', sizeof(buffer));
        memset(send_buffer, '\0', sizeof(send_buffer));
        //在命令终端输入提示符'>'
        if(write(STDOUT_FILENO, prompt, strlen(prompt)) != strlen(prompt)){
            perror("write error");
        }

        //从命令行获取发送到服务器的数据
        if(read(STDIN_FILENO, buffer, sizeof(buffer)) < 0){
            perror("read error");
            continue;
        }
        buffer[strcspn(buffer, "\n")] = '\0';

        //若检测到客户端输入"exit",程序退出并清空资源
        if(!strcmp(buffer, "exit")){
            printf("Client exit and release the resource...\n");
            close(sockfd);
            exit(0);
        }

        //将数据发送到服务器
        if(sendto(sockfd, buffer, sizeof(buffer), 0, 
            (struct sockaddr *)&server_addr, addrlen) < 0){
            perror("sendto error");
            continue;
        }
        //获取来自服务器的消息
        if(recvfrom(sockfd, send_buffer, sizeof(send_buffer), 0, 
            (struct sockaddr *)&server_addr, &addrlen) < 0){
            perror("recvfrom error");
            continue;
        }else{
            printf("Client received server's message is: %s\n",send_buffer);
        }
    }
}

int main(int argc, char **argv)
{
    if(argc < 3){
        fprintf(stderr, "Usage: [%s] [IP] [Port]\n",argv[0]);
        return -1;
    }

    if(signal(SIGINT, signal_handler) == SIG_ERR){
        perror("signal sigint error");
        return -1;
    }

    //1.创建数据报套接字
    CHECK_ERROR(create_socket(&sockfd) < 0, "Failed to create socket");

    //2.与服务器进行通信
    commun_func(sockfd, argv[1], argv[2]);

    //3.关闭套接字描述符
    close(sockfd);

    return 0;
}