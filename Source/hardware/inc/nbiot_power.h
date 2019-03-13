#ifndef __NBIOT_POWER_H___
#define __NBIOT_POWER_H___
#include "sys.h"
#include "PUBLIC.H"

#define NBIOT_POWER_RCC								RCC_APB2Periph_GPIOA
#define NBIOT_POWER_GPIO							GPIOA
#define NBIOT_POWER_PIN								GPIO_Pin_11
#define NBIOT_POWER_RESET_PIN 						PAout(11)


void nbiot_power_init(void);
void nbiot_power_reset(void);
void nbiot_power_loop(void);
void nbiot_power_off(void);
void nbiot_power_on(void);
bool nbiot_power_isenable(void);

#endif



