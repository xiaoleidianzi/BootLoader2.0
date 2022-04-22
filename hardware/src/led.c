/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	led.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2016-11-23
	*
	*	版本： 		V1.0
	*
	*	说明： 		LED初始化，亮灭LED
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//LED头文件
#include "led.h"


LED_STATUS led_status;


/*
************************************************************
*	函数名称：	Led_Init
*
*	函数功能：	LED初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		LED4-PC7	LED5-PC8	LED6-PA12	LED7-PC10
				高电平关灯		低电平开灯
************************************************************
*/
void Led_Init(void)
{
	
		//	GPIO_InitTypeDef gpio_initstruct;

		//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);	//打开GPIOA和GPIOC的时钟
		//	
		//	gpio_initstruct.GPIO_Mode = GPIO_Mode_Out_PP;									//设置为推挽输出模式
		//	gpio_initstruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 |GPIO_Pin_10;					//初始化Pin7、8、10
		//	gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;									//承载的最大频率
		//	GPIO_Init(GPIOC, &gpio_initstruct);												//初始化GPIOC
		//	
		//	gpio_initstruct.GPIO_Pin = GPIO_Pin_12;											//初始化Pin12
		//	GPIO_Init(GPIOA, &gpio_initstruct);												//初始化GPIOA
    
    LedF6_Set(LED_OFF);
    LedF7_Set(LED_OFF);
    LedF8_Set(LED_OFF);
    LedF9_Set(LED_OFF);

}

/*
************************************************************
*	函数名称：	Led4_Set
*
*	函数功能：	LED4控制
*
*	入口参数：	status：LED_ON-开灯	LED_OFF-关灯
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void LedF6_Set(LED_ENUM status)
{

	if(status == LED_ON)
	{
		GPIO_WriteBit(GPIOF, GPIO_Pin_6,Bit_SET);	//点灯
		//led_status.LedF6Sta = status;
	}
	else
	{
		GPIO_WriteBit(GPIOF, GPIO_Pin_6,Bit_RESET);//关灯
	}
}

/*
************************************************************
*	函数名称：	Led5_Set
*
*	函数功能：	LED5控制
*
*	入口参数：	status：LED_ON-开灯	LED_OFF-关灯
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void LedF7_Set(LED_ENUM status)
{

	if(status == LED_ON)
	{
		GPIO_WriteBit(GPIOF, GPIO_Pin_7,Bit_SET);	//点灯
		//led_status.LedF6Sta = status;
	}
	else
	{
		GPIO_WriteBit(GPIOF, GPIO_Pin_7,Bit_RESET);//关灯
	}
}

/*
************************************************************
*	函数名称：	Led6_Set
*
*	函数功能：	LED6控制
*
*	入口参数：	status：LED_ON-开灯	LED_OFF-关灯
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void LedF8_Set(LED_ENUM status)
{

	if(status == LED_ON)
	{
		GPIO_WriteBit(GPIOF, GPIO_Pin_8,Bit_SET);	//点灯
		//led_status.LedF6Sta = status;
	}
	else
	{
		GPIO_WriteBit(GPIOF, GPIO_Pin_8,Bit_RESET);//关灯
	}
}

/*
************************************************************
*	函数名称：	Led7_Set
*
*	函数功能：	LED7控制
*
*	入口参数：	status：LED_ON-开灯	LED_OFF-关灯
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void LedF9_Set(LED_ENUM status)
{

	if(status == LED_ON)
	{
		GPIO_WriteBit(GPIOF, GPIO_Pin_9,Bit_SET);	//点灯
		//led_status.LedF6Sta = status;
	}
	else
	{
		GPIO_WriteBit(GPIOF, GPIO_Pin_9,Bit_RESET);//关灯
	}

}
