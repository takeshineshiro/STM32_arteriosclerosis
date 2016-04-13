#ifndef __MAIN_H
#define __MAIN_H


#define Pump1_Off 	GPIO_ResetBits(GPIOB,GPIO_Pin_9)				//��1 off
#define Pump1_On 	GPIO_SetBits(GPIOB,GPIO_Pin_9)					//��1 on
#define Pump2_Off 	GPIO_ResetBits(GPIOB,GPIO_Pin_8)				//��2 off
#define Pump2_On 	GPIO_SetBits(GPIOB,GPIO_Pin_8)					//��2 on
#define Pump3_Off 	GPIO_ResetBits(GPIOB,GPIO_Pin_7)				//��3 off
#define Pump3_On 	GPIO_SetBits(GPIOB,GPIO_Pin_7)					//��3 on
#define Pump4_Off 	GPIO_ResetBits(GPIOB,GPIO_Pin_6)				//��4 off
#define Pump4_On 	GPIO_SetBits(GPIOB,GPIO_Pin_6)					//��4 on


#define   u8            unsigned   char   
	
#define   u16           unsigned   short   

#define CliqueSlow1_On 	GPIO_ResetBits(GPIOA,GPIO_Pin_6)			//��������1 on
#define CliqueSlow1_Off GPIO_SetBits(GPIOA,GPIO_Pin_6)				//��������1 off
#define CliqueSlow2_On 	GPIO_ResetBits(GPIOA,GPIO_Pin_7)			//��������2 on
#define CliqueSlow2_Off GPIO_SetBits(GPIOA,GPIO_Pin_7)				//��������2 off
#define CliqueSlow3_On 	GPIO_ResetBits(GPIOB,GPIO_Pin_0)			//��������3 on
#define CliqueSlow3_Off GPIO_SetBits(GPIOB,GPIO_Pin_0)				//��������3 off
#define CliqueSlow4_On 	GPIO_ResetBits(GPIOB,GPIO_Pin_1)			//��������4 on
#define CliqueSlow4_Off GPIO_SetBits(GPIOB,GPIO_Pin_1)				//��������4 off

#define Cliquefast1_On 	GPIO_ResetBits(GPIOB,GPIO_Pin_12)			//�������1 on
#define Cliquefast1_Off GPIO_SetBits(GPIOB,GPIO_Pin_12)				//�������1 off
#define Cliquefast2_On 	GPIO_ResetBits(GPIOB,GPIO_Pin_13)			//�������2 on
#define Cliquefast2_Off GPIO_SetBits(GPIOB,GPIO_Pin_13)				//�������2 off
#define Cliquefast3_On 	GPIO_ResetBits(GPIOB,GPIO_Pin_14)			//�������3 on
#define Cliquefast3_Off GPIO_SetBits(GPIOB,GPIO_Pin_14)				//�������3 off
#define Cliquefast4_On 	GPIO_ResetBits(GPIOB,GPIO_Pin_15)			//�������4 on
#define Cliquefast4_Off GPIO_SetBits(GPIOB,GPIO_Pin_15)				//�������4 off

#define LED1_Off 	GPIO_SetBits(GPIOC,GPIO_Pin_7)						//LED1---off
#define LED1_On 	GPIO_ResetBits(GPIOC,GPIO_Pin_7)					//LED1---on
#define LED2_Off 	GPIO_SetBits(GPIOC,GPIO_Pin_8)						//LED2---off
#define LED2_On 	GPIO_ResetBits(GPIOC,GPIO_Pin_8)					//LED2---on
#define LED3_Off 	GPIO_SetBits(GPIOC,GPIO_Pin_9)						//LED3---off
#define LED3_On 	GPIO_ResetBits(GPIOC,GPIO_Pin_9)					//LED3---on


#define Power_Enable 	GPIO_SetBits(GPIOB,GPIO_Pin_9)					//powerʹ��
#define Power_Disable	GPIO_ResetBits(GPIOB,GPIO_Pin_9)				//power��ʹ��
void usart1Printf(char *fmt, ...);
void Uart1Send(char *Data,int Len);
void Transfer_Agreement15x8b_Data(u8 DataValue,u16 Pressure_mmHg1x10,u16 Pressure_mmHg2x10,u16 Pressure_mmHg3x10,u16 Pressure_mmHg4x10);
void Transfer_Agreement15x8b_Command(u8 CommandValue1,u8 CommandValue2);

unsigned short CRC16(unsigned char* puchMsg, unsigned short usDataLen);

#endif
