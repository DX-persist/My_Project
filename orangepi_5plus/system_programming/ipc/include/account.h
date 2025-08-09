#ifndef _ACCOUNT_H
#define _ACCOUNT_H

typedef struct
{
    int acc_num;
    double balance;
	int semid;
}Account;

double withdrawal(Account *a, double amount);
double deposit(Account *a, double amount);
double get_balance(Account *a);

#endif
