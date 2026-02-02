#include "SYSCFG.h"
#include "FT64F0AX.h"

#define EC11_A PA7
#define EC11_B PA6
#define uint unsigned int
#define ulong unsigned long

int Number_Sum = 0;

// 编码器相关变量
unsigned char EC11_State_Old = 0;	 // 编码器上一次状态（A/B相组合）
unsigned char EC11_Debounce_Cnt = 0; // 编码器防抖计数

// Timer1
unsigned int Debounce_count = 0;
unsigned char flag_1ms = 0;

/*-------------------------------------------------
 * 函数名：EC11_Read_State
 * 功能： 读取EC11编码器当前状态（A/B相组合）
 * 返回： 0x00(A=0,B=0) 0x01(A=0,B=1) 0x02(A=1,B=0) 0x03(A=1,B=1)
 --------------------------------------------------*/
unsigned char EC11_Read_State(void)
{
	unsigned char state = 0;
	if (EC11_B)
		state |= 0x02; // EC11_B=A相
	if (EC11_A)
		state |= 0x01; // EC11_A=B相
	return state;
}

/*-------------------------------------------------
 * 函数名：EC11_Process
 * 功能： 处理EC11编码器旋转逻辑（防抖+加减判断）
 --------------------------------------------------*/
void EC11_Process(void)
{
	unsigned char state_new = EC11_Read_State();

	// 步骤1：防抖——连续5ms状态不变才认为是稳定态
	if (state_new == EC11_State_Old)
	{
		if (EC11_Debounce_Cnt < 2) // 防抖时间3ms
		{
			EC11_Debounce_Cnt++;
		}
	}
	else
	{
		EC11_Debounce_Cnt = 0;
		EC11_State_Old = state_new;
		return;
	}

	// 步骤2：稳定态校验通过，判断旋转方向
	if (EC11_Debounce_Cnt == 2)
	{
		// EC11正转（A相超前B相）：0x03→0x02→0x00→0x01→0x03
		// EC11反转（B相超前A相）：0x03→0x01→0x00→0x02→0x03
		static unsigned char state_last = 0x03; // 初始状态
		if (state_new != state_last)
		{
			// 正转判断（顺时针，数值+1）
			if ((state_last == 0x03 && state_new == 0x02) ||
				(state_last == 0x02 && state_new == 0x00) ||
				(state_last == 0x00 && state_new == 0x01) ||
				(state_last == 0x01 && state_new == 0x03))
			{
				Number_Sum = (Number_Sum >= 99) ? 99 : (Number_Sum + 1);
			}
			// 反转判断（逆时针，数值-1）
			else if ((state_last == 0x03 && state_new == 0x01) ||
					 (state_last == 0x01 && state_new == 0x00) ||
					 (state_last == 0x00 && state_new == 0x02) ||
					 (state_last == 0x02 && state_new == 0x03))
			{
				Number_Sum = (Number_Sum <= -99) ? -99 : (Number_Sum - 1);
			}
			state_last = state_new; // 更新上一次状态
		}
		EC11_Debounce_Cnt++; // 避免重复触发
	}
}

