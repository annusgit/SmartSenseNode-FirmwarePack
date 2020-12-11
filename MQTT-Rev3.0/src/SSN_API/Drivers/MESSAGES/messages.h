#ifndef __messages_h__
#define __messages_h__


#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

#include <xc.h>
#include <p32xxxx.h>
#include <plib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../../global.h"

/** Maximum Message size in bytes to send over the network */
#define max_send_message_size   65
/** Maximum Message size in bytes to receive over the network */
#define max_recv_message_size   65

/** Message id for requesting MAC address from Server */
#define GET_MAC_MESSAGE_ID              1
/** Message id for set MAC address message received from Server */
#define SET_MAC_MESSAGE_ID              2
/** Message id for requesting time of day from Server */
#define GET_TIMEOFDAY_MESSAGE_ID        3
/** Message id for set time of day message received from Server */
#define SET_TIMEOFDAY_MESSAGE_ID        4
/** Message id for requesting current sensor configurations from Server */
#define GET_CONFIG_MESSAGE_ID           5
/** Message id for set current sensor configurations message received from Server */
#define SET_CONFIG_MESSAGE_ID           6
/** Message id for acknowledge current sensor configurations message to Server */
#define ACK_CONFIG_MESSAGE_ID           7
/** Message id for status update message to send to Server */
#define STATUS_UPDATE_MESSAGE_ID        8
/** Message id for machine reset accumulated time received from Server */
#define RESET_MACHINE_TIME_MESSAGE_ID   9

/** Message id for requesting SSN EEPROM Clear received from Server */
#define DEBUG_EEPROM_CLEAR_MESSAGE_ID   10
/** Message id for requesting SSN Reset received from Server */
#define DEBUG_RESET_SSN_MESSAGE_ID      11

#define GET_MAC_MESSAGE_Size            7
#define SET_MAC_MESSAGE_Size            13
#define GET_TIMEOFDAY_MESSAGE_Size      7
#define SET_TIMEOFDAY_MESSAGE_Size      11
#define GET_CONFIG_MESSAGE_Size         7
#define SET_CONFIG_MESSAGE_Size         28
#define ACK_CONFIG_MESSAGE_Size         16
#define STATUS_UPDATE_MESSAGE_Size      65
#define RESET_MACHINE_TIME_MESSAGE_Size 4


/**
 * Checks the validity of MAC address whether it is custom MAC or default MAC address of SSN
 * @param mac_address Six bytes of current MAC address of SSN
 * @return 1 if valid; 0 otherwise
 */
uint8_t is_Valid_MAC(uint8_t* mac_address);

/** 
 * Checks the validity of current sensor configurations
 * @param config_array Byte array of current configurations
 * @return 1 if valid; 0 otherwise
 */
uint8_t is_Valid_CONFIG(uint8_t* config_array);

/**
 * 
 * @param message_array
 * @param node_id
 * @return 
 */
void clear_array(uint8_t* this_array, uint32_t this_size);

/** 
 * Constructs GET_MAC request message
 * @param message_array Byte array to keep 
 * @param node_id Identity of SSN, last two bytes of SSN MAC address 
 * @return Message size in bytes
 */
uint8_t construct_get_mac_message(uint8_t* message_array, uint8_t* node_id);

/** 
 * Constructs GET_TimeOfDay request message
 * @param message_array Byte array to keep 
 * @param node_id Identity of SSN, last two bytes of SSN MAC address 
 * @return Message size in bytes
 */
uint8_t construct_get_timeofday_message(uint8_t* message_array, uint8_t* node_id);

/** 
 * Constructs GET_CONFIG request message to retrieve current sensor configurations
 * @param message_array Byte array to keep 
 * @param node_id Identity of SSN, last two bytes of SSN MAC address 
 * @return Message size in bytes
 */
uint8_t construct_get_configuration_message(uint8_t* message_array, uint8_t* node_id);

/** 
 * Constructs Acknowledge Configuration message
 * @param message_array Byte array to keep 
 * @param node_id Identity of SSN, last two bytes of SSN MAC address 
 * @param received_configs Byte array of current sensor configurations received from Server 
 * @return Message size in bytes
 */
uint8_t construct_ack_configuration_message(uint8_t* message_array, uint8_t* node_id, uint8_t* received_configs);

/** 
 * Constructs periodic status update message for Server
 * @param message_array Byte array for keeping the constructed message
 * @param node_id Node identity in two bytes, last two bytes of current MAC address
 * @param temperature_bytes Byte array saving the temperature reading high and low bytes
 * @param relative_humidity_bytes Byte array saving the relative humidity high and low bytes
 * @param Machine_load_currents A float array of machine load currents calculated by SSN
 * @param Machine_load_percentages A byte array of machine load percentages calculated by SSN
 * @param Machine_status A byte array of machine status (ON/OFF/IDLE) calculated by SSN
 * @param Machine_status_duration An array of machine status duration indicating for how long the machines have been in current state
 * @param Machine_status_timestamp An array of machine status timestamp indicating since when the machines have been in current state
 * @param node_uptime_in_seconds A four byte integer containing the number of seconds indicating for how long the SSN has been awake
 * @param abnormal_activity A single byte indicating NORMAL or ABNORMAL ambient condition based on temperature and humidity readings
 * @return Message size in bytes
 */
uint8_t construct_status_update_message(uint8_t* message_array, uint8_t* node_id, uint8_t* temperature_bytes, uint8_t* relative_humidity_bytes, float* Machine_load_currents, 
        uint8_t* Machine_load_percentages, uint8_t* Machine_status, uint8_t Machine_status_flag, uint32_t* Machine_status_duration, uint32_t* Machine_status_timestamp, 
        uint32_t node_uptime_in_seconds, uint8_t abnormal_activity);

/**
 * Deciphers the received message and returns whatever data was received with it
 * @param message Message byte array received from Server
 * @param params The parameters array to save the received data into in specific order
 * @return Message ID of received message
 */
uint8_t decipher_received_message(uint8_t* message, uint8_t* params);


#endif