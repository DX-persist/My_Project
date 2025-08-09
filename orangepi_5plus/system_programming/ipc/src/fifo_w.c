#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr, "usage:%s fifo_path\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int fd;
	char *str = "123456789";

	fd = open(argv[1], O_WRONLY);
	if(fd == -1)
	{
		perror("open fifo error");
		exit(EXIT_FAILURE);
	}
	printf("open fifo successfully\n");

	if(write(fd, str, strlen(str)) != strlen(str))
	{
		perror("write error");
		exit(EXIT_FAILURE);
	}
	printf("successfully write to the fifo\n");

	close(fd);

	return 0;
}
