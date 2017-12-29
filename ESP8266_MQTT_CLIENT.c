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
static void (*s_esp8266_mqtt_client_dns_cb_function)(ip_addr_t*);
static void (*s_esp8266_mqtt_client_data_senD_cb)(void*);
static void (*s_esp8266_mqtt_client_data_recv_cb)(char*, unsigned short);

//END LOCAL LIBRARY VARIABLES/////////////////////////////

//LOCAL LIBRARY FUNCTIONS////////////////////////////////
void ICACHE_FLASH_ATTR s_esp8266_mqtt_client_dns_found_cb(const char* name, ip_addr_t* ipAddr, void* arg);
void ICACHE_FLASH_ATTR s_esp8266_mqtt_client_send_cb(void* arg);
void ICACHE_FLASH_ATTR s_esp8266_mqtt_client_receive_cb(void* arg, char* pusrdata, unsigned short length);
//END LOCAL LIBRARY FUNCTIONS////////////////////////////////


void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_SetDebug(uint8_t debug_on)
{
	//SET DEBUG PRINTF ON(1) OR OFF(0)

    s_esp8266_mqtt_client_debug = debug_on;
}

void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_Initialize(const char* hostname,
														const char* host_ip,
														uint16_t host_port,
														uint16_t buffer_size)
{
    //INITIALIZE MQTT CLIENT MODULE PARAMETERS

    //SET DEBUG ON
    s_esp8266_mqtt_client_debug = 1;
    
    os_printf("ESP8266 MQTT_CLIENT : Initialized\n");
}

void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_SetDnsServer(char num_dns, ip_addr_t* dns)
{
    //SET DNS SERVER FOR HOST NAME RESOLVING

    ESP8266_TCP_GENERIC_SetDnsServer(num_dns, dns);
}

void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_SetCallbackFunctions(void (data_send_cb)(void*),
															    void (data_recv_cb)(char*, unsigned short),
                                                                void (*user_dns_cb_fn)(ip_addr_t*))
{
    //SET MODULE EVENTS CB FUNCTIONS

    s_esp8266_mqtt_client_dns_cb_function = user_dns_cb_fn;
    s_esp8266_mqtt_client_data_senD_cb = data_send_cb;
    s_esp8266_mqtt_client_data_recv_cb = data_recv_cb;
}


//INTERNAL FUNCTIONS
void ICACHE_FLASH_ATTR s_esp8266_mqtt_client_dns_found_cb(const char* name, ip_addr_t* ipAddr, void* arg)
{
    //DNS CB
}

void ICACHE_FLASH_ATTR s_esp8266_mqtt_client_send_cb(void* arg)
{
    //DATA SEND CB
}

void ICACHE_FLASH_ATTR s_esp8266_mqtt_client_receive_cb(void* arg, char* pusrdata, unsigned short length)
{
    //DATA RECV CB
}