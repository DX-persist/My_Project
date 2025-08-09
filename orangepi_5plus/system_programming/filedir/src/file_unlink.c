#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr,"usage: %s [filepath]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	
	int i;

	for(i=1;i<argc;i++)
	{
		if(unlink(argv[i]) < 0)
		{
			perror("unlink");
			continue;
		}
	}
	printf("unlink success\n");

	return 0;
}
