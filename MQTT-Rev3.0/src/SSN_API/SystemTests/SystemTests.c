
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
    
    // 2. Check Temperature Sensor
    int8_t read_status = sample_Temperature_Humidity_bytes_using_DHT22(temperature_bytes, relative_humidity_bytes);
    // TODO: Add gradient based temperature and humidity change check
    bool temperature_sensor_test_status = true;
    if (read_status==SENSOR_READ_ERROR) {
        temperature_sensor_test_status = false;
        printf("LOG: TEMPERATURE SENSOR TEST FAILED DUE TO READING CORRUPTION...\n");
        // Forever, indicate the status of SSN i.e. SELF_TEST_FAILED_STATE from the SSN LED
        SSN_LED_INDICATE(SELF_TEST_FAILED_STATE);
        // while(SSN_IS_ALIVE); Temperature sensor is not crucial, we will not hang for this
    } else if (read_status==SENSOR_CRC_ERROR) {
        temperature_sensor_test_status = false;
        printf("LOG: TEMPERATURE SENSOR TEST FAILED DUE TO CRC CHECK FAILURE...\n");
        // Forever, indicate the status of SSN i.e. SELF_TEST_FAILED_STATE from the SSN LED
        SSN_LED_INDICATE(SELF_TEST_FAILED_STATE);
        // while(SSN_IS_ALIVE); Temperature sensor is not crucial, we will not hang for this
    }
    if (ambient_condition_status() != NORMAL_AMBIENT_CONDITION) {
        temperature_sensor_test_status = false;
        printf("LOG: TEMPERATURE SENSOR TEST FAILED DUE TO ABNORMAL CONDITION...\n");
        // Forever, indicate the status of SSN i.e. SELF_TEST_FAILED_STATE from the SSN LED
        SSN_LED_INDICATE(SELF_TEST_FAILED_STATE);
        // while(SSN_IS_ALIVE); Temperature sensor is not crucial, we will not hang for this
    }
//    if (temperature_sensor_test_status) {
//        printf("LOG: TEMPERATURE SENSOR TEST SUCCESSFUL.\n");   
//        // reduce wait loop count only if test is successful
//        wait_loop_count = I2C_NORMAL_OP_WAIT_LOOP_COUNT;
//    }
    
    // 3. Check Ethernet Physical Link to SSN, i.e., is the Ethernet cable plugged into RJ45 of our SSN
    if (Ethernet_get_physical_link_status() != PHY_LINK_ON) {
        printf("LOG: Ethernet Physical Link BAD. Waiting for link...\n");
		Clear_LED_INDICATOR();
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
