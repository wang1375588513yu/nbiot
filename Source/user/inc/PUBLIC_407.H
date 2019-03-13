#ifndef _PUBLIC_H_
#define _PUBLIC_H_
#include "sys.h"
#include <string.h>
#include "common.h"
///#include "gps.h"

#define VERSON_HIGH 0X00
#define VERSON_LOW  (25)

#define SUPORT_ONENET_MQTT_TEST 


#define bool unsigned char
#define false 0
#define true 1
	
#define  PUBLIC_SEND_BUFSIZE	  10240 

#pragma pack(1)
typedef struct PIPE_TYPE
{
	unsigned char *buf;  //�����Ϣ��buffer
	volatile unsigned short size;//buf�Ĵ�С
	volatile unsigned short pin;   //��ǰ����buffer��λ��
	volatile unsigned short pout;  //��ǰ��buffer��λ��
	volatile unsigned short sum;     //��ǰbuffer��ʣ���ֽ���
	volatile unsigned int	time;//���һ�ν���/�������ݵ�ʱ��;(����)	
}Pipe_t;
#pragma pack()
   




#pragma pack(1)  //����ṹ�����1�ֽڶ��뷽ʽ����

typedef struct SERVER_INFO
{
	unsigned int ip;
	unsigned short port;
}Server_Info_t;

typedef Server_Info_t Remote_Info_t;

typedef struct WIFI_INFO
{
	char ssid[32+1];
	char password[64+1];
}Wifi_Info_t;

typedef struct BLUETOOTH_INFO
{
	char name[18+1];
	char password[6+1];
	char matchstyle;
}Bluetooth_Info_t;

typedef enum DTU_ROLE_TYPE
{
	DTU_ROLE_UNKNOW=0,
	DTU_ROLE_COORDINATOR=1,
	DTU_ROLE_TERMINAL=2,
	DTU_ROLE_REPEATOR=3,
	DTU_ROLE_TERMINAL_AND_REPEATER=4,
}dtu_role_type_t;

typedef struct DTU_CINFIG_EEPROM
{
	unsigned short netaddress;
	unsigned short locaoladdress;
	unsigned short targetaddress;
	dtu_role_type_t  role;
}dtu_config_eeprom_t;

typedef struct DTU_CINFIG_INFO
{
	unsigned char  channel;
	unsigned short baudrate;
	unsigned char  powerlevel;
	dtu_config_eeprom_t eeprom;
}dtu_config_info_t;

typedef struct WIFI_CONFIG_INFO
{
	unsigned char wifi_onOff;       
  unsigned char wifi_authtype; 
  unsigned char wifi_encryptmod;
  unsigned char wifi_mod;             
  unsigned char wifi_broadcast_onoff; 
  unsigned char wifi_channel;              
  unsigned char wifi_ssid[32+1];            
  unsigned char wifi_passwd[64+1];   	
}Wifi_Config_Info_t;


typedef struct POLLINGSTATUS
{
	bool Polling_enable;    //��ѯ�¼�������ʹ��
	bool Polling_alldone;   //
	bool Polling_Ack;       //��Ӧ���
	bool Polling_timeout_event; //��ʱ�¼�
	unsigned int Polling_delay;      //�ȴ�Ack����ʱ
	unsigned int Polling_timeout_cnt;//��ʱ����
	unsigned int Polling_id;	        //������ѯ��ID
	unsigned int Polling_Step;
}PollingStatus_t;

typedef struct WORKTIME
{
	unsigned int starttime;//������ʼʱ��
	unsigned int endtime;//��������ʱ��
	unsigned short startloopid;//��ѯ�м�ID
	unsigned short endloopid;//ĩѯ�м�ID
	unsigned short syncloopcount;//ͬʱ���вɼ����м���
	unsigned int interval;//������������ѯʱ��������
}WorkTime_t;

#define ONENET_INFO_API_ADDR_SIZE 100
#define ONENET_INFO_REGISTCODE_SIZE 100

typedef struct ONENET_INFO
{
  Server_Info_t server;
	unsigned int productid;
	unsigned char api_addr[ONENET_INFO_API_ADDR_SIZE];
  unsigned char regist_code[ONENET_INFO_REGISTCODE_SIZE];
}onenet_info_t;

#pragma pack()


#pragma pack(1)
typedef struct DTU_USER_FRAME_HEADER
{
	unsigned char start[2];
	unsigned char xorcode;
	unsigned long long imei;
	unsigned char datatype;
	unsigned short datalen;	
}dtu_user_frame_header_t;
#pragma pack()


#pragma pack(1)
typedef struct DEVICE_DATATYPE_0X22
{
	unsigned int time;
	unsigned long long imei;
	float pitch;
	float roll;
	float heading;
	unsigned char battery;
	float temperature;
}Device_Data_0X22_t;
#pragma pack()



#pragma pack(1) //�����ƴ�������OneNet��Ϣ��ʽ���ձ�
typedef struct DEVICE_DATA_CHECK_LIST
{
	unsigned long long imei;//������������������ַ
	unsigned short onenet_data_name;//OneNet��������000~999
	unsigned short onenet_data_subname;//���������
	unsigned char  onenet_data_base;// ���ݻ������ͣ�0���з������ͣ�1���޷������ͣ�2������������˫�����ԣ�
	unsigned char  onenet_data_accuracy;//������������С����λ����
}Device_data_check_list_t;
#pragma pack()



extern bool allneedtosleep;
extern unsigned char needtestcount;
extern bool remote_keep_control_enable;
extern unsigned int remote_keep_control_delay;
extern bool bluetoothautoswitch;

extern unsigned int dtuA_inbytecount;
extern unsigned int dtuB_inbytecount;

extern bool is_dtu_role(unsigned char role);

/*****************************************PUBLIC INTERFACE************************************************/

void SoftReset(void);
char* GetBuffer(void);
unsigned char CheckSum(const void *buf,int len);
unsigned char CheckXOR(const void *buf,int len);
void Pipe_Reset(Pipe_t *pipe);
void Pipe_Remove(Pipe_t *pipe,unsigned int len);
bool Pipe_Block_Input(Pipe_t *pipe,const char*packet,unsigned short packetlen);
bool Pipe_Check(Pipe_t *pipe);
long long NMEA_Pow(unsigned char m,unsigned char n);
long long NMEA_Str2num(const unsigned char *buf,unsigned char jinzhi,unsigned char *dx);
bool isBCDencodeing(unsigned long long BCDHEX);
unsigned long long HEX2DEC(unsigned long long BCDHEX);
unsigned long long DEC2HEX(unsigned long long DEC10);
unsigned short Caculate_CRC16(unsigned char *DAT, int Lenth);
unsigned char CRC8 ( uint8_t *data_in, uint16_t number_of_bytes_to_read );
unsigned long CalculateBlockCRC32(unsigned long ulCount, unsigned char *ucBuffer); 
bool IPaddressPhare(const char *string,unsigned char (*ip)[4], unsigned int len);

bool Wifi_Config_Info_Check(Wifi_Config_Info_t *wifi);

bool isDevice_data_check_list_t(const Device_data_check_list_t *plist);



#endif
