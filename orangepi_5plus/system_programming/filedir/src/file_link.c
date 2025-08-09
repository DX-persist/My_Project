#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr,"usage: %s [filepath]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int i;

	for(i=2;i<argc;i++)
	{
		if(link(argv[1], argv[i]) < 0)
		{
			perror("link");
			continue;
		}
	}
	printf("link success\n");

	return 0;
}
