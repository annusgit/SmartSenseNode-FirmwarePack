#include "network.h"

char StatusUpdatesChannel[13] = "StatusUpdates";
char GettersChannel[7] = "Getters";
char NodeExclusiveChannel[17];


void WIZ5500_Reset() {
    /* Reset WIZ5500 for ~460ms */
    PORTSetBits(IOPORT_B, BIT_4);
    PORTClearBits(IOPORT_B, BIT_4);
    PORTSetBits(IOPORT_B, BIT_4);
    PORTClearBits(IOPORT_B, BIT_4);
    delay(2000000);
    PORTSetBits(IOPORT_B, BIT_4);    
}

void open_SPI2() {
	/* Basic SPI configuration, pin setup and reset for WIZNet5500 SPI */
	PORTSetPinsDigitalIn(IOPORT_B, BIT_13); // Pin-B13: Wiz-MISO (In), See page-131 note
	PORTSetPinsDigitalOut(IOPORT_B, BIT_5); // Pin-B5: Wiz-MOSI (Out)
	PORTSetPinsDigitalOut(IOPORT_B, BIT_4); // Pin-B4: Wiz-RST
	// CS pin is not configured here but CS is grounded in hardware
	// Peripheral Pin Select (PPS) for SPI2
	SDI2R = 0x0003; // MISO-2
	RPB5R = 0x0004; // MOSI-2
	// SPI2 configuration settings
	SPI2CON = 0x00008065; // 8-bit transfer (For Framed Data, do: 0x80008065)
	SPI2CON2 = 0x00001F00;
	SPI2BRG = 0x00000002;
}

void Ethernet_Reset() {
    WIZ5500_Reset();
    sleep_for_microseconds(100000);
}

void setup_Ethernet() {
    open_SPI2();
	// Wiz5500 Reset
	WIZ5500_Reset();
	sleep_for_microseconds(3000000);
	// Device Ready for SPI communication
	// register read/write and chip select call backs for non-framed mode
	reg_wizchip_cs_cbfunc(WIZ5500_select, WIZ5500_deselect);
	reg_wizchip_spi_cbfunc(WIZ5500_read_byte, WIZ5500_write_byte);
	reg_wizchip_spiburst_cbfunc(WIZ5500_read_array, WIZ5500_write_array);
}

unsigned int SPI2_send(unsigned int data) {
    /* PIC32MX basic SPI Single Byte transfer method for SPI2 */
    SPI2BUF = data;           // write to shift register to begin transmission
    while(!SPI2STATbits.SPIRBF); // wait for transfer to complete
    return SPI2BUF;              // read the shift register value 
}


//////////////////////////////////////////////////////////////////////////////////////////////
// Call back function for W5500 SPI - Theses used to parameter or reg_wizchip_xxx_cbfunc()  //
//////////////////////////////////////////////////////////////////////////////////////////////
void WIZ5500_select(void) {}
void WIZ5500_deselect(void) {}

void WIZ5500_write_byte(uint8_t wb) {
    SPI2_send(wb);    
}

uint8_t WIZ5500_read_byte() {
    return (0xFF & SPI2_send(0)); // dummy transfer to push MISO
}


void WIZ5500_write_array (uint8_t* addrBuf, uint8_t* pBuf, uint16_t len) {
    /* Assume the addrBuf is the starting address array, and len number of 
       and len number of consecutive elements must be read starting from 
       this address */
    unsigned int counter = 0;
    int16_t offset_address;
    for (counter; counter < len; counter++) { // len is the len of data to be written
        // Write three bytes of address and then whatever byte is to be written
        offset_address = (addrBuf[0] << 8) + addrBuf[1] + counter;
        WIZ5500_write_byte((offset_address & 0xFF00) >> 8); // because offset is to be incremented
        WIZ5500_write_byte((offset_address & 0x00FF));
        WIZ5500_write_byte(addrBuf[2]);
        WIZ5500_write_byte(pBuf[counter]);
    }
}


void WIZ5500_read_array (uint8_t* addrBuf, uint8_t* pBuf, uint16_t len) {
    /* Assume the addrBuf is the starting address array, and len number of 
       and len number of consecutive elements must be read starting from 
       this address */
    unsigned int counter = 0;
    int16_t offset_address;
    for (counter; counter < len; counter++) { // len is the len of data to be written
        // Write three bytes of address and then whatever byte is to be written
        offset_address = (addrBuf[0] << 8) + addrBuf[1] + counter;
        WIZ5500_write_byte((offset_address & 0xFF00) >> 8); // because offset is to be incremented
        WIZ5500_write_byte((offset_address & 0x00FF));
        WIZ5500_write_byte(addrBuf[2]);
        pBuf[counter] = WIZ5500_read_byte();
    }
}

/******************************************************************************
 * @brief  Network Init
 * Intialize the network information to be used in WIZCHIP
 *****************************************************************************/
void WIZ5500_network_initiate(void) {
    uint8_t tmpstr[6] = {0,};
	wiz_NetInfo netinfo;

	// Set Network information from netinfo structure
	ctlnetwork(CN_SET_NETINFO, (void*)&WIZ5500_network_information);

#ifdef _MAIN_DEBUG_
	// Get Network information
	ctlnetwork(CN_GET_NETINFO, (void*)&netinfo);

	// Display Network Information
	ctlwizchip(CW_GET_ID,(void*)tmpstr);

	if(netinfo.dhcp == NETINFO_DHCP) printf("\r\n=== %s NET CONF : DHCP ===\r\n", (char*)tmpstr);
	else printf("\r\n=== %s NET CONF : Static ===\r\n",(char*)tmpstr);

	printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",netinfo.mac[0],netinfo.mac[1],netinfo.mac[2], netinfo.mac[3],netinfo.mac[4],netinfo.mac[5]);
	printf("SIP: %d.%d.%d.%d\r\n", netinfo.ip[0],netinfo.ip[1],netinfo.ip[2],netinfo.ip[3]);
	printf("GAR: %d.%d.%d.%d\r\n", netinfo.gw[0],netinfo.gw[1],netinfo.gw[2],netinfo.gw[3]);
	printf("SUB: %d.%d.%d.%d\r\n", netinfo.sn[0],netinfo.sn[1],netinfo.sn[2],netinfo.sn[3]);
	printf("DNS: %d.%d.%d.%d\r\n", netinfo.dns[0],netinfo.dns[1],netinfo.dns[2],netinfo.dns[3]);
	printf("===========================\r\n");
#endif
}

/*******************************************************
 * @ brief Call back for ip assing & ip update from DHCP
 *******************************************************/
void WIZ5500_IP_assigned_callback(void) {
    getIPfromDHCP(WIZ5500_network_information.ip);
    getGWfromDHCP(WIZ5500_network_information.gw);
    getSNfromDHCP(WIZ5500_network_information.sn);
    getDNSfromDHCP(WIZ5500_network_information.dns);
    WIZ5500_network_information.dhcp = NETINFO_DHCP;
    /* Network initialization */
    WIZ5500_network_initiate();      // apply from dhcp
#ifdef _MAIN_DEBUG_
    printf("DHCP LEASED TIME : %ld Sec.\r\n", getDHCPLeasetime());
#endif
}

/************************************
 * @ brief Call back for ip Conflict
 ************************************/
void WIZ5500_IP_conflict_callback(void) {
#ifdef _MAIN_DEBUG_
	printf("CONFLICT IP from DHCP\r\n");
#endif
   //halt or reset or any...
   while(1); // this example is halt.
}

uint8_t Ethernet_get_physical_link_status() {
    return wizphy_getphylink();
}

void Ethernet_Register_MAC(uint8_t* this_mac) {
    WIZ5500_network_information.mac[0] = this_mac[0];
    WIZ5500_network_information.mac[1] = this_mac[1];
    WIZ5500_network_information.mac[2] = this_mac[2];
    WIZ5500_network_information.mac[3] = this_mac[3];
    WIZ5500_network_information.mac[4] = this_mac[4];
    WIZ5500_network_information.mac[5] = this_mac[5];
	// also create the exclusive MQTT channel here
	sprintf(NodeExclusiveChannel, "%02X:%02X:%02X:%02X:%02X:%02X", this_mac[0], this_mac[1], this_mac[2], this_mac[3], this_mac[4], this_mac[5]);
}

void Ethernet_Save_Static_IP(uint8_t* this_IP) {
    int i; for(i=0; i<4; i++) {
        WIZ5500_network_information.ip[i] = this_IP[i];
    }
}

void Ethernet_Save_Subnet_Mask(uint8_t* this_subnet) {
    int i; for(i=0; i<4; i++) {
        WIZ5500_network_information.sn[i] = this_subnet[i];
    }
}

void Ethernet_Save_Gateway_Address(uint8_t* this_gateway) {
    int i; for(i=0; i<4; i++) {
        WIZ5500_network_information.gw[i] = this_gateway[i];
    }    
}

void Ethernet_Save_DNS(uint8_t* this_dns) {
	int i; for(i=0; i<4; i++) {
        WIZ5500_network_information.dns[i] = this_dns[i];
    }    
}

void Ethernet_get_IP_from_DHCP() {
    uint8_t tmp, memsize[2][8] = {{2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};
    uint16_t my_dhcp_retry = 0;
    start_ms_timer_with_interrupt();
    /* wizchip initialize*/
    if(ctlwizchip(CW_INIT_WIZCHIP,(void*)memsize) == -1) {
       printf("[DHCP] WIZCHIP Initialized fail.\r\n");
       while(1);
    }
    printf("[DHCP] Wizchip initialized successfully\n");
    /* PHY link status check */
    do {
       if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1)
          printf("[DHCP] Unknown PHY Link status.\r\n");
    } while(tmp == PHY_LINK_OFF);
    printf("[DHCP] Physical Link OK\n");
    /* Network initialization */
    WIZ5500_network_initiate(); // Static netinfo setting
    // Set MAC address before initiating DHCP
	setSHAR(WIZ5500_network_information.mac);
    // Step-1: initiate dhcp
    DHCP_init(DHCP_SOCKET, gDATABUF);
    reg_dhcp_cbfunc(WIZ5500_IP_assigned_callback, WIZ5500_IP_assigned_callback, WIZ5500_IP_conflict_callback);
    /* DHCP Request IP Loop */
    int dhcp_status, request_started = 0;
	Clear_LED_INDICATOR();
    while(1) {
        dhcp_status = DHCP_run();
        switch(dhcp_status) {
			case DHCP_IP_ASSIGN:
                printf("[DHCP] DHCP IP Assigned\n");
                break;
			case DHCP_IP_CHANGED:
				printf("[DHCP] DHCP IP Changed\n");
				break;
			case DHCP_IP_LEASED:
				// TO DO YOUR NETWORK APPs.
                printf("[DHCP] DHCP Standby\n");
				break;
			case DHCP_FAILED:
				my_dhcp_retry++;
				if(my_dhcp_retry > MY_MAX_DHCP_RETRY) {
#ifdef _MAIN_DEBUG_
					printf("[DHCP] DHCP %d Failed\r\n", my_dhcp_retry);
#endif
					my_dhcp_retry = 0;
					// if restart, recall DHCP_init()
					DHCP_stop();
					// apply the default static network and print out netinfo to serial
					WIZ5500_network_initiate();   
				}
				break;
			default:
                if (request_started > 0) {
					if (request_started % 2000 == 0) {
						printf(".");
						SSN_LED_INDICATE(GETTING_IP_FROM_DHCP);
					}
				} else
					printf("[LOG] -> DHCP Requesting IP\n");
				request_started++;
				break;
		}
        if (dhcp_status == DHCP_IP_LEASED) {
            printf("\n");
            stop_ms_timer_with_interrupt();
            break;
        }
    }
	Clear_LED_INDICATOR();
    close(DHCP_SOCKET);
}

void Ethernet_set_Static_IP(uint8_t* static_IP, uint8_t* subnet_mask, uint8_t* gateway, uint8_t* dns) {
    // WIZ5500_network_information.dhcp = NETINFO_STATIC;
    uint8_t tmp, memsize[2][8] = {{2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};
    /* wizchip initialize*/
    if(ctlwizchip(CW_INIT_WIZCHIP,(void*)memsize) == -1) {
       printf("[STATIC_IP] -> WIZCHIP Initialized fail.\r\n");
       while(1);
    }
    printf("[STATIC_IP] Wizchip initialized successfully\n");
    /* PHY link status check */
    do {
       if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1)
          printf("[STATIC_IP] Unknown PHY Link status.\r\n");
    } while(tmp == PHY_LINK_OFF);
    printf("[STATIC_IP] Physical Link OK\n");
    /* Network initialization */
    WIZ5500_network_initiate(); // Static netinfo setting
    // Set network credentials, MAC address, IP, subnet mask and gateway
    Ethernet_Save_Static_IP(static_IP);
    Ethernet_Save_Subnet_Mask(subnet_mask);
    Ethernet_Save_Gateway_Address(gateway);
	Ethernet_Save_DNS(dns);
    WIZ5500_network_initiate(); // Static netinfo setting
}
