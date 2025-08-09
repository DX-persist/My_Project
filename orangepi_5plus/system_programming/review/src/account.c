#include "header.h"
#include "account.h"

Account *create_account(int acc_num, double balance)
{
	Account *a = (Account*)malloc(sizeof(Account));	//在堆上开辟空间，防止函数退出造成栈空间被回收
	assert(a != NULL);		//使用断言判断a不为NULL，否则中止程序

	a->account_number = acc_num;
	a->balance = balance;
	pthread_mutex_init(&a->mutex, NULL);	//对互斥锁进行初始化

	return a;
}

double withdrawal(Account *a, double amount)
{
	assert(a != NULL);	

	pthread_mutex_lock(&a->mutex);		//对共享资源进行上锁
	if(amount <= 0 || amount > a->balance)	//若取款金额小于等于0或者大于账户金额就直接返回
	{
		pthread_mutex_unlock(&a->mutex);		//释放互斥锁
		return 0.0;
	}
	double balance = a->balance;	//获取账户余额
	sleep(1);
	balance -= amount;
	a->balance = balance;	//将操作过后的账户余额再赋值给账户余额

	pthread_mutex_unlock(&a->mutex);		//释放互斥锁

	return amount;		//返回操作的金额						
}

double deposit(Account *a, double amount)
{
	assert(a != NULL);

	pthread_mutex_lock(&a->mutex);		//对共享资源进行上锁
	if(amount <= 0)		//若存款金额小于等于0就直接返回0
	{
		pthread_mutex_unlock(&a->mutex);		//释放互斥锁
		return 0.0;
	}

	double balance = a->balance;
	sleep(1);
	balance += amount;
	a->balance = balance;		//将存款后的金额再赋值给账户
								
	pthread_mutex_unlock(&a->mutex);		//释放互斥锁

	return amount;
}

double get_balance(Account *a)
{
	assert(a != NULL);

	pthread_mutex_lock(&a->mutex);		//对共享资源进行上锁
	double balance = a->balance;
	pthread_mutex_unlock(&a->mutex);		//释放互斥锁

	return balance;		//将账户余额返回给调用者
}

void destroy_account(Account *a)
{
	assert(a != NULL);

	pthread_mutex_destroy(&a->mutex);		//销毁互斥锁，注意上下这两句话不能颠倒，要先释放互斥锁再释放空间
	free(a);	//将a释放以后a并不会立即变成空指针
	a = NULL;
}
