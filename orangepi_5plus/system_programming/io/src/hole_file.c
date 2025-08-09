#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

char *buffer = "1234567890";

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		fprintf(stderr,"usage: %s [file]\n",argv[0]);

		exit(EXIT_FAILURE);
	}

	int fd;

	fd = open(argv[1],O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if(fd < 0)
	{
		perror("open error");
		exit(EXIT_FAILURE);
	}

	if(write(fd, buffer, strlen(buffer)) != strlen(buffer))
	{
		fprintf(stderr,"write error: %s\n",strerror(errno));

		exit(EXIT_FAILURE);
	}

	
	lseek(fd, 10, SEEK_END);
	

	if(write(fd, buffer, strlen(buffer)) != strlen(buffer))
	{
		fprintf(stderr,"write error: %s\n",strerror(errno));

		exit(EXIT_FAILURE);
	}

	close(fd);

	return 0;
}
