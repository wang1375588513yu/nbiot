#include "dht11.h"
#include "delay.h"
#include "timer.h"


DHT11_Data_TypeDef DHT11_data = {0};

/*
 * 函数名：DHT11_GPIO_Config
 * 描述  ：配置DHT11用到的I/O口
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
 * 函数名：DHT11_Mode_IPU
 * 描述  ：使DHT11-DATA引脚变为上拉输入模式
 */
static void DHT11_Mode_IPU(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);	 
}

/*
 * 函数名：DHT11_Mode_Out_PP
 * 描述  ：使DHT11-DATA引脚变为推挽输出模式
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
 * 从DHT11读取一个字节，MSB先行
 */
static uint8_t Read_Byte(void)
{
	uint8_t i, temp=0;

	for(i=0;i<8;i++)    
	{	 		
		while(DHT11_DATA_IN()==Bit_RESET);/*每bit以50us低电平标置开始，轮询直到从机发出 的50us 低电平 结束*/  
		/*DHT11 以26~28us的高电平表示“0”，以70us高电平表示“1”，
		 *通过检测 x us后的电平即可区别这两个状 ，x 即下面的延时 */		 
		delay_us(40); //延时x us 这个延时需要大于数据0持续的时间即可	   	  

		if(DHT11_DATA_IN()==Bit_SET)/* x us后仍为高电平表示数据“1” */
		{			
			while(DHT11_DATA_IN()==Bit_SET);/* 等待数据1的高电平结束 */
			temp|=(uint8_t)(0x01<<(7-i));  //把第7-i位置1，MSB先行 
		}
		else	
		{ // x us后为低电平表示数据“0”			   
			temp&=(uint8_t)~(0x01<<(7-i)); //把第7-i位置0，MSB先行
		}
	}
	return temp;
}
/*
 * 一次完整的数据传输为40bit，高位先出
 * 8bit 湿度整数 + 8bit 湿度小数 + 8bit 温度整数 + 8bit 温度小数 + 8bit 校验和 
 */
static uint8_t Read_DHT11(DHT11_Data_TypeDef *DHT11_Data)
{  	
	DHT11_Mode_Out_PP();/*输出模式*/	
	DHT11_DATA_OUT(LOW);/*主机拉低*/	
	delay_ms(18);/*延时18ms*/	
	DHT11_DATA_OUT(HIGH); /*总线拉高 主机延时30us*/
	delay_us(30);   //延时30us
	
	DHT11_Mode_IPU();/*主机设为输入 判断从机响应信号*/ 
	if(DHT11_DATA_IN()==Bit_RESET)     /*判断从机是否有低电平响应信号 如不响应则跳出，响应则向下运行*/  
	{	
		while(DHT11_DATA_IN()==Bit_RESET);/*轮询直到从机发出 的80us 低电平 响应信号结束*/  	
		while(DHT11_DATA_IN()==Bit_SET);/*轮询直到从机发出的 80us 高电平 标置信号结束*/

		/*开始接收数据*/   
		DHT11_Data->humi_int= Read_Byte();
		DHT11_Data->humi_deci= Read_Byte();
		DHT11_Data->temp_int= Read_Byte();
		DHT11_Data->temp_deci= Read_Byte();
		DHT11_Data->check_sum= Read_Byte();
	
		DHT11_Mode_Out_PP();/*读取结束，引脚改为输出模式*/		
		DHT11_DATA_OUT(HIGH);/*主机拉高*/

		/*检查读取的数据是否正确*/
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


DHT11_Data_T dht11_t = {30,70};		//默认值
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
