#include	"SYSCFG.h"
#include 	"FT64F0AX.h" 

 void Delay10Us(void)
{
	for(unsigned char i=0;i<2;i++)
	{
		NOP(); 
		NOP();
 		NOP(); 
		NOP(); 
		NOP(); 
		NOP();   
 		NOP(); 
		NOP();  
		NOP(); 
		NOP();                                                                                        
	}
} 
void DelayMs(unsigned char Time)
 {
	for(unsigned char a=0;a<Time;a++)
	{
		for(unsigned char b=0;b<96;b++)
		{
		 	Delay10Us(); 	
		}
	}
 }
