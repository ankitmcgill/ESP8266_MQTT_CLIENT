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

#include "ESP8266_TCP_GENERIC.h"
#include "ESP8266_MQTT_CLIENT.h"

//LOCAL LIBRARY VARIABLES////////////////////////////////
//DEBUG RELATED
static uint8_t s_esp8266_mqtt_client_debug;

//OPERATION RELATED
static uint16_t s_buffer_size;
static uint16_t s_mqtt_message_id = 0;
static esp8266_mqtt_client_packet_type_t s_current_packet_type;

//CB FUNCTIONS
static void (*s_esp8266_mqtt_client_tcp_conn_cb_function)(void);
static void (*s_esp8266_mqtt_client_dns_cb_function)(ip_addr_t*);
static void (*s_esp8266_mqtt_client_data_send_cb)(void*);
static void (*s_esp8266_mqtt_client_data_recv_cb)(esp8266_mqtt_client_packet_type_t ptype,char*, unsigned short);

//MQTT RELATED (WITH DEFAULT VALUES)
static bool s_flag_dup = false;
static uint8_t s_qos = ESP8266_MQTT_QOS_0;
static uint8_t s_flag_retain = false;
static bool s_flag_username = false;
static char* s_username = NULL;
static bool s_flag_password = false;
static char* s_password = NULL;
static bool s_flag_clean_session = true;
static uint16_t s_keepalive_timer = 0;
static bool s_flag_will = false;
static char* s_will_topic = NULL;
static char* s_will_message = NULL;
static uint8_t s_will_qos = ESP8266_MQTT_QOS_0;
static char* s_client_id = NULL;
static char* s_packet_return_code[7] = {"Connection Accpted",
                                        "Connection Refused : unaccepted protocol version",
                                        "Connection Refused : identifier rejected",
                                        "Connection Refused : server unavailable",
                                        "Connection Refused : bad username / password",
                                        "Connection Refused : not authorized",
                                        "Reserved"};
static char* s_last_topic;
static char* s_last_message;
static esp8266_mqtt_qos_t s_last_qos;
//END LOCAL LIBRARY VARIABLES/////////////////////////////

//LOCAL LIBRARY FUNCTIONS////////////////////////////////
static void ICACHE_FLASH_ATTR s_esp8266_mqtt_print_packet(uint8_t* packet, uint16_t len);
static uint8_t ICACHE_FLASH_ATTR s_esp8266_mqtt_insert_string(uint8_t* dest_buff, 
                                                                char* src_buff, 
                                                                uint8_t len);
static uint8_t ICACHE_FLASH_ATTR s_esp8266_mqtt_calculate_remaining_length(uint16_t len_variable_header, 
                                                                            uint16_t len_payload, 
                                                                            uint8_t* ptr_remaining_length);
static void ICACHE_FLASH_ATTR s_esp8266_mqtt_send_packet(esp8266_mqtt_packet_t packet);
static esp8266_mqtt_client_packet_type_t ICACHE_FLASH_ATTR s_esp8266_mqtt_parse_response_packet(uint8_t* packet, uint16_t len);

static void ICACHE_FLASH_ATTR s_esp8266_mqtt_client_dns_found_cb(ip_addr_t* ipAddr);
static void ICACHE_FLASH_ATTR s_esp8266_mqtt_tcp_conn_cb(void* arg);
static void ICACHE_FLASH_ATTR s_esp8266_mqtt_client_send_cb(void* arg);
static void ICACHE_FLASH_ATTR s_esp8266_mqtt_client_receive_cb(char* pusrdata, unsigned short length);
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
    
    //INTIALIZE UNDERLYING TCP GENERIC MODULE
    ESP8266_TCP_GENERIC_Initialize(hostname, host_ip, host_port, "", buffer_size);
    s_buffer_size = buffer_size;

    os_printf("ESP8266 MQTT_CLIENT : Initialized. Debug ON\n");
}

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
                                                        char* client_id)
{
    //SET THE BASIC GLOBAL MQTT OPTIONS

    s_flag_dup = dup;
    s_qos = qos;
    s_flag_retain = retain;
    s_flag_username = use_username;
    s_username = username;
    s_flag_password = use_password;
    s_password = password;
    s_flag_clean_session = use_clean_session;
    s_keepalive_timer = keepalive_timer_val;
    s_flag_will = use_will;
    s_will_topic = s_will_topic;
    s_will_message = s_will_message;
    s_will_qos = s_will_qos;
    s_client_id = client_id;
    s_mqtt_message_id = 0;
}

void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_SetDnsServer(char num_dns, ip_addr_t* dns)
{
    //SET DNS SERVER FOR HOST NAME RESOLVING

    ESP8266_TCP_GENERIC_SetDnsServer(num_dns, dns);
}

void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_SetCallbackFunctions(void (tcp_conn_cb)(void),
																void (data_send_cb)(void*),
															    void (data_recv_cb)(esp8266_mqtt_client_packet_type_t ptype, char*, unsigned short),
                                                                void (*user_dns_cb_fn)(ip_addr_t*))
{
    //SET MODULE EVENTS CB FUNCTIONS

    s_esp8266_mqtt_client_tcp_conn_cb_function = tcp_conn_cb;
    s_esp8266_mqtt_client_dns_cb_function = user_dns_cb_fn;
    s_esp8266_mqtt_client_data_send_cb = data_send_cb;
    s_esp8266_mqtt_client_data_recv_cb = data_recv_cb;

    //SET TCP LAYER CB FUNCTIONS
    ESP8266_TCP_GENERIC_SetCallbackFunctions(s_esp8266_mqtt_tcp_conn_cb, 
                                                NULL, 
                                                s_esp8266_mqtt_client_send_cb, 
                                                s_esp8266_mqtt_client_receive_cb, 
                                                s_esp8266_mqtt_client_dns_found_cb);
}

void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_ResolveHostName(void)
{
    //RESOLVE TCP SERVER HOSTNAME

    ESP8266_TCP_GENERIC_ResolveHostName();
}

void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_TcpConnect(void)
{
    //CONNECT TO MQTT TCP SERVER

    ESP8266_TCP_GENERIC_Connect();
}

void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_TcpDisonnect(void)
{
    //DISCONNECT FROM MQTT TCP SERVER
    
    ESP8266_TCP_GENERIC_Disonnect();
}

void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_Send_Connect(void)
{
    //SEND MQTT CONNECT PACKET
    //CLEAN_SESSION = TRUE
    //QOS ENABLED (SINCE WE WANT CONNACK BEFORE SENDING PUBLISH)

    s_current_packet_type = ESP8266_MQTT_CONTROL_PACKET_TYPE_CONNECT;

    esp8266_mqtt_packet_t p;
    uint16_t len_variable_header = 0;
    uint16_t len_payload = 0;
    uint8_t* buffer_variable_header;
    uint8_t* buffer_payload;

    //GENERATE PACKET
    buffer_variable_header = (uint8_t*)os_zalloc(ESP8266_MQTT_VARIABLE_HEADER_MAX_SIZE);
    buffer_payload = (uint8_t*)os_zalloc(s_buffer_size - ESP8266_MQTT_VARIABLE_HEADER_MAX_SIZE);

    //FIXED HEADER
    p.fixed_header_byte1 = (ESP8266_MQTT_CONTROL_PACKET_TYPE_CONNECT << 4) | 
                            ESP8266_MQTT_CONTROL_PACKET_FLAG_CONNECT;
    
    //VARIABLE HEADER
    s_esp8266_mqtt_insert_string(buffer_variable_header, "MQIsdp", 6);
    buffer_variable_header[8] = ESP8266_MQTT_PROTOCOL_VERSION;
    buffer_variable_header[9] = (s_flag_username << 7) |
                                    (s_flag_password << 6) |
                                    (s_flag_will << 5) |
                                    (s_will_qos << 4) |
                                    (s_flag_will << 2) |
                                    (s_flag_clean_session);
    buffer_variable_header[10] = (uint8_t)((s_keepalive_timer & 0xFF00) >> 8);
    buffer_variable_header[11] = (uint8_t)((s_keepalive_timer & 0x00FF));
    len_variable_header = 12;

    //PAYLOAD
    if(!s_client_id)
    {
        if(s_esp8266_mqtt_client_debug)
        {
            os_printf("ESP8266 MQTT_CLIENT : Error ! Client id NULL\n");
        }
        return;
    }
    len_payload += s_esp8266_mqtt_insert_string(&buffer_payload[0],
                                                s_client_id, 
                                                strlen(s_client_id));
    if(s_flag_will)
    {
        // WILL IS SET
        if(!s_will_topic)
        {
            if(s_esp8266_mqtt_client_debug)
            {
                os_printf("ESP8266 MQTT_CLIENT : Error ! Will topic not provided\n");
            }
            return;
        }
        len_payload += s_esp8266_mqtt_insert_string(&buffer_payload[len_payload], 
                                                    s_will_topic, 
                                                    strlen(s_will_topic));

        if(!s_will_message)
        {
            if(s_esp8266_mqtt_client_debug)
            {
                os_printf("ESP8266 MQTT_CLIENT : Error ! Will message not provided\n");
            }
            return;
        }
        len_payload += s_esp8266_mqtt_insert_string(&buffer_payload[len_payload], 
                                                    s_will_message, 
                                                    strlen(s_will_message));
    }
    if(s_flag_username)
    {
        if(!s_username)
        {
            if(s_esp8266_mqtt_client_debug)
            {
                os_printf("ESP8266 MQTT_CLIENT : Error ! username not provided\n");
            }
            return;
        }
        len_payload += s_esp8266_mqtt_insert_string(&buffer_payload[len_payload], 
                                                    s_username, 
                                                    strlen(s_username));
    }
    if(s_flag_password)
    {
        if(!s_password)
        {
            if(s_esp8266_mqtt_client_debug)
            {
                os_printf("ESP8266 MQTT_CLIENT : Error ! password not provided\n");
            }
            return;
        }
        len_payload += s_esp8266_mqtt_insert_string(&buffer_payload[len_payload], 
                                                    s_password, 
                                                    strlen(s_password));
    }
    
    //CALCULATE REMAINING LENGTH (IN FIXED HEADER) FIELD
    p.fixed_header_remaining_length_len = s_esp8266_mqtt_calculate_remaining_length(len_variable_header, 
                                                                                    len_payload, 
                                                                                    &p.fixed_header_remaining_length[0]);

    //MAKE MQTT PACKET FROM BUFFERS
    p.variable_header = buffer_variable_header;
	p.variable_header_len = len_variable_header;
	p.payload = buffer_payload;
	p.payload_len = len_payload;
    if(s_esp8266_mqtt_client_debug)
    {
        os_printf("ESP8266 MQTT_CLIENT : CONNECT packet created\n");
    }

    //SEND PACKET
    s_esp8266_mqtt_send_packet(p);
    if(s_esp8266_mqtt_client_debug)
    {
        os_printf("ESP8266 MQTT_CLIENT : CONNECT packet sent\n");
    }

    //FREE BUFFER
    os_free(buffer_variable_header);
    os_free(buffer_payload);

    //INCREMENT MESSAGE ID
    s_mqtt_message_id++;
}

void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_Send_Publish(char* topic,
														char* message,
                                                        esp8266_mqtt_qos_t qos_level)
{
    //SEND MQTT PUBLISH PACKET

    //ONLY QOS = 0 or 1 SUPPORTED
    if(qos_level != ESP8266_MQTT_QOS_0 && qos_level != ESP8266_MQTT_QOS_1)
    {
        os_printf("ESP8266 MQTT_CLIENT : PUBLISH Fail. Only Qos 0 or 1 supported\n");
        return;
    }

    s_current_packet_type = ESP8266_MQTT_CONTROL_PACKET_TYPE_PUBLISH;

    esp8266_mqtt_packet_t p;
    uint16_t len_variable_header = 0;
    uint16_t len_payload = 0;
    uint8_t* buffer_variable_header;
    uint8_t* buffer_payload;

    //STORE DATA REFERENCE
    s_last_topic = topic;
    s_last_message = message;
    s_last_qos = qos_level;

    //GENERATE PACKET
    buffer_variable_header = (uint8_t*)os_zalloc(ESP8266_MQTT_VARIABLE_HEADER_MAX_SIZE);
    buffer_payload = (uint8_t*)os_zalloc(s_buffer_size - ESP8266_MQTT_VARIABLE_HEADER_MAX_SIZE);

    //FIXED HEADER
    p.fixed_header_byte1 = (ESP8266_MQTT_CONTROL_PACKET_TYPE_PUBLISH << 4) | 
                            ESP8266_MQTT_CONTROL_PACKET_FLAG_PUBLISH |
                            (qos_level << 1);
    
    //VARIABLE HEADER
    len_variable_header += s_esp8266_mqtt_insert_string(&buffer_variable_header[len_variable_header],
                                                        topic,
                                                        strlen(topic));
    buffer_variable_header[len_variable_header] = (uint8_t)((s_mqtt_message_id & 0xFF00) >> 8);
    len_variable_header++;
    buffer_variable_header[len_variable_header] = (uint8_t)(s_mqtt_message_id & 0x00FF);
    len_variable_header++;

    //PAYLOAD
    len_payload += s_esp8266_mqtt_insert_string(&buffer_payload[len_payload],
                                                        message,
                                                        strlen(message));

    //CALCULATE REMAINING LENGTH (IN FIXED HEADER) FIELD
    p.fixed_header_remaining_length_len = s_esp8266_mqtt_calculate_remaining_length(len_variable_header, 
                                                                                    len_payload, 
                                                                                    &p.fixed_header_remaining_length[0]);

    //MAKE MQTT PACKET FROM BUFFERS
    p.variable_header = buffer_variable_header;
	p.variable_header_len = len_variable_header;
	p.payload = buffer_payload;
	p.payload_len = len_payload;
    if(s_esp8266_mqtt_client_debug)
    {
        os_printf("ESP8266 MQTT_CLIENT : PUBLISH packet created\n");
        os_printf("ESP8266 MQTT_CLIENT : topic : %s\n", topic);
        os_printf("ESP8266 MQTT_CLIENT : message :%s\n", message);
    }

    //SEND PACKET
    s_esp8266_mqtt_send_packet(p);
    if(s_esp8266_mqtt_client_debug)
    {
        os_printf("ESP8266 MQTT_CLIENT : PUBLISH packet sent\n");
    }

    //FREE BUFFER
    os_free(buffer_variable_header);
    os_free(buffer_payload);

    //INCREMENT MESSAGE ID
    s_mqtt_message_id++;

    //QOS = 0
    //NO PUBACK WILL BE RECEIVED
    //CALL THE TCP DATA RECEIVE CB FUNCTION RIGHT AWAY WITH NULL DATA
    //SET RETRY COUNT TO ESP8266_MQTT_RETRY_COUNT SO NO MORE TRIES ARE ATTEMPTED

    if(qos_level == ESP8266_MQTT_QOS_0)
    {
        if(s_esp8266_mqtt_client_debug)
        {
            os_printf("ESP8266 MQTT_CLIENT : PUBLISH done as Qos = 0. No PUBACK expected\n");
        }
        s_esp8266_mqtt_client_receive_cb(NULL, 0);
    }
}

void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_Send_Pingreq(void)
{
    //SEND MQTT PINGREQ PACKET

    esp8266_mqtt_packet_t p;

    //FIXED HEADER
    p.fixed_header_byte1 = (ESP8266_MQTT_CONTROL_PACKET_TYPE_PINGREQ << 4) | 
                            ESP8266_MQTT_CONTROL_PACKET_FLAG_PINGREQ;
    p.fixed_header_remaining_length[0] = 0;
    p.fixed_header_remaining_length_len = 1;

    //VARIABLE HEADER
    //NONE

    //PAYLOAD
    //NONE

    //MAKE MQTT PACKET FROM BUFFERS
    p.variable_header = NULL;
	p.variable_header_len = 0;
	p.payload = NULL;
	p.payload_len = 0;
    if(s_esp8266_mqtt_client_debug)
    {
        os_printf("ESP8266 MQTT_CLIENT : PINGREQ packet created\n");
    }

    //SEND PACKET
    s_esp8266_mqtt_send_packet(p);
    if(s_esp8266_mqtt_client_debug)
    {
        os_printf("ESP8266 MQTT_CLIENT : PINGREQ packet sent\n");
    }
}

void ICACHE_FLASH_ATTR ESP8266_MQTT_CLIENT_Send_Disconnect(void)
{
    //SEND MQTT DISCONNECT PACKET

    esp8266_mqtt_packet_t p;

    //FIXED HEADER
    p.fixed_header_byte1 = (ESP8266_MQTT_CONTROL_PACKET_TYPE_DISCONNECT << 4) | 
                            ESP8266_MQTT_CONTROL_PACKET_FLAG_DISCONNECT;
    p.fixed_header_remaining_length[0] = 0;
    p.fixed_header_remaining_length_len = 1;

    //VARIABLE HEADER
    //NONE

    //PAYLOAD
    //NONE

    //MAKE MQTT PACKET FROM BUFFERS
    p.variable_header = NULL;
	p.variable_header_len = 0;
	p.payload = NULL;
	p.payload_len = 0;
    if(s_esp8266_mqtt_client_debug)
    {
        os_printf("ESP8266 MQTT_CLIENT : DISCONNECT packet created\n");
    }

    //SEND PACKET
    s_esp8266_mqtt_send_packet(p);
    if(s_esp8266_mqtt_client_debug)
    {
        os_printf("ESP8266 MQTT_CLIENT : DISCONNECT packet sent\n");
    }
}

//INTERNAL FUNCTIONS
static void ICACHE_FLASH_ATTR s_esp8266_mqtt_print_packet(uint8_t* packet, uint16_t len)
{
    //PRINT MQTT PACKET
    //AS HEX BYTES ON TERMINAL

    uint16_t counter;
    for(counter = 0; counter < len; counter++)
    {
        if((counter % 8) == 0)
        {
            os_printf("\n");
        }
        os_printf("%02X ", packet[counter]);
    }
    os_printf("\n\n");
}

static uint8_t ICACHE_FLASH_ATTR s_esp8266_mqtt_insert_string(uint8_t* dest_buff, char* src_buff, uint8_t len)
{
    //PROCESS MQTT STRING
    //INSERT INTO SPECIFIED BUFFER LOCATION + PREPEND 2 BYTES FOR STRING LENGTH
    //AS MQTT REQUIRES

    os_strncpy(&dest_buff[2], src_buff, len);
    dest_buff[0] = (uint8_t)((len & 0xFF00) >> 8);
    dest_buff[1] = (uint8_t)(len & 0x00FF);

    return (len+2);
}

static uint8_t ICACHE_FLASH_ATTR s_esp8266_mqtt_calculate_remaining_length(uint16_t len_variable_header, uint16_t len_payload, uint8_t* ptr_remaining_length)
{
    //CALCULATE THE REMAINING LENGTH FIELD (IN FIXED HEADER) USING VARIABLE HEADER LENGTH &
    //PAYLOAD LENGTH USING MQTT SUPPLIED ALGORITHM

    uint8_t byte;
    uint8_t counter = 0;
    uint16_t len_total = len_variable_header + len_payload;

    do
    {
        byte = len_total % 128;
        len_total = len_total / 128;
        if(len_total > 0)
        {
            byte = byte | 0x80;
        }
        ptr_remaining_length[counter] = byte;
        counter++;
    }while(len_total > 0);
    return counter;
}

static void ICACHE_FLASH_ATTR s_esp8266_mqtt_send_packet(esp8266_mqtt_packet_t packet)
{
    //SEND THE PROVIDED MQTT PAKCET THROUGH TCP LAYER

    //ASSEMBLE THE PACKET
    uint8_t* assembled_blob = (uint8_t*)os_zalloc(s_buffer_size);
    uint16_t counter= 0;

    assembled_blob[0] = packet.fixed_header_byte1;
    counter = 1;
    os_memcpy(&assembled_blob[counter], packet.fixed_header_remaining_length, packet.fixed_header_remaining_length_len);
    counter += packet.fixed_header_remaining_length_len;

    os_memcpy(&assembled_blob[counter], packet.variable_header, packet.variable_header_len);
    counter += packet.variable_header_len;

    os_memcpy(&assembled_blob[counter], packet.payload, packet.payload_len);
    counter += packet.payload_len;

    //PRINT PACKET
    ESP8266_TCP_GENERIC_SendAndGetReply(assembled_blob, counter);

    if(s_esp8266_mqtt_client_debug)
    {
        s_esp8266_mqtt_print_packet(assembled_blob, counter);
        os_printf("ESP8266 MQTT_CLIENT : Packet sent!\n");
    }
    os_free(assembled_blob);
}

esp8266_mqtt_client_packet_type_t ICACHE_FLASH_ATTR s_esp8266_mqtt_parse_response_packet(uint8_t* packet, uint16_t len)
{
    //PARSE THE RESPONSE MQTT PACKET

    //BYTE 0 : MESSAGE TYPE + FLAGS
    if(s_esp8266_mqtt_client_debug)
    {
        os_printf("ESP8266 MQTT_CLIENT : packet type = %u ", (packet[0] & 0xF0) >> 4);
    }
    switch((packet[0] & 0xF0) >> 4)
    {
        case ESP8266_MQTT_CONTROL_PACKET_TYPE_CONNACK:
            if(s_esp8266_mqtt_client_debug)
            {
                os_printf("CONNACK\n");
            }
            break;
            
        case ESP8266_MQTT_CONTROL_PACKET_TYPE_PUBACK:
            if(s_esp8266_mqtt_client_debug)
            {
                os_printf("PUBACK\n");
            }
            break;

        case ESP8266_MQTT_CONTROL_PACKET_TYPE_PINGRESP:
            if(s_esp8266_mqtt_client_debug)
            {
                os_printf("PINGRESP\n");
            }
            break;

        default:
            //NOT A MQTT PACKET
            if(s_esp8266_mqtt_client_debug)
            {
                os_printf("ESP8266 MQTT_CLIENT : Malformed packet!\n");
            }
            return ESP8266_MQTT_CONTROL_PACKET_TYPE_INVALID;
            break;
    }
    if(s_esp8266_mqtt_client_debug)
    {
        os_printf("ESP8266 MQTT_CLIENT : packet flags 0x%02X !\n", (packet[0] & 0x0F));
    }

    //BYTE 1 : REMAINING LENGTH
    if(s_esp8266_mqtt_client_debug)
    {
        os_printf("ESP8266 MQTT_CLIENT : remaining length 0x%02X !\n", packet[1]);
    }

    //BYTE 3, BYTE 4 : VARIABLE HEADER
    //ONLY IF CONNACK PACKET TYPE
    if(((packet[0] & 0xF0) >> 4) == ESP8266_MQTT_CONTROL_PACKET_TYPE_CONNACK)
    {
        if(s_esp8266_mqtt_client_debug)
        {
            os_printf("ESP8266 MQTT_CLIENT : packet return code 0x%02X %s!\n", packet[3], s_packet_return_code[packet[3]]);
        }
    }

    //RETURN PACKET TYPE
    return ((packet[0] & 0xF0) >> 4);
}

static void ICACHE_FLASH_ATTR s_esp8266_mqtt_client_dns_found_cb(ip_addr_t* ipAddr)
{
    //DNS CB
    if(ipAddr == NULL)
    {
        if(s_esp8266_mqtt_client_debug)
        {
            os_printf("ESP8266 MQTT_CLIENT : DNS Failed!\n");
        }
    }
    else
    {
        if(s_esp8266_mqtt_client_debug)
        {
            os_printf("ESP8266 MQTT_CLIENT : DNS Resolved %d.%d.%d.%d!\n", IP2STR(ipAddr));
        }
    }
    
    if(s_esp8266_mqtt_client_dns_found_cb)
    {
        (*s_esp8266_mqtt_client_dns_cb_function)(ipAddr);
    }

}

static void ICACHE_FLASH_ATTR s_esp8266_mqtt_tcp_conn_cb(void* arg)
{
    //TCP CONNECT CB

    //CALL USER TCP CONN CB FUNCTION IF NOT NULL
    if(s_esp8266_mqtt_client_tcp_conn_cb_function)
    {
        (*s_esp8266_mqtt_client_tcp_conn_cb_function)();
    }
}

static void ICACHE_FLASH_ATTR s_esp8266_mqtt_client_send_cb(void* arg)
{
    //DATA SEND CB
    if(s_esp8266_mqtt_client_data_send_cb != NULL)
    {
        (*s_esp8266_mqtt_client_data_send_cb)(arg);
    }
    return;
}

static void ICACHE_FLASH_ATTR s_esp8266_mqtt_client_receive_cb(char* pusrdata, unsigned short length)
{
    //DATA RECV CB

    if(!pusrdata)
    {
        if(s_esp8266_mqtt_client_debug && s_current_packet_type != ESP8266_MQTT_CONTROL_PACKET_TYPE_PUBLISH)
        {
            os_printf("ESP8266 MQTT_CLIENT : reply timeout!\n");
        }
        //CALL USER CB IF NOT NULL
        if(s_esp8266_mqtt_client_data_recv_cb != NULL)
        {
            (*s_esp8266_mqtt_client_data_recv_cb)(ESP8266_MQTT_CONTROL_PACKET_TYPE_INVALID, NULL, 0);
        }
    }
    else
    {
        if(s_esp8266_mqtt_client_debug)
        {
            os_printf("ESP8266 MQTT_CLIENT : Data received!\n");
        }
        //PARSE MQTT PACKET
        esp8266_mqtt_client_packet_type_t ptype = s_esp8266_mqtt_parse_response_packet(pusrdata, length);
        //CALL USER CB IF NOT NULL
        if(s_esp8266_mqtt_client_data_recv_cb != NULL)
        {
            (*s_esp8266_mqtt_client_data_recv_cb)(ptype, pusrdata, length);
        }
    }
}