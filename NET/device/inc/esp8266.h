#ifndef _ESP8266_H_
#define _ESP8266_H_


#define ESP8266_WIFI_INFO		"AT+CWJAP=\"OPPO\",\"liuaixin\"\r\n"

//MQTT������IP���˿ں�
#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"183.230.40.96\",1883\r\n"
//#define ESP8266_ONENET_INFO_APPbin		"AT+CIPSTART=\"TCP\",\"636c-cloud1-8g5omron3a9e9f45-1307076528.tcb.qcloud.la\",80\r\n"
#define ESP8266_ONENET_INFO_APPbin		"AT+CIPSTART=\"TCP\",\"4350l899a0.qicp.vip\",80\r\n"

//MQTT

//��ƷID
#define PROID	 "470285"
//��Ʒkey
#define ACCESS_KEY	"WMwRLiaVDsOGsj7r7iVrQ5cigXE60DQNgp68ylYUnmE="

//�豸key
#define DEVICE_KEY	"kQ7GZvMTlau7KM75YoEqRgEBJN5PYiCOUh6CY9JXM8k="
//�豸����
#define DEVICE_NAME	 "BOOT"

#define REV_OK		0	//������ɱ�־
#define REV_WAIT	1	//����δ��ɱ�־

void ESP8266_Init(void);

void ESP8266_Get_Init(void);

void ESP8266_Clear(void);

void ESP8266_SendData(unsigned char *data, unsigned short len);

unsigned char *ESP8266_GetIPD(unsigned short timeOut);
unsigned char *ESP8266_MQTTIPD(unsigned short timeOut);
_Bool ESP8266_SendCmd(char *cmd, char *res);
#endif
