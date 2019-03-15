#ifndef __USER_M5310_H__
#define __USER_M5310_H__

#include "sys.h"
#include "PUBLIC.H"

//#define Optional		//��ѡ��ָ��
#define ONENET_ENABLE
#define TCP_Mode	0
#define UDP_Mode	1
#define Use_Mode	UDP_Mode

#define TEMPERATURE_HUMIDITY

typedef enum NBIOT_AT_STEP_T
{
	NBIOT_RESET=0X00,			//����	
	NBIOT_AT_TEST,				//����ATָ��
	NBIOT_AT_CGSN,				//��ȡģ���IMEI
	NBIOT_AT_CIMI,				//��SIM����IMSI
	NBIOT_AT_CSQ,				//��ѯ�ź�����
	NBIOT_AT_CGREG,				//����ע��״̬
	NBIOT_AT_CGATT,				//��ѯ���總��״̬
	NBIOT_AT_MIPLCREATE,		//�豸ע��
	NBIOT_AT_MIPLADDOBJ,		//����ʵ��
	NBIOT_AT_MIPLNOTIFY,		//���� Resource 
	NBIOT_AT_MIPLOPEN,			//��½������
	NBIOT_NORMAL,				//�Ѿ���½ONENETƽ̨
	NBIOT_AT_MIPLCLOSE,			//��¼ע��
	NBIOT_AT_MIPLDELOBJ,		//ģ��ⶩ����Դ�ͷ�
	NBIOT_AT_MIPLDEL,			//ģ���ͨ��ʵ���ͷ�
}NBIOT_AT_Step_t;	

//AT+MIPLNOTIFY=0,0,3303,0,5700,4,4,\"33.3\",0,0
#define COMMAND_START		("AT+MIPLNOTIFY=")
#define COMMAND_END			("\r\n")
typedef struct 
{
	unsigned int  object_id;	
	unsigned int  resource_id;	//Resource ID 5700�Ǵ���������	
	unsigned int  data_len;		//���ݳ���
	char* value;				//���������͵���ֵ-----һ��Ĵ��������ݶ���float��
}NBIOT_MIPLNOTIFY_T;

extern Pipe1_t *nbiot_recv_pipe;
void NBIOT_init(void);
void NBIOT_task(void);
bool Get_login_success_flag(void);

#endif
