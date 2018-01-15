/**********************************************************************************
* ESP8266 MQTT CLIENT
*
* NOTE
* -----
*   (1) DOES NOT PROVIDE SUBSCRIBE FUNCTIONALITY. THIS LIB IS ONLY FOR SENDING
*       MESSAGES
*
*   (2) DOES NOT PROVIDE PING FUNCTIONALITY AS THE IDEA IS THAT THE CLIENT WILL
*       NOT NEED TO CONNECT TO MQTT BROKER FOR EXTENDED PERIOD OF TIME. IT WILL
*       CONNECT, PUBLISH THE MESSAGE AND DISCONNECT
*
*   (3) ONLY SUPPORTS QOS = 0 BECAUSE OF POINT (2). THE FUNCTIONALITY PROVIDED
*       IS FIRE AND FORGET
*
*   (4) FREE ONLINE MQTT BROKER
*       DIOTY
*       http://www.dioty.co/mydioty
*       LOGIN WITH MY GOOGLE ACCOUNT
*       MQTT BROKER USERNAME/PASSWORD : IN EMAIL
*
* REFERENCES
* -----------
*   (1) HIVEMQ MQTT REFERENCE
*       http://www.hivemq.com/mqtt_essentials/
*
*   (2) MQTT 3.1 SPECIFICATION
*       https://public.dhe.ibm.com/software/dw/webservices/ws-mqtt/mqtt-v3r1.html
*
* DECEMBER 29 2017
*
* ANKIT BHATNAGAR
* ANKIT.BHATNAGARINDIA@GMAIL.COM
/**********************************************************************************/

#ifndef _ESP8266_MQTT_CLIENT_H_
#define _ESP8266_MQTT_CLIENT_H_

#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"
#include "string.h"

#define ESP8266_MQTT_RETRY_COUNT				(3)
#define ESP8266_MQTT_PROTOCOL_VERSION			(3)
#define ESP8266_MQTT_VARIABLE_HEADER_MAX_SIZE	(100)
#define ESP8266_MQTT_CLIENT_REPLY_TIMEOUT_MS	(5000)

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
	ESP8266_MQTT_CONTROL_PACKET_TYPE_DISCONNECT = 14,
	ESP8266_MQTT_CONTROL_PACKET_TYPE_INVALID = 255
}esp8266_mqtt_client_packet_type_t;

typedef enum
{
	ESP8266_MQTT_CONTROL_PACKET_FLAG_CONNECT = 0x00,
	ESP8266_MQTT_CONTROL_PACKET_FLAG_CONNACK = 0x00,
	ESP8266_MQTT_CONTROL_PACKET_FLAG_PUBLISH = 0x00,
	ESP8266_MQTT_CONTROL_PACKET_FLAG_PUBACK = 0x00,
	ESP8266_MQTT_CONTROL_PACKET_FLAG_PUBREC = 0x00,
	ESP8266_MQTT_CONTROL_PACKET_FLAG_PUBREL = 0x02,
	ESP8266_MQTT_CONTROL_PACKET_FLAG_PUBCOMP = 0x00,
	ESP8266_MQTT_CONTROL_PACKET_FLAG_PINGREQ = 0x00,
	ESP8266_MQTT_CONTROL_PACKET_FLAG_PINGRESP = 0x00,
	ESP8266_MQTT_CONTROL_PACKET_FLAG_DISCONNECT = 0x00,
}esp8266_mqtt_client_packet_flag_t;

typedef enum
{
	ESP8266_MQTT_QOS_0 = 0,	//NO ACK FROM THE BROKER
	ESP8266_MQTT_QOS_1,		//1 ACK ON THE BROKER ON SUCCESSFULL RECEPTION
	ESP8266_MQTT_QOS_2,
	ESP8266_MQTT_QOS_3
}esp8266_mqtt_qos_t;

typedef enum
{
	ESP8266_MQTT_CONNACK_ACCEPTED = 0,
	ESP8266_MQTT_CONNACK_REFUSED_UNACCPETABLE_PROTOCOL_VER,
	ESP8266_MQTT_CONNACK_REFUSED_IDENTIFIER_REJECTED,
	ESP8266_MQTT_CONNACK_REFUSED_SERVER_UNAVAILABLE,
	ESP8266_MQTT_CONTROL_CONNACK_REFUSED_BAD_USERNAME_PASSWORD,
	ESP8266_MQTT_CONTROL_CONNACK_REFUSED_NOT_AUTHORIZED
}esp8266_mqtt_connack_return_code_t;

typedef struct
{
	uint8_t fixed_header_byte1;
	uint8_t	fixed_header_remaining_length[4];
	uint8_t fixed_header_remaining_length_len;
	uint8_t* variable_header;
	uint16_t variable_header_len;
	uint8_t* payload;
	uint16_t payload_len;
}esp8266_mqtt_packet_t;
//END CUSTOM VARIABLE STRUCTURES/////////////////////////

//FUNCTION PROTOTYPES/////////////////////////////////////////////
//CONFIGURATION FUNCTIONS
void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_SetDebug(uint8_t debug_on);
void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_SetOptions(uint8_t dup,
														uint8_t qos,
														uint8_t retain,
														bool use_username,
														char* username,
														bool use_password,
														char* password,
														bool use_clean_session,
														uint16_t keepalive_timer_val,
														bool use_will,
														char* will_topic,
														char* will_message,
														uint8_t will_qos,
														char* client_id);
void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_Initialize(const char* hostname,
														const char* host_ip,
														uint16_t host_port,
														uint16_t buffer_size);
void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_SetDnsServer(char num_dns, ip_addr_t* dns);
void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_SetCallbackFunctions(void (tcp_conn_cb)(void),
																void (data_send_cb)(void*),
															    void (data_recv_cb)(esp8266_mqtt_client_packet_type_t ptype, char*, unsigned short),
                                                                void (*user_dns_cb_fn)(ip_addr_t*));

//OPERATION FUNCTIONS
void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_ResolveHostName(void);
void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_TcpConnect(void);
void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_TcpDisonnect(void);
void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_Send_Connect(void);
void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_Send_Publish(char* topic,
														char* message,
                                                        esp8266_mqtt_qos_t qos_level);
void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_Send_Pingreq(void);
void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_Send_Disconnect(void);

#endif

