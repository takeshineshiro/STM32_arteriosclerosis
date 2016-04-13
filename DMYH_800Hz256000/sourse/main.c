#include "stm32f10x.h"
#include "main.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define ADC1_DR_Address    ((u32)0x4001244C)         //ADC1  0x4001 2400 - 0x4001 27FF

#define FirstStage_1234    	200
#define SecondStage_12    	220
#define SecondStage_34    	240
#define FirstStage_Level    70
#define SecondStage_Level   55
#define CMD_BUFFER_LEN 			100
#define Frame_ST1 		      0x24	//0x24
#define Frame_ST2 		      0x0E	//0x40
#define Frame_ST3 		      0x3B	//0x33
#define Frame_ED1 		      0x0F
#define Frame_ED2 		      0x55
#define UART1_Buf_Num       50

u8 Uart1_Rbuf[64];

u8 Uart1_flag=0;

u8 Uart1_count=0;

u8 Add_Num=0;

u32 Timer_i=0;

u8 Sys_Start_Sign=0;

u8 Sys_End_Sign=0;

u8 Inspection_Mode=0;

u8 Send_Numb=0;

u8 Transfer_DataDisable_CommandEnable_Sign=0;

u16 Pressure_Inspection=0;

u16 Pressure_Inspection_TH=0;

u8 Pressure_Inspection_TL=0;

u8 Inspection_Start_Sign=0;

GPIO_InitTypeDef           GPIO_InitStructure;

ADC_InitTypeDef            ADC_InitStructure;

vu16                       ADC_MultiChannelConvertedValue[40];

TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;

USART_InitTypeDef          USART_InitStructure;

NVIC_InitTypeDef           NVIC_InitStructure;


void RCC_cfg(void)
{  
	ErrorStatus HSEStartUpStatus;
	
	RCC_DeInit();
	
	RCC_HSEConfig(RCC_HSE_ON);  
	
	HSEStartUpStatus = RCC_WaitForHSEStartUp();		
	
	if(HSEStartUpStatus == SUCCESS)        									
	{
		RCC_HCLKConfig(RCC_SYSCLK_Div1); 
		
		RCC_PCLK2Config(RCC_HCLK_Div1);
		
		RCC_PCLK1Config(RCC_HCLK_Div2); 
		
		FLASH_SetLatency(FLASH_Latency_2); 
		
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable); 
		
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);                                       //8*9  =  72Mhz
		
		RCC_PLLCmd(ENABLE);  // πƒ‹PLL  
		
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {} 	
			
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); 	                                             //PLL ±÷”◊ˆœµÕ≥ ±÷”
			
		while(RCC_GetSYSCLKSource() != 0x08)        						
		 { 
		 }    
	}
}


void Usart_Init(u32 BaudRate_Num)
{
	                                                               //»´æ÷ ±÷”≈‰÷√
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO , ENABLE);
	                                                        // ˝æ›∂Àø⁄≈‰÷√
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;                // ‰»Î
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;	
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;               // ‰≥ˆ
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP; 					
	GPIO_Init(GPIOA, &GPIO_InitStructure);
             
	USART_InitStructure.USART_BaudRate 	 = BaudRate_Num;		   //“Ï≤Ω ’∑¢ºƒ¥Ê∆˜≈‰÷√
	USART_InitStructure.USART_WordLength = 8;
	USART_InitStructure.USART_StopBits 	 = 1;
	USART_InitStructure.USART_Parity 	 = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode 		 = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);	

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);              //“Ï≤Ω ’∑¢÷–∂œ”≈œ»º∂≈‰÷√     
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;         //µ⁄0”≈œ»º∂
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_Init(USART1, &USART_InitStructure);
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	USART_ClearFlag(USART1,USART_FLAG_TC);    
	
	USART_Cmd(USART1, ENABLE);                                   //∆Ù”√“Ï≤Ω ’∑¢
	
}


                                            
void Uart1Send(char *Data,int Len)        
{
	u8  i;
	
	for(i = 0; i < Len; i++ )															 
		{
				USART_SendData(USART1, *(Data+i));
			
				while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
			
		}	
		
}





void Timer_Init(void)
{                                                             //»´æ÷ ±÷”≈‰÷√
	                                                           
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
		
	TIM_DeInit(TIM3);                                                                              
	
	TIM_TimeBaseStructure.TIM_Period=2499;                       //÷‹∆⁄
	
	TIM_TimeBaseStructure.TIM_Prescaler= 35; 		
	
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 	      //≤ª∑÷∆µ
	
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;	
	
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);             //≈‰÷√∂® ±ºƒ¥Ê∆˜3     
	
	TIM_DeInit(TIM2);
	
	TIM_TimeBaseStructure.TIM_Period=9999; 	                    //÷‹∆⁄
	
	TIM_TimeBaseStructure.TIM_Prescaler= 7199; 		
	
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;      //≤ª∑÷∆µ 
	
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;	
	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);          //≈‰÷√∂® ±ºƒ¥Ê∆˜2
	
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);                
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; 
	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	       //µ⁄1”≈œ»º∂
	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 		
	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	
	NVIC_Init(&NVIC_InitStructure);                          //≈‰÷√∂® ±∆˜3µƒ÷–∂œ”≈œ»º∂
	
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn; 
	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	
	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 
	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	
	NVIC_Init(&NVIC_InitStructure);                     //≈‰÷√∂® ±∆˜2µƒ÷–∂œ”≈œ»º∂           
	
	
	
	TIM_ClearFlag(TIM2, TIM_FLAG_Update); 
	
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	
	TIM_ClearFlag(TIM3, TIM_FLAG_Update); 
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	
	TIM_ARRPreloadConfig(TIM3, ENABLE); 	
	
	TIM_Cmd(TIM3, ENABLE); 	
	
	TIM_Cmd(TIM2, ENABLE);                             //∆Ù”√∂® ±∆˜
	
}




void ADC1_MultiChannel_Configuration(void)         //AD ˝æ›Ω” ’∆Ù”√DMAÕ®µ¿
{
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	DMA_InitTypeDef DMA_InitStructure;
	
	ADC_InitTypeDef ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |RCC_APB2Periph_AFIO , ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 , ENABLE);
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);               //»´æ÷ ±÷”≈‰÷√£¨∑÷±∆Ù”√∏˜∏ˆºƒ¥Ê∆˜µƒ ±÷”
	
	
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);		               //AD ±÷”∑÷∆µ  fs/8   =  9M Hz
	
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	
	GPIO_Init(GPIOC,&GPIO_InitStructure);   


	
	DMA_DeInit(DMA1_Channel1);                                  //∆Ù”√DMAÕ®µ¿1
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;		   //DMA SOURCE  ADDRESS
	
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADC_MultiChannelConvertedValue;   //DMA   DESTINATION  ADDRESS
	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;          //DMA  ∑ΩœÚ  
	
	DMA_InitStructure.DMA_BufferSize = 40;	                    //DMA buffer size 
	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;     //“ª≤…ºØæÕÀÕDMAÕ®µ¿
	
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;		
	
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;		                //—≠ª∑ ˝◊È	
	
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;		
	
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	
	
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	
	DMA_Cmd(DMA1_Channel1, ENABLE);
	
	

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	
	
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;	
	
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;	
	
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	
	
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	
	ADC_InitStructure.ADC_NbrOfChannel = 4;		
	
	ADC_Init(ADC1, &ADC_InitStructure);                         //ADC1ºƒ¥Ê∆˜≈‰÷√
	
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_239Cycles5);     //AD ±÷”£∫  9M/(239.5+12.5) == 36K   
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_239Cycles5);
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_239Cycles5);
  
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_SampleTime_239Cycles5); 
	
	ADC_DMACmd(ADC1, ENABLE);       //ADCºƒ¥Ê∆˜∆Ù”√DMAπ¶ƒ‹       
	
	ADC_Cmd(ADC1, ENABLE);         //ADCºƒ¥Ê∆˜ πƒ‹
	
	ADC_ResetCalibration(ADC1);
	
	while(ADC_GetResetCalibrationStatus(ADC1));
	
	ADC_StartCalibration(ADC1);
	
	while(ADC_GetCalibrationStatus(ADC1));
	
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	
	
}




void Led_IO_Init(void)
{
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;  		//LEDµ∆≈‰÷√
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	
	GPIO_Init(GPIOC, &GPIO_InitStructure); 
	
	LED1_Off;LED2_Off;LED3_Off;
	
}



void Pump_Clique_IO_Init(void)                         //≥ı ºªØ∆¯∑ß∂Àø⁄
{
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_12|GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;     	//±√µƒ∫ÕFAST SLOWµƒIO
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_6|GPIO_Pin_7; 	
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	Pump1_Off;Pump2_Off;Pump3_Off;Pump4_Off;	
	
	CliqueSlow1_On;CliqueSlow2_On;CliqueSlow3_On;CliqueSlow4_On;
	
	Cliquefast1_On;Cliquefast2_On;Cliquefast3_On;Cliquefast4_On;
	
}








u32      Sum1=0,Sum2=0,Sum3=0,Sum4=0;

u16      AVG1=0,AVG2=0,AVG3=0,AVG4=0;

u16      SecondStage_Pressure_Up12=0,SecondStage_Pressure_Up34=0;

float    Pressure_mmHg1=0,Pressure_mmHg2=0,Pressure_mmHg3=0,Pressure_mmHg4=0;

u8       Protect=0,Stage_1=0,Stage_2=0,Stage_3=0;



void Pressure_Collect(void)                                           //DMA ˝æ›∂¡»°÷¡»´æ÷ºƒ¥Ê∆˜÷–  £®“ª÷°£©
{
	u16 i=0;
	
	Sum1=0;Sum2=0;Sum3=0;Sum4=0;
	
	for(i=0;i<40;i=i+4)                  //Œ™…∂≤ª≤¢∑¢£ø£ø      
	 {
		 Sum1+= ADC_MultiChannelConvertedValue[i];
		 
		 Sum2+= ADC_MultiChannelConvertedValue[i+1];
		 
		 Sum3+= ADC_MultiChannelConvertedValue[i+2];
		 
		 Sum4+= ADC_MultiChannelConvertedValue[i+3];
		 
	 }
	 
	AVG1=Sum1/10;AVG2=Sum2/10;AVG3=Sum3/10;AVG4=Sum4/10;     //«Û∆Ωæ˘
	 
	Pressure_mmHg1=((AVG1-188)*230.6)/(2806-188);            //£ø£ø
	 
	Pressure_mmHg2=((AVG2-149)*211.6)/(2566-149);
	 
	Pressure_mmHg3=((AVG3-173)*257.5)/(3109-173);
	 
	Pressure_mmHg4=((AVG4-181)*222.2)/(2712-181);
	 
}



char TX_buf[20];
                                               //◊È÷°∑¢ÀÕ ˝æ›

                                              //∑¢ÀÕ ˝æ›∏¯…œŒªª˙     £¨Õ®π˝∂® ±÷–∂œ3
void Transfer_Agreement15x8b_Data(u8 DataValue,u16 Pressure_mmHg1x10,u16 Pressure_mmHg2x10,u16 Pressure_mmHg3x10,u16 Pressure_mmHg4x10)
{	
	
	u8  Sum_CRC=0;	
	
	u8 Pressure_mmHg_TL1=0,Pressure_mmHg_TH1=0,Pressure_mmHg_TL2=0,Pressure_mmHg_TH2=0,Pressure_mmHg_TL3=0,Pressure_mmHg_TH3=0,Pressure_mmHg_TL4=0,Pressure_mmHg_TH4=0;
	
	Pressure_mmHg_TL1=(u8)(Pressure_mmHg1x10&0xFF);
	
	Pressure_mmHg_TH1=(u8)((Pressure_mmHg1x10>>8)&0xFF);
	
	Pressure_mmHg_TL2=(u8)(Pressure_mmHg2x10&0xFF);
	
	Pressure_mmHg_TH2=(u8)((Pressure_mmHg2x10>>8)&0xFF);
	
	Pressure_mmHg_TL3=(u8)(Pressure_mmHg3x10&0xFF);
	
	Pressure_mmHg_TH3=(u8)((Pressure_mmHg3x10>>8)&0xFF);
	
	Pressure_mmHg_TL4=(u8)(Pressure_mmHg4x10&0xFF);
	
	Pressure_mmHg_TH4=(u8)((Pressure_mmHg4x10>>8)&0xFF);
	
	TX_buf[0]=Frame_ST1;
	
	TX_buf[1]=Frame_ST2; 
	
	TX_buf[2]=Frame_ST3;
	
	TX_buf[3]=DataValue;            //??//
	
	TX_buf[4]=Pressure_mmHg_TH1;
	
	TX_buf[5]=Pressure_mmHg_TL1;
	
	TX_buf[6]=Pressure_mmHg_TH2;
	
	TX_buf[7]=Pressure_mmHg_TL2;
	
	TX_buf[8]=Pressure_mmHg_TH3;
	
	TX_buf[9]=Pressure_mmHg_TL3;
	
	TX_buf[10]=Pressure_mmHg_TH4;
	
	TX_buf[11]=Pressure_mmHg_TL4;	
	
	Sum_CRC=(u8)((TX_buf[3]+TX_buf[4]+TX_buf[5]+TX_buf[6]+TX_buf[7]+TX_buf[8]+TX_buf[9]+TX_buf[10]+TX_buf[11])&0xFF);
	
	TX_buf[12]=Sum_CRC;
	
	TX_buf[13]=Frame_ED1;
	
	TX_buf[14]=Frame_ED2;
	
	Uart1Send(TX_buf,15);

}

                                       //∑¢ÀÕ÷∏¡Ó∏¯…œŒªª˙

void Transfer_Agreement15x8b_Command(u8 CommandValue1,u8 CommandValue2)
{	
	u8  Sum_CRC=0,TransferNumi=0;	
	
	TX_buf[0]=  Frame_ST1;
	
	TX_buf[1]=  Frame_ST2; 
	
	TX_buf[2]=  Frame_ST3;
	
	TX_buf[3]=  CommandValue1;      //0x01
	
	TX_buf[4]=  CommandValue2;
	
	TX_buf[5]=   0x00;
	
	TX_buf[6]=   0x00;
	
	TX_buf[7]=   0x00;
	
	TX_buf[8]=   0x00;
	
	TX_buf[9]=   0x00;
	
	TX_buf[10]=  0x00;
	
	TX_buf[11]=   0x00;
	
	Sum_CRC=(u8)((TX_buf[3]+TX_buf[4]+TX_buf[5]+TX_buf[6]+TX_buf[7]+TX_buf[8]+TX_buf[9]+TX_buf[10]+TX_buf[11])&0xFF);
	
	TX_buf[12]=  Sum_CRC;
	
	TX_buf[13]=  Frame_ED1;
	
	TX_buf[14]=  Frame_ED2;
	
	Transfer_DataDisable_CommandEnable_Sign=1;	
	
	for(TransferNumi=0;TransferNumi<10;TransferNumi++)       //—≠ª∑∑¢ÀÕ10±È£°£ø£ø//
	
	Uart1Send(TX_buf,15);
	
	Transfer_DataDisable_CommandEnable_Sign=0;
	
}



void Uart1_Receive(void)               //Ω” ’…œŒªª˙÷∏¡Ó£¨Õ®π˝UART“Ï≤Ω÷–∂œ¥•∑¢
{
	
	u8  Uart1i=0;
	
	u16 Uart1j=0; 
	
	for(Uart1j=0;Uart1j<20000;Uart1j++);                 //µ»¥˝XX√Î
	
	
	for(Uart1i=0;Uart1i<(UART1_Buf_Num-15);Uart1i++)     //≤È’““ª◊ÈÕÍ’˚ ˝æ›÷°
	 {
		 if((Uart1_Rbuf[Uart1i]==Frame_ST1)&&(Uart1_Rbuf[Uart1i+1]==Frame_ST2)&&(Uart1_Rbuf[Uart1i+2]==Frame_ST3))   //÷°Õ∑
		  {
				Add_Num=(u8)((Uart1_Rbuf[Uart1i+3]+Uart1_Rbuf[Uart1i+4]+Uart1_Rbuf[Uart1i+5]+Uart1_Rbuf[Uart1i+6]+Uart1_Rbuf[Uart1i+7]+Uart1_Rbuf[Uart1i+8]+Uart1_Rbuf[Uart1i+9]+Uart1_Rbuf[Uart1i+10]+Uart1_Rbuf[Uart1i+11])&0xFF);	
			  if((Add_Num==Uart1_Rbuf[Uart1i+12])&&(Uart1_Rbuf[Uart1i+13]==Frame_ED1)&&(Uart1_Rbuf[Uart1i+14]==Frame_ED2))   //CRC  ∫Õ÷°Œ≤–£—È
				 {
					switch(Uart1_Rbuf[Uart1i+3])             //Ω‚Œˆ÷∏¡Ó            //0X02ø™ º±Ì æ…œŒªª˙œÚœ¬Œªª˙
					 {
							case 0x02:	                         //œµÕ≥≥ı ºªØ≤Ÿ◊˜
							  {
								   Inspection_Mode=0;
									
								 if(Uart1_Rbuf[Uart1i+4]==0x01)    //0x0201           œµÕ≥ø™∆Ù    
								  {
										Sys_Start_Sign=1;             //œµÕ≥ø™∆Ù
										
										Sys_End_Sign=0;
										
										Transfer_Agreement15x8b_Command(0x03,0x01);	  //ø™∆ÙºÏ≤Èƒ£ Ω             
										
								  }
								 else if(Uart1_Rbuf[Uart1i+4]==0x02)    //0x0202       œµÕ≥πÿ±’
								  {
										Sys_Start_Sign=0;
										
										Sys_End_Sign=1;                    //œµÕ≥πÿ±’
										
										Transfer_Agreement15x8b_Command(0x03,0x02);	  //ø™∆ÙºÏ≤Èƒ£ Ω
								  }	
								 else if(Uart1_Rbuf[Uart1i+4]==0x03)  //0x0203         µ⁄∂˛º∂—π¡¶÷µ∏≥÷µ
								  {
										SecondStage_Pressure_Up12=Uart1_Rbuf[Uart1i+5]*10;   //µ⁄Œ6Œª
										
										SecondStage_Pressure_Up34=Uart1_Rbuf[Uart1i+6]*10;   //µ⁄7Œª
										
										Transfer_Agreement15x8b_Command(0x03,0x03);    //ø™∆ÙºÏ≤Èƒ£ Ω
								  }	
							 }
								
							   break;
								
							case 0x03:	               //ºÏ≤Èƒ£ Ω        ◊‘ºÏÃΩ≤‚∫œ  µƒ—π¡¶÷µ
							 {
								 Inspection_Mode=1;	                        
								 
								 if(Uart1_Rbuf[Uart1i+4]==0x01)              //0X0301
								  {
										Transfer_Agreement15x8b_Command(0x04,0x01);	  //Ω¯»Îœ¬“ªCASE      ±√1  on
										
										Inspection_Start_Sign=1;         //ºÏ≤Èø™ º  
								  }
								 else if(Uart1_Rbuf[Uart1i+4]==0x02)   //0X0302
								  {
										Transfer_Agreement15x8b_Command(0x04,0x02);  //Ω¯»Îœ¬“ªCASE      ±√1  off
										
										Inspection_Start_Sign=0;        //ºÏ≤ÈΩ· ¯  
								  }
							 }
							 
							 break;		
							 
							 
							case 0x04:
							 {
								 Inspection_Mode=2;        
								 
								 switch(Uart1_Rbuf[Uart1i+4])    
									{	
										case 0x01:	         //0x0401
										 {
											 		Pump1_On;			//±√1  on
										 }
										 break;
										case 0x02:           //0X0402	
										 {
											 		Pump1_Off;		//±√1  off
										 }break;
										case 0x03:          //0X0403	
										 {
													Pump2_On;			//±√2  on										 
										 }break;
										case 0x04:	        //0X0404
										 {
													Pump2_Off;		//±√2  off											 
										 }break;
										case 0x05:	        //0X0405
										 {
													Pump3_On;			//±√3  on										 
										 }break;
										case 0x06:	        //0X0406
										 {
													Pump3_Off;		//±√3  off										 
										 }break;
										case 0x07:	         //0X0407
										 {
													Pump4_On;			//±√4  on											 
										 }break;
										case 0x08:	         //0X0408
										 {
													Pump4_Off;		//±√4  off											 
										 }break;
										 
										case 0x09:	         //0X0409  
										 {
											 		CliqueSlow1_On;CliqueSlow2_On;CliqueSlow3_On;CliqueSlow4_On;//¬˝ÀŸ∑≈∆¯∑ß on
													Cliquefast1_On;Cliquefast2_On;Cliquefast3_On;Cliquefast4_On;//øÏÀŸ∑≈∆¯∑ß on										 
										 }break;
										case 0x0A:	         //0X040A
										 {
											 		CliqueSlow1_Off;CliqueSlow2_Off;CliqueSlow3_Off;CliqueSlow4_Off;//¬˝ÀŸ∑≈∆¯∑ß off
													Cliquefast1_Off;Cliquefast2_Off;Cliquefast3_Off;Cliquefast4_Off;//øÏÀŸ∑≈∆¯∑ß off										 
										 }break;
										case 0x0B:	          //0X040B
										 {
													Cliquefast1_On;		//øÏÀŸ∑≈∆¯∑ß1 on										 
										 }break;
										case 0x0C:	           //0X040C       
										 {
													Cliquefast1_Off;	//øÏÀŸ∑≈∆¯∑ß1 off										 
										 }break;
										case 0x0D:	           //0X040D
										 {
													Cliquefast2_On;		//øÏÀŸ∑≈∆¯∑ß2 on										 
										 }break;
										case 0x0E:	            //0X040E
										 {
													Cliquefast2_Off;	//øÏÀŸ∑≈∆¯∑ß2 off										 
										 }break;
										case 0x0F:	           //0X040F 
										 {
													Cliquefast3_On;		//øÏÀŸ∑≈∆¯∑ß3 on										 
										 }break;
										case 0x10:	            //0X0410
										 {
													Cliquefast3_Off;	//øÏÀŸ∑≈∆¯∑ß3 off										 
										 }break;
										case 0x11:	             //0X0411
										 {
													Cliquefast4_On;		//øÏÀŸ∑≈∆¯∑ß4 on										 
										 }break;
										case 0x12:	              //0X0412
										 {
													Cliquefast4_Off;	//øÏÀŸ∑≈∆¯∑ß4 off										 
										 }break;
										case 0x13:	              //0X0413
										 {
											 		CliqueSlow1_On;		//¬˝ÀŸ∑≈∆¯∑ß1 on												 
										 }break;
										case 0x14:	             //0X0414
										 {
											 		CliqueSlow1_Off;	//¬˝ÀŸ∑≈∆¯∑ß1 off										 
										 }break;
										case 0x15:	              //0X0415
										 {
											 		CliqueSlow2_On;		//¬˝ÀŸ∑≈∆¯∑ß2 on												 
										 }break;
										case 0x16:	              //0X0416
										 {
											 		CliqueSlow2_Off;	//¬˝ÀŸ∑≈∆¯∑ß2 off										 
										 }break;
										case 0x17:	               //0X0417
										 {
											 		CliqueSlow3_On;		//¬˝ÀŸ∑≈∆¯∑ß3 on												 
										 }break;
										case 0x18:	              //0X0418
										 {
											 		CliqueSlow3_Off;	//¬˝ÀŸ∑≈∆¯∑ß3 off										 
										 }break;
										case 0x19:	              //0X0419
										 {
											 		CliqueSlow4_On;		//¬˝ÀŸ∑≈∆¯∑ß4 on												 
										 }break;
										case 0x1A:	             //0X041A
										 {
											 		CliqueSlow4_Off;	//¬˝ÀŸ∑≈∆¯∑ß4 off										 
										 }break;
										default:
										 {
										 }break;			
									}
								 Transfer_Agreement15x8b_Command(0x05,Uart1_Rbuf[Uart1i+4]);	    //Œ’ ÷    
							 }
							 
							 break;
							 
							 
							  case 0x05:	
							 {
								 if(Uart1_Rbuf[Uart1i+4]==0x01)
								  {
										Transfer_Agreement15x8b_Command(0x06,0x01);			
								  }
							 }
							 
							 break;
							 
							default:
								  
							 break;	
					 }
				 }
				 
		  }
	 } 
	 
	for(Uart1i=0;Uart1i<64;Uart1i++)                     //»´æ÷±‰¡øΩ” ’ ˝◊È«Â¡„
	    Uart1_Rbuf[Uart1i]=0;
	 
	 
}



u8 Channal_1=0,Channal_2=0,Channal_3=0,Channal_4=0;

u8 Inspection_Mode_Num=0;

u16 Protect_Timer=0;



void Inspection_Mode_LifeTime(void)
{	
	
	if(Inspection_Start_Sign==1)             //◊‘ºÏ£ø
	 {
				 
		 Protect=1;
		 
		if(Channal_1 == 0)                      //µ⁄“ª∏ˆ∆¯∑ßÕ®µ¿     
		{
			CliqueSlow1_Off;
			
			Cliquefast1_Off;
			
			Pump1_On;                          //≥‰∆¯
			
		}
		
		if(Pressure_mmHg1 > (300))        //◊‘ºÏ“ª¥Œ∑¢œ÷—π¡¶π˝¥Û     
		{
			Pump1_Off;
			
			CliqueSlow1_On;
			
			Cliquefast1_On;
			
			Channal_1=1;                     //ªªÕ®µ¿
			
		}
		
	 }
	 
	else 
	 {                                                   //◊‘ºÏΩ· ¯
		 
		if((Pressure_mmHg1 <= 19) && (Pressure_mmHg2 <= 19) && (Pressure_mmHg3 <= 19) && (Pressure_mmHg4 <= 19))   //—π¡¶Ã´–°
		 {

			 Transfer_Agreement15x8b_Command(0x02,0x02);        //πÿ±’Õ®µ¿
			 
			 Transfer_Agreement15x8b_Command(0x02,0x02);	      //πÿ±’Õ®µ¿
			 
			 Inspection_Mode_Num++;                             //
			 
			 if(Inspection_Mode_Num>2){
				 
			 Inspection_Mode_Num=0;
				 
			 Inspection_Mode=2;
				 
			 }
			 LED2_Off;
			 
		 }
		 
		Channal_1=0;
		 
		Pump1_Off;Pump2_Off;Pump3_Off;Pump4_Off;                        //πÿ±’≥‰∆¯
		 
		CliqueSlow1_On;CliqueSlow2_On;CliqueSlow3_On;CliqueSlow4_On;		//¬˝ÀŸ∑≈∆¯∑ß on
		 
		Cliquefast1_On;Cliquefast2_On;Cliquefast3_On;Cliquefast4_On;		//øÏÀŸ∑≈∆¯∑ß on
		 
		Protect=0;Protect_Timer=0; 
		 
	 }
	 
	 
	if(Channal_1 == 1)        //£ø£ø    
	{
		Inspection_Start_Sign=0;Channal_1=0;//Channal_2=0;Channal_3=0;Channal_4=0;
		
		Pump1_Off;Pump2_Off;Pump3_Off;Pump4_Off;
		
		CliqueSlow1_On;CliqueSlow2_On;CliqueSlow3_On;CliqueSlow4_On;		//¬˝ÀŸ∑≈∆¯∑ß on
		
		Cliquefast1_On;Cliquefast2_On;Cliquefast3_On;Cliquefast4_On;		//øÏÀŸ∑≈∆¯∑ß on
		
	}
	
	if((Protect_Timer>180)||(Pressure_mmHg1 > 330) || (Pressure_mmHg2 > 330) || (Pressure_mmHg3 > 330) || (Pressure_mmHg4 > 330))     //—π¡¶π˝¥Û
	{
		Sys_End_Sign=0;	Sys_Start_Sign=0;Protect=0;Protect_Timer=0; 
		
		Pump1_Off;Pump2_Off;Pump3_Off;Pump4_Off;
		
		Cliquefast1_On;Cliquefast2_On;Cliquefast3_On;Cliquefast4_On;
		
		CliqueSlow1_On;CliqueSlow2_On; CliqueSlow3_On;CliqueSlow4_On;
		
		Send_Numb=0;Inspection_Mode=0;
		
		Inspection_Start_Sign=0;Channal_1=0;Channal_2=0;Channal_3=0;Channal_4=0;
		
	}
	
	
	
	
}






/*∂ØÃ¨*/
u8 Key_En=0;

u16 Delay=0,Test_Timer=0;

u8 PumpNum1=0,PumpNum2=0,PumpNum3=0,PumpNum4=0,PumpNum5=0,PumpNum6=0,PumpNum7=0,PumpNum8=0;


void Test_double(void)
{
	
	if(Key_En == 1)
	{
		
		if((Pressure_mmHg1 >15) && (Pressure_mmHg2 >15) && (Pressure_mmHg3 >15) && (Pressure_mmHg4 >15))
		 {
			Protect=1;
		 }
		 
		 
		if((Pressure_mmHg1 >= 300) || (Pressure_mmHg2 >= 300) || (Pressure_mmHg3 >= 300) || (Pressure_mmHg4 >= 300))
		 {
			 
			 Transfer_Agreement15x8b_Command(0x02,0x02);
			 
			 Key_En=0;Send_Numb=0;Stage_1=0;Stage_2=0;Stage_3=0;
			 
			 Pump1_Off; Pump2_Off;Pump3_Off;Pump4_Off;
			 
			 Cliquefast1_On;Cliquefast2_On;Cliquefast3_On;Cliquefast4_On;
			 
			 CliqueSlow1_On;CliqueSlow2_On;CliqueSlow3_On;CliqueSlow4_On;
			 
			 Inspection_Start_Sign=0;PumpNum1=0;PumpNum2=0;PumpNum3=0;PumpNum4=0;PumpNum5=0;PumpNum6=0;PumpNum7=0;PumpNum8=0;
			 
			 Sys_End_Sign=0;Protect=0;Protect_Timer=0;
			 
			 SecondStage_Pressure_Up12=SecondStage_12;SecondStage_Pressure_Up34=SecondStage_34;
			 
			 Test_Timer=0;Stage_1=0;Stage_2=0;Stage_3=0;Key_En=0;
			 
		 }
		 
		if(Stage_1 == 1)
		 {
			if(PumpNum1 == 0)
			 {
				if(Pressure_mmHg1 < FirstStage_1234)
				 {
					Pump1_On;CliqueSlow1_Off;Cliquefast1_Off;
				 }
				else
				 {
					Pump1_Off;CliqueSlow1_On;Cliquefast1_Off;
					PumpNum1=1;
				 }
			 }
			else
			 {
				Pump1_Off;
			 }
			if(PumpNum2 == 0)
			 {
				if(Pressure_mmHg2 < FirstStage_1234)
				 {
					Pump2_On;CliqueSlow2_Off;Cliquefast2_Off;
				 }
				else
				 {
					Pump2_Off;CliqueSlow2_On;Cliquefast2_Off;
					PumpNum2=1;
				 }
			 }
			else
			 {
				Pump2_Off;
			 }
			if(PumpNum3 == 0)
			 {
				if(Pressure_mmHg3 < FirstStage_1234)
				 {
					Pump3_On;CliqueSlow3_Off;Cliquefast3_Off;
				 }
				else
				 {
					Pump3_Off;CliqueSlow3_On;Cliquefast3_Off;
					PumpNum3=1;
				 }
			 }
			else
			 {
				Pump3_Off;
			 }
			if(PumpNum4 == 0)
			 {
				if(Pressure_mmHg4 < FirstStage_1234)
				 {
					Pump4_On;CliqueSlow4_Off;Cliquefast4_Off;
				 }
				else
				 {
					Pump4_Off;CliqueSlow4_On;Cliquefast4_Off;
					PumpNum4=1;
				 }
			 }
			else
			 {
				Pump4_Off;
			 }
			if((PumpNum1==1) && (PumpNum2==1) && (PumpNum3==1) && (PumpNum4==1))
			 {
				if((Pressure_mmHg1 < FirstStage_Level) && (Pressure_mmHg2 < FirstStage_Level) && (Pressure_mmHg3 < FirstStage_Level) && (Pressure_mmHg4 < FirstStage_Level))
				 {
					Stage_1 = 0;
					Stage_2 = 1;
					Stage_3 = 0;
				 }
			 }
		 }
		 
		else if(Stage_2 == 1)
		 {
			if(PumpNum5 == 0)
			 {
				if(Pressure_mmHg1 < SecondStage_Pressure_Up12)
				{
					Pump1_On;CliqueSlow1_Off;Cliquefast1_Off;
				}
				else
				{
					Pump1_Off;CliqueSlow1_On;Cliquefast1_Off;
					PumpNum5=1;
				}
			 }
			else if((PumpNum5 == 1)&&(Pressure_mmHg1 < SecondStage_Level))
			 {
				Pump1_On;CliqueSlow1_Off;Cliquefast1_Off;
				PumpNum5=2;
			 }
			else
			 {
				Pump1_Off;
			 }
			if(PumpNum6 == 0)
			 {
				if(Pressure_mmHg2 < SecondStage_Pressure_Up12)
				{
					Pump2_On;CliqueSlow2_Off;Cliquefast2_Off;
				}
				else
				{
					Pump2_Off;CliqueSlow2_On;Cliquefast2_Off;
					PumpNum6=1;
				}
			 }
			else if((PumpNum6 == 1)&&(Pressure_mmHg2 < SecondStage_Level))
			 {
				Pump2_On;CliqueSlow2_Off;Cliquefast2_Off;
				PumpNum6=2;
			 }
			else
			 {
				Pump2_Off;
			 }
			if(PumpNum7 == 0)
			 {
				if(Pressure_mmHg3 < SecondStage_Pressure_Up34)
				 {
					Pump3_On;CliqueSlow3_Off;Cliquefast3_Off;
				 }
				else
				 {
					Pump3_Off;CliqueSlow3_On;Cliquefast3_Off;
					PumpNum7=1;
				 }
			 }
			else if((PumpNum7 == 1)&&(Pressure_mmHg3 < SecondStage_Level))
			 {
				Pump3_On;CliqueSlow3_Off;Cliquefast3_Off;
				PumpNum7=2;
			 }
			else
			 {
				Pump3_Off;
			 }
			if(PumpNum8 == 0)
			 {
				if(Pressure_mmHg4 < SecondStage_Pressure_Up34)
				{
					Pump4_On;CliqueSlow4_Off;Cliquefast4_Off;
				}
				else
				{
					Pump4_Off;CliqueSlow4_On;Cliquefast4_Off;
					PumpNum8=1;
				}
			 }
			else if((PumpNum8 == 1)&&(Pressure_mmHg4 < SecondStage_Level))
			 {
				Pump4_On;CliqueSlow4_Off;Cliquefast4_Off;
				PumpNum8=2;
			 }
			else
			 {
				Pump4_Off;
			 }
			if((PumpNum5==2) && (PumpNum6==2) && (PumpNum7==2) && (PumpNum8==2))
			 {
				Stage_1 = 0;
				Stage_2 = 0;
				Stage_3 = 1;
			 }
		 }
		 
		else if(Stage_3 == 1)
		 {
			Pump1_Off;CliqueSlow1_Off;Cliquefast1_Off;
			Pump2_Off;CliqueSlow2_Off;Cliquefast2_Off;
			Pump3_Off;CliqueSlow3_Off;Cliquefast3_Off;
			Pump4_Off;CliqueSlow4_Off;Cliquefast4_Off;
			if(Test_Timer > 30)
			{
				Transfer_Agreement15x8b_Command(0x02,0x02);		
				Cliquefast1_On;Cliquefast2_On;Cliquefast3_On;Cliquefast4_On;
				CliqueSlow1_On;CliqueSlow2_On;CliqueSlow3_On;CliqueSlow4_On;  
				Test_Timer=0;Stage_1=0;Stage_2=0;Stage_3=0;Key_En=0;Protect_Timer=0;Protect=0;
				PumpNum1=0;PumpNum2=0;PumpNum3=0;PumpNum4=0;PumpNum5=0;PumpNum6=0;PumpNum7=0;PumpNum8=0;
				SecondStage_Pressure_Up12=SecondStage_12;SecondStage_Pressure_Up34=SecondStage_34;
			}
		 }
		 
		else
		 {
			Pump1_Off;Pump2_Off;Pump3_Off;Pump4_Off;
			Cliquefast1_On;Cliquefast2_On;Cliquefast3_On;Cliquefast4_On;
			CliqueSlow1_On;CliqueSlow2_On;CliqueSlow3_On;CliqueSlow4_On;  
			Test_Timer=0;Stage_1=0;Stage_2=0;Stage_3=0;Key_En=0;Protect_Timer=0;Protect=0;
			PumpNum1=0;PumpNum2=0;PumpNum3=0;PumpNum4=0;PumpNum5=0;PumpNum6=0;PumpNum7=0;PumpNum8=0;
			SecondStage_Pressure_Up12=SecondStage_12;SecondStage_Pressure_Up34=SecondStage_34;
		 }
		 
	}
	
}





void IWDG_Init(void)
{
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  
	
	IWDG_SetPrescaler(IWDG_Prescaler_256);   
	
	IWDG_SetReload(0x30D);
	
	IWDG_ReloadCounter();
	
	IWDG_Enable();  
	
}





void IWDG_Reload(void)
{
	IWDG->KR = 0xAAAA;	
}



extern u8 Display_Timer;



int main(void)
{
	RCC_cfg();
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |RCC_APB2Periph_AFIO , ENABLE);
	
	Led_IO_Init();
	
	Pump_Clique_IO_Init();
	
	ADC1_MultiChannel_Configuration();
	
	Usart_Init(256000);
	
	Timer_Init();
	
	if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
		
	RCC_ClearFlag();					                                 		//«Â∏¥Œª±Í÷æ
	
	IWDG_Init();
	
	while(1)                                                      //—≠ª∑µ»¥˝÷–∂œ ¬º˛
	{
		Pressure_Collect();                                        //AD ˝æ›
		
		if(Display_Timer>0){                         
			
		Display_Timer=0;
			
		IWDG_Reload();          
			
		GPIO_WriteBit(GPIOC,GPIO_Pin_9,(BitAction)(1-GPIO_ReadOutputDataBit(GPIOC,GPIO_Pin_9)));      //÷ÿ∆Ù£ø
			
		}
		
		if(Sys_Start_Sign == 1)                 //ø™∆Ù
		 {
			 
			 Transfer_Agreement15x8b_Command(0x02,0x01);
			 
			 PumpNum1=0;PumpNum2=0;PumpNum3=0;PumpNum4=0;PumpNum5=0;PumpNum6=0;PumpNum7=0;PumpNum8=0;
			 
			 Key_En=1;Sys_Start_Sign=0;Stage_1=1;Stage_2=0;Stage_3=0;Protect_Timer=0;Protect=0;Test_Timer=0;
			 
			 SecondStage_Pressure_Up12=SecondStage_12;SecondStage_Pressure_Up34=SecondStage_34;
			 
		 }
		 
		if((Protect_Timer > 250)||(Sys_End_Sign == 1))
		 {
			 
			 Transfer_Agreement15x8b_Command(0x02,0x02);
			 
			 Key_En=0;Send_Numb=0;Stage_1=0;Stage_2=0;Stage_3=0;
			 
			 Pump1_Off; Pump2_Off;Pump3_Off;Pump4_Off;
			 
			 Cliquefast1_On;Cliquefast2_On;Cliquefast3_On;Cliquefast4_On;
			 
			 CliqueSlow1_On;CliqueSlow2_On;CliqueSlow3_On;CliqueSlow4_On;
			 
			 Inspection_Start_Sign=0;PumpNum1=0;PumpNum2=0;PumpNum3=0;PumpNum4=0;PumpNum5=0;PumpNum6=0;PumpNum7=0;PumpNum8=0;
			 
			 Sys_End_Sign=0;Protect=0;Protect_Timer=0;
			 
			 SecondStage_Pressure_Up12=SecondStage_12;SecondStage_Pressure_Up34=SecondStage_34;
			 
			 Test_Timer=0;Stage_1=0;Stage_2=0;Stage_3=0;Key_En=0;
			 
		 }
		 
		 
		if(Inspection_Mode==0)
		 {
			 Test_double();                        //µ⁄“ª÷÷ƒ£ Ω£¨≤‚ ‘∫œ  µƒ∆¯—π∑∂Œß
		 }
		 
		else if(Inspection_Mode==1)              //µ⁄∂˛÷÷ƒ£ Ω
     {
		 Inspection_Mode_LifeTime();
		 }
		else if(Inspection_Mode==2)              //µ⁄»˝÷÷ƒ£ Ω
		 {
			if((Pressure_mmHg1 >= 300) || (Pressure_mmHg2 >= 300) || (Pressure_mmHg3 >= 300) || (Pressure_mmHg4 >= 300))
			 {
				 Key_En=0;Send_Numb=0;Stage_1=0;Stage_2=0;Stage_3=0;
				 
				 Pump1_Off; Pump2_Off;Pump3_Off;Pump4_Off;
				 
				 Cliquefast1_On;Cliquefast2_On;Cliquefast3_On;Cliquefast4_On;
				 
				 CliqueSlow1_On;CliqueSlow2_On;CliqueSlow3_On;CliqueSlow4_On;
				 
				 Inspection_Start_Sign=0;PumpNum1=0;PumpNum2=0;PumpNum3=0;PumpNum4=0;PumpNum5=0;PumpNum6=0;PumpNum7=0;PumpNum8=0;
				 
				 Sys_End_Sign=0;Protect=0;Protect_Timer=0;
				 
				 SecondStage_Pressure_Up12=SecondStage_12;SecondStage_Pressure_Up34=SecondStage_34;
				 
				 Test_Timer=0;Stage_1=0;Stage_2=0;Stage_3=0;Key_En=0;
				 
				 Sys_End_Sign=0;	Sys_Start_Sign=0; 
				 
				 Pump1_Off;Pump2_Off;Pump3_Off;Pump4_Off;
				 
				 Cliquefast1_On;Cliquefast2_On;Cliquefast3_On;Cliquefast4_On;
				 
				 CliqueSlow1_On;CliqueSlow2_On; CliqueSlow3_On;CliqueSlow4_On;
				 
				 Send_Numb=0;Inspection_Mode=0;
				 
				 Inspection_Start_Sign=0;Channal_1=0;Channal_2=0;Channal_3=0;Channal_4=0;
				 
			 }
		 }
		 
		 
		 
		if(Uart1_flag==1)                                     //uart÷–∂œΩ” ’ ˝æ›
		 {
			 Uart1_Receive();
			 
			 Uart1_flag=0;
			 
			 Uart1_count=0;
			 
		 }
		 
		 
		 
	}
	
}

#ifdef  USE_FULL_ASSERT	
/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */


void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif 
