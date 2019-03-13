#ifndef __LED_H__
#define __LED_H__

#include "sys.h"

//LED端口定义
#define LED_SYSTEM_ON			0
#define LED_SYSTEM_OFF			1

#define LED_RGB_RCC				RCC_APB2Periph_GPIOB
#define LED_RGB_GPIO			GPIOB

#define LED_SYSTEM_RCC			RCC_APB2Periph_GPIOC
#define LED_SYSTEM_GPIO			GPIOC

#define BLUE_LED_PIN			GPIO_Pin_14
#define SYSTEM_LED_PIN			GPIO_Pin_13
#define RED_LED_PIN				GPIO_Pin_15

#define RED_LED					PBout(14)
#define SYSTEM_LED				PCout(13)
#define BLUE_LED				PBout(15)

//LED端口定义
#define LED_ON					1
#define LED_OFF					0

#define LED_RCC					RCC_APB2Periph_GPIOC
#define LED_GPIO				GPIOC
#define NBIOT_LED_PIN			GPIO_Pin_11
#define NBIOT_LED				PCout(11)

void LED_Init(void);

#endif



