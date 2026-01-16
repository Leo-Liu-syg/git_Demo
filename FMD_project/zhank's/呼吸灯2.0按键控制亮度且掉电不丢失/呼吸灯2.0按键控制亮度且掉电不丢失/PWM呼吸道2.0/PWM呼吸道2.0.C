//********************************************************* 
/* 文件名：adjustable_breathing_led_fixed.c
* 功能：   FT64F0Ax可调亮度呼吸灯，带可靠的EEPROM保存
* IC:      FT64F0Ax  TSSOP20
* 说明：   两个按键调节LED亮度并可靠保存到EEPROM
*          串口PA6(RXD), PA7(TXD)已配置
*/ 
//*********************************************************

#include "FT64F0AX.h"

// 定义NOP宏
#define NOP() asm("nop")

// 定义按键引脚
#define KEY_INCREASE_PIN   5            // 增加亮度按键连接在PA5
#define KEY_DECREASE_PIN   4            // 减少亮度按键连接在PA4

// 按键消抖参数
#define KEY_DEBOUNCE_TIME  20       // 消抖时间20ms
#define SAVE_DELAY_TIME    500      // 保存延时500ms（缩短以便测试）

// EEPROM地址定义
#define EEPROM_ADDR_BRIGHTNESS  0x00  // 亮度保存地址

// 全局变量
volatile unsigned char ms_flag = 0;      // 1ms标志
volatile unsigned int ms_counter = 0;    // 毫秒计数器
volatile unsigned int save_timer = 0;    // 保存计时器
volatile unsigned char need_save = 0;    // 需要保存标志
unsigned char pwm_duty = 50;             // 当前PWM占空比 (0-100)，默认50%
unsigned char last_saved_duty = 0;       // 上次保存的亮度值

// 按键状态变量
struct {
    unsigned char current_state : 1;      // 当前状态（0=释放，1=按下）
    unsigned char last_state : 1;         // 上次状态
    unsigned char pressed_flag : 1;       // 按下标志（边沿检测）
    unsigned char released_flag : 1;      // 释放标志
    unsigned int debounce_counter;        // 消抖计数器
} key_increase = {0, 0, 0, 0, 0}, key_decrease = {0, 0, 0, 0, 0};

/*-------------------------------------------------
 * 函数名：EEPROM_WaitWriteComplete
 * 功能：  等待EEPROM写操作完成
 --------------------------------------------------*/
void EEPROM_WaitWriteComplete(void)
{
    // 等待WR位清零
    while(EECON1bits.WR)
    {
        // 可选：添加超时检测
        for(volatile int i=0; i<100; i++);  // 短暂延时
    }
}

/*-------------------------------------------------
 * 函数名：Reliable_EEPROM_Read
 * 功能：  可靠的EEPROM读取函数
 * 输入：  addr - EEPROM地址
 * 返回：  读取的数据
 --------------------------------------------------*/
unsigned char Reliable_EEPROM_Read(unsigned char addr)
{
    unsigned char data;
    
    // 等待任何正在进行的写操作
    EEPROM_WaitWriteComplete();
    
    // 设置地址
    EEADRL = addr;
    EEADRH = 0x00;
    
    // 配置控制寄存器
    EECON1bits.EEPGD = 0;  // 选择EEPROM数据存储器
    EECON1bits.CFGS = 0;   // 访问EEPROM
    
    // 启动读取
    EECON1bits.RD = 1;
    
    // 等待读取完成（根据数据手册需要2个NOP）
    NOP();
    NOP();
    
    // 读取数据
    data = EEDATL;
    
    return data;
}

/*-------------------------------------------------
 * 函数名：Reliable_EEPROM_Write
 * 功能：  可靠的EEPROM写入函数
 * 输入：  addr - EEPROM地址
 *         data - 要写入的数据
 * 返回：  1=成功，0=失败
 --------------------------------------------------*/
unsigned char Reliable_EEPROM_Write(unsigned char addr, unsigned char data)
{
    // 等待之前的写操作完成
    EEPROM_WaitWriteComplete();
    
    // 设置地址和数据
    EEADRL = addr;
    EEADRH = 0x00;
    EEDATL = data;
    
    // 配置控制寄存器
    EECON1 = 0x00;          // 清空控制寄存器
    EECON1bits.EEPGD = 0;   // 选择EEPROM数据存储器
    EECON1bits.CFGS = 0;    // 访问EEPROM
    EECON1bits.WREN = 1;    // 使能写操作
    
    // 执行关键写序列（必须严格按照此顺序）
    INTCONbits.GIE = 0;     // 禁止全局中断
    
    EECON2 = 0x55;          // 第一步：写入0x55
    EECON2 = 0xAA;          // 第二步：写入0xAA
    EECON1bits.WR = 1;      // 第三步：启动写操作
    
    INTCONbits.GIE = 1;     // 恢复全局中断
    
    // 等待写操作完成
    EEPROM_WaitWriteComplete();
    
    // 禁止写操作
    EECON1bits.WREN = 0;
    
    // 短暂的延时确保EEPROM稳定
    for(volatile int i=0; i<100; i++);
    
    // 验证写入的数据
    unsigned char readback = Reliable_EEPROM_Read(addr);
    
    if(readback == data)
    {
        return 1;  // 写入成功
    }
    else
    {
        return 0;  // 写入失败
    }
}

/*-------------------------------------------------
 * 函数名：UART_SendHex
 * 功能：  串口发送十六进制数
 * 输入：  value - 要发送的值
 --------------------------------------------------*/
void UART_SendHex(unsigned char value)
{
    unsigned char high_nibble = (value >> 4) & 0x0F;
    unsigned char low_nibble = value & 0x0F;
    
    // 等待发送缓冲区空
    while(!UR1LSRbits.UR1TXEF);
    
    // 发送高4位
    UR1DATAL = (high_nibble < 10) ? (high_nibble + '0') : (high_nibble - 10 + 'A');
    
    // 等待发送缓冲区空
    while(!UR1LSRbits.UR1TXEF);
    
    // 发送低4位
    UR1DATAL = (low_nibble < 10) ? (low_nibble + '0') : (low_nibble - 10 + 'A');
}

/*-------------------------------------------------
 * 函数名：UART_SendChar
 * 功能：  串口发送一个字符
 * 输入：  c - 要发送的字符
 --------------------------------------------------*/
void UART_SendChar(unsigned char c)
{
    // 等待发送缓冲区空
    while(!UR1LSRbits.UR1TXEF);
    
    // 发送字符
    UR1DATAL = c;
}

/*-------------------------------------------------
 * 函数名：UART_SendString
 * 功能：  串口发送字符串
 * 输入：  str - 要发送的字符串
 --------------------------------------------------*/
void UART_SendString(const char *str)
{
    while(*str)
    {
        UART_SendChar(*str++);
    }
}

/*-------------------------------------------------
 * 函数名：EEPROM_Test
 * 功能：  EEPROM功能测试
 --------------------------------------------------*/
void EEPROM_Test(void)
{
    unsigned char test_addr = 0x10;  // 使用测试地址，避免影响用户数据
    unsigned char test_data = 0xA5;  // 测试数据
    unsigned char readback;
    
    UART_SendString("=== EEPROM功能测试 ===\r\n");
    
    // 1. 写入测试数据
    UART_SendString("1. 写入测试数据 0xA5 到地址 0x");
    UART_SendHex(test_addr);
    UART_SendString("...");
    
    if(Reliable_EEPROM_Write(test_addr, test_data))
    {
        UART_SendString("成功\r\n");
    }
    else
    {
        UART_SendString("失败\r\n");
        return;
    }
    
    // 2. 读取验证
    UART_SendString("2. 读取验证...");
    readback = Reliable_EEPROM_Read(test_addr);
    
    if(readback == test_data)
    {
        UART_SendString("成功 (读取到0x");
        UART_SendHex(readback);
        UART_SendString(")\r\n");
    }
    else
    {
        UART_SendString("失败 (读取到0x");
        UART_SendHex(readback);
        UART_SendString(")\r\n");
    }
    
    // 3. 测试亮度存储地址
    UART_SendString("3. 检查亮度存储地址(0x00)...");
    readback = Reliable_EEPROM_Read(EEPROM_ADDR_BRIGHTNESS);
    UART_SendString("当前值: 0x");
    UART_SendHex(readback);
    UART_SendString(" (");
    UART_SendChar(readback/100 + '0');
    UART_SendChar((readback%100)/10 + '0');
    UART_SendChar(readback%10 + '0');
    UART_SendString("%)\r\n");
    
    UART_SendString("=== 测试完成 ===\r\n\r\n");
}

/*-------------------------------------------------
 * 函数名：TIM2_Init
 * 功能：  初始化TIM2定时器，1ms中断
 --------------------------------------------------*/
void TIM2_Init(void)
{
    // 使能TIM2时钟
    PCKENbits.TIM2EN = 1;
    
    // TIM2控制寄存器
    TIM2CR1 = 0x05;  // 00000101：使能计数器，允许更新
    
    // 只使能更新中断
    TIM2IERbits.T2UIE = 1;
    
    // 预分频器设置：16分频
    // 系统时钟16MHz / 16 = 1MHz
    TIM2PSCRbits.T2PSC = 0x0F;  // 0x0F = 15，实际分频=16
    
    // 自动重装载值：1ms中断
    // 1MHz / 1000Hz = 1000
    // ARR = 1000 - 1 = 999
    TIM2ARRH = 0x03;  // 高8位: 0x03E7 = 999
    TIM2ARRL = 0xE7;  // 低8位
    
    // 清除中断标志
    TIM2SR1bits.T2UIF = 0;
}

/*-------------------------------------------------
 * 函数名：TIM1_PWM_Init
 * 功能：  初始化TIM1 PWM，20kHz频率
 --------------------------------------------------*/
void TIM1_PWM_Init(void)
{
    // 使能TIM1时钟
    PCKENbits.TIM1EN = 1;
    
    // TIM1时钟配置：32MHz
    CKOCON = 0x20;
    TCKSRCbits.T1CKSRC = 0x03;  // HIRC的2倍频
    
    // Timer1控制
    TIM1CR1 = 0x85;         // 允许自动装载，使能计数器
    
    // 禁止所有中断
    TIM1IER = 0x00;
    
    // 通道1配置为PWM模式1
    TIM1CCMR1bits.T1CC1S = 0;       // 输出模式
    TIM1CCMR1bits.T1OC1M = 0x06;    // PWM模式1
    
    // 输出极性：高电平有效（LED亮）
    TIM1CCER1bits.T1CC1E = 1;       // 使能通道1输出
    TIM1CCER1bits.T1CC1P = 0;       // 高电平有效
    
    // 20kHz频率设置
    // ARR = 32MHz/20kHz - 1 = 1599
    TIM1ARRH = 0x06;        // 0x063F = 1599
    TIM1ARRL = 0x3F;
    
    // 初始占空比
    unsigned int ccr1 = 1600UL * pwm_duty / 100;
    TIM1CCR1H = (ccr1 >> 8) & 0xFF;
    TIM1CCR1L = ccr1 & 0xFF;
    
    // 预分频器不分频
    TIM1PSCRH = 0x00;
    TIM1PSCRL = 0x00;
    
    // 重复计数器
    TIM1RCR = 0x0F;
    
    // 使能输出
    TIM1BKRbits.T1MOE = 1;
    
    // 无死区时间
    TIM1DTR = 0x00;
}

/*-------------------------------------------------
 * 函数名：UART_Init
 * 功能：  初始化串口
 * 说明：  PA6(RXD), PA7(TXD)，波特率9600
 --------------------------------------------------*/
void UART_Init(void)
{
    // 使能UART时钟
    PCKENbits.UARTEN = 1;
    
    // 配置PA6为RXD输入，PA7为TXD输出
    TRISAbits.TRISA6 = 1;  // PA6输入(RXD)
    TRISAbits.TRISA7 = 0;  // PA7输出(TXD)
    
    // 设置波特率9600 @ 16MHz
    // 计算：16MHz / (16 * 9600) = 104.166
    UR1DLL = 104;          // 分频值低字节
    UR1DLH = 0;            // 分频值高字节
    
    // UART控制寄存器配置
    UR1LCRbits.UR1STOP = 0;     // 1位停止位
    UR1LCRbits.UR1PEN = 0;      // 无奇偶校验
    UR1LCRbits.UR1EVEN = 0;     // 偶校验禁用
    
    // UART模式控制
    UR1MCRbits.UR1RXEN = 1;     // 使能接收
    UR1MCRbits.UR1TXEN = 1;     // 使能发送
    
    // 清空状态
    UR1LSR = 0x00;
}

/*-------------------------------------------------
 * 函数名：GPIO_Init
 * 功能：  GPIO初始化
 --------------------------------------------------*/
void GPIO_Init(void)
{
    // 端口初始化
    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;
    
    // PA0输出（LED）
    TRISAbits.TRISA0 = 0;  // 设置为输出
    
    // PA4输入（减少亮度按键），启用上拉电阻
    TRISAbits.TRISA4 = 1;   // 设置为输入
    
    // PA5输入（增加亮度按键），启用上拉电阻
    TRISAbits.TRISA5 = 1;   // 设置为输入
    
    // 关闭模拟功能，设为数字IO
    ANSELA = 0x00;          // 关闭所有PORTA模拟功能
    
    // 启用按键上拉电阻
    WPUA = 0x00;
    WPUA |= (1 << KEY_INCREASE_PIN);      // 启用PA5上拉电阻
    WPUA |= (1 << KEY_DECREASE_PIN);    // 启用PA4上拉电阻
    
    // 关闭其他弱上拉/下拉
    WPUB = 0x00;
    WPUC = 0x00;
    WPDA = 0x00;
    WPDB = 0x00;
    WPDC = 0x00;
}

/*-------------------------------------------------
 * 函数名：System_Init
 * 功能：  系统初始化
 --------------------------------------------------*/
void System_Init(void)
{
    // 系统时钟16MHz
    OSCCON = 0x71;
    
    // 禁止所有中断
    INTCON = 0x00;
}

/*-------------------------------------------------
 * 函数名：Set_PWM_Duty
 * 功能：  设置PWM占空比
 * 输入：  duty - 占空比 (0-100)
 --------------------------------------------------*/
void Set_PWM_Duty(unsigned char duty)
{
    unsigned int ccr1;
    
    // 限制占空比范围
    if (duty > 100) duty = 100;
    if (duty < 0) duty = 0;
    
    // 更新当前占空比
    pwm_duty = duty;
    
    // 计算CCR1值
    // ARR+1 = 1600, CCR1 = 1600 * duty / 100
    ccr1 = 1600UL * duty / 100;
    
    // 写入比较寄存器
    TIM1CCR1H = (ccr1 >> 8) & 0xFF;
    TIM1CCR1L = ccr1 & 0xFF;
    
    // 通过串口输出当前亮度（调试用）
    UART_SendString("亮度: ");
    if (pwm_duty < 10) UART_SendChar('0');
    if (pwm_duty < 100) UART_SendChar('0');
    UART_SendChar(pwm_duty / 100 + '0');
    UART_SendChar((pwm_duty % 100) / 10 + '0');
    UART_SendChar(pwm_duty % 10 + '0');
    UART_SendString("% (0x");
    UART_SendHex(pwm_duty);
    UART_SendString(")\r\n");
}

/*-------------------------------------------------
 * 函数名：Read_Key
 * 功能：  读取按键状态
 * 输入：  pin - 按键引脚
 * 返回：  1=按键按下，0=按键释放
 --------------------------------------------------*/
unsigned char Read_Key(unsigned char pin)
{
    // 读取指定引脚状态
    if (pin == KEY_INCREASE_PIN)
        return (PORTAbits.PA5 == 0);  // PA5为0表示按下
    else if (pin == KEY_DECREASE_PIN)
        return (PORTAbits.PA4 == 0);  // PA4为0表示按下
    
    return 0;
}

/*-------------------------------------------------
 * 函数名：Key_Scan
 * 功能：  按键扫描函数（1ms调用一次）
 * 说明：  实现消抖和边沿检测
 --------------------------------------------------*/
void Key_Scan(void)
{
    unsigned char key_now;
    
    // 扫描增加亮度按键（PA5）
    key_now = Read_Key(KEY_INCREASE_PIN);
    
    // 保存上次状态
    key_increase.last_state = key_increase.current_state;
    
    // 状态变化检测
    if (key_now != key_increase.current_state)
    {
        // 状态变化，开始消抖计数
        key_increase.debounce_counter++;
        if (key_increase.debounce_counter >= KEY_DEBOUNCE_TIME)
        {
            // 消抖完成，更新状态
            key_increase.current_state = key_now;
            key_increase.debounce_counter = 0;
            
            // 检测按键按下边沿
            if ((key_increase.last_state == 0) && (key_increase.current_state == 1))
            {
                key_increase.pressed_flag = 1;
            }
            
            // 检测按键释放边沿
            if ((key_increase.last_state == 1) && (key_increase.current_state == 0))
            {
                key_increase.released_flag = 1;
            }
        }
    }
    else
    {
        // 状态未变化，重置消抖计数器
        key_increase.debounce_counter = 0;
    }
    
    // 扫描减少亮度按键（PA4）
    key_now = Read_Key(KEY_DECREASE_PIN);
    
    // 保存上次状态
    key_decrease.last_state = key_decrease.current_state;
    
    // 状态变化检测
    if (key_now != key_decrease.current_state)
    {
        // 状态变化，开始消抖计数
        key_decrease.debounce_counter++;
        if (key_decrease.debounce_counter >= KEY_DEBOUNCE_TIME)
        {
            // 消抖完成，更新状态
            key_decrease.current_state = key_now;
            key_decrease.debounce_counter = 0;
            
            // 检测按键按下边沿
            if ((key_decrease.last_state == 0) && (key_decrease.current_state == 1))
            {
                key_decrease.pressed_flag = 1;
            }
            
            // 检测按键释放边沿
            if ((key_decrease.last_state == 1) && (key_decrease.current_state == 0))
            {
                key_decrease.released_flag = 1;
            }
        }
    }
    else
    {
        // 状态未变化，重置消抖计数器
        key_decrease.debounce_counter = 0;
    }
}

/*-------------------------------------------------
 * 函数名：Key_Process
 * 功能：  按键处理函数
 * 说明：  处理按键事件，调整亮度
 --------------------------------------------------*/
void Key_Process(void)
{
    // 处理增加亮度按键（PA5）
    if (key_increase.pressed_flag)
    {
        key_increase.pressed_flag = 0;  // 清除标志
        
        // 增加亮度（步长5%）
        if (pwm_duty < 100)
        {
            pwm_duty += 5;
            if (pwm_duty > 100) pwm_duty = 100;
            Set_PWM_Duty(pwm_duty);
            
            // 设置需要保存标志
            need_save = 1;
            save_timer = 0;  // 重置保存计时器
            
            UART_SendString("按键: PA5 (+5%)\r\n");
        }
    }
    
    if (key_increase.released_flag)
    {
        key_increase.released_flag = 0;  // 清除标志
    }
    
    // 处理减少亮度按键（PA4）
    if (key_decrease.pressed_flag)
    {
        key_decrease.pressed_flag = 0;  // 清除标志
        
        // 减少亮度（步长5%）
        if (pwm_duty > 0)
        {
            if (pwm_duty >= 5)
                pwm_duty -= 5;
            else
                pwm_duty = 0;
                
            Set_PWM_Duty(pwm_duty);
            
            // 设置需要保存标志
            need_save = 1;
            save_timer = 0;  // 重置保存计时器
            
            UART_SendString("按键: PA4 (-5%)\r\n");
        }
    }
    
    if (key_decrease.released_flag)
    {
        key_decrease.released_flag = 0;  // 清除标志
    }
}

/*-------------------------------------------------
 * 函数名：Save_Process
 * 功能：  保存处理函数
 * 说明：  检查是否需要保存亮度到EEPROM
 --------------------------------------------------*/
void Save_Process(void)
{
    if (need_save)
    {
        save_timer++;
        
        // 按键释放后500ms保存
        if (save_timer >= SAVE_DELAY_TIME)
        {
            UART_SendString("保存倒计时结束，检查是否需要保存...\r\n");
            
            // 检查亮度值是否变化
            if (pwm_duty != last_saved_duty)
            {
                UART_SendString("亮度变化，正在保存到EEPROM地址0x00...");
                
                // 使用可靠的EEPROM写入函数
                if (Reliable_EEPROM_Write(EEPROM_ADDR_BRIGHTNESS, pwm_duty))
                {
                    last_saved_duty = pwm_duty;
                    UART_SendString("成功！\r\n");
                    
                    // 立即读取验证
                    unsigned char verify = Reliable_EEPROM_Read(EEPROM_ADDR_BRIGHTNESS);
                    if (verify == pwm_duty)
                    {
                        UART_SendString("  验证通过 (EEPROM值: 0x");
                        UART_SendHex(verify);
                        UART_SendString(")\r\n");
                    }
                    else
                    {
                        UART_SendString("  验证失败！读取到: 0x");
                        UART_SendHex(verify);
                        UART_SendString("\r\n");
                    }
                }
                else
                {
                    UART_SendString("失败！\r\n");
                }
            }
            else
            {
                UART_SendString("亮度未变化，跳过保存\r\n");
            }
            
            // 清除标志
            need_save = 0;
            save_timer = 0;
        }
        else if (save_timer % 100 == 0)  // 每100ms显示倒计时
        {
            UART_SendString("保存倒计时: ");
            UART_SendChar((SAVE_DELAY_TIME - save_timer)/100 + '0');
            UART_SendString(".");
            UART_SendChar(((SAVE_DELAY_TIME - save_timer)%100)/10 + '0');
            UART_SendString("s\r\n");
        }
    }
}

/*-------------------------------------------------
 * 函数名：interrupt isr
 * 功能：  中断服务函数（TIM2中断）
 --------------------------------------------------*/
void interrupt TIM2_ISR(void)
{
    // 检查TIM2更新中断
    if (TIM2SR1bits.T2UIF)
    {
        // 清除中断标志
        TIM2SR1bits.T2UIF = 0;
        
        // 设置1ms标志
        ms_flag = 1;
        
        // 毫秒计数器加1
        ms_counter++;
        
        // 按键扫描（每1ms调用一次）
        Key_Scan();
    }
}

/*-------------------------------------------------
 * 函数名：main
 * 功能：  主函数
 --------------------------------------------------*/
void main(void)
{
    unsigned char saved_brightness;
    
    // 1. 系统初始化
    System_Init();
    
    // 2. GPIO初始化
    GPIO_Init();
    
    // 3. 串口初始化
    UART_Init();
    
    // 4. 输出启动信息
    UART_SendString("\r\n===================================\r\n");
    UART_SendString("    LED亮度控制系统 V2.1\r\n");
    UART_SendString("    带可靠EEPROM存储\r\n");
    UART_SendString("===================================\r\n");
    
    // 5. EEPROM功能测试
    EEPROM_Test();
    
    // 6. 从EEPROM读取保存的亮度
    UART_SendString("正在从EEPROM读取保存的亮度...\r\n");
    saved_brightness = Reliable_EEPROM_Read(EEPROM_ADDR_BRIGHTNESS);
    
    UART_SendString("EEPROM读取结果: 0x");
    UART_SendHex(saved_brightness);
    UART_SendString(" (");
    UART_SendChar(saved_brightness/100 + '0');
    UART_SendChar((saved_brightness%100)/10 + '0');
    UART_SendChar(saved_brightness%10 + '0');
    UART_SendString("%)\r\n");
    
    // 检查读取的值是否有效（0-100）
    if (saved_brightness <= 100)
    {
        pwm_duty = saved_brightness;
        last_saved_duty = saved_brightness;
        UART_SendString("使用EEPROM保存的亮度\r\n");
    }
    else
    {
        // 无效值，使用默认值50%
        UART_SendString("EEPROM值无效，使用默认值50%\r\n");
        pwm_duty = 50;
        last_saved_duty = 50;
        
        // 尝试写入默认值
        UART_SendString("正在写入默认值到EEPROM...");
        if (Reliable_EEPROM_Write(EEPROM_ADDR_BRIGHTNESS, pwm_duty))
        {
            UART_SendString("成功\r\n");
        }
        else
        {
            UART_SendString("失败\r\n");
        }
    }
    
    // 7. TIM1 PWM初始化（在设置亮度之后）
    TIM1_PWM_Init();
    
    // 8. TIM2定时器初始化（1ms中断）
    TIM2_Init();
    
    // 9. 设置初始亮度显示
    UART_SendString("初始亮度: ");
    if (pwm_duty < 10) UART_SendChar('0');
    if (pwm_duty < 100) UART_SendChar('0');
    UART_SendChar(pwm_duty / 100 + '0');
    UART_SendChar((pwm_duty % 100) / 10 + '0');
    UART_SendChar(pwm_duty % 10 + '0');
    UART_SendString("%\r\n");
    
    UART_SendString("按键说明:\r\n");
    UART_SendString("  PA5 (引脚6): 增加亮度 +5%\r\n");
    UART_SendString("  PA4 (引脚5): 减少亮度 -5%\r\n");
    UART_SendString("亮度将在按键松开后0.5秒保存到EEPROM\r\n");
    UART_SendString("===================================\r\n\r\n");
    
    // 10. 使能全局中断和外设中断
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    
    // 11. 使能PWM输出
    TIM1BKRbits.T1MOE = 1;
    
    // 12. 主循环
    while(1)
    {
        // 检查1ms标志
        if (ms_flag)
        {
            ms_flag = 0;  // 清除标志
            
            // 按键处理
            Key_Process();
            
            // 保存处理
            Save_Process();
        }
        
        // 空操作，降低CPU占用
        NOP();
    }
}