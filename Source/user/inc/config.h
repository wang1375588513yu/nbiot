#ifndef __CONFIG___H__

#define __CONFIG___H__
#include "stmflash.h"
#include "PUBLIC.H"
//������ɱ�ʶ
#define IMAGE_STATE_COMPLISH_UPDATE     (0x5d5d5d5d)
//��Ҫ������ʶ
#define IMAGE_STATE_NEED_UPDATE         (0x55dd5d5d)
//����������ڱ�ʶ
#define IMAGE_BOOT_LOADER_EXIST         (0xBDDBDBBD)



//һҳ�Ĵ�С
#define FLASH_PAGE_SIZE                 (0x00000800)   ///2k
//�����������ʼ��ַ
#define BOOT_LOADER_START             	STM32_FLASH_BASE
//������������ռ�
#define BOOT_LOADER_MAX_SIZE          	(0x00002000)  //8k
//���ݶε���ʼ��ַ
#define APP_HEADER_START_ADDR          	(BOOT_LOADER_START+BOOT_LOADER_MAX_SIZE)
//���ݶεĿռ��С	2K
#define APP_HEADER_SIZE                	(FLASH_PAGE_SIZE)
//Ӧ�ó�����ʼ��ַ
#define APP_START_ADDR                 	(APP_HEADER_START_ADDR+APP_HEADER_SIZE)  //���ƫ��26k
//Ӧ�ó������ռ䣬��Flash��һ����Ϊ�ֽ��ߣ�ǰ��ΰ�������������������Ӧ�ó�����
#define APP_MAX_SIZE                   	(STM32_FLASH_SIZE*1024-BOOT_LOADER_MAX_SIZE-APP_HEADER_SIZE)/2  //(1024-8-2)/2=507k

//Flash�ĺ���Ϊ������
#define FROMAPP_START_ADDR             	(APP_START_ADDR +APP_MAX_SIZE )
#define FROMAPP_MAX_SIZE  				APP_MAX_SIZE 

#define FROMAPP_FLAG_BEFOURE

#define IMAGE_RW_PIECE_SIZE             (512)//(32)


//////////////////////////////
#define CONFIG_INFO_SIZE   (0X100)

typedef struct FLASH_HEADER
{
    unsigned int Image_size;//��������С
    unsigned int Image_checkSum;//��������CRCУ��ֵ
    unsigned int Image_update_flags;//��Ҫ������ʶ
    unsigned int Image_boot_loader_exist;//����������ڱ�ʶ
} flash_header_t;

#pragma pack(1)
typedef struct CONFIG_STRUCT
{
	flash_header_t flash_header;	
	unsigned long long imei;
	signed char zone;
	Server_Info_t server[2];
	HCM300B_Data_t hcm300b_data;
	
    unsigned char reserve2[
		(CONFIG_INFO_SIZE-1)-
		(sizeof(flash_header_t)+
		sizeof(unsigned long long)+
		sizeof(signed char)+
		2*sizeof(Server_Info_t)+
		sizeof(HCM300B_Data_t))];
		
    unsigned char xorcode;
} Config_Struct;
#pragma pack()

/////////////////////////////////////////////


extern unsigned int user_app_version;
extern unsigned int user_app_length;

void Config_Init(void);
bool Config_Backup(void);
void Config_Loop(void);


unsigned long long Config_Read_Device_IMEI(void);
bool Config_Write_Device_IMEI(unsigned long long*value);

char Config_Read_Zone(void);
bool Config_Write_Zone(char value);

void Config_Read_ServerInfo(char servernumber,Server_Info_t* value);
bool Config_Write_ServerInfo(char servernumber,Server_Info_t* value);

#endif
