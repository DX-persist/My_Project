#include "socket.h"
#include "msg.h"

int sockfd = -1;

void signal_handler(int signum)
{
    if(signum == SIGINT){
        printf("server close.....\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if(signum == SIGCHLD)
    {
        printf("client close...\n");
        wait(NULL);
    }
}

void do_service(int newSockfd)
{
    char buffer[512];
    size_t size;

    printf("start read and write\n");
    while(1)
    {
        memset(buffer, 0, sizeof(buffer));
        if((size = readMsg(newSockfd, buffer, sizeof(buffer))) < 0){
            perror("protocol error");
            break;
        }else if(size == 0){
            break;
        }else{
            printf("%s\n",buffer);
            if(writeMsg(newSockfd, buffer, sizeof(buffer)) < 0){
                if(errno == EPIPE)  break;
                perror("protocol error");
            }
        }
    }   
}

int main(int argc, char **argv)
{
    if(argc < 2){
        fprintf(stderr,"%s|%s|%d|Usage:[%s] [Port]\n",
            __FILE__,__func__,__LINE__,argv[0]);
        exit(EXIT_FAILURE);
    }

    char *cmd = "ifconfig | grep \"inet 192\" | awk '{print $2}'";
    char *ip = getAddress(cmd);

    if(signal(SIGINT, signal_handler) == SIG_ERR){
        perror("signal sigint error");
        exit(EXIT_FAILURE);
    }
    if(signal(SIGCHLD, signal_handler) == SIG_ERR)
    {
        perror("signal sigchld error");
        exit(EXIT_FAILURE);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("create socket error");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(argv[1]));
    if(inet_pton(AF_INET, ip, &serveraddr.sin_addr.s_addr) < 0){
        perror("inet_pton error");
        exit(EXIT_FAILURE);
    }

    if(bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
    {
        perror("bind error");
        exit(EXIT_FAILURE);
    }

    listen(sockfd, 10);

    printf("host IP:%s Port:%d waiting for connecting\n",ip,atoi(argv[1]));
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    memset(&clientaddr, 0, sizeof(clientaddr));

    while(1)
    {
        int newSockfd = accept(sockfd, (struct sockaddr *)&clientaddr, &len);
        if(newSockfd < 0)
        {
            perror("accept error");
            continue;
        }

        pid_t pid;
        if((pid = fork()) < 0){
            perror("fork error");
            continue;
        }else if(pid == 0){
            out_addr(&clientaddr);
            do_service(newSockfd);
            close(newSockfd);
            break;
        }else{
            close(newSockfd);
        }
    }

    finishCom(ip);

    return 0;
}
