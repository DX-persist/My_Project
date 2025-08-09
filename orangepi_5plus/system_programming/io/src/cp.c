#include "io.h"

int main(int argc, char **argv)
{
	long size;

	if(argc != 3)
	{
		fprintf(stderr,"usage: %s srcfile destfile\n",argv[0]);

		exit(EXIT_FAILURE);
	}

	int fdin, fdout;

	fdin = open(argv[1],O_RDONLY);
	if(fdin < 0)
	{
		fprintf(stderr,"open file error: %s\n",strerror(errno));

		exit(EXIT_FAILURE);
	}

	size = lseek(fdin,0,SEEK_END);

	printf("the size of file is %ld\n",size);

	fdout = open(argv[2],O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if(fdout < 0)
	{
		fprintf(stderr,"open file error: %s\n",strerror(errno));

		exit(EXIT_FAILURE);
	}

	lseek(fdin,-size,SEEK_END);
	copy_function(fdin,fdout);

	close(fdin);
	close(fdout);

	return 0;
}
