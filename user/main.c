/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	main.c
	*
	*	作者： 		徐磊
	*
	*	日期： 		2022-03-08
	*
	*	版本： 		V1.0
	*
	*	说明： 		接入onenet，上传数据和命令控制
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//网络设备
#include "esp8266.h"

//网络协议层
#include "onenethttp.h"
#include "onenet.h"


//硬件驱动
#include "delay.h"
#include "beep.h"
#include "usart.h"
#include "i2c.h"
#include "sht20.h"
#include "led.h"
#include "timer.h"
//程序引导
#include "iap.h"
//C库
#include <string.h>

void GPIO_Configuration(void);
void RCC_Configuration(void);

void Hardware_Init(void);

extern void OneNET_Subscribe(const char * topic,...);
extern _Bool OneNet_DevLink(void);
extern void OneNet_SendData(void);
extern void OneNet_RevPro(unsigned char *cmd);
double power;
double temperatrue;

GPIO_InitTypeDef GPIO_InitStructure;

_Bool UPSTART=0;
char Pkg_Count = 0;								//应用程序升级总包数
char Pkg_Index=1;									//APPBINx.PKG、初始为APPBIN1.PKG
uint32_t APPLength = 0;					 	//APP数据有效长度
unsigned int Load_Finish;					//程序加载完成标志

extern PKG_Data AppPkg_Data;			//升级包信息
extern char Appdata_buf[20480];		//应用程序数据
unsigned int Flash_AddrOffset=0;	//FLASH数据写地址偏移
unsigned int Software_Ver = 0;		//数据包版本号
unsigned int Compiler_Date =0;		//数据包生产日期
char Xml_Err = 0;									//描述文件获取失败次数
XML_Data Xml_Data;								//描述文件信息
unsigned short BootStatus = 0;		//状态码表


/*
************************************************************
*	函数名称：	main
*
*	函数功能：	
*
*	入口参数：	无
*
*	返回参数：	0
*
*	说明：		
************************************************************
*/
int main(void)
{
	
	char DataStatus=0;							//数据状态
	char WriteFlashStatus = 0;			//写FLASH状态
	unsigned char *dataPtr = NULL;
	
	Hardware_Init();					//初始化外围硬件
	
	LedF6_Set(LED_OFF);				//提示接入成功
	
	DelayXms(250);						//延时250ms
	
	LedF6_Set(LED_ON);				//亮灯提示接入成功
	

	while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT")) //接入OneNET MQTTS服务器
	DelayXms(500);
	
	while(OneNet_DevLink())			//接入OneNET
	DelayXms(500);

	OneNET_Subscribe("$sys/%s/%s/cmd/#", PROID, DEVICE_NAME);					//向平台发送订阅请求(便于后续接收系统下发指令)	

	TIM3_Int_Init(9999,7199);//10Khz的计数频率，计数周期为1s
	
	while(1)
	{
			//接收ONENET平台指令
			dataPtr = ESP8266_MQTTIPD(10);
			if(dataPtr != NULL)
			{
				  OneNet_RevPro(dataPtr);	//解析ONENET数据
			}
			
				
			if(TickMs > TIMEOUT)
      {
            if(((*(vu32*)(FLASH_APP_ADDR +4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
            {
							
								BootStatus = bitop(BootStatus,0,1);//未收到MQTT下发的升级指令，加载旧代码进行跳转
                iap_load_app(FLASH_APP_ADDR);//执行FLASH APP代码
            }
            else
            {

                TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
                TIM_Cmd(TIM3, DISABLE);
								BootStatus = bitop(BootStatus,1,1);//未收到MQTT下发的升级指令，加载旧代码失败
                break;
            }
      }
			
			if(UPSTART)
      {
            UPSTART = 0;
						BootStatus = bitop(BootStatus,2,1);//收到MQTT下发的升级指令
						ESP8266_Clear();
				
						TickMs = 0;//重新计时
            break;
      }
						
			DelayXms(10);
	}
	
		//收到了开始升级指令，在应用程序升级服务器上获取升级包
		//获取描述文件
		do
		{
					ESP8266_Init();						//初始化ESP8266
					while(ESP8266_SendCmd(ESP8266_ONENET_INFO_APPbin, "CONNECT"))//连接HTTP服务器
					DelayXms(500);
					OneNet_SendData_Get(0);		
					DelayXms(1000);
					
					while(((dataPtr = ESP8266_GetIPD(0))==NULL)&&(TickMs < 20));
						 
					if(dataPtr != NULL)
					{
							DataStatus = GetXmlData_andCheck((char*)dataPtr,&Xml_Data);//解析应用程序升级包描述文件
							
							if(DataStatus == 1)
							{
								
									BootStatus = bitop(BootStatus,3,1);//程序升级包描述文件解析成功
								
									Pkg_Count = Xml_Data.PKG_Count;			 //获取总包数										
									Software_Ver = Xml_Data.Software_Ver; //获取升级包对应应用程序版本号
									Compiler_Date= Xml_Data.Compiler_Date;//获取升级包对应应用程序生成日期
								
									if(Pkg_Count == 0)
									{
											Xml_Err++;
									}
							}
							else
							{
									BootStatus = bitop(BootStatus,4,1);//程序升级包描述文件解析失败
									Xml_Err++;	
							}
					}		
		}while((Pkg_Count == 0)&&(Xml_Err<3)&&(TickMs < 20));//3次未读到则放弃或20s超时
		
	
		if(Pkg_Count != 0)//总包数不为0，则读取.pkg文件
		{
				do
				{			
									
					 OneNet_SendData_Get(Pkg_Index);
					
					 DelayXms(1000);
					
					 //接收HTTP数据 超时时间5s
					 while(((dataPtr = ESP8266_GetIPD(0))==NULL)&&(TickMs < 50));
					
					 
					 if(dataPtr != NULL)
					 {
								//解析HTTP数据
								DataStatus = OneNet_RevPro_HTTP(dataPtr);			
								
								BootStatus = bitop(BootStatus,5,1);//获取应用程序升级数据包成功
						 
								if(DataStatus == 1)//数据解析正确
								{
											BootStatus = bitop(BootStatus,6,1);//解析应用程序升级数据包正确
											char FlashWrite_cont = 0;
											for(FlashWrite_cont=0;FlashWrite_cont<3;FlashWrite_cont++)
											{
														WriteFlashStatus = iap_write_appbin(FLASH_APP_ADDR + Flash_AddrOffset ,(u8*)Appdata_buf+16 ,AppPkg_Data.Length);//更新FLASH代码;//写入flash
																			
														if(WriteFlashStatus == 1)//写FLASH成功
														{
																Flash_AddrOffset += AppPkg_Data.Length;//FLASH指针偏移到被写入数据区后
																Pkg_Index++;
																TickMs = 0;					//重新计时
																BootStatus = bitop(BootStatus,7,1);//应用程序升级数据包写FLASH成功
																break;
														}
														else//写FLASH失败
														{
																BootStatus = bitop(BootStatus,8,1);//应用程序升级数据包写FLASH失败;
														}
											}
											
											if(FlashWrite_cont == 3)
											{
													BootStatus = bitop(BootStatus,9,1);//写FLASH失败超3次，退出升级流程;
													break;//FLASH3次写失败，放弃升级，从备份程序中恢复上一版本
											}
								}
								else//数据解析错误
								{
										BootStatus = bitop(BootStatus,10,1);//解析应用程序升级数据包错误
								}
					 }
					 else
					 {
								BootStatus = bitop(BootStatus,11,1);//获取应用升级程序包超时
					 }	 
						
					 DelayXms(10);	
					
				}while((Pkg_Index<=Pkg_Count)&&(TickMs<TIMEOUT));
				
				
				
	}
	else//直接加载旧程序
	{
		
		BootStatus = bitop(BootStatus,12,1);//升级总包数为0，直接加载旧包
		
			if(((*(vu32*)(FLASH_APP_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
			{
					iap_load_app(FLASH_APP_ADDR);//执行FLASH APP代码	
			}
			else
			{
					;//加载程序失败;
			}
	}
	
	
	if(TickMs < TIMEOUT) //搬移FLASH完成后，加载升级后的应用程序
	{
			BootStatus = bitop(BootStatus,13,1);//程序升级完成，加载新程序
			
			if(((*(vu32*)(FLASH_APP_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
			{
					
					iap_load_app(FLASH_APP_ADDR);//执行FLASH APP代码
					
			}
			else
			{
					;//加载程序失败;
			}
	}
	else//超时，分两种情况：1.flash被重新擦写过；2.flash未被擦写
	{

			if(Pkg_Index > 1)//写入Flash成功过
			{
						BootStatus = bitop(BootStatus,14,1);//flash各别块被擦写，加载备份程序
						//备份程序代码
						// ......
						//
						iap_load_app(FLASH_APP_ADDR);//执行FLASH APP代码;
			}
			else
			{
					if(((*(vu32*)(FLASH_APP_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
					{
							BootStatus = bitop(BootStatus,15,1);//flash未被擦写，加载旧程序
							iap_load_app(FLASH_APP_ADDR);//执行FLASH APP代码
							
					}
					else
					{
							;//加载程序失败;
					}			
			}
	}
	
}
void RCC_Configuration(void)
{   

		ErrorStatus HSEstatue;
		//1 复位时钟
		RCC_DeInit();
	
		//2 HSE使能并等待其就绪
		RCC_HSEConfig(RCC_HSE_ON);
		HSEstatue = RCC_WaitForHSEStartUp();
	
		if(HSEstatue==SUCCESS)
		{
			//3 HSE使能预取值，配置等待周期
			FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
			FLASH_SetLatency(FLASH_Latency_2);
			
			//4 配置时钟来源和倍频系数8M×9=72M
			RCC_PLLConfig(RCC_PLLSource_HSE_Div1,RCC_PLLMul_9);
			
			//5 使能PLL并等待其稳定
			RCC_PLLCmd(ENABLE);
			while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY)==RESET);
			
			//6 选择系统时钟
			RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
			while(RCC_GetSYSCLKSource() != 0x08);
			
			//7 设置HCLK,PCLK2,PCLK2时钟
			RCC_HCLKConfig(RCC_SYSCLK_Div1);
			//8 设置APB1
			RCC_PCLK1Config(RCC_HCLK_Div2);
			//9 设置APB2
			RCC_PCLK2Config(RCC_HCLK_Div1);
			
		}
		else
		{
			//配置错误执行的代码块
		}

      /* Enable DMA1 clock */
      RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
      /* Enable ADC1, ADC2, ADC3 and GPIOC clocks */
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 |RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOF | RCC_APB2Periph_ADC1 , ENABLE);
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
			//GPIO重映射
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
		//内部时钟配置
		
//    RCC_DeInit();
//    // Enable HSI 
//    RCC_HSICmd(ENABLE);
//    //Wait till HSE is ready 
//    while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET){;}
//    if(1)
//    {
//      /* Enable Prefetch Buffer */
//      //FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

//      /* Flash 2 wait state */
//      //FLASH_SetLatency(FLASH_Latency_2);
// 
//      /* HCLK = SYSCLK */
//      RCC_HCLKConfig(RCC_SYSCLK_Div1); 
//  
//      /* PCLK2 = HCLK */
//      RCC_PCLK2Config(RCC_HCLK_Div1); 

//      /* PCLK1 = HCLK/2 */
//      RCC_PCLK1Config(RCC_HCLK_Div2);
//      /* ADCCLK = PCLK2/4 */
//      RCC_ADCCLKConfig(RCC_PCLK2_Div4);

//      /* PLLCLK = 8MHz /2* 14 = 64 MHz 内部晶振*/
//      RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_14);//RCC_PLLMul_14
//      /* Enable PLL */ 
//      RCC_PLLCmd(ENABLE);

//      /* Wait till PLL is ready */
//      while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
//      {
//      }
//      /* Select PLL as system clock source */
//      RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
//  
//      /* Wait till PLL is used as system clock source */
//      while(RCC_GetSYSCLKSource() != 0x08)
//      {
//      }
//    }

}

void GPIO_Configuration(void)
{

		GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
		//GPIO_Pin
  
		//UART1重映射 PB6、PB7
		/* Configure USART1 Tx (PB.6) as input floating */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//GPIO_Mode_AF_PP;//GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Configure USART1 Rx (PB.7) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
		
      /* Configure USART2 Tx (PA.02) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USART2 Rx (PA.03) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    //LED F6/7/8/9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
   
    /*SPI模拟接口*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;//miso
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;//sck
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//mosi
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
     
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;//nss
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//reset
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
		//ADC电压输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;//reset
    //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
		//ESP8266复位引脚
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;					//GPIOB8-复位
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
	
                 //SWJ_CFG
    //AFIO->MAPR =(0x02<<24);
    
    //GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
    // Configure PB.03 (JTDO) and PB.04 (JTRST) as output push-pull 

}

/*
************************************************************
*	函数名称：	Hardware_Init
*
*	函数功能：	硬件初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		初始化单片机功能以及外接设备
************************************************************
*/
void Hardware_Init(void)
{
	
	RCC_Configuration();
	
	GPIO_Configuration();
		
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断控制器分组设置
	NVIC_SetPriority (SVCall_IRQn,0xFF-1);  				//设置SVC NVIC中断
	
	Delay_Init();								    	//systick初始化
	
	Usart1_Init(115200);							//串口1，打印信息用
	
	Usart2_Init(115200);							//串口2，驱动ESP8266用
	
	Led_Init();										  	//LED初始化

	ESP8266_Init();						//初始化ESP8266
	
	UsartPrintf(USART_DEBUG, " Hardware init OK\r\n");
	
}



