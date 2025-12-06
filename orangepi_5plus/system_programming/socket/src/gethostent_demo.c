#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>

void out_host_msg(struct hostent *host)
{
	//打印主机的正式名
	printf("Official name:[%s]\n",host->h_name);
	//打印主机的别名(判断主机是否有别名)
	if(host->h_aliases != NULL){
		char **alias = host->h_aliases;
		while(*alias != NULL){
			printf("Alias name:[%s]\n",*alias);
			alias++;
		}
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
	printf("-------------------------\n");
}

int main(void)
{
	//打开主机数据库,stayopen = 1表示保持打开状态
	sethostent(1);

	struct hostent *host = NULL;

	//一直遍历直到文件末尾
	while((host = gethostent()) != NULL){
		out_host_msg(host);
	}

	//关闭主机数据库
	endhostent();

	return 0;
}
