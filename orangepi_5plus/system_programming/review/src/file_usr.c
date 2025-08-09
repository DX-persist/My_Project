#include "header.h"
#include "io.h"

int main(int argc,char **argv)
{
	if(argc < 3)
	{
		fprintf(stderr,"usage: %s [srcfile] [destfile]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int fdin, fdout;

	fdin = open(argv[1],O_RDONLY);
	if(fdin < 0)
	{
		perror("open file error");
		exit(EXIT_FAILURE);
	}

	fdout = open(argv[2],O_WRONLY | O_CREAT | O_TRUNC, 
					S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH);

	if(fdout < 0)
	{
		perror("open file error");
		exit(EXIT_FAILURE);
	}

	copy(fdin, fdout);

	close(fdin);
	close(fdout);

	return 0;
}
