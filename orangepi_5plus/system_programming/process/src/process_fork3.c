#include "header.h"

int g_v = 30;

int main(void)
{
	int a_v = 30;
	static int s_v = 30;

	pid_t pid;

	printf("parent pid is %d\n",getpid());
	pid = fork();
	if(pid > 0)
	{
		g_v = 40; a_v = 40; s_v = 40;
		printf("this is parent process, getpid: %d getppid: %d pid: %d\n",getpid(),getppid(),pid);
		printf("ag_v:%p aa_v:%p as_v:%p\n",&g_v,&a_v,&s_v);
		printf("pid is %d g_v:%d a_v:%d s_v:%d\n",getpid(),g_v,a_v,s_v);
	}
	else if(pid == 0)
	{
		g_v = 50; a_v = 50; s_v = 50;
		printf("this is child process,getpid: %d getppid: %d pid: %d\n",getpid(),getppid(),pid);
		printf("ag_v:%p aa_v:%p as_v:%p\n",&g_v,&a_v,&s_v);
		printf("pid is %d g_v:%d a_v:%d s_v:%d\n",getpid(),g_v,a_v,s_v);
	}
	else
	{
		perror("fork error");
	}


	return 0;
}
