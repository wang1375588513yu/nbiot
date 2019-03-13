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
//���ڱ���
uint16_t len = 0;

static unsigned char m5310_uart[300];//m5310���������ַ���

int regstat;//M5310�Ƿ���ONENETƽ̨ע��ɹ�

static char sendret[200];
extern int con;//sendcmd�����Ƿ�ִ�����,������������con���˴�����con����

extern int menchiclear;//�жϻ�ȡ�Ŵ�״̬��������������menchiclear���˴�����menchiclear����
extern int zhengdongclear;//�жϻ�ȡ��״̬��������������zhengdongclear���˴�����zhengdongclear����


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

int m5310_receive(char *buf,char *cmd)//M5310��������������ַ������бȽ�
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
//���������M5310������������������������ڷ��ص������ʱֵ�����������ֵ�ȶԽ��,���ʹ�����
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
                HAL_UART_Transmit(&huart1,(uint8_t *)usart1_rec_buffer,usart1_rcv_len,0xff);	//���ͽ��յ�������
                delay_ms(500);
                strcpy((char *)m5310_uart,(char *)usart1_rec_buffer);
                memset(usart1_rec_buffer,0,usart1_rcv_len);																		//������յ�����
                usart1_rcv_len = 0;																														//��ռ���
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

bool onenetconstate(bool con)//ONENETƽ̨����״̬
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















