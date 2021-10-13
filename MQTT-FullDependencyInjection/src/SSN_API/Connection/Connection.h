#ifndef __Connection_h__
#define __Connection_h__

#include "../global.h"
#include "../Drivers/UART/uart.h"
#include "../Drivers/NETWORK/network.h"

#define DNS_SOCKET_NUMBER   1

/** 
 * Sets up Ethernet Connection for SSN
 * @param ssn_mac_address Six byte array containing the six bytes of SSN MAC address
 * @param UDP_SOCKET SSN UDP socket number  
 * @return Socket Number, should be the same as UDP_SOCKET if successfully created
 */
uint8_t SetupConnectionWithDHCP(uint8_t* ssn_mac_address, char* node_exclusive_channel, wiz_NetInfo* wiznet_network_information_object);
uint8_t SetupConnectionWithStaticIP(uint8_t* ssn_mac_address, char* node_exclusive_channel, wiz_NetInfo* wiznet_network_information_object, uint8_t* static_IP, uint8_t* subnet_mask, 
        uint8_t* gateway, uint8_t* dns);
uint8_t SetupConnectionWithStaticIPAndReturnSocket(uint8_t UDP_SOCKET, uint8_t* ssn_mac_address, char* node_exclusive_channel, wiz_NetInfo* wiznet_network_information_object, 
	uint8_t* static_IP, uint8_t* subnet_mask, uint8_t* gateway, uint8_t* dns);

/**
 * Resets Ethernet chip and gets MAC address, IP and other network credentials assigned again
 * @param ssn_current_mac The MAC address to assign to W5500 when it resets
 * @param UDP_SOCKET udp socket number for SSN communication
 * @return Will return a new socket for communication
 */
uint8_t ResetConnection(uint8_t* ssn_current_mac, char* node_exclusive_channel, uint8_t UDP_SOCKET);
void GetServerIP_UsingDNS(uint8_t* dns_address, uint8_t* default_server_ip, unsigned char* mqtt_server_dns_string, uint8_t* ssn_server_ip);

#endif