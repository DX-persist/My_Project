#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr,"usage: %s [srcfile] [linkfile]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int i;

	for(i=1;i<argc;i++)
	{
		if(remove(argv[i]) < 0)
		{
			perror("remove error");
			continue;
		}
	}

	printf("remove finished\n");

	return 0;
}
