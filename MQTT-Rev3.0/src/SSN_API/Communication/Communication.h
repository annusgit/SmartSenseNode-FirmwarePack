#ifndef __Communication_h__
#define __Communication_h__

#include "../global.h"
#include "../Drivers/UART/uart.h"
#include "../Drivers/EEPROM/eeprom.h"
#include "../Drivers/NETWORK/network.h"
#include "../Drivers/CURRENT_SENSOR/current_sensor.h"
#include "../Drivers/Messages/messages.h"

static uint8_t message_to_send[max_send_message_size];
static uint8_t message_to_recv[max_recv_message_size];
static uint8_t params[max_recv_message_size];
static unsigned char stringtosend[stringtosendsize];    
static unsigned char* errorstring;

/**
 * Sends a MAC Request message to receive a custom MAC address for host SSN
 * @param NodeID Two byte identity of SSN which are the last two bytes of the MAC address
 * @param SSN_Socket UDP socket used by SSN
 * @param SSN_SERVER_IP IP of the destination Server
 * @param SSN_SERVER_PORT Port of the destination Server
 */
int Send_GETMAC_Message(uint8_t* NodeID);

/**
 * Sends a Sensor Configuration Request message to receive a configuration for SSN to compute statistics of the connected machines
 * @param NodeID Two byte identity of SSN which are the last two bytes of the MAC address
 * @param SSN_Socket UDP socket used by SSN
 * @param SSN_SERVER_IP IP of the destination Server
 * @param SSN_SERVER_PORT Port of the destination Server
 */
int Send_GETCONFIG_Message(uint8_t* NodeID);

/**
 * Sends an Acknowledge for Sensor Configuration message received from SSN Server
 * @param NodeID Two byte identity of SSN which are the last two bytes of the MAC address
 * @param SSN_Socket UDP socket used by SSN
 * @param SSN_SERVER_IP IP of the destination Server
 * @param SSN_SERVER_PORT Port of the destination Server
 * @param SSN_CONFIG Configurations array previously received from SSN Server 
 */
int Send_ACKCONFIG_Message(uint8_t* NodeID, uint8_t* SSN_CONFIG);

/**
 * Sends a Time of Day Request message to SSN Server to receive the current time of day 
 * @param NodeID Two byte identity of SSN which are the last two bytes of the MAC address
 * @param SSN_Socket UDP socket used by SSN
 * @param SSN_SERVER_IP IP of the destination Server
 * @param SSN_SERVER_PORT Port of the destination Server
 */
int Send_GETTimeOfDay_Message(uint8_t* NodeID);

/**
 * Sends a Status Update message to SSN Server containing the machine status and ambient conditions
 * @param NodeID Two byte identity of SSN which are the last two bytes of the MAC address
 * @param SSN_Socket UDP socket used by SSN
 * @param SSN_SERVER_IP IP of the destination Server
 * @param temperature_bytes Two byte temperature reading containing the high and low byte of temperature in that order
 * @param relative_humidity_bytes Two byte relative humidity reading containing the high and low byte of humidity in that order
 * @param Machine_load_currents A byte array of machine load currents calculated by SSN
 * @param Machine_load_percentages A byte array of machine load percentages calculated by SSN
 * @param Machine_status A byte array of machine status (ON/OFF/IDLE) calculated by SSN
 * @param Machine_status_duration An array of machine status duration indicating for how long the machines have been in current state
 * @param Machine_status_timestamp An array of machine status timestamp indicating since when the machines have been in current state
 * @param ssn_uptime_in_seconds A four byte integer containing the number of seconds indicating for how long the SSN has been awake
 * @param abnormal_activity A single byte indicating NORMAL or ABNORMAL ambient condition based on temperature and humidity readings
 */
int Send_STATUSUPDATE_Message(uint8_t* NodeID, uint8_t* temperature_bytes, uint8_t* relative_humidity_bytes, float* Machine_load_currents, uint8_t* Machine_load_percentages, 
        uint8_t* Machine_status, uint8_t Machine_status_flag, uint32_t* Machine_status_duration, uint32_t* Machine_status_timestamp, uint32_t ssn_uptime_in_seconds, 
        uint8_t abnormal_activity);
#endif
