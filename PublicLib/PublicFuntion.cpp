
#include "PublicFunction.h"

//求最大公约数
int gcd(int a, int b)
{
	while (b ^= a ^= b ^= a %= b);
	return a;
}

//求a、b最小公倍数
int lcm(int a, int b)
{
	return a*b / gcd(a, b);
}