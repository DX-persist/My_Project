#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr,"usage: %s [filename]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int i;
	struct stat type;

	for(i=1;i<argc;i++)
	{
		if(lstat(argv[i], &type) < 0)
		{
			perror("lstat error");
			continue;
		}

		if(S_ISREG(type.st_mode))
		{
			printf("regularly file\n");
		}
		else if(S_ISDIR(type.st_mode))
		{
			printf("dictionary\n");
		}
		else if(S_ISLNK(type.st_mode))
		{
			printf("link file\n");
		}
		else if(S_ISFIFO(type.st_mode))
		{
			printf("named pipe\n");
		}
		else if(S_ISSOCK(type.st_mode))
		{
			printf("socket device\n");
		}
		else if(S_ISCHR(type.st_mode))
		{
			printf("character device\n");
		}
		else if(S_ISBLK(type.st_mode))
		{
			printf("block device\n");
		}
		else 
		{
			printf("unknown file type\n");
		}
	}


	return 0;
}
