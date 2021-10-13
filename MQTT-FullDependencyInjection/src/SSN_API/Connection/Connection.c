#include "Connection.h"

uint8_t SetupConnectionWithDHCP(uint8_t* ssn_mac_address, char* node_exclusive_channel, wiz_NetInfo* wiznet_network_information_object) {
	// Start Ethernet Now with a MAC address (either default MAC or custom SSN MAC)
	Ethernet_Register_MAC(ssn_mac_address, node_exclusive_channel);
	// configure wiz object to accept Static IP
	wiznet_network_information_object->dhcp = NETINFO_DHCP;
	// Get IP from DHCP, will only return once we have an IP
	Ethernet_get_IP_from_DHCP();
}

uint8_t SetupConnectionWithStaticIP(uint8_t* ssn_mac_address, char* node_exclusive_channel, wiz_NetInfo* wiznet_network_information_object, uint8_t* static_IP, uint8_t* subnet_mask, 
	uint8_t* gateway, uint8_t* dns) {
	// Start Ethernet Now with a MAC address (either default MAC or custom SSN MAC)
	Ethernet_Register_MAC(ssn_mac_address, node_exclusive_channel);
	// configure wiz object to accept Static IP
	wiznet_network_information_object->dhcp = NETINFO_STATIC;
	// Setup static ip for the SSN
	Ethernet_set_Static_IP(static_IP, subnet_mask, gateway, dns);
}

uint8_t SetupConnectionWithStaticIPAndReturnSocket(uint8_t UDP_SOCKET, uint8_t* ssn_mac_address, char* node_exclusive_channel, wiz_NetInfo* wiznet_network_information_object, 
	uint8_t* static_IP, uint8_t* subnet_mask, uint8_t* gateway, uint8_t* dns) {
	SetupConnectionWithStaticIP(ssn_mac_address, node_exclusive_channel, wiznet_network_information_object, static_IP, subnet_mask, gateway, dns);
	// Our main UDP socket is defined now
	return socket(UDP_SOCKET, Sn_MR_UDP, SSN_DEFAULT_PORT, 0x00);
}

uint8_t ResetConnection(uint8_t* ssn_mac_address, char* node_exclusive_channel, uint8_t UDP_SOCKET) {
	// Reset the device first
	Ethernet_Reset();
	// Start Ethernet Now with a MAC address (either default MAC or custom SSN MAC)
	Ethernet_Register_MAC(ssn_mac_address, node_exclusive_channel);
	// Initiate the Network again
	WIZ5500_network_initiate();
	// return a new socket from the W5500
	return socket(UDP_SOCKET, Sn_MR_UDP, SSN_DEFAULT_PORT, 0x00);
}

void GetServerIP_UsingDNS(uint8_t* dns_address, uint8_t* default_server_ip, unsigned char* mqtt_server_dns_string, uint8_t* ssn_server_ip){
	start_ms_timer_with_interrupt();
    unsigned char tempBuffer[BUFFER_SIZE] = {0};
    unsigned int timeout_count_to_1sec = 1000;
    DNS_init(DNS_SOCKET_NUMBER, tempBuffer);
    T5CON = 0x8000;
    TMR5 = 0;
    Clear_LED_INDICATOR();
	printf("[LOG]: Looking Up Domain [%s] at DNS IP [%d.%d.%d.%d]\n", mqtt_server_dns_string, dns_address[0], dns_address[1], dns_address[2], dns_address[3]);
    while (DNS_run(dns_address, mqtt_server_dns_string, ssn_server_ip) == 0){
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
        ssn_server_ip[0] = default_server_ip[0];
        ssn_server_ip[1] = default_server_ip[1];
        ssn_server_ip[2] = default_server_ip[2];
        ssn_server_ip[3] = default_server_ip[3];
        printf("[LOG]: DNS Lookup couldn't retrieve server IP. Using hard-coded server IP: %d.%d.%d.%d\n", ssn_server_ip[0], ssn_server_ip[1], ssn_server_ip[2],ssn_server_ip[3]);
    } else {
        printf("[LOG]: DNS Lookup Successful. Server IP: %d.%d.%d.%d\n", ssn_server_ip[0], ssn_server_ip[1], ssn_server_ip[2],ssn_server_ip[3]);
    }
    // reset timer parameters
    TMR5 = 0;
    T5CONCLR = 0x8000;
    Clear_LED_INDICATOR();
	stop_ms_timer_with_interrupt();
}