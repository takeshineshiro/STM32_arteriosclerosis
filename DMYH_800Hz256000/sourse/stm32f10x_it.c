#include "stm32f10x.h"
#include "stm32f10x_it.h" 
#include "MAIN.h"
/**
  ******************************************************************************
  * @file    GPIO/IOToggle/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/



 
void NMI_Handler(void)
{
}
 

void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}



 
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
	
}


 

void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
	
}


void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}



#define UART1_Buf_Num 64

extern u8 Uart1_Rbuf[64];

extern u8 Uart1_flag;

extern u8 Uart1_count;

extern u16 Delay,Test_Timer,Protect_Timer,Stage_Timer_1,Stage_Timer_2,Stage_Timer_3,Stage_Timer_4;

extern u8 Key_En,Transfer_DataDisable_CommandEnable_Sign,Inspection_Mode;

extern u8 Restart,Protect,Stage_1,Stage_2,Stage_3,Stage_4;

extern float  Pressure_mmHg1,Pressure_mmHg2,Pressure_mmHg3,Pressure_mmHg4;



void USART1_IRQHandler(void)                                        //ÖÐ¶ÏÒì²½½ÓÊÕÉÏÎ»»ú·¢ËÍÊý¾Ý
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)             //Î´¸´Î»
		{
		  USART_ClearITPendingBit(USART1,USART_IT_RXNE);               //ÇåÖÐ¶Ï
			
			Uart1_Rbuf[Uart1_count]=USART_ReceiveData(USART1);
			
			Uart1_count++;
			
			if(Uart1_count>2)
				{
					
					Uart1_flag=1;                                                //ÓÐÐ§Êý¾Ý¿ªÊ¼                      
					
				}
			if(Uart1_count>(UART1_Buf_Num-1))
				{
					
					Uart1_count=0;
					
				}
		}
}




u8 Display_Timer=0;



void TIM2_IRQHandler(void)                                       //¶¨Ê±ÖÐ¶Ï2
{  
	if (TIM_GetITStatus(TIM2 , TIM_IT_Update) == SET)              //¸üÐÂ
	{  
		
		TIM_ClearITPendingBit(TIM2 , TIM_IT_Update);                //Çå¶¨Ê±ÖÐ¶Ï
		
		Display_Timer++;
		
		if(Display_Timer>250)
			               Display_Timer=0;
		
		if(Protect == 1)                                          //??
		 {
			Protect_Timer++;
			 
			if(Protect_Timer > 253)Protect_Timer=253;
			 
		 }
		else Protect_Timer=0;
		 
		if(Stage_3 == 1)                                       //??
		 {
			 
			Test_Timer++;
			 
			if(Test_Timer > 253)Test_Timer=253;
			 
		 }
		 
		else	Test_Timer=0;
		 
	}	
	
} 





void TIM3_IRQHandler(void)                                         //¶¨Ê±ÖÐ¶Ï3       ¶¨Ê±·¢ËÍÊý¾Ý¸øÉÏÎ»»ú
{  
	if (TIM_GetITStatus(TIM3 , TIM_IT_Update) == SET)                //¶¨Ê±¸üÐÂ
	{  
		
		TIM_ClearITPendingBit(TIM3 , TIM_IT_Update);                   //Çå¿Õ¶¨Ê±ÖÐ¶Ï
		
		if((Inspection_Mode!=0)||(Key_En == 1))                        //??
		{
			
			if(Transfer_DataDisable_CommandEnable_Sign==0)                  //?¡
			{
				
				if(Pressure_mmHg1<1.5)
					Pressure_mmHg1=0;
				else 
					Pressure_mmHg1=Pressure_mmHg1;
				
				if(Pressure_mmHg2<1.5)
					Pressure_mmHg2=0;
				else 
					Pressure_mmHg2=Pressure_mmHg2;
				if(Pressure_mmHg3<1.5)
					Pressure_mmHg3=0;
				else
					Pressure_mmHg3=Pressure_mmHg3;
				if(Pressure_mmHg4<1.5)
					Pressure_mmHg4=0;
				else 
					Pressure_mmHg4=Pressure_mmHg4;
				
				Transfer_Agreement15x8b_Data(0x01,(u16)(Pressure_mmHg1*10),(u16)(Pressure_mmHg2*10),(u16)(Pressure_mmHg3*10),(u16)(Pressure_mmHg4*10));	
				                            //0x01  ±íÊ¾·½ÏòÎªÏÂÎ»»úÏòÉÏÎ»»ú
			}
		}		
	}	
} 




void SVC_Handler(void)
{
}
 


void DebugMon_Handler(void)
{
}
 


void PendSV_Handler(void)
{
}
 


void SysTick_Handler(void)
{
}



/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/
