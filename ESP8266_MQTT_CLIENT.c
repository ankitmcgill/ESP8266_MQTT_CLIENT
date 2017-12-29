#include "ESP8266_TCP_GENERIC.h"
#include "ESP8266_MQTT_CLIENT.h"

//LOCAL LIBRARY VARIABLES////////////////////////////////
//DEBUG RELATED
static uint8_t s_esp8266_mqtt_client_debug;

//OPERATION RELATED

//TIMER RELATED

//HTML DATA RELEATED

//SSID RELATED

//CB FUNCTIONS

//END LOCAL LIBRARY VARIABLES/////////////////////////////


void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_SetDebug(uint8_t debug)
{
	//SET DEBUG PRINTF ON(1) OR OFF(0)

    s_esp8266_mqtt_client_debug = debug_on;
}
