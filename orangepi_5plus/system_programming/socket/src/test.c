#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
	if(argc < 3){
		fprintf(stderr, "Usage: [%s] [Hostname/Alias/IP] [Port]\n",argv[0]);
		return -1;
	}

	struct hostent *host = NULL;
	char ip[16];
	while((host = gethostent()) != NULL){
		if(!strcmp(host->h_name, argv[1])){
			printf("Official name:[%s]\n",host->h_name);
			memset(ip, '\0', sizeof(ip));
			inet_ntop(AF_INET, host->h_addr_list[0], ip, sizeof(ip));
			printf("ip:[%s]\n",ip);
		}else{
			char **alias = host->h_aliases;
			while(*alias != NULL){
				if(!strcmp(*alias, argv[1])){
					printf("Alias name:[%s]\n",*alias);
					memset(ip, '\0', sizeof(ip));
					inet_ntop(AF_INET, host->h_addr_list[0], ip, sizeof(ip));
					printf("ip:[%s]\n",ip);
				}
				alias++;
			}
		}
	}

	return 0;
}
