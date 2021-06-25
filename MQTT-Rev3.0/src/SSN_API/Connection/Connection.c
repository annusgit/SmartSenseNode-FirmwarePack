#include "Connection.h"

uint8_t SetupConnectionWithDHCP(uint8_t* SSN_MAC_ADDRESS) {
	// Start Ethernet Now with a MAC address (either default MAC or custom SSN MAC)
	Ethernet_Register_MAC(SSN_MAC_ADDRESS);
	// configure wiz object to accept Static IP
	WIZ5500_network_information.dhcp = NETINFO_DHCP;
	// Get IP from DHCP, will only return once we have an IP
	Ethernet_get_IP_from_DHCP();
}

uint8_t SetupConnectionWithStaticIP(uint8_t* SSN_MAC_ADDRESS, uint8_t* static_IP, uint8_t* subnet_mask, uint8_t* gateway, uint8_t* dns) {
	// Start Ethernet Now with a MAC address (either default MAC or custom SSN MAC)
	Ethernet_Register_MAC(SSN_MAC_ADDRESS);
	// configure wiz object to accept Static IP
	WIZ5500_network_information.dhcp = NETINFO_STATIC;
	// Setup static ip for the SSN
	Ethernet_set_Static_IP(static_IP, subnet_mask, gateway, dns);
}

uint8_t SetupConnectionWithStaticIPAndReturnSocket(uint8_t UDP_SOCKET, uint8_t* SSN_MAC_ADDRESS, uint8_t* static_IP, uint8_t* subnet_mask, uint8_t* gateway, uint8_t* dns) {
	SetupConnectionWithStaticIP(SSN_MAC_ADDRESS, static_IP, subnet_mask, gateway, dns);
	// Our main UDP socket is defined now
	return socket(UDP_SOCKET, Sn_MR_UDP, SSN_DEFAULT_PORT, 0x00);
}


uint8_t ResetConnection(uint8_t* SSN_MAC_ADDRESS, uint8_t UDP_SOCKET) {
	// Reset the device first
	Ethernet_Reset();
	// Start Ethernet Now with a MAC address (either default MAC or custom SSN MAC)
	Ethernet_Register_MAC(SSN_MAC_ADDRESS);
	// Initiate the Network again
	WIZ5500_network_initiate();
	// return a new socket from the W5500
	return socket(UDP_SOCKET, Sn_MR_UDP, SSN_DEFAULT_PORT, 0x00);
}

void GetServerIP_UsingDNS(uint8_t* DNS_ADDRESS, uint8_t* DEFAULT_SERVER_IP, unsigned char* MQTT_SERVER_DNS_STRING, uint8_t* SSN_SERVER_IP){
	start_ms_timer_with_interrupt();
    unsigned char tempBuffer[BUFFER_SIZE] = {0};
    unsigned int timeout_count_to_1sec = 1000;
    DNS_init(DNS_SOCKET_NUMBER, tempBuffer);
    T5CON = 0x8000;
    TMR5 = 0;
    Clear_LED_INDICATOR();
	printf("[LOG]: Looking Up Domain [%s] at DNS IP [%d.%d.%d.%d]\n", MQTT_SERVER_DNS_STRING, DNS_ADDRESS[0], DNS_ADDRESS[1], DNS_ADDRESS[2], DNS_ADDRESS[3]);
    while (DNS_run(DNS_ADDRESS, MQTT_SERVER_DNS_STRING, SSN_SERVER_IP) == 0){
        if (TMR5 > PERIPH_CLK/10000) {
            if (timeout_count_to_1sec--) {
                if (timeout_count_to_1sec % 200) {
                    SSN_LED_INDICATE(LOOKING_UP_DNS);
                    ServiceWatchdog();
                }
            } else {
                // our max number of timeouts have been exceeded. 
                break;
            }
            TMR5 = 0;
        }
    }
    if (timeout_count_to_1sec <= 0) {
        SSN_SERVER_IP[0] = DEFAULT_SERVER_IP[0];
        SSN_SERVER_IP[1] = DEFAULT_SERVER_IP[1];
        SSN_SERVER_IP[2] = DEFAULT_SERVER_IP[2];
        SSN_SERVER_IP[3] = DEFAULT_SERVER_IP[3];
        printf("[LOG]: DNS Lookup couldn't retrieve server IP. Using hard-coded server IP: %d.%d.%d.%d\n", SSN_SERVER_IP[0], SSN_SERVER_IP[1], SSN_SERVER_IP[2],SSN_SERVER_IP[3]);
    } else {
        printf("[LOG]: DNS Lookup Successful. Server IP: %d.%d.%d.%d\n", SSN_SERVER_IP[0], SSN_SERVER_IP[1], SSN_SERVER_IP[2],SSN_SERVER_IP[3]);
    }
    // reset timer parameters
    TMR5 = 0;
    T5CONCLR = 0x8000;
    Clear_LED_INDICATOR();
	stop_ms_timer_with_interrupt();
}