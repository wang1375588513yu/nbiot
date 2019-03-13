#include "stmflash.h"
#include <string.h>
#include "stm32f10x_flash.h"

//读取指定地址的半字(16位数据)
//faddr:读地址(此地址必须为2的倍数!!)
//返回值:对应数据.
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
    return *(vu16*)faddr;
}
#if STM32_FLASH_WREN	//如果使能了写   
//不检查的写入
//WriteAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{
    u16 i;
    for(i=0; i<NumToWrite; i++)
    {
        FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
        WriteAddr+=2;//地址增加2.
    }
}
//从指定地址开始写入指定长度的数据
//WriteAddr:起始地址(此地址必须为2的倍数!!)
//pBuffer:数据指针
//NumToWrite:半字(16位)数(就是要写入的16位数据的个数.)
#if STM32_FLASH_SIZE<=256
#define STM_SECTOR_SIZE 1024 //字节
#else
#define STM_SECTOR_SIZE	2048
#endif
#define STM_SECTOR_MAX (((1024*STM32_FLASH_SIZE)/STM_SECTOR_SIZE)-1)

u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];//最多是2K字节，除以2是因为这里的数组数据类型是u16,STMFLASH_BUF字节总数等于STM_SECTOR_SIZE
u16 STMFLASH_CMP[STM_SECTOR_SIZE/2];

char STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{
    u32 secpos;	   //扇区地址
    u16 secoff;	   //扇区内偏移地址(16位字计算)
    u16 secremain; //扇区内剩余地址(16位字计算)
    u16 i;
    u32 offaddr;   //去掉0X08000000后的地址
    char result=0,neederase=0;
    if(NumToWrite==0)return 0;
    if(WriteAddr%2!=0||WriteAddr<STM32_FLASH_BASE||pBuffer==NULL||(WriteAddr+2*NumToWrite>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return 0xFF;//非法地址

    if(memcmp((const void*)WriteAddr,(const void*)pBuffer,2*NumToWrite)==0)
    {   //要写的内容与原始保存的一模一样，无需重写
        return 0;
    }

    offaddr=WriteAddr-STM32_FLASH_BASE;		//实际偏移地址.
    secpos=offaddr/STM_SECTOR_SIZE;			//扇区地址  0~127 for STM32F103RBT6
    secoff=(offaddr%STM_SECTOR_SIZE)/2;		//在扇区内的偏移(2个字节为基本单位.)
    secremain=STM_SECTOR_SIZE/2-secoff;		//扇区剩余空间大小
    if(NumToWrite<=secremain)secremain=NumToWrite;//不大于该扇区范围
    while(1)
    {
        //这里应当先判断扇区地址是否超出了最大扇区数
        if(secpos>STM_SECTOR_MAX)
        {
            result= 0xFF;
            break;
        }
        memset((void*)STMFLASH_BUF,0xCC,STM_SECTOR_SIZE);
        //读出一个扇区数据
        if(STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2)!=0)//读出整个扇区的内容
        {
            result= 0xFF;
            break;
        }
        neederase=0;
        for(i=0; i<secremain; i++) //检查即将要写入的Flash存储状况
        {
            //只要出现一次非全FF就需要擦除
            if(STMFLASH_BUF[secoff+i]!=0XFFFF)
            {
                neederase=1;//需要擦除
            }
            //为了方便比较本次写入数据与重读数据是否一致来判断是否写入成功，需要全部复制
            STMFLASH_BUF[i+secoff]=pBuffer[i];//复制
        }

        if(neederase==1)//需要擦除
        {   //只有写的时候需要解锁上锁，而且写完之后，重新上锁之前不能进行读操作
            FLASH_Unlock();	//解锁
            FLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//擦除这个扇区
            STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//写入整个扇区
            FLASH_Lock();//上锁
        }
        else
        {
            FLASH_Unlock();	//解锁
            STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//写已经擦除了的,直接写入扇区剩余区间.
            FLASH_Lock();//上锁
        }
        memset((void*)STMFLASH_CMP,0x00,STM_SECTOR_SIZE);
        //重新读整个扇区
        if(STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_CMP,STM_SECTOR_SIZE/2)!=0)//读出整个扇区的内容
        {
            result= 0xFF;
            break;
        }
        if(memcmp((const void*)STMFLASH_BUF,(const void*)STMFLASH_CMP,STM_SECTOR_SIZE)!=0)
        {
            result= 0xFF;
            break;
        }


        if(NumToWrite==secremain)break;//写入结束了
        else//写入未结束
        {
            secpos++;				//扇区地址增1
            secoff=0;				//偏移位置为0
            pBuffer+=secremain;  	//指针偏移
            WriteAddr+=secremain*2;	//写地址偏移(按字节数偏移secremain*2个)
            NumToWrite-=secremain;	//字节(16位)数递减
            if(NumToWrite>(STM_SECTOR_SIZE/2))
            {   //未写完的数据容量超过下一个扇区的空间
                secremain=STM_SECTOR_SIZE/2;//下一个扇区还是写不完
            }
            else
            {   //最后一个扇区足够写入这些数据流
                secremain=NumToWrite;//下一个扇区可以写完了
            }
        }
    }

    return result;
}
#endif

//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数
char STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)
{
    u16 i;

    if(NumToRead==0)return 0;
    if(ReadAddr%2!=0||pBuffer==NULL||((ReadAddr+2*NumToRead)>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return 0xFF;

    for(i=0; i<NumToRead; i++)
    {
        if(ReadAddr<STM32_FLASH_BASE||(ReadAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return 0xFF;//非法地址
        pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//读取2个字节.
        ReadAddr+=2;//偏移2个字节.
    }
    return 0;
}


















