
#ifndef __Connection_h__
#define __Connection_h__

#include "../global.h"
#include "../Drivers/UART/uart.h"
#include "../Drivers/NETWORK/network.h"

/** 
 * Sets up Ethernet Connection for SSN
 * @param SSN_MAC_ADDRESS Six byte array containing the six bytes of SSN MAC address
 * @param UDP_SOCKET SSN UDP socket number  
 * @return Socket Number, should be the same as UDP_SOCKET if successfully created
 */
uint8_t SetupConnectionWithDHCP(uint8_t* SSN_MAC_ADDRESS, uint8_t UDP_SOCKET);

uint8_t SetupConnectionWithStaticIP(uint8_t* SSN_MAC_ADDRESS, uint8_t* static_IP, uint8_t* subnet_mask, uint8_t* gateway, uint8_t* dns);
uint8_t SetupConnectionWithStaticIPAndReturnSocket(uint8_t UDP_SOCKET, uint8_t* SSN_MAC_ADDRESS, uint8_t* static_IP, uint8_t* subnet_mask, uint8_t* gateway, uint8_t* dns);

/**
 * Resets Ethernet chip and gets MAC address, IP and other network credentials assigned again
 * @param SSN_CURRENT_MAC The MAC address to assign to W5500 when it resets
 * @param UDP_SOCKET udp socket number for SSN communication
 * @return Will return a new socket for communication
 */
uint8_t ResetConnection(uint8_t* SSN_CURRENT_MAC, uint8_t UDP_SOCKET);

struct MQTTClient SetupMQTTClientConnection(Network* net, MQTTClient* mqtt_client, opts_struct* MQTTOptions, uint8_t *MQTT_IP, char* cliendId, void* messageArrivedoverMQTT);

#endif