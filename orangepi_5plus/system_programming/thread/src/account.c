#include "account.h"
#include "header.h"

//创建账户
Account* create_account(int acc_num, double balance)
{
    Account *a = (Account*)malloc(sizeof(Account));
    assert(a != NULL);      //使用断言判断创建出来的账户不为空

    a->acc_num = acc_num;
    a->balance = balance;   

	pthread_rwlock_init(&a->rwlock, NULL);	//对读写锁进行初始化
		
    return a;
}

//取钱
double withdrawal(Account *a, double amount)
{
    assert(a != NULL);

	pthread_rwlock_wrlock(&a->rwlock);		//加写锁

    if(amount <= 0 || amount > a->balance)
    {
        //printf("输入错误，请重新输入\n");
        pthread_rwlock_unlock(&a->rwlock);		//释放读写锁
		return 0.0;
    }

    double balance = a->balance;
    sleep(1);
    balance -= amount;
    a->balance = balance;

	pthread_rwlock_unlock(&a->rwlock);		//释放读写锁

    return amount;
}

//存款
double deposit(Account *a, double amount)
{
    assert(a != NULL);

	pthread_rwlock_wrlock(&a->rwlock);		//加写锁
    if(amount <= 0)
    {
		pthread_rwlock_unlock(&a->rwlock);		//释放读写锁
        return 0.0;
    }
    double balance = a->balance;
    sleep(1);
    balance += amount;
    a->balance = balance;
	pthread_rwlock_unlock(&a->rwlock);		//释放读写锁

    return amount;
}

//查询余额
double get_balance(Account *a)
{
    assert(a != NULL);

    pthread_rwlock_rdlock(&a->rwlock);		//加读锁
	double balance = a->balance;
    pthread_rwlock_rdlock(&a->rwlock);
    return balance;
}

//销毁账户（释放堆空间）
void destroy_account(Account* a)
{
    assert(a != NULL);

    pthread_rwlock_destroy(&a->rwlock);       //销毁读写锁
    free(a);
    a = NULL;
}
