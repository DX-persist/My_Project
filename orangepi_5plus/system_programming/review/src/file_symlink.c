#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		fprintf(stderr,"usage: %s [srcfile] [linkfile]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	
	int i;
	char buffer[4096];
	int fd;
	ssize_t nbytes;

	for(i=2;i<argc;i++)
	{
		if(symlink(argv[1],argv[i]) < 0)
		{
			perror("symlink error");
			exit(EXIT_FAILURE);
		}
	}
	printf("symlink finished\n");

	fd = open(argv[2],O_RDONLY);
	if(fd < 0)
	{
		perror("open file error");
		exit(EXIT_FAILURE);
	}

	memset(buffer,'\0',sizeof(buffer));

	//将符号链接文件里边的内容输出到屏幕上
	if((nbytes = read(fd, buffer, sizeof(buffer))) < 0)
	{
		perror("read error");
		exit(EXIT_FAILURE);
	}
	else
	{
		if(write(STDOUT_FILENO,buffer,nbytes) != nbytes)
		{
			perror("write error");
			exit(EXIT_FAILURE);
		}
	}
	
	memset(buffer,'\0',sizeof(buffer));
	if((nbytes = readlink(argv[2], buffer, sizeof(buffer))) < 0)
	{
		perror("readlink error");
		exit(EXIT_FAILURE);
	}
	else
	{
		fprintf(stdout,"readlink: \n%s\n",buffer);
	}

	return 0;
}
