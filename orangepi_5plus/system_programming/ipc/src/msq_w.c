#include "header.h"

typedef struct
{
	long mtype;
	int data;
}MSG;
int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr, "usage: %s keyvalue\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	key_t key;
	MSG msg;
	int msq_id;

	key = atoi(argv[1]);			//将外部传参argv[1]赋值给键值用于创建唯一的消息队列id
	
	//根据键值创建的消息队列，消息队列的权限为拥有者，同组人拥有可读可写可执行的权限，其他有只有可读的权限
	if((msq_id = msgget(key, IPC_CREAT | IPC_EXCL | S_IRWXU | S_IRWXG | S_IROTH)) < 0)
	{
		perror("msgget error");
		exit(EXIT_FAILURE);
	}
	printf("msq_id:%d\n",msq_id);


	memset(&msg, 0, sizeof(msg));
	
	msg.mtype = 100;
	msg.data = 123;
	if(msgsnd(msq_id, &msg, sizeof(MSG)-sizeof(long), IPC_NOWAIT) < 0)
	{
		perror("msgsnd error");
		exit(EXIT_FAILURE);
	}
	struct msqid_ds ds;
	msgctl(msq_id, IPC_STAT, &ds);
	printf("msg number:%ld last write pid:%d\n",ds.msg_qnum,ds.msg_lspid);

	return 0;
}
