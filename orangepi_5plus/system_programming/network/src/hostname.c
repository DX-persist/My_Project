#include "socket.h"

void out_message(struct hostent *host)
{
    printf("the host's name is:%s\n",host->h_name);
    
    int i = 0;
    while(host->h_aliases[i] != NULL){
        printf("aliases:%s\n",host->h_aliases[i]);
        i++;
    }
    printf("type:%s\n",host->h_addrtype == AF_INET ? "IPV4":"IPV6");

    char ip[16];
    memset(ip, 0, sizeof(ip));
    inet_ntop(AF_INET, host->h_addr_list[0], ip, sizeof(ip));
    printf("ip address:%s\n",ip);
}

int main(int argc, char **argv)
{
    if(argc < 2){
        fprintf(stderr, "Usage:[%s] [host's name]\n",argv[0]);
        exit(EXIT_FAILURE);
    }
    struct hostent *host;

    while((host = gethostent()) != NULL){
        if(!strcmp(argv[1], host->h_name)){
            out_message(host);
            break;
        }else{
            int i = 0;
            while(host->h_aliases[i] != NULL){
                if(!strcmp(host->h_aliases[i], argv[1])){
                    out_message(host);
                    break;
                }
                i++;
            }
        }
    }
    

    return 0;
}