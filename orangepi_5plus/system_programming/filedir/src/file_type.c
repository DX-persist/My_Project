#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr,"usage: %s [file]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	
	int i;
	struct stat file_type;

	for(i=1; i<argc; i++)
	{
		if(lstat(argv[i],&file_type) < 0)
		{
			perror("lstat error");
			continue;
		}
		
		printf("%-20s",argv[i]);	
		
		if(S_ISREG(file_type.st_mode))
		{
			printf("regular file\n");
		}
		else if(S_ISDIR(file_type.st_mode))
		{
			printf("dictionary\n");
		}
		else if(S_ISBLK(file_type.st_mode))
		{
			printf("block device\n");
		}
		else if(S_ISCHR(file_type.st_mode))
		{
			printf("character device\n");
		}
		else if(S_ISFIFO(file_type.st_mode))
		{
			printf("named pipe\n");
		}
		else if(S_ISSOCK(file_type.st_mode))
		{
			printf("sock device\n");
		}
		else if(S_ISLNK(file_type.st_mode))
		{
			printf("link file\n");
		}
		else
		{
			printf("unknown file type\n");
		}
	}

	return 0;
}
