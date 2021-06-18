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
	// Setup static IP for the SSN
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
    start_ms_timer_with_interrupt();
//    printf("here\n");
    unsigned char tempBuffer[BUFFER_SIZE] = {};
    unsigned int timeout_count_to_1sec = 100;
    bool dnsnotfound = false;
    DNS_init(1, tempBuffer);
    T5CON = 0x8000;
    TMR5 = 0;
    Clear_LED_INDICATOR();
    while (DNS_run(DNS_ADDRESS, MQTT_SERVER_DNS, SSN_SERVER_IP) == 0){
//        printf("\n\n****hello\n");
//        printf("timeout is ***%d\n", timeout_count_to_1sec);
        if (TMR5 > PERIPH_CLK/10000) {
//            printf("timer5 is %d\n",TMR5);
            timeout_count_to_1sec--;
//            printf("%d\n", timeout_count_to_1sec);
//            printf("helloworld\n");
            if (timeout_count_to_1sec % 10) {
                SSN_LED_INDICATE(LOOKING_UP_DNS);
            }
            if (timeout_count_to_1sec <= 0) {
            dnsnotfound = true;
            break;
        }                
        }
//        TMR5 = 0;    
    }
    if (dnsnotfound) {
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
//    printf("%s\n", MQTT_SERVER_DNS);
//    printf("server %d.%d.%d.%d\n", SSN_SERVER_IP[0], SSN_SERVER_IP[1], SSN_SERVER_IP[2], SSN_SERVER_IP[3]);
}

//void GetServerIP_UsingDNS(uint8_t* DEFAULT_SERVER_IP, unsigned char* MQTT_SERVER_DNS, uint8_t* SSN_SERVER_IP){
//    unsigned char tempBuffer[BUFFER_SIZE] = {};
//    unsigned int timeout_count_to_1sec = 1000;
//    DNS_init(1, tempBuffer);
//    T5CON = 0x8000;
//    TMR5 = 0;
//    Clear_LED_INDICATOR();
//    while (DNS_run(DNS_ADDRESS, MQTT_SERVER_DNS, SSN_SERVER_IP) == 0){
//        if (TMR5 > PERIPH_CLK/10000) {
//            if (timeout_count_to_1sec--) {
//                if (timeout_count_to_1sec % 200) {
//                    SSN_LED_INDICATE(LOOKING_UP_DNS);
//                    ServiceWatchdog();
//                }
//            } else {
//                // our max number of timeouts have been exceeded. 
//                break;
//            }
//            TMR5 = 0;
//        }
//    }
//    if (timeout_count_to_1sec <= 0) {
//        SSN_SERVER_IP[0] = DEFAULT_SERVER_IP[0];
//        SSN_SERVER_IP[1] = DEFAULT_SERVER_IP[1];
//        SSN_SERVER_IP[2] = DEFAULT_SERVER_IP[2];
//        SSN_SERVER_IP[3] = DEFAULT_SERVER_IP[3];
//        printf("[LOG]: DNS Lookup couldn't retrieve server IP. Using hard-coded server IP: %d.%d.%d.%d\n", SSN_SERVER_IP[0], SSN_SERVER_IP[1], SSN_SERVER_IP[2],SSN_SERVER_IP[3]);
//    } else {
//        printf("[LOG]: DNS Lookup Successful. Server IP: %d.%d.%d.%d\n", SSN_SERVER_IP[0], SSN_SERVER_IP[1], SSN_SERVER_IP[2],SSN_SERVER_IP[3]);
//    }
//    // reset timer parameters
//    TMR5 = 0;
//    T5CONCLR = 0x8000;
//    Clear_LED_INDICATOR();
////    printf("%s\n", MQTT_SERVER_DNS);
////    printf("server %d.%d.%d.%d\n", SSN_SERVER_IP[0], SSN_SERVER_IP[1], SSN_SERVER_IP[2], SSN_SERVER_IP[3]);
//}
