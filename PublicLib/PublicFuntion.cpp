
#include "PublicFunction.h"

//�����Լ��
int gcd(int a, int b)
{
	while (b ^= a ^= b ^= a %= b);
	return a;
}

//��a��b��С������
int lcm(int a, int b)
{
	return a*b / gcd(a, b);
}