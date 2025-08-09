#include "header.h"

int main(void)
{
	FILE *fp;	
	char buffer[128];

	fp = popen("cat /etc/passwd | grep root", "r");
	if(fp == NULL)
	{
		perror("fopen error");
		exit(EXIT_FAILURE);
	}
	
	memset(buffer, '\0', sizeof(buffer));		
	while(fgets(buffer, sizeof(buffer), fp))
	{
		printf("%s",buffer);
	}
	pclose(fp);

	printf("--------------------------------------\n");

	fp = popen("wc -l", "w");
	fprintf(fp, "%s", "hello\nhaha\nlala\n");

	pclose(fp);

	return 0;
}
