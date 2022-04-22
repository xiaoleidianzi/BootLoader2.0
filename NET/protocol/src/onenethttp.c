/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	onenet.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-05-09
	*
	*	�汾�� 		V1.1
	*
	*	˵���� 		��onenetƽ̨�����ݽ����ӿڲ�
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//�����豸
#include "esp8266.h"

//Э���ļ�
#include "onenethttp.h"

//Ӳ������
#include "usart.h"
#include "delay.h"
#include "sht20.h"

//C��
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//stdlib.h
//HTTP
#define DEVID	"913689709"
#define APIKEY	"wW3Wt=53QZL=rwNtoEiYYBTJ=O4="

//Ӧ�ó������ݽṹ��
PKG_Data AppPkg_Data= {0};
//Ӧ�ó�����������
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
//	�������ƣ�	OneNet_SendData_Get
//
//	�������ܣ�	�ϴ����ݵ�ƽ̨
//
//	��ڲ�����	type���������ݵĸ�ʽ
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNet_SendData_Get(char n)
{
	
	char buf[256];
	
	memset(buf, 0, sizeof(buf));
	
	OneNet_FillBuf_Get(buf,n);									//��װ������
	
	ESP8266_SendData((unsigned char *)buf, strlen(buf));	//�ϴ�����
	
	UsartPrintf(USART_DEBUG, "%s\r\n",buf);
}



/*==========================================================
	��    �ߣ�����

	�������ƣ�	OneNet_RevProOneNet_RevPro_HTTP

	�������ܣ�	ƽ̨�������ݼ��

	��ڲ�����	dataPtr��ƽ̨���ص�����
						
	���ز�����	������Ч״̬
==========================================================*/
char OneNet_RevPro_HTTP(unsigned char *dataPtr)
{
	char IPD_Count = 0;	//ESP8266����
	char DataStatus = 0;//������Ч1��������Ч0
	char MergeStatus = 0;//���ݺϲ�״̬�ɹ�1��ʧ��0
	
	ptr_struct PTRSTRUCT[20]={0};//IPD,XXXX:��ʼ��ַ��������ַ�Լ�IPD���ݳ���
	//���������ʼ��
	memset(Appdata_buf,NULL,sizeof(Appdata_buf));
	//Ӧ�����ݽṹ���ʼ��
	memset(&AppPkg_Data,0,sizeof(AppPkg_Data));
	//��ȡESP8266�����Լ�IPD,XXXX:��ʼ��ַ��������ַ�Լ�IPD���ݳ���
	IPD_Count = Get_Splice_Points(dataPtr,PTRSTRUCT);
	
	//ESP8266����ƴ��
	MergeStatus = Merge_Pkg((char *)dataPtr,Appdata_buf,PTRSTRUCT,IPD_Count);
	
	//��ȡPKG���ݣ���֤������Ч��
	DataStatus = GetPkgData_andCheck(Appdata_buf, &AppPkg_Data);
	
	ESP8266_Clear();
	
	return DataStatus;
}

/*==========================================================
	��    �ߣ�����

	�������ƣ�	Get_Splice_Points

	�������ܣ�	��ȡESP8266�ְ��ڵ���Ϣ

	��ڲ�����	1.Դ����ָ��dataPtr
						
	���ز�����	1.IPD,XXXX:��ʼ��ַ��������ַ�Լ�IPD���ݳ���Ptr_Struct
						2.ESP8266����IPD_Count

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
			//����IPDƴ�ӵ㣬������20��ƴ�ӵ�
			for(IPD_Count=0;IPD_Count<20;IPD_Count++)
			{
					index = 0;
					PTRSTRUCT[IPD_Count].ptrstart   = strstr((char *)dataPtr + sun, "+IPD,");							//������+IPD��ͷ
					PTRSTRUCT[IPD_Count].ptrend 		= strchr((char *)dataPtr + sun, ':');									//������:��ͷ
					if((PTRSTRUCT[IPD_Count].ptrstart!=NULL)&&(PTRSTRUCT[IPD_Count].ptrend!=NULL))
					{
							ptrIPDTemp = PTRSTRUCT[IPD_Count].ptrstart + 5;
							while(ptrIPDTemp < PTRSTRUCT[IPD_Count].ptrend)		//��ȡ���ݳ���
							{
									numBuf[index++] = *(ptrIPDTemp++);
							}
							numBuf[index] = 0;
							num = atoi((char *)numBuf);													//תΪ��ֵ��ʽ
							PTRSTRUCT[IPD_Count].length = num;
							sun += num + 5 + index + 1;													//����+IPD,XXX:����ĳ���
					}
					else
					{
							break;//�Ѿ����ַ�����β
					}
			}
			return 	IPD_Count;	
}

/*==========================================================
	��    �ߣ�����

	�������ƣ�	Merge_Pkg

	�������ܣ�	��ESP8266�ְ����ݽ�������

	��ڲ�����	1.Դ����ָ��dataPtr
							2.Ŀ������ָ��databuff
							3.IPD,XXXX:��ʼ��ַ��������ַ�Լ�IPD���ݳ���Ptr_Struct
							4.ESP8266�������n

	���ز�����	�ϲ����ݳɹ� 1 �ϲ�����ʧ�� 0

==========================================================*/
char  Merge_Pkg( char * dataPtr,char * databuff, ptr_struct *Ptr_Struct,char n)
{
			char * ptrstarttemp = NULL;	//��+IPD��xxxx:���׵�ַ
			char * ptrendtemp = NULL;		//��+IPD��xxxx:��β��ַ
			char * ptrHEAD = NULL;			//	.pkg��ַ
			
			char ipd_index=0;						//ESP8266���������
			unsigned int i = 0;    			//Ŀǰ����20KB�����������ڴ��Ŀ�����ݣ�i�������20480
	
			ptrHEAD = strstr((char *)dataPtr, ".pkg");//������.pkg��ͷ
			if(ptrHEAD == NULL)
			{
					return 0;
			}
			
			do
			{
						//�����������ݰ������dataptr�Ƿ��ڡ�  +IPD��xxxx:��������
						for(ipd_index=0;ipd_index<n;ipd_index++)
						{	
								ptrstarttemp = Ptr_Struct[ipd_index].ptrstart - 2;//-2�����ڡ�+IPD��xxxx:��ǰ�滹��һ���س����ͻ��з���ռ2���ֽ�
								ptrendtemp 	 = Ptr_Struct[ipd_index].ptrend;
								//���ڡ�  +IPD��xxxx:�������ڣ�����תԴ���ݵ���:����һ����ַ
								if((ptrstarttemp <= ptrHEAD)&&(ptrendtemp >=ptrHEAD))
								{
										ptrHEAD = Ptr_Struct[ipd_index].ptrend + 1;
										break;
								}
								//�����ڡ�  +IPD��xxxx:�������ڣ��򲻽��в���
								else
								{
										;//ptrHEAD = ptrHEAD;
								}
						}
			
						//��Դ��ַ��Ч���ݷ���Ŀ���ַ
						databuff[i++] = *ptrHEAD;
						ptrHEAD++;
										
		}while(ptrHEAD <= Ptr_Struct[n-1].ptrend + Ptr_Struct[n-1].length);//��ȡԴ���ݵ�ַ��ֱ���ַ�������
			
		return 1;	
}

/*==========================================================
	��    �ߣ�����

	�������ƣ�	GetPkgData_andCheck

	�������ܣ�	��ȡӦ�ó���������PKG���ݣ���֤������Ч��

	��ڲ�����	Ŀ������ָ��data_buf
	���������	Ӧ�ó���������ϢPKG_DATA

	���ز�����	������Ч 1��������Ч 0

==========================================================*/
char GetPkgData_andCheck(char * data_buf,PKG_Data* PKG_DATA)
{
			char Error_Flag = 0xAA; 
		  char * ptrDataStart = NULL;		//��Ч������ʼ��ַ
	    char * ptrDataEnd = NULL;	 		//��Ч���ݽ�����ַ
			unsigned int * PTR_PKG = NULL;
			//��������
			if(data_buf != NULL)					//���û�ҵ���������IPDͷ���ӳ٣�������Ҫ�ȴ�һ�ᣬ�����ᳬ���趨��ʱ��
			{		
					//��ͷ
					PTR_PKG = (unsigned int *)data_buf;
					PKG_DATA->HEAD = *(unsigned int *)PTR_PKG;
					if(PKG_DATA->HEAD != 0x676B702E)	
					{
							Error_Flag = 0xBB;
					}
				
					//�ܰ���
					PTR_PKG += 1;
					PKG_DATA->PKG_Count = *(unsigned int *)PTR_PKG;
					
					//��ǰ����
					PTR_PKG += 1;
					PKG_DATA->PKG_SunCount = *(unsigned int *)PTR_PKG;
					
					//��Ч���ݳ���
					PTR_PKG += 1;
					PKG_DATA->Length = *(unsigned int *)PTR_PKG;
					if(PKG_DATA->Length > 20480)//��Ч���ݳ��ȴ���20KB��ֱ���ж�������Ч
					{
							return 0;
					}
					//��Ч����
					ptrDataStart = data_buf + 16;
					ptrDataEnd   = ptrDataStart + PKG_DATA->Length;
					//У���
					PTR_PKG = (unsigned int *)ptrDataEnd;
					PKG_DATA->CRC32 = *(unsigned int *)PTR_PKG;
					//��β
					PTR_PKG += 1;
					PKG_DATA->END = *(unsigned int *)PTR_PKG;
					if(PKG_DATA->END != 0x6B70672E)	
					{
							Error_Flag = 0xBB;
					}
					
					if(Error_Flag == 0xAA)//����У��ɹ�
					{
							return 1;
					}
					else//����У��ʧ��
					{
							return 0;
					}
															
			}
			else
			{
					UsartPrintf(USART_DEBUG, "no A5A5A5A5\r\n");//ң�⣬������
					return 0;//���ݻ�ȡʧ��
			}
}


/*==========================================================
	��    �ߣ�����

	�������ƣ�	GetXmlData_andCheck

	�������ܣ�	��ȡӦ�ó��������������ļ�.xml����֤�ļ���Ч��

	��ڲ�����	Ŀ������ָ��data_buf
	���������	Ӧ�ó���������ϢXML_DATA

	���ز�����	������Ч 1��������Ч 0

==========================================================*/
char GetXmlData_andCheck(char * data_buf,XML_Data* XML_DATA)
{
			char Error_Flag = 0xAA; 
		  char * ptrHEAD = NULL;				//��Ч������ʼ��ַ
			unsigned int * PTR_XML = NULL;
	
			ptrHEAD = strstr((char *)data_buf, ".xml");//������.xml��ͷ

			//��������
			if(ptrHEAD != NULL)					//���û�ҵ���������IPDͷ���ӳ٣�������Ҫ�ȴ�һ�ᣬ�����ᳬ���趨��ʱ��
			{		
					//��ͷ
					PTR_XML = (unsigned int *)ptrHEAD;
					XML_DATA->HEAD = *(unsigned int *)PTR_XML;
					if(XML_DATA->HEAD != 0x6C6D782E)	
					{
							Error_Flag = 0xBB;
					}
				
					//����汾��
					PTR_XML += 1;
					XML_DATA->Software_Ver = *(unsigned int *)PTR_XML;
					
					//�����������
					PTR_XML += 1;
					XML_DATA->Compiler_Date = *(unsigned int *)PTR_XML;
					
					//�ܰ���
					PTR_XML += 1;
					XML_DATA->PKG_Count = *(unsigned int *)PTR_XML;
					if(XML_DATA->PKG_Count > 20)//�ܰ���������20
					{
							return 0;
					}
					//����λ
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
					
					//У���
					PTR_XML += 1;
					XML_DATA->CRC32 = *(unsigned int *)PTR_XML;
					
					//��β
					PTR_XML += 1;
					XML_DATA->END = *(unsigned int *)PTR_XML;
					if(XML_DATA->END != 0x786D6C2E)	
					{
							Error_Flag = 0xBB;
					}
					
					if(Error_Flag == 0xAA)//����У��ɹ�
					{
							return 1;
					}
					else//����У��ʧ��
					{
							return 0;
					}
															
			}
			else
			{
					UsartPrintf(USART_DEBUG, "no A5A5A5A5\r\n");//ң�⣬������
					return 0;//���ݻ�ȡʧ��
			}
}


/*==========================================================
	��    �ߣ�����

	�������ƣ�	GetXmlData_andCheck

	�������ܣ�����λ��ֵ

	��ڲ�����	����λ��������ֵa
						λ��k 
					  ��ֵb 
	���������	����λ���������ֵ

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

