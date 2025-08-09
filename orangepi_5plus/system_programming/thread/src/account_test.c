#include "header.h"
#include "account.h"

typedef struct
{
    char name[32];
    Account *a;
    double balance;
}OperArg;

void* withdrawal_func(void *arg)
{
    OperArg *r = (OperArg*)arg;

    double amount = withdrawal(r->a, r->balance);
    printf("[thread id:%lx %s] get %lf\n",pthread_self(),r->name,amount);

    pthread_exit(NULL);
}

void* deposit_func(void *arg)
{
    OperArg *r = (OperArg*)arg;

    double amount = deposit(r->a, r->balance);
    printf("[thread id:%lx %s] get %lf\n",pthread_self(),r->name,amount);

    pthread_exit(NULL);
}

int main(void)
{
    int err = -1;
    pthread_t boy,girl;
    OperArg arg_boy,arg_girl;
    memset((void*)&arg_boy,'\0',sizeof(arg_boy));
    memset((void*)&arg_girl,'\0',sizeof(arg_girl));
    Account *a = create_account(10086,10000);

    strcpy(arg_boy.name,"boy");
    arg_boy.a = a;
    arg_boy.balance = 10000;

    strcpy(arg_girl.name,"girl");
    arg_girl.a = a;
    arg_girl.balance = 10000;

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

    pthread_join(boy, NULL);
    pthread_join(girl, NULL);

    printf("amount:%lf\n",get_balance(a));

    destroy_account(a);

    return 0;
}