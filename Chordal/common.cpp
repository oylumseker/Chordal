#include "common.h"

double GetTimeUsage(clock_t clockEnd, clock_t clockBegin)
{
	double diffticks = clockEnd - clockBegin;
	double diffms = (diffticks * 1000) / CLOCKS_PER_SEC;
	return diffms;
}

long long CalculateNChooseK(int n, int k)
{
	if (n < 2 * k)
		k = n - k; 
	long long int numerator = 1;
	for (int i = 0; i < k; ++i)
		numerator *= (n - i);
	return numerator / Factorial(k);

	//return Factorial(n) / (Factorial(n - k)*Factorial(k));
}

long long Factorial(int n)
{
	if (n == 0)
		return 1;
	else
		return n*Factorial(n - 1);
}