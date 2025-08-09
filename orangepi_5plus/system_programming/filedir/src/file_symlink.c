#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		fprintf(stderr,"usage: %s [srcfile] [link_filepath]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int fd;
	char buffer[4096];
	ssize_t nbytes;

	if(symlink(argv[1], argv[2]) < 0)
	{
		perror("symlink");
		exit(EXIT_FAILURE);
	}

	if((fd = open(argv[2], O_RDONLY)) < 0)
	{
		perror("open file error");
		exit(EXIT_FAILURE);
	}
	
	memset(buffer,'\0',sizeof(buffer));

	if((nbytes = (read(fd, buffer, sizeof(buffer)))) < 0)
	{
		perror("read file error");
		exit(EXIT_FAILURE);
	}
	else
	{
		fprintf(stdout,"read content from file \n%s\n",buffer);
		/*
		if(write(STDOUT_FILENO, buffer, nbytes) != nbytes)
		{
			perror("write file error");
			exit(EXIT_FAILURE);
		}*/
	}

	memset(buffer, '\0',sizeof(buffer));

	if((nbytes = readlink(argv[2], buffer,sizeof(buffer))) < 0)
	{
		perror("readlink error");
		exit(EXIT_FAILURE);
	}

	fprintf(stdout,"readlink: %s\n",buffer);

	close(fd);

	return 0;
}
