#include "socket.h"

int socket_init(const char *ipaddr,const char *ipport)
{
	int sock_fd = -1;
	int bind_ret;
	int listen_ret;
	struct sockaddr_in s_addr;

	memset(&s_addr,'\0',sizeof(s_addr));

	sock_fd = socket(AF_INET,SOCK_STREAM,0);
	//第一个参数是指定了套接字使用的协议族，AF_INET表示使用ipaddrv4网络通信，用于TCP网络传输
	//第二个参数是套接字的类型，SOCK_STREAM常用于TCP协议
	//第三个参数通常设置为0.表示系统会根据前两个选项给套接字自动分配协议
	if(sock_fd == -1)
	{
		perror("socket");
		return -1;
	}

	//设置SO_REUSEADDR,允许端口复用
	int opt = 1;
	if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
		perror("setsockopt");
        return -1;
	}

	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(atoi(ipport));
	//htons函数的作用是将主机字节序转换为网络字节序，网络字节序是大端字节序
	s_addr.sin_addr.s_addr = inet_addr(ipaddr);
	//inet_addr函数的作用是将点分十进制的ipv4地址转换为网络字节序的长整型

	//int bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
	//bind函数是将套接字绑定到特点的地址和端口上，以便客户端能够连接到指定的地址和端口
	//第一个参数是需要绑定的套接字的文件描述符
	//第二个参数是包含本地地址信息的sockaddr的结构体指针
	//第三个参数是结构体指针的长度，通过sizeof获取

	bind_ret = bind(sock_fd,(const struct sockaddr *)&s_addr,sizeof(s_addr));
	if(bind_ret == -1)
	{
		perror("bind");
		return -1;
	}

	//int listen(int sockfd, int backlog);
	//listen函数将套接字转换为监听套接字，一边接受客户端的请求
	//函数的第一个参数是使用socket函数创建的套接字的返回的文件描述符
	//函数的第二个参数用于设置当未和服务器完成连接的等待队列的大小
	listen_ret = listen(sock_fd,1);
	if(listen_ret == -1)
	{
		perror("listen");
		return -1;
	}

	return sock_fd;
}
