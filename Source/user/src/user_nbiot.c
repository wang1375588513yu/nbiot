#include "user_nbiot.h"
#include "timer.h"
#include "PUBLIC.H"
#include "common.h"
#include "led.h"
#include "usart.h"
#include "m5310_hard.h"
#include "delay.h"

#if 0
Pipe1_t *nbiot_recv_pipe=NULL;

void NBIOT_Pipe_Reset(void)
{
    Pipe1_Reset(nbiot_recv_pipe);
}

void NBIOT_init(void)
{
    uart1_init(115200);
    uart5_init(9600);
    nbiot_recv_pipe = &uart_pipe5;
//    NBIOT_Status_Reset();
    NBIOT_Pipe_Reset();
}
//串口变量
uint16_t len = 0;

static unsigned char m5310_uart[300];//m5310串口输入字符串

int regstat;//M5310是否在ONENET平台注册成功

static char sendret[200];
extern int con;//sendcmd命令是否执行完毕,在主函数定义con，此处引用con变量

extern int menchiclear;//中断获取门磁状态，在主函数定义menchiclear，此处引用menchiclear变量
extern int zhengdongclear;//中断获取振动状态，在主函数定义zhengdongclear，此处引用zhengdongclear变量


/*
char* senddata()
{
    char menci[2];
    char zhendong[2];
    char* wenshi;
    char* gps;
    char rettemp[100];

    sprintf(menci,"%d",menchiclear);
    sprintf(zhendong,"%d",rumblestate());

//    wenshi=wenshidu();
//    gps=jingweidu();

//const char* test="AT+MIPLNOTIFY=0,3336,0,5514,1,\"1,1,25.234,65.111,111.12345,29.11111\",1\r\n";
    memset(sendret,0,200);
    strcat(sendret,"AT+MIPLNOTIFY=0,3336,0,5514,1,\"");
    strcat(sendret,menci);
    strcat(sendret,",");
	strcat(sendret,zhendong);
    strcat(sendret,",");
    strcpy(rettemp,wenshi);
    strcat(sendret,rettemp);
    strcat(sendret,",");
    strcpy(rettemp,gps);
    strcat(sendret,rettemp);
    strcat(sendret,"\",1\r\n");
    return sendret;

}*/

int m5310_receive(char *buf,char *cmd)//M5310返回数据与给定字符串进行比较
{
    if((uint8_t*)strstr((const char *)buf,(const char *)cmd)==NULL)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*
//发送命令给M5310，函数参数“待发送命令，串口返回的命令，延时值，返回命令返回值比对结果,发送次数”
void sendcmd(char *sendcommand,char *receivecommand,int delay1,int delay2,int back,int num)
{
    int i=1;

    memset(m5310_uart,0,300);
    if(con==1)
    {
        while(i<=num)
        {
            printf("%s",sendcommand);
            delay_ms(delay1);
            if(nbiot_recv_pipe->sum >= 1)
            {
                HAL_UART_Transmit(&huart1,(uint8_t *)usart1_rec_buffer,usart1_rcv_len,0xff);	//发送接收到的数据
                delay_ms(500);
                strcpy((char *)m5310_uart,(char *)usart1_rec_buffer);
                memset(usart1_rec_buffer,0,usart1_rcv_len);																		//清除接收的数据
                usart1_rcv_len = 0;																														//清空计数
                delay_ms(1000);
				delay_ms(2000);
            }
            if(m5310_receive((char *)m5310_uart,receivecommand)==back)
            {
                delay_ms(delay2);
                con =1;

                break;
            }
            else
            {
                con=0;
                i++;
            }
        }
    }
}*/

bool onenetconstate(bool con)//ONENET平台连接状态
{
    if(con == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}

#endif















