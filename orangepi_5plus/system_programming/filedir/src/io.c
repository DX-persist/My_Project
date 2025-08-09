#include "io.h"

#define BUFFER_SIZE 1024

void copy_function(int fdin, int fdout)
{
	ssize_t nbytes = 0;
	char buffer[BUFFER_SIZE];

	memset(buffer,'\0',BUFFER_SIZE);

	//从源文件中读取到buffer中
	while((nbytes = read(fdin, buffer, BUFFER_SIZE)) > 0)
	{
//		printf("the size of read bytes is %ld\n",lseek(fdin,0,SEEK_CUR));
		//将buffer里边的数据写入到目标文件中去
		if(write(fdout, buffer, nbytes) != nbytes)
		{
			fprintf(stderr,"write error: %s\n",strerror(errno));

			exit(EXIT_FAILURE);
		}
	}

	if(nbytes < 0)
	{
		fprintf(stderr,"read error: %s\n",strerror(errno));

		exit(EXIT_FAILURE);
	}
}

void set_fl(int fd, int flag)
{
	//获取文件状态标志
	int val = fcntl(fd,F_GETFL);
	if(val == -1)
	{
		perror("fcntl");
	}	
	//通过函数调由外部传入想要增加的标志，增加标志使用 |=
	val |= flag;
	//重新设置文件状态标志
	if(fcntl(fd,F_SETFL,val) < 0)
	{
		perror("fcntl");
	}
}

void clr_flag(int fd, int flag)
{
	int val = fcntl(fd,F_GETFL);
	if(val == -1)
	{
		perror("fcntl");
	}

	//清除对应的文件状态标志使用 &=
	val &= ~flag;

	if(fcntl(fd,F_SETFL,val) < 0)
	{
		perror("fcntl");
	}
}
