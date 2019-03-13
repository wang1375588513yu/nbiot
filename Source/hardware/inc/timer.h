#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

extern u32 millisec_tick;
extern u32 sec_tick;

void TIM3_Int_Init(u16 arr,u16 psc);
void TIM3_set_tick(unsigned int ticks);
unsigned int Get_SysTick(void);
void TIM4_Int_Init(u16 arr,u16 psc);
unsigned int Get_SysmilliTick(void);
void Set_MillisecTimer_Rs485_delay(unsigned int delay);
unsigned int Get_SysRunTick(void);

#ifdef HCM300B_SEND
void Set_MillisecTimer_Sensor_Rs485_delay(unsigned int delay);
void Set_MillisecTimer_Bluetooth_delay(unsigned int delay);
unsigned int Get_MillisecTimer_Bluetooth_delay(void);
void Set_MillisecTimer_NBIOT_delay(unsigned int delay);
unsigned int Get_MillisecTimer_NBIOT_delay(void);
#endif




#endif
