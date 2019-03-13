#ifndef ___ADC_H___
#define ___ADC_H___
#include "sys.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"  

void Adc_Init(void);

u16 Get_Adc_Average(u16 nChannel) ; 
float Get_CPU_Temprate(void);

#endif
