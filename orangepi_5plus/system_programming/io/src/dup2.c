#include "io.h"


int main(int argc, char **argv)
{
	int fdin = STDIN_FILENO;
	int fdout = STDOUT_FILENO;
	int fd1;
	int fd2;
   
	
	fd1 = open(argv[1],O_RDONLY);
	if(fd1 < 0)
	{
		perror("open file error");
		exit(EXIT_FAILURE);
	}
	//将标准输入重定向到文件
	if(dup2(fd1,fdin) != fdin)
	{
		perror("dup2 error");
		exit(EXIT_FAILURE);
	}

	close(fd1);

	//将标准输出重定向到文件
	fd2 = open(argv[2],O_WRONLY | O_CREAT | O_TRUNC,0777);
	if(fd2 < 0)
	{
		perror("open file error");
		exit(EXIT_FAILURE);
	}

	if(dup2(fd2,fdout) != fdout)
	{
		perror("dup2 error");
		exit(EXIT_FAILURE);
	}

	close(fd2);

	copy_function(fdin,fdout);

	close(fdin);
	close(fdout);

	return 0;
}
