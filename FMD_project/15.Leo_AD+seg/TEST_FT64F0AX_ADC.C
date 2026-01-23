// Project: TEST_FT64F0AX_iotrigger.prj
// Device:  FT64F0AX
// Memory:  PROM=10Kx14, SRAM=1KB, EEPROM=128                             
// Description: ADC_ETR�����������AN0�ڱ�������ADֵ���������ѹ
//
// RELEASE HISTORY
// VERSION DATE     DESCRIPTION
// 1.6        25-6-4        �޸�ϵͳʱ��Ϊ8MHz��ʹ��LVR
//
//                FT64F0A5  TSSOP20
//              -------------------
// NC----------|1(PA5)   	(PA4)20|-----------ADC_ETR     
// NC----------|2(PA6)   	(PA3)19|-----------NC 
// NC----------|3(PA7)   	(PA2)18|-----------NC
// NC----------|4(PC0)   	(PA1)17|-----------NC
// NC----------|5(PC1)		(PA0)16|-----------NC	
// NC----------|6(PB7)		(PB0)15|-----------NC
// GND---------|7(GND)		(PB1)14|----------AN0
// NC----------|8(PB6)		(PB2)13|-----------NC
// VDD---------|9(VDD)		(PB3)12|-----------NC
// NC----------|10(PB5)		(PB4)11|-----------NC
//				-------------------
// 
//*********************************************************
#include "SYSCFG.h"
#include "FT64F0AX.h"
#include "TDelay.h"    // 先包含延时头文件，再包含依赖延时的IIC头文件
#include "TM1650_IIC.h"
#include "EEPROM.h"

// **************************宏定义**************************************
#define TM1650_WRITE_ADDR 0x48
#define TM1650_CMD_DISP_ON 0x01 // 8级亮度，7段显示，开启显示使能（手册好像反了）
#define TM1650_CMD_ADDR_BASE 0x68
#define SCL PB3
#define SDA PB4

#define TM1650_SCL_HOLD_TIME 5 // 单位μs（≥4μs）
#define TM1650_ACK_DELAY 2	   // SCL上升沿后延时
#define ACK_SAMPLE_RETRY 3	   // 采样次数    可以更改


// -----------------------------数码管处理相关变量定义---------------------------
unsigned char seg_code[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F}; // 0~9
unsigned char led_place[] = {0x68, 0x6A, 0x6C, 0x6E};

unsigned char Brightness[] = {0x00, 0x11, 0x21, 0x31, 0x41, 0x51, 0x61, 0x71, 0x01};
unsigned char B_Seg_Level = 0;
unsigned char B_Seg_flag = 0;

unsigned int Seg_Count = 0;
unsigned char T_1sFlag = 0;
unsigned int Number_Sum = 0;
unsigned int Number_Ge = 0;
unsigned int Number_Shi = 0;
unsigned int Number_Bai = 0;
unsigned int Number_Qian = 0;

//Variable definition
volatile char W_TMP  @ 0x70 ;//ϵͳռ�ò�����ɾ�����޸�
volatile char BSR_TMP  @ 0x71 ;//ϵͳռ�ò�����ɾ�����޸�
volatile	unsigned int	adcData;
volatile	unsigned int	theVoltage;

void DelayUs(unsigned char Time);
void IIC_WrByte_TM1650(unsigned char txd);
void I2C_Start_TM1650(void);
void I2C_Stop_TM1650(void);
unsigned char TM1650_IIC_wait_ack(void);
void TM1650_cfg_display(unsigned char param);
void TM1650_Set(unsigned char add, unsigned char dat);
void TM1650_DisplayClose(void);
void TM1650_Init(void);
void user_isr();

//===========================================================
//Funtion name��interrupt ISR
//parameters����
//returned value����
//===========================================================
void interrupt ISR(void)
{
#asm;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
	NOP;//ϵͳ���ò�����ɾ�����޸�
#endasm;//ϵͳ���ò�����ɾ�����޸�
	user_isr(); //�����û��жϺ���
}
void user_isr() //�����û��жϺ���
{
}
/*-------------------------------------------------
 * ��������POWER_INITIAL
 * ���ܣ� 	 �ϵ�ϵͳ��ʼ��
 * ���룺	 ��
 * ����� 	 ��
 --------------------------------------------------*/
 void POWER_INITIAL(void)
 {
	OSCCON=0B01100001;			//ϵͳʱ��ѡ��Ϊ�ڲ�����8MHz,��Ƶ��Ϊ1:1
    
    PCKEN|=0B00000001;			//ADģ��ʱ��ʹ��
    
	INTCON=0;					//��ֹ�����ж�
    
    PORTA=0B00000000;
    PORTB=0B00000000;
    PORTC=0B00000000;
    
	WPUA=0B00000000;			//�������Ŀ��أ�0-�أ�1-��		
	WPUB=0B00000000;
	WPUC=0B00000000;	

	WPDA=0B00010000;			//�������Ŀ��أ�0-�أ�1-�� ��PA4����
	WPDB=0B00000000;
	WPDC=0B00000000;
	
	TRISA=0B00010000;			//PA5输入
	TRISB=0B00000010;		    //PB1输入	
	TRISC=0B00000000;

	PSRC0=0B11111111;			//Դ�����������
	PSRC1=0B11111111;
	PSRC2=0B00001111;

	PSINK0=0B11111111;			//������������
	PSINK1=0B11111111;
	PSINK2=0B00000011;

	ANSELA=0B00000000;			//���ö�Ӧ��IOΪ����IO	
    
    AFP0 = 0B00000000;          //ADC_ETR :PA4
 }
 

/*-------------------------------------------------
 * ��������SPI_INITIAL
 * ���ܣ�  ��ʼ��IIC
 * ���룺  ��
 * �����  ��
 --------------------------------------------------*/
void ADC_INITIAL(void)
{
	ANSELA=0B00000001;			//����IO����ģ���룬1����Ӧ��IOΪģ��ܽţ�0����Ӧ��IOΪ����IO������AN0Ϊģ��ܽ�
	ADCON0=0B00000100;
	//Bit[6:4]:000-ѡ��ģ��ͨ��AN0
	//Bit2:1-���ⲿ����Դ����ʱ����ADת��

	ADCON1=0B11100100;
	//Bit7:	1-ADCת������Ҷ��룬��װ��ת�����ʱ��ADRESH�ĸ�4λ������Ϊ0
	//Bit[6:4]:110-ADCת��ʱ������ΪFosc/64
	//Bit[3:2]:01-���ο���ѹ-GND
	//Bit[1:0]:00-���ο���ѹ-�ڲ��ο���ѹ

	ADCON2=0B01010111;
	//Bit[7:6]:01-ADC�ڲ��ο���ѹ2V
    //Bit[5:4]:01-�����ش���
    //Bit[3]:00-LEB��������8λ
    //Bit[2:0]:111-����ԴѡADC_ETR
    
	ADCON3=0B00000000;
    //Bit[3]:0-��ֹLEB����ʱ����ADCת��
    LEBCON = 0B00000000;
    //Bit[7]:0-��ֹLEB
    //Bit[6:5]:00-LEB�ź�Դ��TIM1_CH1
    //Bit[3]:0-�����ش���LEB
    
	ADDLY=0B00000000;			//�ⲿ������ʱ
	ADCMPH=0B00000000;			//ADC�Ƚ���ֵ������ADC�����8λ�Ƚ�
	ADON=1;					    //ʹ��ADC

	TDelay_us(450);  //��ADCģ�����ȴ�ADC�ȶ�ʱ��Tst(~15us);��ѡ���ڲ��ο���ѹʱ��ȴ��ڲ��ο���ѹ���ȶ�ʱ��Tvrint(~450us)

}

/*-------------------------------------------------
 * ��������GET_ADC_DATA
 * ���ܣ�  ��ȡͨ��ADCֵ
 * ���룺  adcChannelͨ�����
 * �����  INT����ADֵ�����β������˲���
 --------------------------------------------------*/
unsigned int GET_ADC_DATA(unsigned char adcChannel)
{   
	ADCON0&=0B00001111;
	ADCON0|=adcChannel<<4;
	TDelay_us(10);                     //TACQ��ʱ2us,�ⲿ��������С��21k��
								    //TACQ��ʱ4us,�ⲿ��������43k��
	//TACQʱ�䣺������ͨ���л���GO/DONE��1��ʱ��,��֤�ڲ� ADC ������ݳ�����
	//TACQ > 0.09*(R+1)us;RΪ�ⲿ��������(k��),��������ԽСԽ�ã����Ҫ����50k��
	ADCIF = 1;  //д1��0
    
    PA5 = 0;
    PA5 = 1;     
   
    TDelay_us(10);                   //��������ʱ��(ADDLY +6) *TAD
	while(ADCIF == 0);
    //��GO = 1 ---> ��GO = 0,ת��������Ҫ16TAD
    //TAD(us)��ת��ʱ��Fosc/ADCS[2:0]�й�
	return (unsigned int
)(ADRESH<<8|ADRESL);//��8λ+��8λ
}


void main(void)
{
    POWER_INITIAL();			//ϵͳ��ʼ��
    ADC_INITIAL();				//ADC��ʼ��
    TM1650_Init();
    while(1)
    {    
		TM1650_Set(led_place[0], seg_code[3]);
        adcData=GET_ADC_DATA(0);
        theVoltage=(unsigned long)adcData*2*1000/4096;
        NOP();
        NOP();
    }
}