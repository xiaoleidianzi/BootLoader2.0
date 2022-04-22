#ifndef _ESP8266_H_
#define _ESP8266_H_


#define ESP8266_WIFI_INFO		"AT+CWJAP=\"OPPO\",\"liuaixin\"\r\n"

//MQTT服务器IP、端口号
#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"183.230.40.96\",1883\r\n"
//#define ESP8266_ONENET_INFO_APPbin		"AT+CIPSTART=\"TCP\",\"636c-cloud1-8g5omron3a9e9f45-1307076528.tcb.qcloud.la\",80\r\n"
#define ESP8266_ONENET_INFO_APPbin		"AT+CIPSTART=\"TCP\",\"4350l899a0.qicp.vip\",80\r\n"

//MQTT

//产品ID
#define PROID	 "470285"
//产品key
#define ACCESS_KEY	"WMwRLiaVDsOGsj7r7iVrQ5cigXE60DQNgp68ylYUnmE="

//设备key
#define DEVICE_KEY	"kQ7GZvMTlau7KM75YoEqRgEBJN5PYiCOUh6CY9JXM8k="
//设备名称
#define DEVICE_NAME	 "BOOT"

#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志

void ESP8266_Init(void);

void ESP8266_Get_Init(void);

void ESP8266_Clear(void);

void ESP8266_SendData(unsigned char *data, unsigned short len);

unsigned char *ESP8266_GetIPD(unsigned short timeOut);
unsigned char *ESP8266_MQTTIPD(unsigned short timeOut);
_Bool ESP8266_SendCmd(char *cmd, char *res);
#endif
