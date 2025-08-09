#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr,"usage: %s [filename]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	
	int i;

	for(i=1;i<argc;i++)
	{
		if(!access(argv[i], F_OK))
		{
			printf("file: %s exits\n",argv[i]);
		}
		else
		{
			printf("there is no such file: %s\n",argv[i]);
			continue;
		}

		if(!access(argv[i],R_OK))
		{
			printf("file: %s can read\n",argv[i]);
		}
		else
		{
			printf("file: %s can't read\n",argv[i]);
		}
		if(!access(argv[i],W_OK))
		{
			printf("file: %s can write\n",argv[i]);
		}
		else
		{
			printf("file: %s can't write\n",argv[i]);
		}
		if(!access(argv[i],X_OK))
		{
			printf("file: %s can excute\n",argv[i]);
		}
		else
		{
			printf("file: %s can't excute\n",argv[i]);
		}
	}

	return 0;
}
