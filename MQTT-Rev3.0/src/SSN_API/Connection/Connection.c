#include "Connection.h"
uint8_t DNS_ADDRESS[4] = {8, 8, 8, 8};

uint8_t SetupConnectionWithDHCP(uint8_t* SSN_MAC_ADDRESS) {
	// Start Ethernet Now with a MAC address (either default MAC or custom SSN MAC)
	Ethernet_Register_MAC(SSN_MAC_ADDRESS);
	// Get IP from DHCP, will only return once we have an IP
	Ethernet_get_IP_from_DHCP();
	// Our main UDP socket is defined now
	// return socket(UDP_SOCKET, Sn_MR_UDP, SSN_DEFAULT_PORT, 0x00);
}

uint8_t SetupConnectionWithStaticIP(uint8_t* SSN_MAC_ADDRESS, uint8_t* static_IP, uint8_t* subnet_mask, uint8_t* gateway, uint8_t* dns) {
	// Start Ethernet Now with a MAC address (either default MAC or custom SSN MAC)
	Ethernet_Register_MAC(SSN_MAC_ADDRESS);
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

void GetServerIP_UsingDNS(uint8_t* DEFAULT_SERVER_IP, unsigned char* MQTT_SERVER_DNS, uint8_t* SSN_SERVER_IP){
    unsigned char tempBuffer[BUFFER_SIZE] = {};
    DNS_init(1, tempBuffer);
    T5CON = 0x8000;
    TMR5 = 0;
    while ((DNS_run(DNS_ADDRESS, MQTT_SERVER_DNS, SSN_SERVER_IP) == 0) && (TMR5 < PERIPH_CLK)){
        SSN_SERVER_IP[0] = DEFAULT_SERVER_IP[0];
        SSN_SERVER_IP[1] = DEFAULT_SERVER_IP[1];
        SSN_SERVER_IP[2] = DEFAULT_SERVER_IP[2];
        SSN_SERVER_IP[3] = DEFAULT_SERVER_IP[3];        
        break;
    }
    TMR5 = 0;
    T5CONCLR = 0x8000;
//    printf("%s\n", MQTT_SERVER_DNS);
//    printf("server %d.%d.%d.%d\n", SSN_SERVER_IP[0], SSN_SERVER_IP[1], SSN_SERVER_IP[2], SSN_SERVER_IP[3]);
}
