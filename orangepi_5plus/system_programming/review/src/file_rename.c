#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		fprintf(stderr,"usage: %s [srcfile] [renamefile]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int i;

	for(i=2;i<argc;i++)
	{
		if(rename(argv[1],argv[i]) < 0)
		{
			perror("rename error");
			continue;
		}
	}

	printf("rename finished\n");

	return 0;
}
