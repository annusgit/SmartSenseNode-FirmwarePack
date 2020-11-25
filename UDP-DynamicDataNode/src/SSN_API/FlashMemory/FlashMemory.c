
#include "FlashMemory.h"


uint8_t FindMACInFlashMemory(uint8_t* SSN_MAC_ADDRESS, uint8_t* SSN_DEFAULT_MAC) {
    // 1. Check EEPROM for MAC address
    EEPROM_Read_Array(EEPROM_BLOCK_0, EEPROM_MAC_LOC, SSN_MAC_ADDRESS, EEPROM_MAC_SIZE);
    uint8_t i, valid_MAC_in_EEPROM = is_Valid_MAC(SSN_MAC_ADDRESS);
    if (!valid_MAC_in_EEPROM) {
        // we don't have a valid MAC address, use default MAC instead
        printf("LOG: Invalid MAC in EEPROM. Using Default MAC %X:%X:%X:%X:%X:%X\n", SSN_DEFAULT_MAC[0], SSN_DEFAULT_MAC[1], SSN_DEFAULT_MAC[2], SSN_DEFAULT_MAC[3], SSN_DEFAULT_MAC[4], 
                SSN_DEFAULT_MAC[5]);
        for (i = 0; i < 6; i++)
            SSN_MAC_ADDRESS[i] = SSN_DEFAULT_MAC[i];
        // Since we have no custom MAC address with us, set state to ask for MAC address with Get_MAC message
        return NO_MAC_STATE;
    } 
    else {
        printf("LOG: Found MAC in EEPROM -> %X:%X:%X:%X:%X:%X\n", SSN_MAC_ADDRESS[0], SSN_MAC_ADDRESS[1], SSN_MAC_ADDRESS[2], SSN_MAC_ADDRESS[3], SSN_MAC_ADDRESS[4], SSN_MAC_ADDRESS[5]);
        return NO_CONFIG_STATE;
    }
}


uint8_t FindSensorConfigurationsInFlashMemory(uint8_t* SSN_CONFIG, uint8_t* SSN_REPORT_INTERVAL, uint8_t* SSN_CURRENT_SENSOR_RATINGS, uint8_t* SSN_CURRENT_SENSOR_THRESHOLDS, 
        uint8_t* SSN_CURRENT_SENSOR_MAXLOADS) {
    
    // 2. Check CONFIG in EEPROM only if MAC was valid
    // Check EEPROM for current sensor configurations
    EEPROM_Read_Array(EEPROM_BLOCK_0, EEPROM_CONFIG_LOC, SSN_CONFIG, EEPROM_CONFIG_SIZE);
    uint8_t i, valid_CONFIG_in_EEPROM = is_Valid_CONFIG(SSN_CONFIG);
    if (!valid_CONFIG_in_EEPROM) {
        // we don't have a valid config, need to send Get_Config message to SSN Server
        printf("LOG: Invalid CONFIG in EEPROM. Getting CONFIG from SSN_SERVER now...\n");
        return NO_CONFIG_STATE;
    } 
    else {
        // Copy from the configurations, the sensor ratings, thresholds and maximum load values into our variables
        for (i = 0; i < NO_OF_MACHINES; i++) {
            // Get the parameters from the Configurations
            SSN_CURRENT_SENSOR_RATINGS[i]    = SSN_CONFIG[3*i+0];
            SSN_CURRENT_SENSOR_THRESHOLDS[i] = SSN_CONFIG[3*i+1];
            SSN_CURRENT_SENSOR_MAXLOADS[i]   = SSN_CONFIG[3*i+2];
        }
        *SSN_REPORT_INTERVAL = SSN_CONFIG[12];
        printf("LOG: Found Current Sensor Configuration in EEPROM: \n"
                "     >> S1-Rating: %03d A | M1-Threshold: %03d A | M1-Maxload: %03d A |\n"
                "     >> S2-Rating: %03d A | M2-Threshold: %03d A | M2-Maxload: %03d A |\n"
                "     >> S3-Rating: %03d A | M3-Threshold: %03d A | M3-Maxload: %03d A |\n"
                "     >> S4-Rating: %03d A | M4-Threshold: %03d A | M4-Maxload: %03d A |\n"
                "     >> Reporting Interval: %d sec\n", 
                SSN_CURRENT_SENSOR_RATINGS[0], SSN_CURRENT_SENSOR_THRESHOLDS[0], SSN_CURRENT_SENSOR_MAXLOADS[0],
                SSN_CURRENT_SENSOR_RATINGS[1], SSN_CURRENT_SENSOR_THRESHOLDS[1], SSN_CURRENT_SENSOR_MAXLOADS[1],
                SSN_CURRENT_SENSOR_RATINGS[2], SSN_CURRENT_SENSOR_THRESHOLDS[2], SSN_CURRENT_SENSOR_MAXLOADS[2],
                SSN_CURRENT_SENSOR_RATINGS[3], SSN_CURRENT_SENSOR_THRESHOLDS[3], SSN_CURRENT_SENSOR_MAXLOADS[3], *SSN_REPORT_INTERVAL);
        // We have our MAC address and the sensor configurations, only thing remaining is the time of day
        return NO_TIMEOFDAY_STATE;
    }
}