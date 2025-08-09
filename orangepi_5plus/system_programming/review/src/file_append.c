#include "header.h"
#include "io.h"

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		fprintf(stderr,"usage: %s [string] [file_path(filename)]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int fd;
	int length;

	fd = open(argv[2], O_WRONLY | O_CREAT, 0777);
	if(fd < 0)
	{
		perror("open file error");
		exit(EXIT_FAILURE);
	}

	sleep(10);

	set_fl(fd, O_APPEND);
	
	length = strlen(argv[1]);

	if(write(fd, argv[1], length) != length)
	{
		perror("write error");
		exit(EXIT_FAILURE);
	}

	close(fd);
	

	return 0;
}
