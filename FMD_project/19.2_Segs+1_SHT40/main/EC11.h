#ifndef	_EC11_H_
#define	_EC11_H_

#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long
#define EC11_A PA7
#define EC11_B PA6

unsigned char EC11_Read_State(void);
void EC11_Process(void);

#endif