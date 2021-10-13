#ifndef __network_h__
#define __network_h__

#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

#include <plib.h>
#include <stdio.h>
#include <string.h>
#include "../../global.h"
#include "Ethernet/socket.h"
#include "Internet/DHCP/dhcp.h"

#define WIZ5500_R_COMMON_RTR    0x001A0100 // Reset Value: 0xD0
#define WIZ5500_W_COMMON_RTR    0x001A05F1 // Write Value: 0xF1
#define WIZ5500_R_COMMON_RCR    0x001B0100 // Reset Value: 0x08
#define WIZ5500_W_COMMON_RCR    0x001B05F1 // Write Value: 0xF1

#define TCP_SOCKET      2
#define MAX_LEN         100
#define BUFFER_SIZE     2048
#define MQTTPort        1883    // mqtt server port

///////////////////////////////////////
// Debugging Message Printout enable //
///////////////////////////////////////
#define _MAIN_DEBUG_
#define _DHCP_DEBUG_

/***************************************
 * SOCKET NUMBER DEFINION for Examples *
 ***************************************/
#define DHCP_SOCKET			0
#define MY_MAX_DHCP_RETRY	3

/**********************************************
 * Shared Buffer Definition for LOOPBACK TEST *
 **********************************************/
#define DATA_BUF_SIZE   2048

extern uint8_t gDATABUF[DATA_BUF_SIZE];
/*****************************************************************************
 * @brief Default Network Inforamtion
 *****************************************************************************/
extern uint32_t msTicks;  /* counts 1ms timeTicks */
extern uint32_t prevTick;          /* */
extern wiz_NetInfo WIZ5500_network_information;

// Device level functions

/**
 * Resets W5500 Ethernet offload chip
 */
void WIZ5500_Reset();

/**
 * Restarts Ethernet connection by reseting W5500 Ethernet chip
 */
void Ethernet_Reset();

/**
 * Opens SPI2 for communication
 */
void open_SPI2();

/**
 * Sets up Ethernet for communication
 */
void setup_Ethernet();

/**
 * Sends and Receives a single byte over SPI interface
 * @param data Single byte to send over SPI
 * @return Single byte received over SPI
 */
unsigned int SPI2_send(unsigned int data);

/**
 * Selects the W5500 chip for communication
 */
void WIZ5500_select(void);

/**
 * Deselects the W5500 chip for communication
 */
void WIZ5500_deselect(void);

/**
 * Writes a single byte to W5500 chip
 * @param wb Byte to write
 */
void WIZ5500_write_byte(uint8_t wb);

/**
 * Reads a single byte from W5500 chip
 * @return Single byte received from W5500 chip
 */
uint8_t WIZ5500_read_byte();

/**
 * Writes a byte array to W5500 chip
 * @param addrBuf Byte array containing the addresses to write at
 * @param pBuf Byte array of values to write at those locations
 * @param len The number of values to write at W5500 
 */
void WIZ5500_write_array(uint8_t* addrBuf, uint8_t* pBuf, uint16_t len);

/**
 * Reads a byte array from W5500 chip
 * @param addrBuf Byte array containing the addresses to read from
 * @param pBuf Byte array to write the read values into
 * @param len The number of values to read from W5500 
 */
void WIZ5500_read_array(uint8_t* addrBuf, uint8_t* pBuf, uint16_t len);

/**
 * Initializes the W5500 chip from network communication
 */
void WIZ5500_network_initiate(void);

/**
 * Callback function for when IP is received via DHCP
 */
void WIZ5500_IP_assigned_callback(void);

/**
 * Callback function for when IP conflict occurs
 */
void WIZ5500_IP_conflict_callback(void);

// API level function
/**
 * Gets physical link status from the W5500 chip whether a network cable is connected to network or not
 * @return <b>PHY_LINK_ON</b> if connection is available; <b>PHY_LINK_OFF</b> otherwise
 */
uint8_t Ethernet_get_physical_link_status();

/**
 * Saves a MAC address for W5500 chip. Does **not** assign the MAC address to Wiz5500
 * @param this_mac The byte array containing the MAC address
 */
void Ethernet_Register_MAC(uint8_t* this_mac, char* node_exclusive_channel);
void Ethernet_Save_Static_IP(uint8_t* this_IP);
void Ethernet_Save_Subnet_Mask(uint8_t* this_subnet);
void Ethernet_Save_Gateway_Address(uint8_t* this_gateway);
void Ethernet_Save_DNS(uint8_t* this_dns);

/**
 * Gets an IP from DHCP; does not return until an IP is successfully retrieved
 */
void Ethernet_get_IP_from_DHCP();

/**
 * Setup a Static IP
 */
void Ethernet_set_Static_IP(uint8_t* static_IP, uint8_t* subnet_mask, uint8_t* gateway, uint8_t* dns);

#endif
