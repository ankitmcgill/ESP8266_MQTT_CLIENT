/*************************************************
* ESP8266 MQTT CLIENT
*
*
*
* REFERENCES
* -----------
*   (1) HIVEMQ MQTT REFERENCE
*       http://www.hivemq.com/mqtt_essentials/
*
* DECEMBER 29 2017
*
* ANKIT BHATNAGAR
* ANKIT.BHATNAGARINDIA@GMAIL.COM
************************************************/

#ifndef _ESP8266_MQTT_CLIENT_H_
#define _ESP8266_MQTT_CLIENT_H_

#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"
#include "string.h"

//CUSTOM VARIABLE STRUCTURES/////////////////////////////
typedef enum
{
	ESP8266_MQTT_CONTROL_PACKET_CONNECT = 1,
	ESP8266_MQTT_CONTROL_PACKET_CONNACK = 2,
	ESP8266_MQTT_CONTROL_PACKET_PUBLISH = 3,
	ESP8266_MQTT_CONTROL_PACKET_PUBACK = 4,
	ESP8266_MQTT_CONTROL_PACKET_PUBREC = 5,
	ESP8266_MQTT_CONTROL_PACKET_PUBREL = 6,
	ESP8266_MQTT_CONTROL_PACKET_PUBCOMP = 7,
	ESP8266_MQTT_CONTROL_PACKET_PINGREQ = 12,
	ESP8266_MQTT_CONTROL_PACKET_PINGRESP = 13,
	ESP8266_MQTT_CONTROL_PACKET_DISCONNECT = 14
}esp8266_mqtt_client_packet_type_t;

//END CUSTOM VARIABLE STRUCTURES/////////////////////////

/FUNCTION PROTOTYPES/////////////////////////////////////////////
//CONFIGURATION FUNCTIONS
void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_SetDebug(uint8_t debug);


//OPERATION FUNCTIONS


//INTERNAL FUNCTIONS


#endif

