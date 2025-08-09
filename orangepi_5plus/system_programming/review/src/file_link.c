#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		fprintf(stderr,"usage: %s [srcfile] [linkfile]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int i;

	for(i=2;i<argc;i++)
	{
		if(link(argv[1],argv[i]) < 0)
		{
			perror("link error");
			continue;
		}
	}

	printf("link finished\n");

	return 0;
}
