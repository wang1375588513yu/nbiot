#ifndef __DHT11_H
#define	__DHT11_H

#include "Sys.h"

#define HIGH  1
#define LOW   0


//���κ꣬��������������һ��ʹ��,����ߵ�ƽ��͵�ƽ
#define DHT11_DATA_OUT(a)	if(a) GPIO_SetBits(GPIOA,GPIO_Pin_8);\
					else GPIO_ResetBits(GPIOA,GPIO_Pin_8)
 //��ȡ���ŵĵ�ƽ
#define  DHT11_DATA_IN()	   GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)

typedef struct
{
	uint8_t  humi_int;		//ʪ�ȵ���������
	uint8_t  humi_deci;	 	//ʪ�ȵ�С������
	uint8_t  temp_int;	 	//�¶ȵ���������
	uint8_t  temp_deci;	 	//�¶ȵ�С������
	uint8_t  check_sum;	 	//У���          
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









