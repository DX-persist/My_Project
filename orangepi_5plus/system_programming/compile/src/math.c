int Add_Function(int num1, int num2)
{
	return num1 + num2;
}

int Sub_Function(int num1, int num2)
{
	return num1 - num2;
}

int Mul_Function(int num1, int num2)
{
	return num1 * num2;
}

float Div_Function(int num1, int num2)
{
	if(num2 == 0)
	{
		return -1;
	}

	return (float)num1 / num2;
}

