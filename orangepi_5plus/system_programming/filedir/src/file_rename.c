#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		fprintf(stderr,"usage: %s [filepath]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int i;

	for(i=2;i<argc;i++)
	{
		if(rename(argv[1], argv[i]) < 0)
		{
			perror("rename");
			continue;
		}
		else
		{
			printf("rename  %s success\n",argv[i]);
		}
	}

	return 0;
}
