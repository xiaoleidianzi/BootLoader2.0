#ifndef __IAP_H__
#define __IAP_H__
#include "sys.h"  
#include "usart.h"
#include <stdio.h>

#define WaitTimeMs 120 //�ȴ�ʱ�䣬120s��boot����������δ�ڸ�ʱ���ڷ���ͨѶ�Զ���ת����app����

//�û������Լ�����Ҫ����
#define STM32_FLASH_SIZE 512 	 		//��ѡSTM32��FLASH������С(��λΪK)

#if STM32_FLASH_SIZE < 256
#define STM_SECTOR_SIZE 1024 //�ֽ�
#else
#define STM_SECTOR_SIZE 2048
#endif

#define TIMEOUT (120) 
//////////////////////////////////////////////////////////////////////////////////////////////////////

//FLASH��ʼ��ַ
#define STM32_FLASH_BASE    0x08000000   	//STM32 FLASH����ʼ��ַ
#define VERINFO_Length   		0x08 				 //��Ű汾��FLASH�ĳ���
#define FLASH_APP_ADDR		0x08008000  	//Ӧ�ó�����ʼ��ַ(�����FLASH)
											//����0X08000000~0X08003FFF�Ŀռ�ΪBootloaderʹ�ã�16KB��	
                                            
typedef  void (*iapfun)(void);				//����һ���������͵Ĳ���.   
//void (*iapfun)(void);				//����һ���������͵Ĳ���.   
void iap_load_app(u32 appxaddr);			//��ת��APP����ִ��
char iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 applen);	//��ָ����ַ��ʼ,д��bin


typedef struct 
{
	unsigned int HEAD;					//��ͷ
	unsigned int PKG_Count; 		//�ܰ���
	unsigned int PKG_SunCount;	//�Ӱ���
	unsigned int Length;				//���ݰ�����
	
	unsigned int CRC32;					//У���
	unsigned int END;						//��β
	
}PKG_Data;

typedef struct 
{	
		unsigned int HEAD;				//��ͷ
		unsigned int Software_Ver;//���ݰ��汾��
		unsigned int Compiler_Date;//���ݰ���������
		unsigned int PKG_Count;		//�ܰ���
		unsigned int Bank[5];			//����
		unsigned int CRC32;				//У���
		unsigned int END;					//��β
}XML_Data;


typedef struct
{
	char * ptrstart;
	char * ptrend;
	int length;
}ptr_struct;

#endif







































