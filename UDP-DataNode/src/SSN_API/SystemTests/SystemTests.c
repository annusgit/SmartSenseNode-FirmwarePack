
#include "SystemTests.h"

void RunSystemTests() {
    // 1. Check EEPROM
    printf("\n\n\nLOG: SSN AWAKE...\n");
    if (EEPROM_Check() == EEPROM_TEST_PASSED) {
        printf("LOG: EEPROM TEST SUCCESSFUL.\n");
    } 
    else {
        printf("LOG: EEPROM TEST FAILED. SSN Halted...\n");
        // Forever, indicate the status of SSN i.e. SELF_TEST_FAILED_STATE from the SSN LED
        SSN_LED_INDICATE(SELF_TEST_FAILED_STATE);
        while(SSN_IS_ALIVE);
    }
    // 2. Check Ethernet Physical Link to SSN, i.e., is the Ethernet cable plugged into RJ45 of our SSN
    if (Ethernet_get_physical_link_status() != PHY_LINK_ON) {
        printf("LOG: Ethernet Physical Link BAD. Waiting for link...\n");
        while (Ethernet_get_physical_link_status() != PHY_LINK_ON) {
            // Indicate the status of SSN i.e. NO_ETHERNET_STATE from the SSN LED as long as we don't get a stable physical link
            SSN_LED_INDICATE(NO_ETHERNET_STATE);
            sleep_for_microseconds(500000);
        }
        /* At this point, we have tested our EEPROM, our Temperature sensor and our Ethernet Connection
         * Now we can move into our regular SSN state machine */ 
        printf("LOG: Ethernet Physical Link OK\n");
    } 
    else {
        printf("LOG: ETHERNET PHYSICAL LINK TEST SUCCESSFUL\n");
        /* At this point, we have tested our EEPROM, our Temperature sensor and our Ethernet Connection.
         * Now we can move into our regular SSN state machine */
    }
}
