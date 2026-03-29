#include "SYSCFG.h"
#include "FT61F0AX.h"
#include "TDelay.h"
#include "TM1650_IIC_1.h"
#include "TM1650_IIC_2.h"
#include "IIC_SHT.h"

#define u8 unsigned char
#define u16 unsigned int

#define Key_1 PA4
#define Key_2 PA5
#define Buzz PB5
// 摄氏度/华氏度
#define S 0
#define H 1

extern volatile u8 MODE_SorH = 0;

// 外部全局标志（你程序里必须有）
extern u8 power_on_key2_flag; // 上电时检测到Key2按住（main里初始化一次）
// 工作状态
extern volatile u8 Calibration = 0;
extern volatile u8 standard_work = 0;
extern volatile u8 Work_Mode = 0;

// 报警关闭计时
volatile u16 count_10s = 0;
u8 count_10s_flag = 0;

u8 key_state_buffer = 0;
u8 key_state = 0;
u8 key_count = 0;
u8 lightness[] = {0x01, 0x51, 0x21}; // 辉度设置
volatile i = 0;

// 新增按键计时与状态
u16 key1_press_time = 0; // Key1按下时长（1ms单位）
u16 key2_press_time = 0; // Key2按下时长
u8 key1_short_done = 0;  // 短按已执行标志
u8 key1_long_done = 0;   // 长按已执行标志
u8 key2_long_done = 0;   // 长按已执行标志

/**********************************************************
 函数名：Key_Scan_NonBlock
 功能 ：非阻塞按键扫描
        Key1：短按=功能1，长按2s=功能2
        Key2：已标定时，按住上电=功能3
        报警时，长按Key2 2s=功能4
 **********************************************************/
void Key_Scan_NonBlock(void)
{
    //==================== 1. 读取按键原始状态 ====================
    if (Key_1 && Key_2)
    {
        key_state_buffer = 0; // 无按键
    }
    else if (!Key_1 && Key_2)
    {
        key_state_buffer = 1; // 仅Key1按下
    }
    else if (Key_1 && !Key_2)
    {
        key_state_buffer = 2; // 仅Key2按下
    }
    else
    {
        key_state_buffer = 3; // 两个都按下
    }

    //==================== 2. 消抖20ms ====================
    if (key_state != key_state_buffer)
    {
        key_count++;
        if (key_count >= 20) // 稳定20ms
        {
            key_count = 0;
            
            // 按键状态切换 → 重置计时
            key1_press_time = 0;
            key2_press_time = 0;
            key1_short_done = 0;
            key1_long_done = 0;
            key2_long_done = 0;
            key_state = key_state_buffer;
        }
    }
    else
    {
        key_count = 0; // 状态一致，清零计数
    }

    //==================== 3. Key1 短按 + 长按2秒 ====================
    if (key_state == 1) // Key1 按住
    {
        if (key1_press_time < 2000) // 计时最大2000ms
            key1_press_time++;

        // 短按：按下 <2秒 松开时触发
        if (key1_press_time < 2000 && !key1_short_done)
        {
            if (key_state_buffer != 1) // 松开了
            {
                if (i < 2)
                {
                    i++;
                }
                else
                {
                    i = 0;
                }
                TM1650_1_cfg_display(lightness[i]);
                TM1650_2_cfg_display(lightness[i]);
                key1_short_done = 1;
            }
        }

        // 长按：按住达到2秒
        if (key1_press_time >= 2000 && !key1_long_done)
        {
            if (MODE_SorH == S)
            {
                MODE_SorH = H;
            }
            else
            {
                MODE_SorH = S;
            }
            key1_long_done = 1; // 只执行一次
        }
    }

    //==================== 4. Key2 功能 ====================
    if (key_state == 2) // Key2 按住
    {
        if (key2_press_time < 2000)
            key2_press_time++;

        // 功能3：已标定 + 按住Key2上电
        if (Work_Mode == 1 && power_on_key2_flag)
        {
            Work_Mode = 0;
            Calibration = 0;
            power_on_key2_flag = 0; // 只执行一次
        }

        // 功能4：报警状态下，长按2秒
        if (standard_work == 3 && key2_press_time >= 2000 && !key2_long_done)
        {
            Buzz = 1; // 关闭蜂鸣器
            count_10s_flag = 1;
            key2_long_done = 1; // 只执行一次
        }
    }
    // 功能4
    if (count_10s_flag == 1)
    {
        count_10s++;
        if (count_10s >= 10000)
        {
            count_10s = 0;
            Buzz = 0;
            count_10s_flag = 0;
        }
    }
}