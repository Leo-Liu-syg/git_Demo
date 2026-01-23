#include "SYSCFG.h"
#include "FT64F0AX.h"

// us_Dealy-1:3.622
void TDelay_us(unsigned int Rt_TM1650) // 这个差不多是输入1，输出3.622的样子
{
	while (Rt_TM1650--)
		;
}
// ms 比较精确1：1
void TDelay_ms(unsigned char Time)
{
	unsigned char a, b;
	for (a = 0; a < Time; a++)
	{
		for (b = 0; b < 5; b++)
		{
			TDelay_us(545);
		}
	}
}
// 精确
void TDelay_s(unsigned char Time)
{
	unsigned char a, b;
	for (a = 0; a < Time; a++)
	{
		for (b = 0; b < 10; b++)
		{
			TDelay_ms(100);
		}
	}
}

