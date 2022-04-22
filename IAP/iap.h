#ifndef __IAP_H__
#define __IAP_H__
#include "sys.h"  
#include "usart.h"
#include <stdio.h>

#define WaitTimeMs 120 //等待时间，120s，boot程序启动后未在该时间内发起通讯自动跳转进入app程序

//用户根据自己的需要设置
#define STM32_FLASH_SIZE 512 	 		//所选STM32的FLASH容量大小(单位为K)

#if STM32_FLASH_SIZE < 256
#define STM_SECTOR_SIZE 1024 //字节
#else
#define STM_SECTOR_SIZE 2048
#endif

#define TIMEOUT (120) 
//////////////////////////////////////////////////////////////////////////////////////////////////////

//FLASH起始地址
#define STM32_FLASH_BASE    0x08000000   	//STM32 FLASH的起始地址
#define VERINFO_Length   		0x08 				 //存放版本号FLASH的长度
#define FLASH_APP_ADDR		0x08008000  	//应用程序起始地址(存放在FLASH)
											//保留0X08000000~0X08003FFF的空间为Bootloader使用（16KB）	
                                            
typedef  void (*iapfun)(void);				//定义一个函数类型的参数.   
//void (*iapfun)(void);				//定义一个函数类型的参数.   
void iap_load_app(u32 appxaddr);			//跳转到APP程序执行
char iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 applen);	//在指定地址开始,写入bin


typedef struct 
{
	unsigned int HEAD;					//包头
	unsigned int PKG_Count; 		//总包数
	unsigned int PKG_SunCount;	//子包数
	unsigned int Length;				//数据包长度
	
	unsigned int CRC32;					//校验和
	unsigned int END;						//包尾
	
}PKG_Data;

typedef struct 
{	
		unsigned int HEAD;				//包头
		unsigned int Software_Ver;//数据包版本号
		unsigned int Compiler_Date;//数据包生产日期
		unsigned int PKG_Count;		//总包数
		unsigned int Bank[5];			//备用
		unsigned int CRC32;				//校验和
		unsigned int END;					//包尾
}XML_Data;


typedef struct
{
	char * ptrstart;
	char * ptrend;
	int length;
}ptr_struct;

#endif







































