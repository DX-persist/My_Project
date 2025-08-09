#ifndef _ACCOUNT_H
#define _ACCOUNT_H

#include <pthread.h>

typedef struct
{
    int acc_num;
    double balance;
	pthread_rwlock_t rwlock;
}Account;

Account* create_account(int acc_num, double balance);
double withdrawal(Account *a, double amount);
double deposit(Account *a, double amount);
double get_balance(Account *a);
void destroy_account(Account* a);

#endif
