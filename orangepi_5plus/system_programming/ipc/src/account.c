#include "account.h"
#include "header.h"
#include "pv.h"

//取钱
double withdrawal(Account *a, double amount)
{
    assert(a != NULL);

    //作P操作，对信号量的值作减一操作
    //传入信号量集的id，信号量的编号，默认从0开始，对信号量操作的步长
    P(a->semid, 0, 1);

    if(amount <= 0 || amount > a->balance)
    {
        //若进程意外退出，作V操作，对信号量的值作加一操作，防止造成死锁
        V(a->semid, 0, 1);
		return 0.0;
    }

    double balance = a->balance;
    sleep(1);
    balance -= amount;
    a->balance = balance;

    //作V操作，对信号量的值作加一操作，使得另外一个进程能够拿到信号量并继续操作
    V(a->semid, 0, 1);

    return amount;
}

//存款
double deposit(Account *a, double amount)
{
    assert(a != NULL);

    //作P操作，对信号量的值作减一操作
    //传入信号量集的id，信号量的编号，默认从0开始，对信号量操作的步长
    P(a->semid, 0, 1);

    if(amount <= 0)
    {
        //若进程意外退出，作V操作，对信号量的值作加一操作，防止造成死锁
        V(a->semid, 0, 1);
        return 0.0;
    }
    double balance = a->balance;
    sleep(1);
    balance += amount;
    a->balance = balance;

    //作V操作，对信号量的值作加一操作，使得另外一个进程能够拿到信号量并继续操作
    V(a->semid, 0, 1);

    return amount;
}

//查询余额
double get_balance(Account *a)
{
    assert(a != NULL);

    //作P操作，对信号量的值作减一操作
    //传入信号量集的id，信号量的编号，默认从0开始，对信号量操作的步长
    P(a->semid, 0, 1);
	double balance = a->balance;
    //作V操作，对信号量的值作加一操作，使得另外一个进程能够拿到信号量并继续操作
    V(a->semid, 0, 1);

    return balance;
}
