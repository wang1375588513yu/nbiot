#include "stmflash.h"
#include <string.h>
#include "stm32f10x_flash.h"

//��ȡָ����ַ�İ���(16λ����)
//faddr:����ַ(�˵�ַ����Ϊ2�ı���!!)
//����ֵ:��Ӧ����.
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
    return *(vu16*)faddr;
}
#if STM32_FLASH_WREN	//���ʹ����д   
//������д��
//WriteAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{
    u16 i;
    for(i=0; i<NumToWrite; i++)
    {
        FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
        WriteAddr+=2;//��ַ����2.
    }
}
//��ָ����ַ��ʼд��ָ�����ȵ�����
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
//pBuffer:����ָ��
//NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���.)
#if STM32_FLASH_SIZE<=256
#define STM_SECTOR_SIZE 1024 //�ֽ�
#else
#define STM_SECTOR_SIZE	2048
#endif
#define STM_SECTOR_MAX (((1024*STM32_FLASH_SIZE)/STM_SECTOR_SIZE)-1)

u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];//�����2K�ֽڣ�����2����Ϊ�������������������u16,STMFLASH_BUF�ֽ���������STM_SECTOR_SIZE
u16 STMFLASH_CMP[STM_SECTOR_SIZE/2];

char STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{
    u32 secpos;	   //������ַ
    u16 secoff;	   //������ƫ�Ƶ�ַ(16λ�ּ���)
    u16 secremain; //������ʣ���ַ(16λ�ּ���)
    u16 i;
    u32 offaddr;   //ȥ��0X08000000��ĵ�ַ
    char result=0,neederase=0;
    if(NumToWrite==0)return 0;
    if(WriteAddr%2!=0||WriteAddr<STM32_FLASH_BASE||pBuffer==NULL||(WriteAddr+2*NumToWrite>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return 0xFF;//�Ƿ���ַ

    if(memcmp((const void*)WriteAddr,(const void*)pBuffer,2*NumToWrite)==0)
    {   //Ҫд��������ԭʼ�����һģһ����������д
        return 0;
    }

    offaddr=WriteAddr-STM32_FLASH_BASE;		//ʵ��ƫ�Ƶ�ַ.
    secpos=offaddr/STM_SECTOR_SIZE;			//������ַ  0~127 for STM32F103RBT6
    secoff=(offaddr%STM_SECTOR_SIZE)/2;		//�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
    secremain=STM_SECTOR_SIZE/2-secoff;		//����ʣ��ռ��С
    if(NumToWrite<=secremain)secremain=NumToWrite;//�����ڸ�������Χ
    while(1)
    {
        //����Ӧ�����ж�������ַ�Ƿ񳬳������������
        if(secpos>STM_SECTOR_MAX)
        {
            result= 0xFF;
            break;
        }
        memset((void*)STMFLASH_BUF,0xCC,STM_SECTOR_SIZE);
        //����һ����������
        if(STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2)!=0)//������������������
        {
            result= 0xFF;
            break;
        }
        neederase=0;
        for(i=0; i<secremain; i++) //��鼴��Ҫд���Flash�洢״��
        {
            //ֻҪ����һ�η�ȫFF����Ҫ����
            if(STMFLASH_BUF[secoff+i]!=0XFFFF)
            {
                neederase=1;//��Ҫ����
            }
            //Ϊ�˷���Ƚϱ���д���������ض������Ƿ�һ�����ж��Ƿ�д��ɹ�����Ҫȫ������
            STMFLASH_BUF[i+secoff]=pBuffer[i];//����
        }

        if(neederase==1)//��Ҫ����
        {   //ֻ��д��ʱ����Ҫ��������������д��֮����������֮ǰ���ܽ��ж�����
            FLASH_Unlock();	//����
            FLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//�����������
            STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//д����������
            FLASH_Lock();//����
        }
        else
        {
            FLASH_Unlock();	//����
            STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������.
            FLASH_Lock();//����
        }
        memset((void*)STMFLASH_CMP,0x00,STM_SECTOR_SIZE);
        //���¶���������
        if(STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_CMP,STM_SECTOR_SIZE/2)!=0)//������������������
        {
            result= 0xFF;
            break;
        }
        if(memcmp((const void*)STMFLASH_BUF,(const void*)STMFLASH_CMP,STM_SECTOR_SIZE)!=0)
        {
            result= 0xFF;
            break;
        }


        if(NumToWrite==secremain)break;//д�������
        else//д��δ����
        {
            secpos++;				//������ַ��1
            secoff=0;				//ƫ��λ��Ϊ0
            pBuffer+=secremain;  	//ָ��ƫ��
            WriteAddr+=secremain*2;	//д��ַƫ��(���ֽ���ƫ��secremain*2��)
            NumToWrite-=secremain;	//�ֽ�(16λ)���ݼ�
            if(NumToWrite>(STM_SECTOR_SIZE/2))
            {   //δд�����������������һ�������Ŀռ�
                secremain=STM_SECTOR_SIZE/2;//��һ����������д����
            }
            else
            {   //���һ�������㹻д����Щ������
                secremain=NumToWrite;//��һ����������д����
            }
        }
    }

    return result;
}
#endif

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��
char STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)
{
    u16 i;

    if(NumToRead==0)return 0;
    if(ReadAddr%2!=0||pBuffer==NULL||((ReadAddr+2*NumToRead)>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return 0xFF;

    for(i=0; i<NumToRead; i++)
    {
        if(ReadAddr<STM32_FLASH_BASE||(ReadAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return 0xFF;//�Ƿ���ַ
        pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
        ReadAddr+=2;//ƫ��2���ֽ�.
    }
    return 0;
}


















