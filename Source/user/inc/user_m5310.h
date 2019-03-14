#ifndef __USER_M5310_H__
#define __USER_M5310_H__

#include "sys.h"
#include "PUBLIC.H"

//#define Optional		//可选的指令
#define ONENET_ENABLE
#define TCP_Mode	0
#define UDP_Mode	1
#define Use_Mode	UDP_Mode

#define TEMPERATURE_HUMIDITY

typedef enum NBIOT_AT_STEP_T
{
	NBIOT_RESET=0X00,			//重启	
	NBIOT_AT_TEST,				//发送AT指令
	NBIOT_AT_CGSN,				//获取模块的IMEI
	NBIOT_AT_CIMI,				//读SIM卡的IMSI
	NBIOT_AT_CSQ,				//查询信号质量
	NBIOT_AT_CGREG,				//网络注册状态
	NBIOT_AT_CGATT,				//查询网络附着状态
	#ifdef Optional
	NBIOT_AT_NUESTATS,			//查询当前驻留小区相关信息
	NBIOT_AT_CSCON_WRITE,		//网络信号变更提示主动上报(可选)
	NBIOT_AT_CEREG_WRITE,		//注册状态主动上报(可选)
	#endif   
	NBIOT_AT_MIPLCREATE,		//设备注册
	NBIOT_AT_MIPLADDOBJ,		//创建实体
	NBIOT_AT_MIPLNOTIFY,		//订阅 Resource 
	NBIOT_AT_MIPLOPEN,			//登陆服务器
	NBIOT_NORMAL,				//已经登陆ONENET平台
	
	NBIOT_AT_MIPLCLOSE,			//登录注销
	NBIOT_AT_MIPLDELOBJ,		//模组测订阅资源释放
	NBIOT_AT_MIPLDEL,			//模组侧通信实体释放
}NBIOT_AT_Step_t;	



extern Pipe1_t *nbiot_recv_pipe;

void NBIOT_init(void);
void NBIOT_task(void);
bool Get_login_success_flag(void);

#endif
