#ifndef __DHT11_H
#define	__DHT11_H

#include "Sys.h"

#define HIGH  1
#define LOW   0


//带参宏，可以像内联函数一样使用,输出高电平或低电平
#define DHT11_DATA_OUT(a)	if(a) GPIO_SetBits(GPIOA,GPIO_Pin_8);\
					else GPIO_ResetBits(GPIOA,GPIO_Pin_8)
 //读取引脚的电平
#define  DHT11_DATA_IN()	   GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)

typedef struct
{
	uint8_t  humi_int;		//湿度的整数部分
	uint8_t  humi_deci;	 	//湿度的小数部分
	uint8_t  temp_int;	 	//温度的整数部分
	uint8_t  temp_deci;	 	//温度的小数部分
	uint8_t  check_sum;	 	//校验和          
}DHT11_Data_TypeDef;

typedef struct
{
	float temperature;
	float humidity;
}DHT11_Data_T;


void DHT11_init(void);
void DHT11_task(void);
void DHT11_Get_temphumi(void);
DHT11_Data_T* Get_Dht11_Data(void);

#endif









