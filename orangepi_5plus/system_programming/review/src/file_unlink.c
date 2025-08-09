#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		fprintf(stderr,"usage: %s [srcfile] [linkfile]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int i;

	for(i=1;i<argc;i++)
	{
		if(unlink(argv[i]) < 0)
		{
			perror("unlink error");
			continue;
		}
	}

	printf("unlink finished\n");

	return 0;
}
