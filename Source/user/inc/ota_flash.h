#ifndef __OTA_FLASH__H__
#define __OTA_FLASH__H__

extern const unsigned char OTA_UPDATA_APPFLAG1[];
extern const unsigned char OTA_UPDATA_APPFLAG2[];

typedef enum OTA_STATUS
{
    OTA_SUCCESS=0x00,
    OTA_BOOTLOADER_NOT_EXIST=0x01,
    OTA_BINFILE_TOOBIG=0X02,
    OTA_LENGTH_ERROR=0X03,
    OTA_FLASH_FAIL=0X04,
    OTA_STEP_ERROR=0X05,
    OTA_XOR_ERROR=0X06,
    OTA_LOST_PACKET=0X07,
    OTA_BIN_ERROR=0X08,
    OTA_CRC_ERROR=0X09,
} ota_status_t;

typedef enum OTA_STEP
{
    OTA_IDLE=0,
    OTA_SECOND_RCV_DATA,
    OTA_THIRD_COMPLISH,
    OTA_END
} ota_step_t;


ota_step_t get_ota_step(void);
#if 0
ota_status_t ota_start(unsigned int binfilesize,unsigned int firstflag,unsigned int secondflag);

#else
ota_status_t ota_start(unsigned int binfilesize);

#endif


ota_status_t ota_rcv_data(unsigned short updata_curnum,const unsigned char xorcode,const char *pdata,unsigned int len);
ota_status_t ota_complish(void);

char get_ota_rcv_data_persent(void);

void ota_loop(void);

#endif
