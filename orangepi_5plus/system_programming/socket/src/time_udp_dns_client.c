#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>

int sockfd = -1;

void signal_handler(int signum)
{
    if(signum == SIGINT){
        printf("Recvived a signal is SIGINT, client exit and release resource...\n");
        close(sockfd);
        exit(EXIT_SUCCESS);
    }

}

void get_host_msg(char *param1, char *param2, struct sockaddr_in *addr)
{
    struct in_addr in;
    struct hostent *host = NULL;
    char ip[16];
    int flag = 0;

    //判断命令行传入的是否为IP地址
    if(inet_pton(AF_INET, param1, &in) == 1){
        strncpy(ip, param1, sizeof(ip)-1);
        ip[sizeof(ip)-1] = '\0';
        flag = 1;
    }else{
        //打开主机数据库检索是否输入的是主机的正式名(/etc/hosts)
        sethostent(1);

        //判断命令行传入的是否是主机的正式名
        while((host = gethostent()) != NULL){
            if(!strcmp(host->h_name, param1)){
                memset(ip, '\0', sizeof(ip));
                inet_ntop(AF_INET, host->h_addr_list[0], ip, sizeof(ip));
                flag = 1;
                break;
            }else{
                //判断命令行传入的是否是主机的别名
                char **alias = host->h_aliases;
                while(*alias != NULL){
                    if(!strcmp(*alias, param1)){
                        memset(ip, '\0', sizeof(ip));
                        inet_ntop(AF_INET, host->h_addr_list[0], ip, sizeof(ip));
                        flag = 1;
                        break;
                    }
                    alias++;
                }
            }
        }
        //关闭主机数据库
        endhostent();
    }
    if(flag == 1){
        //填充主机相关信息
        addr->sin_family = AF_INET;
        addr->sin_port = htons(atoi(param2));
        //将主机字节序转换为网络字节序
        inet_pton(AF_INET, ip, &addr->sin_addr);
    }else{
        printf("Input error:[%s]\n",param1);
    }
}

void create_socket(int *sockfd)
{
    *sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(*sockfd < 0){
        perror("socket error");
        return;
    }
}

void commun_func(struct sockaddr_in *addr, int sockfd)
{
    char *prompt = ">";
    char write_buf[512];
    char read_buf[512];

    while(1){
        memset(write_buf, '\0', sizeof(write_buf));
        memset(read_buf, '\0', sizeof(read_buf));
        //从命令行获取要发送到主机的信息
        if(write(STDOUT_FILENO, prompt, strlen(prompt)) != strlen(prompt)){
            perror("write error");
            continue;
        }
        if(read(STDIN_FILENO, write_buf, sizeof(write_buf)) < 0){
            perror("write error");
            continue;
        }
        write_buf[(strcspn(write_buf, "\n"))] = '\0';
        //调用sendto函数将数据发送给主机
        socklen_t addrlen = sizeof(struct sockaddr_in);
        if(sendto(sockfd, write_buf, strlen(write_buf), 0, (struct sockaddr *)addr, addrlen) < 0){
            perror("sendto error");
            continue;
        }
        //调用recvfrom函数获取来自主机的信息
        if(recvfrom(sockfd, read_buf, sizeof(read_buf), 0, (struct sockaddr *)addr, &addrlen) <= 0){
            perror("recvfrom error");
            continue;
        }
        printf("Recvive from server message: %s\n",read_buf);
    }
}

int main(int argc, char **argv)
{
    //判断命令行传入的参数(可执行文件名称、正式名/别名/IP、端口)
    if(argc < 3){
        fprintf(stderr, "Usage: [%s] [Hostname/Alias/IP] [Port]\n",argv[0]);
        return -1;
    }

    if(signal(SIGINT, signal_handler) == SIG_ERR){
        perror("signal sigint error");
        return -1;
    }

    struct sockaddr_in addr;
    get_host_msg(argv[1], argv[2], &addr);

    //1.创建数据报套接字
    create_socket(&sockfd);
    //2. 与主机进行通信   
    commun_func(&addr, sockfd);
    //3. 关闭数据报套接字
CLEANUP:    
    close(sockfd);

    return 0;
}