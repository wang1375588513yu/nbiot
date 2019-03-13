#include "config.h"
#include "user_config.h"
#include "timer.h"
#include "chip_flash.h"
#include "oscrc32.h"
#include "PUBLIC.h"

#include "ota_flash.h"

unsigned int user_app_version=0x01;
unsigned int user_app_length=0;

bool _checkApp(void)
{
#ifdef USE_BOOTLOADER
    volatile unsigned char*   __i;
    unsigned int   __tmpCrc32, __crc32Value;

    for(__tmpCrc32 = 0,user_app_length=0, __i = (unsigned char*)APP_START_ADDR; ; __i++,user_app_length++)
    {
        //FEED_DOG();
        if(
            (*(__i + 0)  == OTA_UPDATA_APPFLAG1[0]) &&
            (*(__i + 1)  == OTA_UPDATA_APPFLAG1[1]) &&
            (*(__i + 2)  == OTA_UPDATA_APPFLAG1[2]) &&
            (*(__i + 3)  == OTA_UPDATA_APPFLAG1[3]) &&
            (*(__i + 8)  == OTA_UPDATA_APPFLAG2[0]) &&
            (*(__i + 9)  == OTA_UPDATA_APPFLAG2[1]) &&
            (*(__i + 10) == OTA_UPDATA_APPFLAG2[2]) &&
            (*(__i + 11) == OTA_UPDATA_APPFLAG2[3])
        )
        {
            memcpy((unsigned char*)(&__crc32Value), (unsigned char*)(__i + 4), 4);
            if(__crc32Value == __tmpCrc32)
            {
                user_app_length+=16;
                memcpy((unsigned char*)(&user_app_version), (unsigned char*)(__i + 12), 4);
                return(true);
            }
            else
            {
                user_app_length=0;
                return(false);
            }
        }
        else if(( unsigned int)__i >(APP_START_ADDR+ APP_MAX_SIZE))
        {
            user_app_length=0;
            return(false);
        }
        else
        {
            __tmpCrc32 = u4_oscrc32(__tmpCrc32, (unsigned char*)__i, 1);
        }
    }
#else
    return false;
#endif
}

#ifdef USE_BOOTLOADER
#define CONFIG_INFO_START  (APP_HEADER_START_ADDR-CONFIG_INFO_SIZE)
#else
#define CONFIG_INFO_START  (FROMAPP_START_ADDR-CONFIG_INFO_SIZE)
#endif

#define CONFIG_DEBUG(fmt, args...) do {;} while(0)

bool config_ischange=false;
unsigned char config_delay=60;

Config_Struct config_info= {0};
Config_Struct config_compare= {0};
//由eeprom过渡到flash机制
//程序运行配置信息以eeprom为准，直接将eeprom备份至flash(先读出来，若一致则不需要重新备份)
//完全使用Flash备份机制
//1.上电初始化后，先读出Flash的之前的配置，如果发现校验和不匹配则，重新计算校验并立即备份
//2.原始信息被修改则1分钟无操作后自动备份

static bool Config_Read(Config_Struct* config)
{
    if(config==NULL)return false;
    return(0==chip_read_flash(CONFIG_INFO_START,(unsigned short *)config,sizeof(Config_Struct)/2));
}

static bool Config_Write(Config_Struct* config)
{
    unsigned char xorc=0,*p;
    unsigned short i=0;
    if(config==NULL)return false;
    if(!Config_Read(&config_compare))return false;

    p=(unsigned char*)config;

    i=sizeof(Config_Struct);

    for(i=0; i<sizeof(Config_Struct)-1; i++)
    {
        xorc^=*p++;
    }

    if(0==memcmp((const void*)&config_compare,(const void*)config,sizeof(Config_Struct)))
    {   //要写入的跟原来的信息一模一样
        if(config_compare.xorcode==xorc)
        {
            config_ischange=false;
            return true;
        }
    }
    config->xorcode=xorc;

    if(0!=chip_write_flash(CONFIG_INFO_START,(unsigned short *)config,sizeof(Config_Struct)/2))
        return false;

    if(!Config_Read(&config_compare))
        return false;

    if(0!=memcmp((const void*)&config_compare,(const void*)config,sizeof(Config_Struct)))
        return false;

    config_ischange=false;
    return true;
}

static void Config_Change_Event(void)
{
    config_ischange=true;
    config_delay=60;
}

unsigned int backupms=0;
unsigned int backupmsmax=0;
bool Config_Backup(void)
{
    bool ret=Config_Write(&config_info);
	backupms=Get_SysmilliTick();
	
    config_ischange=false;
    config_delay=60;
	backupms=Get_SysmilliTick()-backupms;
    backupmsmax=backupmsmax>backupms?backupmsmax:backupms;
    return	ret;
}

void Config_Init(void)
{
    Config_Read(&config_info);

    //启动默认强制设置
    config_info.server[0].ip=(unsigned int)182<<24|(unsigned int)92<<16|(unsigned int)130<<8|111;
    config_info.server[0].port=65039;
    config_info.server[1].ip=(unsigned int)101<<24|(unsigned int)200<<16|(unsigned int)138<<8|119;
    config_info.server[1].port=65039;
	
    Config_Backup();
    _checkApp();
}

void Config_Loop(void)
{
    static u32 bak_sec_tick=0xffffffff;
    if(bak_sec_tick==Get_SysTick())  return;
    bak_sec_tick=Get_SysTick();

    if(config_ischange)
    {
        if(config_delay)config_delay--;
        else
        {
            if(Config_Backup())
            {
                CONFIG_DEBUG((char*)"\r\nwrite  redundancy success\r\n");
            }
            else
            {
                CONFIG_DEBUG((char*)"\r\nwrite  redundancy fail\r\n");
            }
        }
    }
}

unsigned long long Config_Read_Device_IMEI(void)
{
#ifdef USE_BOOTLOADER
    if(isBCDencodeing(config_info.imei))
    {
        //掩码0xFFFFFFFFFFFF1F80
        return ((config_info.imei&0xFFFFFFFFFFFF3F81)|0x0000000000003081);
    }
    else
    {
        //乱码
        return 0x0000000000003081;
    }
#else
    //8130000000888888
    return 0x8888880000003081;

#endif
}

bool Config_Write_Device_IMEI(unsigned long long*value)
{
    if(value==NULL)return false;
    if(isBCDencodeing(*value))
    {
        if(config_info.imei==*value)return true;

        config_info.imei=*value;
        Config_Change_Event();
        return true;
    }
    else return false;
}

char Config_Read_Zone(void)
{
	return config_info.zone;
}

bool Config_Write_Zone(char value)
{
	if(config_info.zone==value)return true;
	
	config_info.zone=value;
	Config_Change_Event();
	return true;	
}

void Config_Read_ServerInfo(char servernumber,Server_Info_t* value)
{
    if(value==NULL)return;
    if(servernumber==1)
    {
        *value=config_info.server[0];
    }
    else if(servernumber==2)
    {
        *value=config_info.server[1];
    }
}

bool Config_Write_ServerInfo(char servernumber,Server_Info_t* value)
{
    if(value==NULL)return false;
    if(servernumber==1)
    {
        if(0==memcmp((const void*)&config_info.server[0],(const void*)value,sizeof(Server_Info_t)))
            return true;

        config_info.server[0]=*value;
        Config_Change_Event();
        return true;
    }
    else if(servernumber==2)
    {
        if(0==memcmp((const void*)&config_info.server[1],(const void*)value,sizeof(Server_Info_t)))
            return true;

        config_info.server[1]=*value;

        Config_Change_Event();
        return true;
    }
    return false;
}

