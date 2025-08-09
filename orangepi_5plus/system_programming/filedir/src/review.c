#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr,"usage: %s [filepath]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int i;
	struct stat file_type;

	for(i=1;i<argc;i++)
	{
		//根据外部传参来判断文件类型，lstat函数不仅能够判断文件，还能够判断符号链接文件
		if(lstat(argv[i], &file_type) < 0)
		{
			perror("lstat error");
			continue;
		}	
		printf("%-20s",argv[i]);	//先输出字符串，然后再输出20个空格

		if(S_ISREG(file_type.st_mode))
		{
			printf("regularly file");
		}
		else if(S_ISDIR(file_type.st_mode))
		{
			printf("dictionary");
		}
		else if(S_ISLNK(file_type.st_mode))
		{
			printf("link file");
		}
		else if(S_ISBLK(file_type.st_mode))
		{
			printf("block device file");
		}
		else if(S_ISCHR(file_type.st_mode))
		{
			printf("character device file");
		}
		else if(S_ISSOCK(file_type.st_mode))
		{
			printf("socket device file");
		}
		else if(S_ISFIFO(file_type.st_mode))
		{
			printf("named pipe");
		}
		else
		{
			printf("unknown file type");
		}
		printf("\n");
	}



	return 0;
}
