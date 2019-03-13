#include <string.h>
#include "config.h"
#include "main.h"
#include "chip_flash.h"
#include "ota_flash.h"
#include "common.h"
#include "timer.h"
#include "PUBLIC.H"



//#define OTA_DEBUG(fmt, args...) do {Bluetooth_Debug_info(fmt,##args);} while(0)
#define OTA_DELAY_MAX 300


//升级包的标识应该与引导程序日期以及产品类型相关，30100301(产品类型)  20181114(引导程序的版本日期)
//升级包第一标识---引导程序的版本日期
const unsigned char OTA_UPDATA_APPFLAG1[] = {0x20,0x18,0x11,0x14};
//升级包第二标识---产品类型
const unsigned char OTA_UPDATA_APPFLAG2[] = {0x30,0x10,0x03,0x01};


static ota_step_t ota_step_v=OTA_IDLE;
unsigned int app_length=0;
unsigned int need_length=0;
int update_No=-1;
unsigned int Full_crc=0;


unsigned int otarecvdelay=OTA_DELAY_MAX;
unsigned char softresetdelay=5;


void set_ota_step(ota_step_t s)
{
    ota_step_v=s;
}

ota_step_t get_ota_step(void)
{
    return ota_step_v;
}

char get_ota_rcv_data_persent(void)
{
    if(ota_step_v==OTA_SECOND_RCV_DATA)
    {
        if(need_length!=0)
        {
            return 100*app_length/need_length;
        }
        else return 0;
    }
    else if(ota_step_v==OTA_THIRD_COMPLISH||ota_step_v==OTA_END)
    {
        return 100;
    }
    else return 0;
}


void ota_loop(void)
{
    static u32 bak_sec_tick=0xffffffff;
    if(bak_sec_tick==Get_SysTick())  return;
    bak_sec_tick=Get_SysTick();

    if(ota_step_v==OTA_END)
    {
        if(softresetdelay)softresetdelay--;
        else
        {
            SoftReset();
        }
    }
    if(ota_step_v==OTA_SECOND_RCV_DATA ||ota_step_v==OTA_THIRD_COMPLISH || ota_step_v==OTA_END)
    {
        if(otarecvdelay)otarecvdelay--;
        else
        {
			ota_step_v=OTA_IDLE;
            otarecvdelay=OTA_DELAY_MAX;
        }
    }
}

#if 0
ota_status_t ota_start(unsigned int binfilesize,unsigned int firstflag,unsigned int secondflag)
{
#ifndef USE_BOOTLOADER
    return OTA_BOOTLOADER_NOT_EXIST;
#endif

    if(binfilesize>APP_MAX_SIZE)
    {
        return OTA_BINFILE_TOOBIG;
    }

    if(binfilesize%4!=0)
    {
        return OTA_LENGTH_ERROR;
    }
    if(0!=memcmp((const void*)&firstflag,(const void*)OTA_UPDATA_APPFLAG1,sizeof(unsigned int)))
        return OTA_BIN_ERROR;
    if(0!=memcmp((const void*)&secondflag,(const void*)OTA_UPDATA_APPFLAG2,sizeof(unsigned int)))
        return OTA_BIN_ERROR;

    ota_step_v=OTA_SECOND_RCV_DATA;
    app_length=0;
    update_No=-1;
    need_length=binfilesize;
    otarecvdelay=OTA_DELAY_MAX;
    Full_crc=0;
    return OTA_SUCCESS;
}
#else

ota_status_t ota_start(unsigned int binfilesize)
{
    if(binfilesize>APP_MAX_SIZE)
    {
        return OTA_BINFILE_TOOBIG;
    }
    if(binfilesize%4!=0)
    {
        return OTA_LENGTH_ERROR;
    }
    ota_step_v=OTA_SECOND_RCV_DATA;
    app_length=0;
    update_No=-1;
    need_length=binfilesize;
    otarecvdelay=OTA_DELAY_MAX;
    Full_crc=0;
	
    return OTA_SUCCESS;
}
#endif

ota_status_t ota_rcv_data(const unsigned short updata_curnum,const unsigned char xorcode,const char *pdata,unsigned int datalen)
{
    unsigned char chksum=0;
  unsigned int i=0;	
    if(ota_step_v!=OTA_SECOND_RCV_DATA)
        return OTA_STEP_ERROR;

    if(pdata==NULL||datalen<4||datalen%4!=0)
        return OTA_LENGTH_ERROR;

    for(i=0; i<datalen; i++) chksum^=pdata[i];

    if(chksum!=xorcode)
        return OTA_XOR_ERROR;


    if(update_No==updata_curnum)
    {
        //OTA_DEBUG("\r\n In %d:resend!\r\n",update_No);
        return OTA_SUCCESS;
    }

    if((update_No+1)!=updata_curnum)
    {
        //OTA_DEBUG("\r\n In %s:--OTA_LOST_PACKET--\r\n",__FUNCTION__);
        return OTA_LOST_PACKET;
    }
  i=FROMAPP_START_ADDR;
	i+=app_length;
  if(0!=chip_write_flash(i,(unsigned short *)(pdata),datalen/2))
        return OTA_FLASH_FAIL;
    update_No++;
    //OTA_DEBUG("\r\n update_No %d:ok!\r\n",update_No);
    app_length+=datalen;

    if(app_length==need_length)
    {
        otarecvdelay=OTA_DELAY_MAX;
    }

    otarecvdelay=OTA_DELAY_MAX;
    return OTA_SUCCESS;
}

ota_status_t ota_complish(void)
{
    unsigned int calculate_crc=0,read_crc,flagcode;
    flash_header_t flash_header;
    otarecvdelay=OTA_DELAY_MAX;
    if(ota_step_v!=OTA_SECOND_RCV_DATA)
        return OTA_STEP_ERROR;

    if(need_length!=app_length||app_length<16)
        return OTA_LENGTH_ERROR;

    if(chip_read_flash(FROMAPP_START_ADDR+app_length-16,(unsigned short *)&flagcode,2)!=0)
        return OTA_FLASH_FAIL;

    if(0!=memcmp((const void*)&flagcode,(const void*)OTA_UPDATA_APPFLAG1,4))
        return OTA_BIN_ERROR;

    if(chip_read_flash(FROMAPP_START_ADDR+app_length-8,(unsigned short *)&flagcode,2)!=0)
        return OTA_FLASH_FAIL;

    if(0!=memcmp((const void*)&flagcode,(const void*)OTA_UPDATA_APPFLAG2,4))
        return OTA_BIN_ERROR;

    if(chip_crc_flash(FROMAPP_START_ADDR,app_length-16,&calculate_crc)!=0)
        return OTA_FLASH_FAIL;

    if(chip_read_flash(FROMAPP_START_ADDR+app_length-12,(unsigned short *)&read_crc,2)!=0)
        return OTA_FLASH_FAIL;

    if(read_crc!=calculate_crc)
        return OTA_CRC_ERROR;

    flash_header.Image_checkSum=calculate_crc;
    flash_header.Image_size=app_length;
    flash_header.Image_update_flags=IMAGE_STATE_NEED_UPDATE;

 //   chip_erase_flash(APP_HEADER_START_ADDR,1);
    if(chip_write_flash(APP_HEADER_START_ADDR,(unsigned short *)&flash_header,sizeof(flash_header_t)/2)!=0)
        return OTA_FLASH_FAIL;
    //OTA_DEBUG("\r\n %s\r\n",__FUNCTION__);
    ota_step_v=OTA_END;
    softresetdelay=5;

    return OTA_SUCCESS;
}


