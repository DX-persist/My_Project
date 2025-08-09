#include "header.h"

typedef struct
{
	long mtype;
	int data;
}MSG;

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		fprintf(stderr,"usage:%s key_value, message_type\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	key_t key;
	int msq_id;
	MSG msg;
	ssize_t nbytes;

	//将外部传参argv[1],argv[2]赋值给键值和消息类型
	key = atoi(argv[1]);
	msg.mtype = atoi(argv[2]);

	//打开消息队列获取唯一的标识符
	if((msq_id = msgget(key, S_IRWXU | S_IRWXG | S_IROTH)) < 0)
	{
		perror("msgget error");
		exit(EXIT_FAILURE);
	}

	//从消息队列中读取消息
	nbytes = msgrcv(msq_id, &msg, sizeof(MSG)-sizeof(long), msg.mtype, IPC_NOWAIT);

	printf("%d\n",msg.data);

	struct msqid_ds ds;
	msgctl(msq_id, IPC_STAT, &ds);		//获取消息队列的属性
	printf("last rcv pid:%d last rcv time:%scurrent message numbers in queue:%ld\n",
								ds.msg_lrpid, ctime(&ds.msg_rtime),ds.msg_cbytes);									
	
	if(nbytes < 0)	
	{
		perror("msgrcv error");
		if(!strcmp(strerror(errno),"No message of desired type") && ds.msg_qnum == 0)
		{
			msgctl(msq_id, IPC_RMID, NULL);			//将消息队列从内核中移除
		}
		exit(EXIT_FAILURE);
	}

	return 0;
}
