#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr,"usage: %s [filename]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int i;

	for(i=1; i<argc; i++)
	{
		if(!access(argv[i],F_OK))
		{
			printf("file: %s exists\n",argv[i]);
		}
		else
		{
			printf("file: there is no such file: %s\n",argv[i]);
		}

		if(!access(argv[i],R_OK))
		{
			printf("file can read\n");
		}
		else
		{
			printf("file can't read\n");
		}

		if(!access(argv[i],W_OK))
		{
			printf("file can write\n");
		}
		else
		{
			printf("file can't write\n");
		}
		
		if(!access(argv[i],X_OK))
		{
			printf("file can excute\n");
		}
		else
		{
			printf("file can't excute\n");
		}
	}


	return 0;
}
