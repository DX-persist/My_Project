#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "io.h"


int main(int argc, char **argv)
{
	if(argc != 3)
	{
		fprintf(stderr,"usage: %s [string] [filename]\n",argv[0]);

		exit(EXIT_FAILURE);
	}

	int fd;
	char length = strlen(argv[1]);

	fd = open(argv[2], O_WRONLY | O_CREAT | O_CREAT , 0777);
	if(fd < 0)
	{
		perror("open file error");
		exit(EXIT_FAILURE);
	}
	//设置追加标志位
	set_fl(fd,O_APPEND);	
	//清除追加标志位
	//clr_flag(fd,O_APPEND);
	
	sleep(10);

	if(write(fd,argv[1],length) != length)
	{
		perror("write message error");
		exit(EXIT_FAILURE);
	}

	close(fd);

	return 0;
}
