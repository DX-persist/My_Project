#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>

void out_host_msg(struct hostent *host)
{
	//打印主机的正式名
	printf("Official name:[%s]\n",host->h_name);
	//打印主机的别名
	char **alias = host->h_aliases;
	while(*alias != NULL){
		printf("Alias name:[%s]\n",*alias);
		alias++;
	}
	//打印主机的地址类型
	printf("Host type:[%s]\n",host->h_addrtype == AF_INET ? "IPv4" : "IPv6");
	//打印主机的IP地址
	char **addr_list = host->h_addr_list;
	char ip[16];

	while(*addr_list != NULL){
		memset(ip, '\0', sizeof(ip));
		inet_ntop(AF_INET, *addr_list, ip, sizeof(ip));
		addr_list++;
		printf("Host IP:[%s]\n",ip);
	}
}

int main(int argc, char **argv)
{
	//从命令行获取主机的正式名/别名
	if(argc < 2){
		fprintf(stderr, "Usage: [%s] [Hostname]\n",argv[0]);
		return -1;
	}

	struct hostent *host = NULL;

	host = gethostbyname(argv[1]);
	if(host == NULL){
		herror("gethostbyname");
		return -1;
	}

	out_host_msg(host);

	return 0;
}
