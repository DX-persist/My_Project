#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		fprintf(stderr,"usage: %s [pid] [signum]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	//从外部传入要发送进程的PID号和信号编号
	pid_t pid = atoi(argv[1]);
	int signum = atoi(argv[2]);

	if((kill(pid,signum)) < 0)
	{
		perror("kill perror");
		exit(EXIT_FAILURE);
	}

	return 0;
}
