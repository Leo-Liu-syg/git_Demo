//Project: project.prj
// Device: FT61F0AX
// Memory: Flash 10KX14b, EEPROM 128X8b, SRAM 1KX8b
// Author: 
//Company: 
//Version:
//   Date: 
//===========================================================
//===========================================================
#include "SYSCFG.h"
#include "FT61F0AX.h"
// 引入各模块头文件
#include "hw_gpio.h"
#include "hw_timer1.h"
#include "app_led.h"
#include "app_key.h"
//===========================================================
// 系统自带变量定义（不可删除和修改）
volatile char W_TMP  @ 0x70 ;
volatile char BSR_TMP  @ 0x71 ;
// 替换原有全局变量，使用模块中的全局变量（如需保留原有变量名，可做别名映射）
#define TIM1_Count     g_LED_TimerCount
#define Key_Mode       g_LED_CurrentMode
#define Intermediate   g_LED_Intermediate
#define Delay_20ms_Count g_Key_DelayCount
//===========================================================
void user_isr();//用户中断程序，不可删除
//===========================================================

//===========================================================
// 系统自带中断函数（不可删除和修改，仅修改用户逻辑部分）
void interrupt ISR(void)
{
#asm;
	NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
	NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
	NOP;NOP;NOP;NOP;
#endasm;
	user_isr(); //用户中断函数
}

// 用户中断函数（修改为调用模块接口，简化逻辑）
void user_isr()
{
    g_Key_DelayCount++; // 消抖计数器累计（中断中更新）
    
    if(T1UIE && T1UIF)
    {
        APP_LED_Update(); // 调用LED逻辑更新接口（替代原有分散的LED逻辑）
        HW_TIMER1_Clear_ITFlag(); // 调用定时器清除标志位接口
    }   
}
//===========================================================
// 系统自带电源初始化（不可修改，保留原有逻辑）
void POWER_INITIAL(void)
{
OSCCON=0B01100001;			//系统时钟选择为内部振荡器8MHz, 分频比为1:1

INTCON=0;					//禁止所有中断
   
   PORTA=0B00000000;
   PORTB=0B00000000;
   PORTC=0B00000000;
   
WPUA=0B00000000;			//弱上拉的开关，0-关，1-开		
WPUB=0B00000000;
WPUC=0B00000000;	

WPDA=0B00000000;			//弱下拉的开关，0-关，1-开
WPDB=0B00000000;
WPDC=0B00000000;

TRISA=0B00010000;			//PA输入输出，0-输出，1-输入 PA4-输入,PA5输出
TRISB=0B00000000;			//PB输入输出，0-输出，1-输入 
TRISC=0B00000000;

PSRC0=0B11111111;			//源电流设置最大
PSRC1=0B11111111;
PSRC2=0B00001111;

PSINK0=0B11111111;			//灌电流设置最大
PSINK1=0B11111111;
PSINK2=0B00000011;

ANSELA=0B00000000;			//设置对应的IO为数字IO	
}
//===========================================================
// 主函数：整合所有模块，简化逻辑
void main()
{
    // 1. 系统底层初始化（原有逻辑，不可修改）
    POWER_INITIAL();
    
    // 2. 硬件模块初始化（调用GPIO、定时器接口）
    HW_GPIO_Init();
    HW_TIMER1_Init();
    
    // 3. 应用层模块初始化（调用LED、按键接口）
    APP_LED_Init();
    APP_KEY_Init();
    
    // 4. 使能总中断和外设中断（原有逻辑，不可修改）
    INTCON=0B11000000;
    
    // 5. 主循环：仅调用按键处理接口，其他逻辑由模块自行维护
    while(1)
    {
		NOP();       
        APP_KEY_Process();

    }
}
//===========================================================