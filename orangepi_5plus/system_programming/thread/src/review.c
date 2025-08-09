#include "header.h"

typedef struct
{
	long account_num;
	double balance;
	pthread_mutex_t mutex;
}Account;

typedef struct
{
	char name[32];
	Account *a;
	double oper_amount;
}OperArg;

Account *create_account(int account_num, double balance)
{
	Account *a = (Account*)malloc(sizeof(Account));
	assert(a != NULL);

	a->account_num = account_num;
	a->balance = balance;
	pthread_mutex_init(&a->mutex, NULL);		//对互斥锁进行初始化

	return a;
}

double withdrawal(Account *a, double amount)
{
	assert(a != NULL);

	pthread_mutex_lock(&a->mutex);		//对共享资源进行上锁
	if(amount <= 0 || amount > a->balance)
	{
		pthread_mutex_unlock(&a->mutex);
		return 0.0;
	}

	double balance = a->balance;
	sleep(1);
	balance -= amount;
	a->balance = balance;
	pthread_mutex_unlock(&a->mutex);

	return amount;
}

double deposit(Account *a, double amount)
{
	assert(a != NULL);

	pthread_mutex_lock(&a->mutex);
	if(amount <= 0)
	{
		pthread_mutex_unlock(&a->mutex);
		return 0.0;
	}

	double balance = a->balance;
	balance += amount;
	a->balance = balance;
	pthread_mutex_unlock(&a->mutex);

	return amount;
}

double get_balance(Account *a)
{
	assert(a != NULL);

	pthread_mutex_lock(&a->mutex);
	double balance = a->balance;
	pthread_mutex_unlock(&a->mutex);

	return balance;
}

void destory_account(Account *a)
{
	assert(a != NULL);

	pthread_mutex_destroy(&a->mutex);		//这里顺序不能颠倒，如果先把a释放掉，然后再销毁互斥锁就会造成段错误，因为它对空指针进行操作了
	free(a);
	a = NULL;
}

void* withdrawal_func(void *arg)
{
	OperArg *r = (OperArg*)arg;

	double amount = withdrawal(r->a, r->oper_amount);
	puts("===========================================");
	printf("[Thread id:%lx Operator:%s] [Operate account:%ld] [Operate amount:%f] [get amount:%f]\n",
			pthread_self(),r->name,r->a->account_num,r->oper_amount,amount);
}

int main(void)
{
	int err = -1;
	pthread_t boy,girl;
	OperArg arg_boy,arg_girl;

	Account *a = create_account(100055466,10000);

	strcpy(arg_boy.name,"boy");
	arg_boy.a = a;
	arg_boy.oper_amount = 10000;
	
	strcpy(arg_girl.name,"girl");
	arg_girl.a = a;
	arg_girl.oper_amount = 10000;


	if((err = pthread_create(&boy, NULL, withdrawal_func, (void*)&arg_boy)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	if((err = pthread_create(&girl, NULL, withdrawal_func, (void*)&arg_girl)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	pthread_join(girl, NULL);
	pthread_join(boy,  NULL);

	printf("account's balance:%f\n",get_balance(a));

	destroy_account(a);

	return 0;
}
