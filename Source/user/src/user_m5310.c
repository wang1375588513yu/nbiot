#include "user_m5310.h"
#include "timer.h"
#include "PUBLIC.H"
#include "common.h"
#include "led.h"
#include "usart.h"
#include "m5310_hard.h"
#include "m5310_onenet.h"
#include "nbiot_power.h"
#include "dht11.h"


#define NBIOT_Printf(fmt, args...)	 	do{uart2_printf(fmt,##args);}while(0)		//����1��MCU����NBģ��Ĵ���
#define NBIOT_DEBUG(fmt, args...) 		do{uart1_printf(fmt,##args);} while(0)		//��λ����ӡ��Ϣ�˿�

#define parse_at_commard
#define NBIOT_USART					USART2
#define DEBUG_USART					USART1

#define NBIOT_AT_ENDSTR 			("\r\n")
#define NBIOT_ERROR_CNT_MAX			20
#define NBIOT_ERROR_CNT_MIN			10
#define SIMCCIDLEN					15
#define IMEILEN						15


NBIOT_AT_Step_t nbiot_at_step = NBIOT_RESET;
Pipe1_t *nbiot_recv_pipe=NULL;

char simiccidstring[SIMCCIDLEN+1] = {0};		//SIM��id��Ҳ����IMSI
char nbiotIMEIstring[IMEILEN+1] = {0};			//ģ��IMEI

unsigned int  nbiot_run_timespan=0;				//�Ӹ�λ����ǰ��ʱ������������
unsigned int  nbiot_at_delay = 0xfffffff;			//AT������ʱ

unsigned char nbiot_signal_strenth_rssi=0;		//�����ź�ǿ��
unsigned char nbiot_signal_strenth_ber=0;		//�źŵȼ�
unsigned char network_registration_status = 0;	//ע�������״̬
unsigned char nbiot_logic_onenet_timeout = 0;	//��½��ʱ�¼�
unsigned char nbiot_close_event = 0;			//�ر��¼�
unsigned char upload_data_cnt = 0;				//�ϱ����ݼ���
unsigned char nbiot_ref_id = 0;					//�豸ͨ��ʵ��ID

bool nbiot_close_success_flag = false;			//�رճɹ���־
bool nbiot_login_onenet_flag = false;			//��½�ɹ���־
bool nbiot_send_request=true;					//һ��ʼ�ȴ�PB DONE
bool simiccidgetok=false;						//ICCID��ȡ�ɹ���־
bool isTrueRunATLoop = true;					//�Ƿ�������ִ��״̬���������ʱ��ͣ����ĳһ���������
bool nbiot_is_ready = false;					//����ONENET֮ǰ��״̬���Ƿ�����


void NBIOT_Pipe_Reset(void)
{
    Pipe1_Reset(nbiot_recv_pipe);
}

void NBIOT_Status_Reset(void)
{
    nbiot_run_timespan = 0;
    nbiot_at_delay = 0xfffffff;
    nbiot_send_request = true;
    nbiot_is_ready = false;
    nbiot_logic_onenet_timeout = 0;
    nbiot_close_event = 0;
    nbiot_login_onenet_flag = false;
    nbiot_close_success_flag = false;
	upload_data_cnt = 0;	
    nbiot_at_step = NBIOT_RESET;
}

static void NBIOT_power_reset(void)
{//��Դ��λ
	NBIOT_Status_Reset();
	nbiot_power_reset();
}

void NBIOT_init(void)
{
    uart_init(DEBUG_USART,115200);
    uart_init(NBIOT_USART,9600);
    nbiot_recv_pipe = &uart_pipe2;
	nbiot_power_init();
	NBIOT_power_reset();
    NBIOT_Pipe_Reset();
}

#ifdef parse_at_commard
static void Nbiot_AT_Reset(void)
{
//	nbiot_at_step = NBIOT_AT_TEST;
//	nbiot_at_delay = 1500;
	nbiot_send_request = false;
}
#if 0
static void Nbiot_Booting(void)
{
	if((u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"	M5310-A", 7))
	{
		nbiot_at_step = NBIOT_AT_TEST;
		nbiot_at_delay = 1500;
		nbiot_send_request = false;
	}
}
#endif

static void Nbiot_AT_Test(void)
{
    static u8 error_cnt = 0;
    if((u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"OK", 2))
    {
        isTrueRunATLoop = true;
        nbiot_at_delay = 500;
        //nbiot_at_step = NBIOT_AT_CFUN_READ;
		nbiot_at_step = NBIOT_AT_CSQ;
        nbiot_send_request = false;
    }
    else if((u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"ERROR", 5))
    {
        error_cnt++;
        if(error_cnt > NBIOT_ERROR_CNT_MAX)
        {
            error_cnt = 0;
            NBIOT_init();
        }
    }
}

static void Nbiot_AT_CGSN(void)		//��ȡģ���IMEI��ע��ƽ̨��
{
    static bool get_info_flag = false;
    u8 *find=NULL;
    //nbiotIMEIstring
    find = (u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"86", 2);
    if(find)
    {
        get_info_flag = true;
        memset((void*)nbiotIMEIstring,0,IMEILEN+1);
        memcpy((void*)nbiotIMEIstring,find,IMEILEN);
    }
    if((u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"OK", 2) && (get_info_flag == true))
    {
        nbiot_at_delay=500;
        nbiot_at_step=NBIOT_AT_CIMI;
        nbiot_send_request=false;
        simiccidgetok=true;
    }
}

static void Nbiot_AT_CIMI(void)		//��ȡSIM����IMSI��ע��ƽ̨��
{
    static bool get_info_flag = false;
    u8 *find=NULL;
    find=(u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"46", 2);
    if(find)
    {
        get_info_flag = true;
        memset((void*)simiccidstring,0,SIMCCIDLEN+1);
        memcpy((void*)simiccidstring,find,SIMCCIDLEN);
    }
    if((u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"OK", 2) && (get_info_flag == true))
    {
        nbiot_at_delay=500;
        nbiot_at_step=NBIOT_AT_CSQ;
        nbiot_send_request=false;
        simiccidgetok=true;
    }
}

static void Nbiot_AT_CSQ(unsigned char *tail)
{
    u8 *find=NULL,*data=NULL,dot=0;
    if(tail==NULL)return;
    static u8 resendcnt = 0;

    find=(u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"+CSQ:", 5);
    if(find)
    {
        data=find+5;
        nbiot_signal_strenth_rssi=NMEA_Str2num((const unsigned char*)data,10,&dot);
        find=(u8*)my_memmem(data, nbiot_recv_pipe->sum-(data-nbiot_recv_pipe->buf),",", 1);
        data=find+1;
        nbiot_signal_strenth_ber=NMEA_Str2num((const unsigned char*)data,10,&dot);
        if(nbiot_signal_strenth_rssi  <= 0 || nbiot_signal_strenth_rssi > 32)
        {
            nbiot_send_request=false;
            nbiot_at_step = NBIOT_AT_CSQ;
            nbiot_at_delay=500;
            resendcnt++;
            if(resendcnt > NBIOT_ERROR_CNT_MAX)
            {
                NBIOT_init();
                resendcnt = 0;
            }
        }
        else if(nbiot_signal_strenth_rssi == 99)
        {   //����ź�ǿ��Ϊ99��ֱ���Ǵ���״̬��Ӧ��ֱ������
            NBIOT_init();
            resendcnt = 0;
        }
        else if(nbiot_signal_strenth_rssi < 32 || nbiot_signal_strenth_rssi > 0)
        {
            nbiot_send_request=false;
            nbiot_at_step =	NBIOT_AT_CGREG;
            isTrueRunATLoop = true;
            resendcnt = 0;
            nbiot_at_delay=500;
        }
    }
}

static void Nbiot_AT_CGREG(unsigned char *tail)
{
    u8 *find=NULL,*data=NULL,dot=0;
    static unsigned int Errcnt = 0;
    if(tail==NULL)return;

    find=(u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"+CEREG:", 7);
    if(find)
    {
        data = find + 7;
        if(find)
        {
            find=(u8*)my_memmem(data, nbiot_recv_pipe->sum-(data-nbiot_recv_pipe->buf),",", 1);
            data=find+1;
            network_registration_status = NMEA_Str2num((const unsigned char*)data,10,&dot);
            if(network_registration_status == 1 || network_registration_status == 5)
            {
                nbiot_send_request=false;
                nbiot_at_step=NBIOT_AT_CGATT;
                isTrueRunATLoop = true;
                nbiot_at_delay=500;
                Errcnt = 0;
            }
            else
            {
                Errcnt++ ;
                if(Errcnt > 200)
                {
                    NBIOT_init();
                    Errcnt = 0;
                }
                else
                {
                    nbiot_at_step=NBIOT_AT_CGREG;

                }
                isTrueRunATLoop = false;
                nbiot_at_delay=500;
                return ;
            }
        }
    }
}

static void Nbiot_AT_CGATT(unsigned char *tail)
{
    static unsigned int Errcnt = 0;
    if(tail==NULL)return;

    if((u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"+CGATT:0", 8))
    {
        nbiot_send_request=false;
        nbiot_at_step=NBIOT_AT_CGATT;
        nbiot_at_delay=500;
    }
    else if((u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"+CGATT:1", 8))
    {
        nbiot_send_request=false;
        nbiot_at_step=NBIOT_AT_MIPLCREATE;
        nbiot_at_delay=500;
    }
    else
    {
        if(Errcnt > NBIOT_ERROR_CNT_MAX)
        {
            NBIOT_init();
            Errcnt = 0;
        }
    }
}
#ifdef Optional
static void Nbiot_AT_CSCON(void)
{
    if((u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"OK", 2))
    {
        nbiot_at_step=NBIOT_AT_CEREG_WRITE;
        nbiot_at_delay=500;
        nbiot_send_request=false;
    }
}

static void Nbiot_AT_CEREG(void)
{
    if((u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"OK", 2))
    {
        nbiot_at_step=NBIOT_AT_MIPLCREATE;
        nbiot_at_delay=500;
        nbiot_send_request=false;
    }
}
#endif

static void Nbiot_AT_MIPLCREATE(void)
{
	static unsigned char errcnt = 0;
	#if 0
    if((u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"+MIPLCREAT:0", 12))
	#else
	if((u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"OK", 2))
	#endif
    {
        nbiot_at_step=NBIOT_AT_MIPLADDOBJ;
        nbiot_at_delay=500;
        nbiot_send_request=false;
    }
	else if((u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"error", 5))
	{
		errcnt++;
		if(errcnt > NBIOT_ERROR_CNT_MAX)
		{
			NBIOT_init();
			errcnt = 0;
		}
	}
}

static void Nbiot_AT_MIPLADDOBJ(void)
{
    if((u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"OK", 2))
    {
        nbiot_at_step=NBIOT_AT_MIPLNOTIFY;
        nbiot_at_delay=500;
        nbiot_send_request=false;
    }
}

static void Nbiot_AT_MIPLNOTIFY(void)
{
    if((u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"OK", 2))
    {
        nbiot_at_step=NBIOT_AT_MIPLOPEN;
        nbiot_at_delay=500;
        nbiot_send_request=false;
    }
}

/*
+MIPLEVENT:0,6
+MIPLOBSERVE:0,23646,1,3303,0,-1
+MIPLOBSERVE:0,23647,1,3304,0,-1
+MIPLDISCOVER:0,23648,3303
+MIPLDISCOVER:0,23649,3304*/
unsigned char NBIOT_login_onenet_event = 0;		// ��½ONENET�¼�
static void Nbiot_AT_MIPLOPEN(void)
{
	static unsigned char errcnt = 0;

    if((u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"+MIPLEVENT:0,6", 14))
    {//+MIPLEVENT:0,6
		nbiot_login_onenet_flag = true;		//��½�ɹ�
		nbiot_at_step=NBIOT_NORMAL;			//���������������շ�����
		nbiot_at_delay=500;
		nbiot_send_request=false;

    }
	else if((u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"+MIPLEVENT:0,7", 14))
	{//��½�ɹ�
		nbiot_login_onenet_flag = false;	//��½ʧ��
		nbiot_at_step = NBIOT_RESET;
		nbiot_at_delay=500;
		nbiot_send_request=false;
		return;
	}
	
	if((u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"ERROR", 5))
	{
		errcnt++;
		if(errcnt > NBIOT_ERROR_CNT_MAX)
		{
			NBIOT_init();
			errcnt = 0;
			nbiot_login_onenet_flag = false;
		}
	}
}

static void Nbiot_AT_MIPLCLOSE(void)
{   //����OKֻ��ָ��ִ�гɹ����յ� +MIPLEVENT:0,15 ����ע���ɹ�
    if((u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"+MIPLEVENT:0,15", 15))
    {
		nbiot_close_success_flag = true;	//�رճɹ�
		nbiot_send_request=false;
		nbiot_at_step=NBIOT_AT_MIPLDELOBJ;
		nbiot_at_delay=500;
    }
}

static void Nbiot_AT_MIPLDELOBJ(void)
{
    if((u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"OK", 2))
    {
        nbiot_at_step=NBIOT_AT_MIPLDEL;
        nbiot_at_delay=500;
        nbiot_send_request=false;
    }
}

static void Nbiot_AT_MIPLDEL(void)
{
	if((u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum,"OK", 2))
    {
        nbiot_at_step=NBIOT_RESET;
        nbiot_at_delay=5000;
        nbiot_send_request=false;
    }
}

#endif

static void NBIOT_parse_AT_loop(void)
{
    u8 *find=NULL;
    if(nbiot_recv_pipe->sum==0)return;

    find=(u8*)my_memmem((const void*)nbiot_recv_pipe->buf, nbiot_recv_pipe->sum, NBIOT_AT_ENDSTR, 2);
    if(find)
    {
        if(nbiot_send_request)
        {
            switch(nbiot_at_step)
            {
            case NBIOT_RESET:
            {
                Nbiot_AT_Reset();
                break;
            }
			#if 0
			case NBIOT_BOOTING:
			{
				Nbiot_Booting();
			}
			#endif
            case NBIOT_AT_TEST:			//����ATָ��
            {
                Nbiot_AT_Test();
                break;
            }
            case NBIOT_AT_CGSN:			//��SIM��ID--�����
            {
                Nbiot_AT_CGSN();
                break;
            }
            case NBIOT_AT_CIMI:			//��SIM��ID--�����
            {
                Nbiot_AT_CIMI();
                break;
            }
            case NBIOT_AT_CSQ:			//��ѯ�ź�����
            {
                Nbiot_AT_CSQ(find);
                break;
            }
            case NBIOT_AT_CGREG:			//����ע��״̬
            {
                Nbiot_AT_CGREG(find);
                break;
            }
            case NBIOT_AT_CGATT:
            {
                Nbiot_AT_CGATT(find);
                break;
            }
			#ifdef Optional
            case NBIOT_AT_CSCON_WRITE:
            {
                Nbiot_AT_CSCON();
                break;
            }
            case NBIOT_AT_CEREG_WRITE:
            {
                Nbiot_AT_CEREG();
                break;
            }
			#endif
            case NBIOT_AT_MIPLCREATE:
            {
                Nbiot_AT_MIPLCREATE();
                break;
            }
            case NBIOT_AT_MIPLADDOBJ:
            {
                Nbiot_AT_MIPLADDOBJ();
                break;
            }
			case NBIOT_AT_MIPLNOTIFY:
            {
                Nbiot_AT_MIPLNOTIFY();
                break;
            }
            case NBIOT_AT_MIPLOPEN:
            {
                Nbiot_AT_MIPLOPEN();
                break;
            }
            case NBIOT_NORMAL:
            {
                break;
            }
			case NBIOT_AT_MIPLCLOSE:
            {
                Nbiot_AT_MIPLCLOSE();
                break;
            }
            case NBIOT_AT_MIPLDELOBJ:
            {
                Nbiot_AT_MIPLDELOBJ();
                break;
            }
            case NBIOT_AT_MIPLDEL:
            {
                Nbiot_AT_MIPLDEL();
                break;
            }
            default:
                break;
            }
        }
    }
	Pipe1_Remove(nbiot_recv_pipe,find-nbiot_recv_pipe->buf+2);
}

static void NBIOT_AT_loop(void)
{   /*����ATָ��*/
    static u32 bak_millisec_tick=0xffffffff;
    if(bak_millisec_tick==Get_SysmilliTick())return;
    bak_millisec_tick=Get_SysmilliTick();

    if(nbiot_at_delay)
	{
        nbiot_at_delay--;
		return;
	}
    else
    {
        nbiot_send_request=true;
        NBIOT_Pipe_Reset();
        switch(nbiot_at_step)
        {
        case NBIOT_RESET:
        {
            NBIOT_Printf("AT+NRB\r\n");
            nbiot_at_delay=1000;
            NBIOT_DEBUG("\r\nNBIOT_AT_RESET\r\n");
            break;
        }
        case NBIOT_AT_TEST:			//����ATָ��
        {
			Pipe1_Reset(nbiot_recv_pipe);
            NBIOT_Printf("AT\r\n");
            nbiot_at_delay=1000;
            NBIOT_DEBUG("\r\nNBIOT_AT_TEST\r\n");
            break;
        }
        case NBIOT_AT_CGSN:			//��SIM��ID--�����
        {
            NBIOT_Printf("AT+CGSN=1\r\n");
            nbiot_at_delay=1000;
            NBIOT_DEBUG("\r\nNBIOT_AT_CGSN\r\n");
            break;
        }
        case NBIOT_AT_CIMI:			//��SIM��ID--�����
        {
            NBIOT_Printf("AT+CIMI\r\n");
            nbiot_at_delay=1000;
            NBIOT_DEBUG("\r\nNBIOT_AT_CIMI\r\n");
            break;
        }
        case NBIOT_AT_CSQ:			//��ѯ�ź�����
        {
            NBIOT_Printf("AT+CSQ\r\n");
            nbiot_at_delay=1000;
            NBIOT_DEBUG("\r\nNBIOT_AT_CSQ\r\n");
            break;
        }
        case NBIOT_AT_CGREG:			//����ע��״̬
        {
            NBIOT_Printf("AT+CEREG?\r\n");
            nbiot_at_delay=1000;
            NBIOT_DEBUG("\r\nNBIOT_AT_CEREG\r\n");
            break;
        }
        case NBIOT_AT_CGATT:
        {
            NBIOT_Printf("AT+CGATT?\r\n");
            nbiot_at_delay=1000;
            NBIOT_DEBUG("\r\nNBIOT_AT_CGATT\r\n");
            break;
        }
		#ifdef Optional
        case NBIOT_AT_CSCON_WRITE:
        {
            NBIOT_Printf("AT+CSCON=1\r\n");
            nbiot_at_delay=1000;
            NBIOT_DEBUG("\r\nNBIOT_AT_CSCON_WRITE\r\n");
            break;
        }
        case NBIOT_AT_CEREG_WRITE:
        {
            NBIOT_Printf("AT+CEREG=1\r\n");
            nbiot_at_delay=1000;
            NBIOT_DEBUG("\r\nNBIOT_AT_CEREG_WRITE\r\n");
            break;
        }
		#endif
        case NBIOT_AT_MIPLCREATE:
        {
			NBIOT_Printf("AT+MIPLCREATE=49,130031F10003F2002304001100000000000000123138332E3233302E34302E34303A35363833000131F30008C000000000,0,49,0\r\n");
            nbiot_at_delay=2000;
            NBIOT_DEBUG("\r\nNBIOT_AT_MIPLCREATE\r\n");
            break;
        }
        case NBIOT_AT_MIPLADDOBJ:	//����ʵ��
        {
			#ifdef TEMPERATURE_HUMIDITY
			NBIOT_Printf("AT+MIPLADDOBJ=0,3303,1,\"1\",1,0\r\n");		//ע���¶�object
//			NBIOT_Printf("AT+MIPLADDOBJ=0,3304,1,\"1\",1,0\r\n");		//ע��ʪ��object
			#else
			NBIOT_Printf("AT+MIPLADDOBJ=0,3336,0,\"0\",0,0\r\n");
			#endif
            
            nbiot_at_delay=1000;
            NBIOT_DEBUG("\r\nNBIOT_AT_MIPLADDOBJ\r\n");
            break;
        }
		case NBIOT_AT_MIPLNOTIFY:
		{
			#ifdef TEMPERATURE_HUMIDITY
			NBIOT_Printf("AT+MIPLNOTIFY=0,0,3303,0,5700,4,4,\"37.5\",0,0\r\n");
//			NBIOT_Printf("AT+MIPLNOTIFY=0,0,3304,0,5700,4,4,\"80.0\",0,0\r\n");
			#else
			NBIOT_Printf("AT+MIPLNOTIFY=0,3336,0,5514,1,\"1,1,25.234,65.111,111.69871,29.04701\",1\r\n");
			#endif
            nbiot_at_delay=1000;
            NBIOT_DEBUG("\r\nNBIOT_AT_MIPLADDOBJ\r\n");
            break;
		}
        case NBIOT_AT_MIPLOPEN:	//��½
        {
            NBIOT_Printf(" AT+MIPLOPEN=0,3000,30\r\n");		//��½onenet��������ʱ��3000s��30s��δ���յ���½�ɹ�ָ��㳬ʱ
            nbiot_at_delay=1000;
            NBIOT_DEBUG("\r\nNBIOT_AT_MIPLOPEN\r\n");
            break;
        }
        case NBIOT_NORMAL:
        {
            nbiot_is_ready = true;
            break;
        }
		case NBIOT_AT_MIPLCLOSE://�ر�ONENET����
        {
            NBIOT_Printf("AT+MIPLCLOSE=0\r\n");
			nbiot_at_delay=3000;
            NBIOT_DEBUG("\r\nNBIOT_AT_MIPLOPEN\r\n");
            break;
        }
        case NBIOT_AT_MIPLDELOBJ:	//ɾ��ʵ��
        {
			#ifdef TEMPERATURE_HUMIDITY
			NBIOT_Printf("AT+MIPLDELOBJ=0,3303\r\n");
//			NBIOT_Printf("AT+MIPLDELOBJ=0,3304\r\n");
			#else
            NBIOT_Printf("AT+MIPLDELOBJ=0,3336\r\n");
			#endif
            break;
        }		
		case NBIOT_AT_MIPLDEL:	//ɾ��ʵ��
        {
            NBIOT_Printf("AT+MIPLDEL=0\r\n");
            break;
        }   
        default:
            break;
        }
    }
}

/*******************************************************************************
** FunctionName:  	NBIOT_loop
** Description: 	loop�������������������ÿ���һ��
** Parameters:		None
** Returned:		None
** The Author:		WangYu
** Creation Date:	2018-11-26
********************************************************************************/
static void NBIOT_loop(void)
{
    static unsigned int timersec=0xffffffff;
    if(timersec==Get_SysTick())return;
    timersec=Get_SysTick();		/*1s����һ��*/

    nbiot_run_timespan++;
	nbiot_power_loop();
	
	if(nbiot_run_timespan > 10 && (nbiot_at_step == NBIOT_RESET))
    {
        NBIOT_DEBUG("gprs_at_step = NBIOT_AT_TEST\r\n");
		Pipe1_Reset(nbiot_recv_pipe);//�����������Ϣ
		nbiot_at_step = NBIOT_AT_TEST;
        nbiot_at_delay = 2000;
    }

    if(nbiot_at_step == NBIOT_AT_MIPLOPEN)
    {   //��½������30�볬ʱ
        nbiot_logic_onenet_timeout++;
        if(nbiot_logic_onenet_timeout > 30)
        {
            nbiot_login_onenet_flag = true;
        }
    }
    else
    {
        nbiot_logic_onenet_timeout = 0;
    }
}

DHT11_Data_T* dht11_data;
static void NBIOT_Report_Data(void)
{   //�ϱ����ݺ���
	#ifdef TEMPERATURE_HUMIDITY
	dht11_data = Get_Dht11_Data();
	NBIOT_Printf("AT+MIPLNOTIFY=0,0,3303,0,5700,4,4,\"33.5\",0,0\r\n");		//�ϱ��¶�����, dht11_data->temperature
//	NBIOT_Printf("AT+MIPLNOTIFY=0,0,3304,0,5700,4,4,\"%s\",0,0", dht11_data->humidity);			//�ϱ�ʪ������
	#else
	
	#endif
}


unsigned int nbiot_upload_loop_delay = 0;		//�ϱ�����loop ��ʱ������ʱ��0���ϱ�һ��
void NBIOT_updata_loop(void)
{
	static unsigned int timersec=0xffffffff;
    if(timersec==Get_SysTick())return;
    timersec=Get_SysTick();		/*1s����һ��*/
	
	if(nbiot_login_onenet_flag == false)return;
	
	if(nbiot_upload_loop_delay)
	{
		nbiot_upload_loop_delay--;
		return;
	}
	else
	{
		NBIOT_Report_Data();		//�ϱ�һ������
		upload_data_cnt++;
		nbiot_upload_loop_delay = 10;
		if(upload_data_cnt > 20)
		{
			upload_data_cnt = 0;
			nbiot_at_step = NBIOT_AT_MIPLCLOSE;
			nbiot_at_delay = 1000;
			Pipe1_Reset(nbiot_recv_pipe);
		}
	}
}

/*******************************************************************************
** FunctionName:  	NBIOT_task
** Description: 	���������ṩ���������Ľӿں���
** Parameters:		None
** Returned:		None
** The Author:		WangYu
** Creation Date:	2018-11-26
********************************************************************************/
void NBIOT_task(void)
{
    NBIOT_loop();
	if(nbiot_at_step != NBIOT_NORMAL)
	{//û�е�¼��ʱ��ȥִ��atָ��
		NBIOT_parse_AT_loop();	//����ģ��Ļ�Ӧ��Ϣ
		NBIOT_AT_loop();			//����ATָ��
	} 
	else
	{//��¼�ɹ���������ģʽ��������
		NBIOT_updata_loop();	
	}
    NBIOT_normal_output();
}

NBIOT_AT_Step_t Get_AT_Step(void)
{
    return nbiot_at_step;
}

bool Get_login_success_flag(void)
{
    return nbiot_login_onenet_flag;
}

