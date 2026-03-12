#include "SYSCFG.h"
#include "FT64F0AX.h"
#include "ir_nec.h"

volatile u8 ir_parse_ok = 0;
volatile u8 ir_key_code = 0;

volatile unsigned char ir_key_value = 0; // 保存按键值
volatile unsigned char ir_key_ready = 0; // 新键到达标志

static volatile u8 ir_state = 0;
static volatile u16 ir_t100us_count = 0;
static volatile u32 ir_data = 0;
static volatile u8 ir_bit_cnt = 0;

void TIM1_Init(void) // 100us
{
    PCKEN |= 0B00000010;
    CKOCON = 0B00100000;
    TCKSRC = 0B00000011;

    TIM1CR1 = 0B10000101;

    TIM1IER = 0B00000001;

    TIM1ARRH = 0x0C; // �Զ�װ�ظ�8λH
    TIM1ARRL = 0x7E; // �Զ�װ�ص�8λH

    INTCON = 0B11000000;
}

//////////////////////////////////////////////////////
// 外部中断 PA0
//////////////////////////////////////////////////////
void EXTI_Init(void)
{
    TRISA |= 0x01;   // PA0设为输入
    ANSELA &= ~0x01; // 禁用模拟功能，启用数字输入
    WPUA |= 0x01;    // 开启弱上拉，稳定空闲高电平

}

// 红外NEC初始化（修正：补充外设中断使能）
//////////////////////////////////////////////////////
void IR_NEC_Init(void)
{
    TIM1_Init();
    EXTI_Init();

    INTCON |= 0xC0; // 开启全局中断（GIE=1）+ 外设中断（PEIE=1）
    ir_state = 0;
    ir_t100us_count = 0;
    ir_data = 0;
    ir_bit_cnt = 0;
    ir_key_ready = 0;
    ir_key_value = 0;
}
//////////////////////////////////////////////////////

void IR_Edge_Process(void)
{

    switch (ir_state)
    {
    case 0: // 等待低电平
        if (PA0 == 0)
        {
            ir_t100us_count = 0;
            ir_state = 1;
        }
        break;
    case 1: // 等待9ms低电平
        if (ir_t100us_count > 82 && ir_t100us_count < 95)
        {
            if (PA0 == 0)
            {
                ir_state = 2;
                ir_t100us_count = 0; // 下一步检测引导码高电平（4.5ms）
            }
            else
            {
                ir_state = 0;
                ir_t100us_count = 0;
            }
        }

        break;

    case 2: // 等待4.5ms高电平
        if (PA0 == 1)
        {
            if (ir_t100us_count > 41 && ir_t100us_count < 50)
            {
                if (PA0 == 1)
                {
                    ir_state = 3;
                    ir_bit_cnt = 0;
                    ir_data = 0;
                    ir_t100us_count = 0;
                }
                else
                {
                    // 重复码高电平：2250us ±500us（可选，长按重复）
                    ir_key_ready = 1;
                    ir_state = 0;
                    ir_t100us_count = 0;
                }
            }
        }

        break;

    case 3:
        if (PA ==1)
        {}

            // 状态3：解析数据位（高电平时间区分0/1）
            break;
        case 4:
            // 逻辑0：高电平560us ±200us
            if (ir_t100us_count > 36 && ir_t100us_count < 75)
            {
                ir_data &= ~(1UL << ir_bit_cnt); // 该位设为0（默认0，可省略）
            }
            // 逻辑1：高电平1680us ±200us
            else if (ir_t100us_count > 15 && ir_t100us_count < 19)
            {
                ir_data |= (1UL << ir_bit_cnt); // 该位设为1
            }
            else
            {
                ir_state = 0; // 时间不符，复位
                break;
            }

        // 数据位计数+1
        ir_bit_cnt++;

        // 检查是否解析完32位（地址+地址反码+命令+命令反码）
        if (ir_bit_cnt >= 32)
        {
            // 验证命令和命令反码（确保数据有效）
            u8 cmd = (ir_data >> 16) & 0xFF;     // 提取命令码（键值）
            u8 cmd_inv = (ir_data >> 24) & 0xFF; // 提取命令反码

            if (cmd == (unsigned char)(~cmd_inv)) // 反码验证通过
            {
                ir_key_value = cmd; // 保存有效键值
                ir_key_ready = 1;   // 置位就绪标志
            }

            ir_state = 0; // 解析完成，回到空闲状态
            break;
        }

        // 解析完1个位，回到状态2，准备解析下一个位
        ir_state = 2;
        // 复位计时，准备下一个位的低电平计时
        ir_t100us_count = 0;
        TIM1CNTRH = 0x00;
        TIM1CNTRL = 0x00;
        break;

    default:
        ir_state = 0;
    }
}

//////////////////////////////////////////////////////

void IR_NEC_Parse(void)
{
    // 主循环调用即可
}