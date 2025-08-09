#include "header.h"
#include "account.h"

typedef struct
{
	char name[32];
	Account *a;
	double amount;
}OperArg;

void* withdrawal_func(void *arg)
{
	OperArg *r = (OperArg*)arg;

	double amount = withdrawal(r->a, r->amount);

	//将线程的id，操作者的名字，操作账户，操作金额以及最后从账户里拿到的钱打印出来
	printf("[Thread id:%lx Oper_name:%s Oper_account:%d]: [Oper_amount:%f get_amount:%f]\n",
				pthread_self(),r->name,r->a->account_number,r->amount,amount);

	pthread_exit(NULL);
}

void* deposit_func(void *arg)
{
	OperArg *r = (OperArg*)arg;

	double amount = deposit(r->a, r->amount);

	//将线程的id，操作者的名字，操作账户，操作金额以及最后向账户里存的钱打印出来
	printf("[Thread id:%lx Oper_name:%s Oper_account:%d]: [Oper_amount:%f get_amount:%f]\n",
				pthread_self(),r->name,r->a->account_number,r->amount,amount);

}

int main(void)
{
	int err = -1;
	pthread_t oper1,oper2;
	OperArg arg_oper1,arg_oper2;

	Account *a = create_account(633522411, 10000);

	//给两个操作者赋值，操作的都是同一个用户，而且操作的金额都是10000元，最后通过执行结果看他们能不能都取到10000元
	strcpy(arg_oper1.name, "operator1");
	arg_oper1.a = a;
	arg_oper1.amount = 10000;

	strcpy(arg_oper2.name, "operator2");
	arg_oper2.a = a;
	arg_oper2.amount = 10000;

	if((err = pthread_create(&oper1, NULL, withdrawal_func, (void*)&arg_oper1)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	if((err = pthread_create(&oper2, NULL, withdrawal_func, (void*)&arg_oper2)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	//由于没有设置线程为分离线程，所以需要使用pthread_join函数来回收子线程的资源
	pthread_join(oper1, NULL);
	pthread_join(oper2, NULL);
	
	//将账户的余额打印出来
	printf("the balance of account is %f\n",get_balance(a));

	destroy_account(a);

	return 0;
}
