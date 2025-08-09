#include "mymath.h"
#include "header.h"

int main()
{
	int data1,data2;


	puts("please enter two integer numbers");
	scanf("%d%d",&data1,&data2);		

	printf("sum: %d\n",Add_Function(data1,data2));
	printf("sub: %d\n",Sub_Function(data1,data2));
	printf("mul: %d\n",Mul_Function(data1,data2));
	printf("div: %.1f\n",Div_Function(data1,data2));

	return 0;
}
