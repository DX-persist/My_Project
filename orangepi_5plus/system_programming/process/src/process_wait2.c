#include "header.h"

void normal_exit_func()
{
	exit(EXIT_SUCCESS);
}

void abnormal_exit_func()
{
	char *p = NULL;
	*p = 'c';
	puts(p);	//对空指针操作肯定会造成段错误，由此引发SIGSEGV信号
}

void print_exit_mesg_func(int status)
{
	if(WIFEXITED(status))
		printf("child process exit normally and the status is %d\n",WEXITSTATUS(status));		//正常退出会将退出码打印出来
	else if(WIFSIGNALED(status))
		printf("child process exit abnormally and the signal number is %d\n",WTERMSIG(status));		//如果是异常导致信号退出的话，会将信号的编号打印出来
	else if(WIFSTOPPED(status))
		printf("child process stopped by the signal number is %d\n",WSTOPSIG(status));		//如果子进程在运行过程中发生过停止就会将使它停止的信号的编号打印出来
	else
		printf("unknown status\n");
}

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr,"usage: %s [normal | abnormal | stop]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	pid_t pid;
	int status;

	pid = fork();
	if(pid < 0)
	{
		perror("fork error");
		exit(EXIT_FAILURE);
	}
	else if(pid == 0)
	{
		printf("child pid is %d and the parent pid is %d\n",getpid(),getppid());
		if(!(strcmp(argv[1],"normal")))		//根据外部传参来确定子进程的退出方式
			normal_exit_func();
		else if(!(strcmp(argv[1],"abnormal")))
			abnormal_exit_func();
		else if(!strcmp(argv[1],"stop"))
			pause();		//pause函数会让它一直卡着，作用和while加sleep函数一样
		else
			printf("input error,there is no such choice\n");	
	}
	else
	{
		wait(&status);		//将子进程的退出状态收集，然后写入到status这片空间里，然后使用宏对status进行判断和解析
		print_exit_mesg_func(status);
	}
	
	return 0;
}
