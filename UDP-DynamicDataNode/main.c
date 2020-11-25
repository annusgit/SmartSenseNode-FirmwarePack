#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

#pragma config FWDTEN   = OFF           // Turn off watchdog timer
#pragma config WDTPS    = PS4096        // Watchdog timer period
#pragma config FSOSCEN  = OFF           // Secondary Oscillator Enable (Disabled)
#pragma config FNOSC    = FRCPLL        // Select 8MHz internal Fast RC (FRC) oscillator with PLL
#pragma config FPLLIDIV = DIV_2         // Divide PLL input (FRC) -> 4MHz
#pragma config FPLLMUL  = MUL_15        // PLL Multiplier -> 60MHz
#pragma config FPLLODIV = DIV_1         // PLL Output Divider -> 60MHz
#pragma config FPBDIV   = DIV_2         // Peripheral Clock divisor -> 30MHz
#pragma config ICESEL   = ICS_PGx2      // ICE/ICD Comm Channel Select (Communicate on PGEC2/PGED2)
#pragma config JTAGEN   = OFF           // JTAG Enable (Disabled)

#include "src/SSN_API/SSN_API.h"

int main() {
    // Setup Smart Sense Node
    SSN_Setup();
    // Check the EEPROM, temperature sensor and network connection before proceeding
    RunSystemTests();
    // We need a watchdog to make sure we don't get stuck forever
    EnableWatchdog();
    // Assign default hard-coded MAC address
    for (i = 0; i < 6; i++) {
        SSN_MAC_ADDRESS[i] = SSN_DEFAULT_MAC[i];    
    }
    // We can chose two ways to operate over UDP; static or dynamic IP
    //SSN_UDP_SOCKET = SetupConnectionWithDHCP(SSN_MAC_ADDRESS, SSN_UDP_SOCKET_NUM);
    SSN_UDP_SOCKET = SetupConnectionWithStaticIP(SSN_UDP_SOCKET_NUM, SSN_MAC_ADDRESS, SSN_STATIC_IP, SSN_SUBNET_MASK, SSN_GATWAY_ADDRESS);
    // Clear the watchdog
    ServiceWatchdog();
    //InterruptEnabled = true;
	printf("(LOG) Waiting for Messages...\n");
    while(SSN_IS_ALIVE) {
        // Make sure Ethernet is working fine (blocking if no physical link available)
        SSN_CHECK_ETHERNET_CONNECTION();
        // Receive time of day or new configurations if they are sent from the server
        Route_Messages(SSN_UDP_SOCKET, SENDER_IP, SENDER_PORT);
        // Clear the watchdog
        ServiceWatchdog();
        // sleep for 100 milliseconds
        sleep_for_microseconds(1e5);
    }
    // we should never reach this point
    return 0;
}

