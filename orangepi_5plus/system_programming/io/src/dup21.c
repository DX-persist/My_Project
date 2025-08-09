#include "io.h"

int main(int argc, char **argv)
{
	int fdin,fdout;
	int i;
	int flag = 0;

	for(i=1;i<argc;i++)
	{
		//+定义为输入重定向
		if(!strcmp("+",argv[i]))
		{
			fdin = open(argv[++i],O_RDONLY);
			if(fdin < 0)
			{
				perror("open file error");
				exit(EXIT_FAILURE);
			}

			if(dup2(fdin,STDIN_FILENO) != STDIN_FILENO)
			{
				perror("dup2 error");
				exit(EXIT_FAILURE);
			}
			close(fdin);
		}
		else if(!strcmp("-",argv[i]))
		{
			fdout = open(argv[++i],O_WRONLY | O_CREAT | O_TRUNC, 0777);
			if(fdout < 0)
			{
				perror("open file error");
				exit(EXIT_FAILURE);
			}

			if(dup2(fdout,STDOUT_FILENO) != STDOUT_FILENO)
			{
				perror("dup2 error");
				exit(EXIT_FAILURE);
			}
			close(fdout);
		}
		else
		{
			flag = 0;

			fdin = open(argv[i],O_RDONLY);
			if(fdin < 0)
			{
				perror("open file error");
				exit(EXIT_FAILURE);
			}

			if(dup2(fdin,STDIN_FILENO) != STDIN_FILENO)
			{
				perror("dup2 error");
				exit(EXIT_FAILURE);
			}

			copy_function(STDIN_FILENO, STDOUT_FILENO);
		}

		if(!flag)
		{
			copy_function(STDIN_FILENO,STDOUT_FILENO);
		}
	}

	return 0;
}
