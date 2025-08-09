#include "socket.h"
#include "msg.h"

int main(int argc, char **argv)
{
    if(argc < 2){
        fprintf(stderr,"%s|%s|%d|Usage:[%s] [host's PORT]\n",
                                    __FILE__,__func__,__LINE__,argv[0]);
        exit(EXIT_FAILURE);
    }

    //step1:创建套接字用于和服务器通信
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("create socket error");
        exit(EXIT_FAILURE);
    }

    //step2:使用connect函数连接到主机
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(argv[1]));
    if(inet_pton(AF_INET, IPADDR, &serveraddr.sin_addr.s_addr) < 0){
        perror("inet_pton error");
        exit(EXIT_FAILURE);
    }
    if(connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0){
        perror("connect error");
        return -1;
    }

    //step3:和服务器进行读写操作
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
        if(writeMsg(sockfd, buffer, sizeof(buffer)) < 0){
            continue;
        }else{
            if(readMsg(sockfd, buffer, sizeof(buffer)) < 0){
                continue;
            }else{
                printf("%s\n",buffer);
            }
        }
    }
    
    //step4:关闭套接字
    close(sockfd);

    return 0;
}