#include "header.h"

void mysystem_func(char *cmd)
{
	pid_t pid;
	
	if((pid = fork()) < 0)
	{
		perror("fork error");
		exit(EXIT_FAILURE);
	}
	else if(pid == 0)
	{
		char *buffer[] = {"bash","-c",cmd,NULL};	//这里的参数bash表示使用bash来解析命令行，-c参数表示后面的字符串作为命令来执行

		if(execvp("bash",buffer) < 0)	//这里的execvp函数表明将后边的参数构建出一个指针数组，然后当作参数使用
										//由于这里使用的是p，所以会默认在系统环境变量里查找指定的可执行程序
		{
			perror("execlp error");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		wait(NULL);
	}
}

int main(void)
{
	char *cmd1 = "ls";
	char *cmd2 = "ls -l > ls.log";
	char *cmd3 = "-l";
	char buffer[28] = {'\0'};

	sprintf(buffer,"%s %s",cmd1,cmd3);
	system("clear");
	system(cmd1);
	system(buffer);
	mysystem_func(cmd2);

	return 0;
}
