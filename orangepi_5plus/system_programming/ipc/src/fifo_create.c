#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr,"usage:%s fifo_path\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	if(mkfifo(argv[1], S_IRWXU | S_IRWXG | S_IROTH) < 0)
	{
		perror("mkfifo error");
		exit(EXIT_FAILURE);
	}

	return 0;
}
