#ifndef _ACCOUNT_H
#define _ACCOUNT_H

typedef struct
{
	int account_number;
	double balance;
	pthread_mutex_t mutex;		//定义互斥锁类型
}Account;

extern Account *create_account(int acc_num, double balance);
extern double withdrawal(Account *a, double amount);
extern double deposit(Account *a, double amount);
extern double get_balance(Account *a);
extern void destroy_account(Account *a);

#endif
