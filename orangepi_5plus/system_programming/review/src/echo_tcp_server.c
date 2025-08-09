#include "socket.h"

int sockfd = -1;

void signal_handler(int signum)
{
	if(signum == SIGINT){
		printf("server closed....\n");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	if(signum == SIGCHLD){
		printf("client closed....\n");
		wait(NULL);
	}
}

int main(int argc, char **argv)
{
	if(argc < 2){
		fprintf(stderr,"%s|%s|%d|Usage:[%s] [Port]\n",
				 	__FILE__,__func__,__LINE__,argv[0]);
		exit(EXIT_FAILURE);
	}

	char *cmd = "ifconfig | grep \"inet 192\" | awk '{print $2}'";
	char *ip = getAddress(cmd);
	
	//向内核注册信号处理函数,如果收到SIGINT信号就关闭服务器退出
	//如果收到SIGCHLD信号就去执行信号处理函数去回收子进程的资源防止子进程变成僵尸进程
	if(signal(SIGINT, signal_handler) == SIG_ERR)
	{
		perror("signal sigint error");
		exit(EXIT_FAILURE);
	}
	if(signal(SIGCHLD, signal_handler) == SIG_ERR)
	{
		perror("signal sigchld error");
		exit(EXIT_FAILURE);
	}

	sockfd = initSocket(sockfd, ip, argv[1]);
	//step4:接受客户端的连接,从内核维护的队列(已完成三次握手的客户端请求)
	//中选一个客户端进行连接	

	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	memset(&client_addr, 0, len);
	int newSockfd;

	while(1)
	{
		if((newSockfd = accept(sockfd, (struct sockaddr *)&client_addr, &len)) < 0){
			perror("accept error");
			continue;
		}

		//step5:创建子进程使用accept函数返回的新的文件描述符和客户端进行通信
		pid_t pid;
		if((pid = fork()) < 0){
			perror("fork error");
			continue;
		}else if(pid == 0){
			out_addr(&client_addr);
			do_service(newSockfd);
			close(newSockfd);
			break;
		}else{
			close(newSockfd);
		}
	}
	
	finish_commun(ip);
	
	return 0;
}
