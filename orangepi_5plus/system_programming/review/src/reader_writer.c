#include "header.h"

typedef struct
{
	int val;
	int semid;
}Storage;

void init_sem(Storage *s)
{
	assert(s != NULL);

	//创建信号量集，信号量的个数是2个
	s->semid = semget(IPC_PRIVATE, 2, IPC_CREAT | IPC_EXCL | 0774);
	if(s->semid < 0)
	{
		perror("semget error");
		exit(EXIT_FAILURE);
	}
	
	union semun
	{
		int val;
		struct semid_ds *buf;
		unsigned short *array;
	};
	union semun un;

	unsigned short array[2] = {0, 0};
	un.array = array;

	if(semctl(s->semid, 0, SETALL, un) < 0)
	{
		perror("semctl error");
		exit(EXIT_FAILURE);
	}
}

void write_func(Storage *s, int value)
{
	assert(s != NULL);

	s->val = value;
	printf("write process:%d write:%3d\n",getpid(),s->val);

	struct sembuf semops_v[1] = {{0, 1, SEM_UNDO}};
	struct sembuf semops_p[1] = {{1, -1, SEM_UNDO}};

	if(semop(s->semid, semops_v, sizeof(semops_v)/sizeof(semops_v[0])) < 0)
	{
		perror("semop error");
		exit(EXIT_FAILURE);
	}
	
	if(semop(s->semid, semops_p, sizeof(semops_p)/sizeof(semops_p[0])) < 0)
	{
		perror("semop error");
		exit(EXIT_FAILURE);
	}
}

void read_func(Storage *s)
{
	assert(s != NULL);

	struct sembuf semops_p[1] = {{0, -1, SEM_UNDO}};
	struct sembuf semops_v[1] = {{1, 1, SEM_UNDO}};

	if(semop(s->semid, semops_p, sizeof(semops_p)/sizeof(semops_p[0])) < 0)
	{
		perror("semop error");
		exit(EXIT_FAILURE);
	}
	printf("read process:%d read:%5d\n",getpid(),s->val);

	if(semop(s->semid, semops_v, sizeof(semops_v)/sizeof(semops_v[0])) < 0)
	{
		perror("semop error");
		exit(EXIT_FAILURE);
	}
}

void destroy_sem(Storage *s)
{
	assert(s != NULL);

	if(semctl(s->semid, 0, IPC_RMID, NULL) < 0)
	{
		perror("semctl error");
		exit(EXIT_FAILURE);
	}
}

int main(void)
{
	//创建共享内存，共享内存的大小是Stroage
	int shmid = shmget(IPC_PRIVATE, sizeof(Storage), IPC_CREAT | IPC_EXCL | 0774);
	if(shmid < 0)
	{
		perror("shmget error");
		exit(EXIT_FAILURE);
	}

	//将共享内存映射到当前进程中
	Storage *s = (Storage*)shmat(shmid, 0, 0);
	if(s == (Storage*)-1)
	{
		perror("shmat error");
		exit(EXIT_FAILURE);
	}
	init_sem(s);

	pid_t pid;

	if((pid = fork()) < 0)
	{
		perror("fork error");
		exit(EXIT_FAILURE);
	}
	else if(pid > 0)
	{
		int i = 1;
		for(; i <= 100; i++)
		{
			write_func(s, i);
		}
		wait(NULL);
		destroy_sem(s);
		shmdt(s);
		shmctl(shmid, IPC_RMID, NULL);			
	}
	else
	{
		int i = 1;
		for(; i <= 100; i++)
		{
			read_func(s);
		}
		shmdt(s);
	}

	return 0;
}
