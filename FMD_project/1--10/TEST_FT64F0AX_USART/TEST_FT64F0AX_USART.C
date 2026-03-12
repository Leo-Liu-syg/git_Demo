// Project: TEST_FT64F0AX_USART.prj
// Device:  FT64F0AX
// Memory:  PROM=10Kx14, SRAM=1KB, EEPROM=128                             
// Description: 串口上电发送10个字符，然后等待接收10个字节数据（通过串口助手发送接收）
//
// RELEASE HISTORY
// VERSION DATE     DESCRIPTION
// 1.6        25-6-5        修改系统时钟为8MHz，使能LVR
//                FT64F0A5  TSSOP20
//              -------------------
// NC----------|1(PA5)   	(PA4)20|-----------NC     
// TXIO--------|2(PA6)   	(PA3)19|-----------NC 
// RXIO--------|3(PA7)   	(PA2)18|-----------NC
// NC----------|4(PC0)   	(PA1)17|-----------NC
// NC----------|5(PC1)		(PA0)16|-----------NC	
// NC----------|6(PB7)		(PB0)15|-----------NC
// GND---------|7(GND)		(PB1)14|-----------NC
// NC----------|8(PB6)		(PB2)13|-----------NC
// VDD---------|9(VDD)		(PB3)12|-----------NC
// NC----------|10(PB5)		(PB4)11|-----------NC
//				-------------------
// 
//*********************************************************
#include	"SYSCFG.h";
#include 	"FT64F0AX.h";
//***********************宏定义****************************
#define		uchar		unsigned char

//Variable definition
volatile char W_TMP  @ 0x70 ;//系统占用不可以删除和修改
volatile char BSR_TMP  @ 0x71 ;//系统占用不可以删除和修改
volatile	uchar		receivedata[10]=0;
volatile	uchar		senddata=0;

volatile	uchar		toSend[11]={0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x0d,0x0a};
uchar		i=0;
uchar		mmm=0;

void user_isr();//用户中断程序，不可删除
//===========================================================
//Funtion name：interrupt ISR
//parameters：无
//returned value：无
//===========================================================
void interrupt ISR(void)
{
#asm;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
#endasm;//系统设置不可以删除和修改
	user_isr(); //调用用户中断函数
}
void user_isr() //调用用户中断函数
{
    //中断处理程序
    if(UR1RXNE&&UR1RXNEF)				//接收中断
    {
        receivedata[mmm++] =UR1DATAL;
        
        if(mmm>=10)
        {
            mmm=0;
        }
        NOP();
	}
    
	if(UR1TCEN&&UR1TCF)					//发送中断
    {
        UR1TCF=1;
        
        if(i<10)
        {
            UR1DATAL =toSend[i++];
		}
        else
        {
            i=0;
		}
        NOP();
    }
}
/*-------------------------------------------------
 * 函数名：POWER_INITIAL
 * 功能：  上电系统初始化
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
 void POWER_INITIAL(void)
 {
	OSCCON=0B01100001;			//系统时钟选择为内部振荡器8MHz,分频比为1:1
    
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
	
	TRISA=0B10000000;			//输入输出设置，0-输出，1-输入
	TRISB=0B00000000;	
	TRISC=0B00000000;

	PSRC0=0B11111111;			//源电流设置最大
	PSRC1=0B11111111;
	PSRC2=0B00001111;

	PSINK0=0B11111111;			//灌电流设置最大
	PSINK1=0B11111111;
	PSINK2=0B00000011;

	ANSELA=0B00000000;			//设置对应的IO为数字IO	
 }
/*-------------------------------------------------
 * 函数名：Delay10Us
 * 功能：  短延时函数
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void Delay5_6Us(unsigned char times)//一个函数本身就要5us
{
  for(unsigned char i=0;i<times;i++)
  {
	NOP();
    NOP();
    NOP();//一个就是0.5us
    NOP();
  }

}    
/*-------------------------------------------------
 * 函数名：DelayMs
 * 功能：  短延时函数
 * 输入：  Time延时时间长度 延时时长Time ms
 * 输出：  无
 --------------------------------------------------*/
 void DelayMs(uchar Time)
 {
	for(uchar a=0;a<Time;a++)
	{
		for(uchar b=0;b<96;b++)
		{
		 	Delay5_6Us(1); 	
		}
	}
 }
 /*-------------------------------------------------
 * 函数名：UART_INITIAL
 * 功能：  初始化串口
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
 void UART_INITIAL(void)
 {
    PCKEN|=0B00100000;			//使能UART1模块时钟
    UR1IER=0B00100001;			//使能发送完成中断，使能接收数据中断
    UR1LCR=0B00000001;			//8位数据长度，1位停止位，无奇偶校验位
    UR1MCR=0B00011000;			//使能发送和接收接口
       
    UR1DLL=52;					//波特率=Fmaster/(16*{URxDLH,URxDLL})=9600
    UR1DLH=0;  
    UR1TCF=1;
    INTCON=0B11000000;
 }
/*-------------------------------------------------
 * 函数名：main
 * 功能：  主函数 
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void main(void)
{
    POWER_INITIAL();			//系统初始化
    UART_INITIAL();
    DelayMs(100);
    if(UR1TXEF)					//上电发送10+1个数据
    {
        UR1DATAL=0X41;
    }
    
    while(1)
    {
		PB3=~PB3;	
        Delay5_6Us(150);
    }
}