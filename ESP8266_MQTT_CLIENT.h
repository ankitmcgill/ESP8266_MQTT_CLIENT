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

#define ESP8266_MQTT_CLIENT_REPLY_TIMEOUT_MS	5000

//CUSTOM VARIABLE STRUCTURES/////////////////////////////
typedef enum
{
	ESP8266_MQTT_CONTROL_PACKET_TYPE_CONNECT = 1,
	ESP8266_MQTT_CONTROL_PACKET_TYPE_CONNACK = 2,
	ESP8266_MQTT_CONTROL_PACKET_TYPE_PUBLISH = 3,
	ESP8266_MQTT_CONTROL_PACKET_TYPE_PUBACK = 4,
	ESP8266_MQTT_CONTROL_PACKET_TYPE_PUBREC = 5,
	ESP8266_MQTT_CONTROL_PACKET_TYPE_PUBREL = 6,
	ESP8266_MQTT_CONTROL_PACKET_TYPE_PUBCOMP = 7,
	ESP8266_MQTT_CONTROL_PACKET_TYPE_PINGREQ = 12,
	ESP8266_MQTT_CONTROL_PACKET_TYPE_PINGRESP = 13,
	ESP8266_MQTT_CONTROL_PACKET_TYPE_DISCONNECT = 14
}esp8266_mqtt_client_packet_type_t;

typedef enum
{
	ESP8266_MQTT_CONTROL_PACKET_FLAG_CONNECT = 0x00,
	ESP8266_MQTT_CONTROL_PACKET_FLAG_CONNACK = 0x00,
	ESP8266_MQTT_CONTROL_PACKET_FLAG_PUBACK = 0x00,
	ESP8266_MQTT_CONTROL_PACKET_FLAG_PUBREC = 0x00,
	ESP8266_MQTT_CONTROL_PACKET_FLAG_PUBREL = 0x02,
	ESP8266_MQTT_CONTROL_PACKET_FLAG_PUBCOMP = 0x00,
	ESP8266_MQTT_CONTROL_PACKET_FLAG_PINGREQ = 0x00,
	ESP8266_MQTT_CONTROL_PACKET_FLAG_PINGRESP = 0x00,
	ESP8266_MQTT_CONTROL_PACKET_FLAG_DISCONNECT = 0x00,
}esp8266_mqtt_client_packet_flag_t;

//END CUSTOM VARIABLE STRUCTURES/////////////////////////

//FUNCTION PROTOTYPES/////////////////////////////////////////////
//CONFIGURATION FUNCTIONS
void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_SetDebug(uint8_t debug_on);
void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_Initialize(const char* hostname,
														const char* host_ip,
														uint16_t host_port,
														uint16_t buffer_size);
void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_SetDnsServer(char num_dns, ip_addr_t* dns);
void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_SetCallbackFunctions(void (data_send_cb)(void*),
															    void (data_recv_cb)(char*, unsigned short),
                                                                void (*user_dns_cb_fn)(ip_addr_t*));
//OPERATION FUNCTIONS


#endif

