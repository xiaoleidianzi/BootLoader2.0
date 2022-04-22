/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	onenet.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-05-09
	*
	*	版本： 		V1.1
	*
	*	说明： 		与onenet平台的数据交互接口层
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

//协议文件
#include "onenethttp.h"

//硬件驱动
#include "usart.h"
#include "delay.h"
#include "sht20.h"

//C库
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//stdlib.h
//HTTP
#define DEVID	"913689709"
#define APIKEY	"wW3Wt=53QZL=rwNtoEiYYBTJ=O4="

//应用程序数据结构体
PKG_Data AppPkg_Data= {0};
//应用程序数据数组
char Appdata_buf[20480] = {NULL};


void OneNet_FillBuf_Get(char *buf,char n)
{
//	char index[2];
//	char text[24];
//	char buf1[128];
	
//	memset(text, 0, sizeof(text));
//	memset(buf1, 0, sizeof(buf1));
//	
//	strcpy(buf1, "{");
//	
//	memset(text, 0, sizeof(text));
//	sprintf(text, "\"Temperature\":%0.2f,", sht20_info.tempreture);
//	strcat(buf1, text);
//	
//	memset(text, 0, sizeof(text));
//	sprintf(text, "\"Humidity\":%0.2f", sht20_info.humidity);
//	strcat(buf1, text);
//	
//	strcat(buf1, "}");
//	
//	sprintf(buf, "POST /devices/%s/datapoints?type=3 HTTP/1.1\r\napi-key:%s\r\nHost:api.heclouds.com\r\n"
//					"Content-Length:%d\r\n\r\n",
//	
//					DEVID, APIKEY, strlen(buf1));
//					
//	strcat(buf, buf1);

//	sprintf(buf, "GET /LoadApp.bin HTTP/1.1\r\nHost:636c-cloud1-8g5omron3a9e9f45-1307076528.tcb.qcloud.la\r\nConnection:keep-alive\r\n\r\n\r\n");

		if(n==0)
		{
				sprintf(buf, "GET http://4350l899a0.qicp.vip/APPBIN/LoadApp.xml HTTP/1.1\r\nHost: 4350l899a0.qicp.vip\r\nConnection:keep-alive\r\n\r\n\r\n");
		}
		else
		{
				sprintf(buf, "GET http://4350l899a0.qicp.vip/APPBIN/LoadApp%d.pkg HTTP/1.1\r\nHost: 4350l899a0.qicp.vip\r\nConnection:keep-alive\r\n\r\n\r\n",n);
		}
//		itoa(n,index,10);
//		sprintf(buf, "GET http://4350l899a0.qicp.vip/APPBIN/LoadApp1.pkg HTTP/1.1\r\nHost: 4350l899a0.qicp.vip\r\nConnection:keep-alive\r\n\r\n\r\n");
	
}

//==========================================================
//	函数名称：	OneNet_SendData_Get
//
//	函数功能：	上传数据到平台
//
//	入口参数：	type：发送数据的格式
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void OneNet_SendData_Get(char n)
{
	
	char buf[256];
	
	memset(buf, 0, sizeof(buf));
	
	OneNet_FillBuf_Get(buf,n);									//封装数据流
	
	ESP8266_SendData((unsigned char *)buf, strlen(buf));	//上传数据
	
	UsartPrintf(USART_DEBUG, "%s\r\n",buf);
}



/*==========================================================
	作    者：徐磊

	函数名称：	OneNet_RevProOneNet_RevPro_HTTP

	函数功能：	平台返回数据检测

	入口参数：	dataPtr：平台返回的数据
						
	返回参数：	数据有效状态
==========================================================*/
char OneNet_RevPro_HTTP(unsigned char *dataPtr)
{
	char IPD_Count = 0;	//ESP8266包数
	char DataStatus = 0;//数据有效1，数据无效0
	char MergeStatus = 0;//数据合并状态成功1，失败0
	
	ptr_struct PTRSTRUCT[20]={0};//IPD,XXXX:起始地址，结束地址以及IPD数据长度
	//接收数组初始化
	memset(Appdata_buf,NULL,sizeof(Appdata_buf));
	//应用数据结构体初始化
	memset(&AppPkg_Data,0,sizeof(AppPkg_Data));
	//获取ESP8266包数以及IPD,XXXX:起始地址，结束地址以及IPD数据长度
	IPD_Count = Get_Splice_Points(dataPtr,PTRSTRUCT);
	
	//ESP8266数据拼包
	MergeStatus = Merge_Pkg((char *)dataPtr,Appdata_buf,PTRSTRUCT,IPD_Count);
	
	//提取PKG数据，验证数据有效性
	DataStatus = GetPkgData_andCheck(Appdata_buf, &AppPkg_Data);
	
	ESP8266_Clear();
	
	return DataStatus;
}

/*==========================================================
	作    者：徐磊

	函数名称：	Get_Splice_Points

	函数功能：	获取ESP8266分包节点信息

	入口参数：	1.源数据指针dataPtr
						
	返回参数：	1.IPD,XXXX:起始地址，结束地址以及IPD数据长度Ptr_Struct
						2.ESP8266包数IPD_Count

==========================================================*/
char Get_Splice_Points(unsigned char *dataPtr,ptr_struct * PTRSTRUCT)
{
			//ptr_struct PTRSTRUCT[20];
			char IPD_Count = 0;
			//char * ptrIPD = NULL;
			char * ptrIPDTemp = NULL;
			char numBuf[10]={0};
			char index = 0;
			int num = 0;
			unsigned int sun = 0;
			//查找IPD拼接点，最多查找20个拼接点
			for(IPD_Count=0;IPD_Count<20;IPD_Count++)
			{
					index = 0;
					PTRSTRUCT[IPD_Count].ptrstart   = strstr((char *)dataPtr + sun, "+IPD,");							//搜索“+IPD”头
					PTRSTRUCT[IPD_Count].ptrend 		= strchr((char *)dataPtr + sun, ':');									//搜索“:”头
					if((PTRSTRUCT[IPD_Count].ptrstart!=NULL)&&(PTRSTRUCT[IPD_Count].ptrend!=NULL))
					{
							ptrIPDTemp = PTRSTRUCT[IPD_Count].ptrstart + 5;
							while(ptrIPDTemp < PTRSTRUCT[IPD_Count].ptrend)		//获取数据长度
							{
									numBuf[index++] = *(ptrIPDTemp++);
							}
							numBuf[index] = 0;
							num = atoi((char *)numBuf);													//转为数值形式
							PTRSTRUCT[IPD_Count].length = num;
							sun += num + 5 + index + 1;													//算上+IPD,XXX:自身的长度
					}
					else
					{
							break;//已经到字符串结尾
					}
			}
			return 	IPD_Count;	
}

/*==========================================================
	作    者：徐磊

	函数名称：	Merge_Pkg

	函数功能：	将ESP8266分包数据进行整合

	入口参数：	1.源数据指针dataPtr
							2.目的数据指针databuff
							3.IPD,XXXX:起始地址，结束地址以及IPD数据长度Ptr_Struct
							4.ESP8266传输包数n

	返回参数：	合并数据成功 1 合并数据失败 0

==========================================================*/
char  Merge_Pkg( char * dataPtr,char * databuff, ptr_struct *Ptr_Struct,char n)
{
			char * ptrstarttemp = NULL;	//“+IPD，xxxx:”首地址
			char * ptrendtemp = NULL;		//“+IPD，xxxx:”尾地址
			char * ptrHEAD = NULL;			//	.pkg地址
			
			char ipd_index=0;						//ESP8266传输包索引
			unsigned int i = 0;    			//目前开辟20KB缓冲区，用于存放目标数据，i计数最大20480
	
			ptrHEAD = strstr((char *)dataPtr, ".pkg");//搜索“.pkg”头
			if(ptrHEAD == NULL)
			{
					return 0;
			}
			
			do
			{
						//遍历所有数据包，监测dataptr是否在“  +IPD，xxxx:”区间内
						for(ipd_index=0;ipd_index<n;ipd_index++)
						{	
								ptrstarttemp = Ptr_Struct[ipd_index].ptrstart - 2;//-2是由于“+IPD，xxxx:”前面还有一个回车符和换行符，占2个字节
								ptrendtemp 	 = Ptr_Struct[ipd_index].ptrend;
								//若在“  +IPD，xxxx:”区间内，则跳转源数据到“:”后一个地址
								if((ptrstarttemp <= ptrHEAD)&&(ptrendtemp >=ptrHEAD))
								{
										ptrHEAD = Ptr_Struct[ipd_index].ptrend + 1;
										break;
								}
								//若不在“  +IPD，xxxx:”区间内，则不进行操作
								else
								{
										;//ptrHEAD = ptrHEAD;
								}
						}
			
						//将源地址有效数据放入目标地址
						databuff[i++] = *ptrHEAD;
						ptrHEAD++;
										
		}while(ptrHEAD <= Ptr_Struct[n-1].ptrend + Ptr_Struct[n-1].length);//读取源数据地址，直至字符串结束
			
		return 1;	
}

/*==========================================================
	作    者：徐磊

	函数名称：	GetPkgData_andCheck

	函数功能：	提取应用程序升级包PKG数据，验证数据有效性

	入口参数：	目的数据指针data_buf
	输出参数：	应用程序数据信息PKG_DATA

	返回参数：	数据有效 1，数据无效 0

==========================================================*/
char GetPkgData_andCheck(char * data_buf,PKG_Data* PKG_DATA)
{
			char Error_Flag = 0xAA; 
		  char * ptrDataStart = NULL;		//有效数据起始地址
	    char * ptrDataEnd = NULL;	 		//有效数据结束地址
			unsigned int * PTR_PKG = NULL;
			//处理数据
			if(data_buf != NULL)					//如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
			{		
					//包头
					PTR_PKG = (unsigned int *)data_buf;
					PKG_DATA->HEAD = *(unsigned int *)PTR_PKG;
					if(PKG_DATA->HEAD != 0x676B702E)	
					{
							Error_Flag = 0xBB;
					}
				
					//总包数
					PTR_PKG += 1;
					PKG_DATA->PKG_Count = *(unsigned int *)PTR_PKG;
					
					//当前包数
					PTR_PKG += 1;
					PKG_DATA->PKG_SunCount = *(unsigned int *)PTR_PKG;
					
					//有效数据长度
					PTR_PKG += 1;
					PKG_DATA->Length = *(unsigned int *)PTR_PKG;
					if(PKG_DATA->Length > 20480)//有效数据长度大于20KB，直接判定数据无效
					{
							return 0;
					}
					//有效数据
					ptrDataStart = data_buf + 16;
					ptrDataEnd   = ptrDataStart + PKG_DATA->Length;
					//校验和
					PTR_PKG = (unsigned int *)ptrDataEnd;
					PKG_DATA->CRC32 = *(unsigned int *)PTR_PKG;
					//包尾
					PTR_PKG += 1;
					PKG_DATA->END = *(unsigned int *)PTR_PKG;
					if(PKG_DATA->END != 0x6B70672E)	
					{
							Error_Flag = 0xBB;
					}
					
					if(Error_Flag == 0xAA)//数据校验成功
					{
							return 1;
					}
					else//数据校验失败
					{
							return 0;
					}
															
			}
			else
			{
					UsartPrintf(USART_DEBUG, "no A5A5A5A5\r\n");//遥测，无数据
					return 0;//数据获取失败
			}
}


/*==========================================================
	作    者：徐磊

	函数名称：	GetXmlData_andCheck

	函数功能：	提取应用程序升级包描述文件.xml，验证文件有效性

	入口参数：	目的数据指针data_buf
	输出参数：	应用程序数据信息XML_DATA

	返回参数：	数据有效 1，数据无效 0

==========================================================*/
char GetXmlData_andCheck(char * data_buf,XML_Data* XML_DATA)
{
			char Error_Flag = 0xAA; 
		  char * ptrHEAD = NULL;				//有效数据起始地址
			unsigned int * PTR_XML = NULL;
	
			ptrHEAD = strstr((char *)data_buf, ".xml");//搜索“.xml”头

			//处理数据
			if(ptrHEAD != NULL)					//如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
			{		
					//包头
					PTR_XML = (unsigned int *)ptrHEAD;
					XML_DATA->HEAD = *(unsigned int *)PTR_XML;
					if(XML_DATA->HEAD != 0x6C6D782E)	
					{
							Error_Flag = 0xBB;
					}
				
					//软件版本号
					PTR_XML += 1;
					XML_DATA->Software_Ver = *(unsigned int *)PTR_XML;
					
					//软件生成日期
					PTR_XML += 1;
					XML_DATA->Compiler_Date = *(unsigned int *)PTR_XML;
					
					//总包数
					PTR_XML += 1;
					XML_DATA->PKG_Count = *(unsigned int *)PTR_XML;
					if(XML_DATA->PKG_Count > 20)//总包数不大于20
					{
							return 0;
					}
					//备用位
					PTR_XML += 1;
					XML_DATA->Bank[0] = *(unsigned int *)PTR_XML;
					
					PTR_XML += 1;
					XML_DATA->Bank[1] = *(unsigned int *)PTR_XML;
					
					PTR_XML += 1;
					XML_DATA->Bank[2] = *(unsigned int *)PTR_XML;
					
					PTR_XML += 1;
					XML_DATA->Bank[3] = *(unsigned int *)PTR_XML;
					
					PTR_XML += 1;
					XML_DATA->Bank[4] = *(unsigned int *)PTR_XML;
					
					//校验和
					PTR_XML += 1;
					XML_DATA->CRC32 = *(unsigned int *)PTR_XML;
					
					//包尾
					PTR_XML += 1;
					XML_DATA->END = *(unsigned int *)PTR_XML;
					if(XML_DATA->END != 0x786D6C2E)	
					{
							Error_Flag = 0xBB;
					}
					
					if(Error_Flag == 0xAA)//数据校验成功
					{
							return 1;
					}
					else//数据校验失败
					{
							return 0;
					}
															
			}
			else
			{
					UsartPrintf(USART_DEBUG, "no A5A5A5A5\r\n");//遥测，无数据
					return 0;//数据获取失败
			}
}


/*==========================================================
	作    者：徐磊

	函数名称：	GetXmlData_andCheck

	函数功能：进行位赋值

	入口参数：	进行位操作的数值a
						位号k 
					  赋值b 
	输出参数：	进行位操作后的数值

==========================================================*/
unsigned short bitop(unsigned short a, unsigned int k, unsigned int b)
{
	unsigned int c = 0x1;
	c = c << k;
	c = ~c;
	a = a & c;

	c = b & 0x1;
	c = c << k;
	a = a | c;
	return a;
}

