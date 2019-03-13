/********************************Copyright (c)**********************************\
**
**                   (c) Copyright 2018, WangYu, China, SZ.
**                           All Rights Reserved
**
**                        By(�����б�������Ϣ�Ƽ����޹�˾)
**
**----------------------------------File Information----------------------------
** FileName:      main.c
** The Author:    WangYu
** Creation Date: 2018-09-15
** Description:   
**
**--------------------------------Version Information---------------------------
** Version NO: V1.0
** Version Description: Initial Version
**
**------------------------------------------------------------------------------
\********************************End of Head************************************/

#include "main.h"
#include "sys.h"
#include "delay.h"
#include "timer.h"
#include "config.h"
#include "wdg.h"
#include "ota_flash.h"
#include "adc.h"
#include "user_led.h"
#include "usart.h"
#include "user_config.h"
#include "user_m5310.h"


static void SoftTimer(void)
{
    static unsigned int _tick=0x00000;
    if(_tick==Get_SysTick())return;
    _tick=Get_SysTick();

    Config_Loop();
	uart2_printf("uart2_printf\r\n");
}

int main(void)
{
   	delay_init();	    	 //��ʱ������ʼ��
    user_led_init();
    NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	#ifdef UnderClocking
    TIM3_Int_Init(9999,2400-1);
    TIM4_Int_Init(9,2400-1);
	#else
	TIM3_Int_Init(9999,7200-1);
    TIM4_Int_Init(9,7200-1);
	#endif
	NBIOT_init();
//	uart_init(USART1,115200);
//    Adc_Init();
   // IWDG_Init(6,625);    //���ʱ��ΪTout=((4*2^prer)*rlr)/40= ((4*2^6)*625)/40=4000(ms).
	
    while(1)
    {
	//	IWDG_Feed();
		SoftTimer();
		user_led_task();	
		NBIOT_task();
    }
}

