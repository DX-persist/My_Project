#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr, "usage:%s pipe_path\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	
	int fd;
	char buffer[128];

	fd = open(argv[1], O_RDONLY | O_TRUNC);
	if(fd == -1)
	{
		perror("open file error");
		exit(EXIT_FAILURE);
	}
	printf("open fifo successfully\n");

	memset(buffer, '\0', sizeof(buffer));

	if(read(fd, buffer, sizeof(buffer)) < 0)
	{
		perror("read error");
		exit(EXIT_FAILURE);
	}
	printf("%s\n",buffer);

	close(fd);

	return 0;
}
