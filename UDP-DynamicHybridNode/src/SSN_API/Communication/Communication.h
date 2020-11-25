
#ifndef __Communication_h__
#define __Communication_h__

#include "../global.h"
#include "../Drivers/UART/uart.h"
#include "../Drivers/EEPROM/eeprom.h"
#include "../Drivers/NETWORK/network.h"
#include "../Drivers/CURRENT_SENSOR/current_sensor.h"
#include "../Drivers/Messages/messages.h"

extern uint8_t message_to_send[max_send_message_size];
extern uint8_t message_to_recv[max_recv_message_size];
extern uint8_t params[max_recv_message_size];

/** Data Node Specific Variables */
extern uint8_t SENDER_IP[4];
extern uint16_t SENDER_PORT;

/** This dictionary maps MAC addresses to IP addresses */
#define IP_LEN          4
#define MAC_LEN         6
#define DICT_LEN        10
#define No_Match_Found  -1
typedef struct MAC_IP_Dictionary {
    uint8_t count;
    uint8_t mac_addresses[DICT_LEN*MAC_LEN];
    uint8_t ip_addresses[DICT_LEN*IP_LEN];
} MAC_IP_Dictionary;
extern MAC_IP_Dictionary routing_dictionary;

/**
 * Sends a message over UDP
 * @param SSN_Socket UDP socket used by SSN
 * @param SSN_SERVER_IP IP of the destination Server
 * @param SSN_SERVER_PORT Port of the destination Server
 * @param message_to_send The byte or char array of message
 * @param ssn_message_to_send_size Message size in bytes
 */
bool SendMessage(uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT, uint8_t* message_to_send, uint8_t ssn_message_to_send_size);

/**
 * Sends a MAC Request message to receive a custom MAC address for host SSN
 * @param NodeID Two byte identity of SSN which are the last two bytes of the MAC address
 * @param SSN_Socket UDP socket used by SSN
 * @param SSN_SERVER_IP IP of the destination Server
 * @param SSN_SERVER_PORT Port of the destination Server
 */
void Send_GETMAC_Message(uint8_t* NodeID, uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT);

/**
 * Sends a Sensor Configuration Request message to receive a configuration for SSN to compute statistics of the connected machines
 * @param NodeID Two byte identity of SSN which are the last two bytes of the MAC address
 * @param SSN_Socket UDP socket used by SSN
 * @param SSN_SERVER_IP IP of the destination Server
 * @param SSN_SERVER_PORT Port of the destination Server
 */
void Send_GETCONFIG_Message(uint8_t* NodeID, uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT);

/**
 * Sends an Acknowledge for Sensor Configuration message received from SSN Server
 * @param NodeID Two byte identity of SSN which are the last two bytes of the MAC address
 * @param SSN_Socket UDP socket used by SSN
 * @param SSN_SERVER_IP IP of the destination Server
 * @param SSN_SERVER_PORT Port of the destination Server
 * @param SSN_CONFIG Configurations array previously received from SSN Server 
 */
void Send_ACKCONFIG_Message(uint8_t* NodeID, uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT, uint8_t* SSN_CONFIG);

/**
 * Sends a Time of Day Request message to SSN Server to receive the current time of day 
 * @param NodeID Two byte identity of SSN which are the last two bytes of the MAC address
 * @param SSN_Socket UDP socket used by SSN
 * @param SSN_SERVER_IP IP of the destination Server
 * @param SSN_SERVER_PORT Port of the destination Server
 */
void Send_GETTimeOfDay_Message(uint8_t* NodeID, uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT);

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
bool Send_STATUSUPDATE_Message(uint8_t* NodeID, uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT, uint8_t* temperature_bytes, uint8_t* relative_humidity_bytes, 
    float* Machine_load_currents, uint8_t* Machine_load_percentages, uint8_t* Machine_status, uint8_t Machine_status_flag, uint32_t* Machine_status_duration, 
    uint32_t* Machine_status_timestamp, uint32_t ssn_uptime_in_seconds, uint8_t abnormal_activity);

/**
 * Receives a destination MAC for incoming message and decides whether it is meant for itself or not
 */
bool SSN_I_AM_DESTINATION(uint8_t* SSN_Mac_Address, uint8_t* destination_of_message);

/** This function will look for a MAC address in our routing table */
int8_t find_in_dictionary(MAC_IP_Dictionary* dictionary, uint8_t* mac_address);

/**
 * Receives a response for MAC requested from SSN Server
 * @param SSN_Socket UDP socket used by SSN
 * @param SSN_SERVER_IP IP of the destination Server
 * @param SSN_SERVER_PORT Port of the destination Server
 */
void Receive_MAC(uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT);

/**
 * Receives a response for Sensor Configurations requested from SSN Server
 * @param SSN_Socket UDP socket used by SSN
 * @param SSN_SERVER_IP IP of the destination Server
 * @param SSN_SERVER_PORT Port of the destination Server
 * @param SSN_CONFIG Byte array to save received configurations
 * @param SSN_REPORT_INTERVAL Pointer to a single byte that saves the report interval for SSN status update message
 * @param SSN_CURRENT_SENSOR_RATINGS Byte array for saving current sensor ratings
 * @param SSN_CURRENT_SENSOR_THRESHOLDS Float array for saving machine thresholds for deciding IDLE state of machines
 * @param SSN_CURRENT_SENSOR_MAXLOADS Byte array for saving machine maximum loads for calculating percentage loads
 * @param Machine_status Byte array of current machine status (ON/OFF/IDLE) 
 * @return 1 if received, else 0
 */
uint8_t Receive_CONFIG(uint8_t SSN_Socket, uint8_t* SSN_MAC_ADDRESS, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT, uint8_t* SSN_CONFIG, uint8_t* SSN_REPORT_INTERVAL, uint8_t* TEMPERATURE_MIN_THRESHOLD, 
    uint8_t* TEMPERATURE_MAX_THRESHOLD, uint8_t* HUMIDITY_MIN_THRESHOLD, uint8_t* HUMIDITY_MAX_THRESHOLD, uint8_t* SSN_CURRENT_SENSOR_RATINGS,  uint8_t* SSN_CURRENT_SENSOR_THRESHOLDS, 
    uint8_t* SSN_CURRENT_SENSOR_MAXLOADS, float* SSN_CURRENT_SENSOR_VOLTAGE_SCALARS, uint8_t* Machine_status);

/**
 * Receives a response for Time of Day requested from SSN Server
 * @param SSN_Socket UDP socket used by SSN
 * @param SSN_SERVER_IP IP of the destination Server
 * @param SSN_SERVER_PORT Port of the destination Server
 * @return 1 if received, else 0
 */
uint8_t Receive_TimeOfDay(uint8_t SSN_Socket, uint8_t* SSN_MAC_ADDRESS, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT);

#endif
