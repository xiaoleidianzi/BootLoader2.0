#ifndef _LED_H_
#define _LED_H_





typedef struct
{

	_Bool LedF6Sta;
	_Bool LedF7Sta;
	_Bool LedF8Sta;
	_Bool LedF9Sta;

} LED_STATUS;

extern LED_STATUS led_status;


typedef enum
{

	LED_OFF = 0,
	LED_ON = 1

} LED_ENUM;


void Led_Init(void);

void LedF6_Set(LED_ENUM status);

void LedF7_Set(LED_ENUM status);

void LedF8_Set(LED_ENUM status);

void LedF9_Set(LED_ENUM status);


#endif
