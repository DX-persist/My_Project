#include "socket.h"

int sockfd = -1;

void signal_handler(int signum)
{
    if(signum == SIGINT){
        printf("server closed...\n");
        if(sockfd != -1){
            close(sockfd);
        }
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0){
        perror("create socket error");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("setsockopt error");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));

    struct hostent *host;

    while((host = gethostent()) != NULL){
        if(!strcmp(argv[1], host->h_name)){
            printf("hostname:%s\n",host->h_name);
            memcpy(&server_addr.sin_addr.s_addr, host->h_addr_list[0], 16);
        }
    }

    bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    memset(&client_addr, 0, len);
    char recvbuffer[1024];

    while(1){
        memset(recvbuffer, 0, sizeof(recvbuffer));
        if(recvfrom(sockfd, recvbuffer, sizeof(recvbuffer), 0, 
                (struct sockaddr *)&client_addr, &len) < 0){
            perror("recvfrom error");
            continue;
        }else{
            int client_port = ntohs(client_addr.sin_port);
            char ip[16];
            memset(ip, 0, sizeof(ip));
            inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip, sizeof(ip));
            printf("recv client's IP:[%s] Port:[%d] context:[%s]\n",ip,client_port,recvbuffer);

            time_t time_now = time(NULL);
            char *tim = ctime(&time_now);
            size_t size = strcspn(tim, "\n");
            tim[size] = '\0';
            if(sendto(sockfd, tim, strlen(tim), 0,
                    (struct sockaddr *)&client_addr, len) < 0){
                perror("sendto error");
                continue;
            }
        }

    }

    return 0;
}