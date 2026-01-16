void DelayMs(uchar Time)
 {
	for(uchar a=0;a<Time;a++)
	{
		for(uchar b=0;b<96;b++)
		{
		 	Delay10Us(); 	
		}
	}
 }
 void Delay10Us(void)
{
	for(uchar i=0;i<2;i++)
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