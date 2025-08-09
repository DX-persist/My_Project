#ifndef _PV_H
#define _PV_H

//创建信号量集，并对信号量集中的信号量进行赋值
extern int I(int nsems, int value);

//对信号量集编号为semid，信号量编号为semnum的信号量作P操作
extern void P(int semid, int semnum, int value);

//对信号量集编号为semid，信号量编号为semnum的信号量作V操作
extern void V(int semid, int semnum, int value);

//销毁编号为semid的信号量集
extern void D(int semid);

#endif
