#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

int main()
{
	char buffer[1024];
	int nbytes;

	memset(buffer,'\0',sizeof(buffer));

	//获取文件状态标志
	int val = fcntl(STDIN_FILENO,F_GETFL);
	//将要设置的文件状态写入
	val |= O_NONBLOCK;
	//设置新的文件状态标志为非阻塞方式
	if(fcntl(STDIN_FILENO,F_SETFL,val) < 0)
	{
		perror("fcntl");
	}

	sleep(3);

	nbytes = read(STDIN_FILENO,buffer,sizeof(buffer));
	if(nbytes < 0)
	{
		perror("read error");
		exit(EXIT_FAILURE);
	}
	else if(nbytes == 0)
	{
		printf("read finished\n");
	}
	else
	{
		if(write(STDOUT_FILENO,buffer,nbytes) != nbytes)
		{
			perror("write error");
			exit(EXIT_FAILURE);
		}
	}

	return 0;
}
