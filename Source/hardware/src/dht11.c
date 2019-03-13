#include "dht11.h"
#include "delay.h"
#include "timer.h"


DHT11_Data_TypeDef DHT11_data = {0};

/*
 * ��������DHT11_GPIO_Config
 * ����  ������DHT11�õ���I/O��
 */
void DHT11_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); 														   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);	

	GPIO_SetBits(GPIOA, GPIO_Pin_8);	 
}

/*
 * ��������DHT11_Mode_IPU
 * ����  ��ʹDHT11-DATA���ű�Ϊ��������ģʽ
 */
static void DHT11_Mode_IPU(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);	 
}

/*
 * ��������DHT11_Mode_Out_PP
 * ����  ��ʹDHT11-DATA���ű�Ϊ�������ģʽ
 */
static void DHT11_Mode_Out_PP(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
	
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;	
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);	 	 
}

/* 
 * ��DHT11��ȡһ���ֽڣ�MSB����
 */
static uint8_t Read_Byte(void)
{
	uint8_t i, temp=0;

	for(i=0;i<8;i++)    
	{	 		
		while(DHT11_DATA_IN()==Bit_RESET);/*ÿbit��50us�͵�ƽ���ÿ�ʼ����ѯֱ���ӻ����� ��50us �͵�ƽ ����*/  
		/*DHT11 ��26~28us�ĸߵ�ƽ��ʾ��0������70us�ߵ�ƽ��ʾ��1����
		 *ͨ����� x us��ĵ�ƽ��������������״ ��x ���������ʱ */		 
		delay_us(40); //��ʱx us �����ʱ��Ҫ��������0������ʱ�伴��	   	  

		if(DHT11_DATA_IN()==Bit_SET)/* x us����Ϊ�ߵ�ƽ��ʾ���ݡ�1�� */
		{			
			while(DHT11_DATA_IN()==Bit_SET);/* �ȴ�����1�ĸߵ�ƽ���� */
			temp|=(uint8_t)(0x01<<(7-i));  //�ѵ�7-iλ��1��MSB���� 
		}
		else	
		{ // x us��Ϊ�͵�ƽ��ʾ���ݡ�0��			   
			temp&=(uint8_t)~(0x01<<(7-i)); //�ѵ�7-iλ��0��MSB����
		}
	}
	return temp;
}
/*
 * һ�����������ݴ���Ϊ40bit����λ�ȳ�
 * 8bit ʪ������ + 8bit ʪ��С�� + 8bit �¶����� + 8bit �¶�С�� + 8bit У��� 
 */
static uint8_t Read_DHT11(DHT11_Data_TypeDef *DHT11_Data)
{  	
	DHT11_Mode_Out_PP();/*���ģʽ*/	
	DHT11_DATA_OUT(LOW);/*��������*/	
	delay_ms(18);/*��ʱ18ms*/	
	DHT11_DATA_OUT(HIGH); /*�������� ������ʱ30us*/
	delay_us(30);   //��ʱ30us
	
	DHT11_Mode_IPU();/*������Ϊ���� �жϴӻ���Ӧ�ź�*/ 
	if(DHT11_DATA_IN()==Bit_RESET)     /*�жϴӻ��Ƿ��е͵�ƽ��Ӧ�ź� �粻��Ӧ����������Ӧ����������*/  
	{	
		while(DHT11_DATA_IN()==Bit_RESET);/*��ѯֱ���ӻ����� ��80us �͵�ƽ ��Ӧ�źŽ���*/  	
		while(DHT11_DATA_IN()==Bit_SET);/*��ѯֱ���ӻ������� 80us �ߵ�ƽ �����źŽ���*/

		/*��ʼ��������*/   
		DHT11_Data->humi_int= Read_Byte();
		DHT11_Data->humi_deci= Read_Byte();
		DHT11_Data->temp_int= Read_Byte();
		DHT11_Data->temp_deci= Read_Byte();
		DHT11_Data->check_sum= Read_Byte();
	
		DHT11_Mode_Out_PP();/*��ȡ���������Ÿ�Ϊ���ģʽ*/		
		DHT11_DATA_OUT(HIGH);/*��������*/

		/*����ȡ�������Ƿ���ȷ*/
		if(DHT11_Data->check_sum == DHT11_Data->humi_int + DHT11_Data->humi_deci + DHT11_Data->temp_int+ DHT11_Data->temp_deci)
			return SUCCESS;
		else 
			return ERROR;
	}
	else
	{		
		return ERROR;
	}   
}


DHT11_Data_T dht11_t = {30,70};		//Ĭ��ֵ
void DHT11_Get_temphumi(void)
{
	if(Read_DHT11(&DHT11_data) == SUCCESS)
	{
		dht11_t.temperature = DHT11_data.temp_int + DHT11_data.temp_deci;
		dht11_t.humidity = DHT11_data.humi_int + DHT11_data.humi_deci;
	}
}

static void DHT11_loop(void)
{
	static unsigned int softimersec=0xffffffff;
    if(softimersec==Get_SysTick())return;
    softimersec=Get_SysTick();
	
	DHT11_Get_temphumi();
}

void DHT11_task(void)
{
	DHT11_loop();
}

DHT11_Data_T* Get_Dht11_Data(void)
{
	return &dht11_t;
}
