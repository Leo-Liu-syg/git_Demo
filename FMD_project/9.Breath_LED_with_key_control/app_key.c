#include "hw_gpio.h"
#include "app_led.h"
#include "app_key.h"

// ȫ�ֱ������壺����������������volatile���Σ��ж����޸ģ�
volatile unsigned char g_Key_DelayCount = 0;

// ����Ӧ�ò��ʼ��
void APP_KEY_Init(void)
{
    g_Key_DelayCount = 0;
}

// �����߼����������ģ�����ԭ����ѭ���еİ����߼���
void APP_KEY_Process(void)
{
    // ��һ������⵽��������
    if(HW_KEY_Read() == 1)
    {
        // �ڶ�����������ʱ����������ʼ�ۼ�20ms
        g_Key_DelayCount = 0;
        
        // ���������ȴ��ۼ�20ms����������������
        if(g_Key_DelayCount < KEY_DELAY_20MS)
        {return;}
        
        // ���Ĳ�����ʱ���ٴ�ȷ�ϰ����Ƿ��԰��£���Ч�����жϣ�
        if(HW_KEY_Read() == 1) 
        {
            // ���岽������LEDģʽ�л�
            APP_LED_SwitchMode();
            
            // ���������ȴ������ͷţ������ظ�����
            while(HW_KEY_Read() == 1);
        }
    }
}
