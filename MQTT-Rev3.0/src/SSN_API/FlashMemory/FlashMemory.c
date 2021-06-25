#include "FlashMemory.h"


uint8_t FindMACInFlashMemory(uint8_t* SSN_MAC_ADDRESS, uint8_t* SSN_DEFAULT_MAC) {
    // 1. Check EEPROM for MAC address
    EEPROM_Read_Array(EEPROM_MAC_BLOCK, EEPROM_MAC_LOC, SSN_MAC_ADDRESS, EEPROM_MAC_SIZE);
    int i, valid_MAC_in_EEPROM = is_Valid_MAC(SSN_MAC_ADDRESS);
    if (!valid_MAC_in_EEPROM) {
        // we don't have a valid MAC address, use default MAC instead
        printf("[LOG] Invalid MAC in EEPROM. Using Default MAC %X:%X:%X:%X:%X:%X\n", SSN_DEFAULT_MAC[0], SSN_DEFAULT_MAC[1], SSN_DEFAULT_MAC[2], SSN_DEFAULT_MAC[3], SSN_DEFAULT_MAC[4], 
                SSN_DEFAULT_MAC[5]);
        for (i = 0; i < EEPROM_MAC_SIZE; i++) {
			SSN_MAC_ADDRESS[i] = SSN_DEFAULT_MAC[i];	
		}
        // Since we have no custom MAC address with us, set state to ask for MAC address with Get_MAC message
        return NO_MAC_STATE;
    }
    else {
        printf("[LOG] Found MAC in EEPROM -> %X:%X:%X:%X:%X:%X\n", SSN_MAC_ADDRESS[0], SSN_MAC_ADDRESS[1], SSN_MAC_ADDRESS[2], SSN_MAC_ADDRESS[3], SSN_MAC_ADDRESS[4], 
                SSN_MAC_ADDRESS[5]);
        EEPROM_Read_Array(EEPROM_MAC_STRING_BLOCK, EEPROM_MAC_STRING_LOC, NodeExclusiveChannel, EEPROM_MAC_STRING_SIZE);
        int valid_MAC_string_in_EEPROM = is_Valid_MAC_String(NodeExclusiveChannel);
        if (!valid_MAC_string_in_EEPROM) {
            printf("[LOG] Acquired INVALID Node Exclusive Channel from EEPROM: %s\n", NodeExclusiveChannel);
            printf("[LOG] Creating new valid EEPROM string...\n");
            sprintf(NodeExclusiveChannel, "%02X:%02X:%02X:%02X:%02X:%02X", SSN_MAC_ADDRESS[0], SSN_MAC_ADDRESS[1], SSN_MAC_ADDRESS[2], SSN_MAC_ADDRESS[3], 
                    SSN_MAC_ADDRESS[4], SSN_MAC_ADDRESS[5]);
            EEPROM_Write_Array(EEPROM_BLOCK_0, EEPROM_MAC_STRING_LOC, NodeExclusiveChannel, EEPROM_MAC_STRING_SIZE);
            printf("[LOG] Written new valid EEPROM string in EEPROM: %s\n", NodeExclusiveChannel);
        } else {
            printf("[LOG] Acquired Valid Node Exclusive Channel from EEPROM: %s\n", NodeExclusiveChannel);
        }
        return NO_CONFIG_STATE;
    }
}


uint8_t FindSensorConfigurationsInFlashMemory(uint8_t* SSN_CONFIG, uint8_t* SSN_REPORT_INTERVAL, uint8_t* TEMPERATURE_MIN_THRESHOLD, uint8_t* TEMPERATURE_MAX_THRESHOLD, 
	uint8_t* HUMIDITY_MIN_THRESHOLD, uint8_t* HUMIDITY_MAX_THRESHOLD, uint8_t* SSN_CURRENT_SENSOR_RATINGS, float* SSN_CURRENT_SENSOR_THRESHOLDS,  uint8_t* SSN_CURRENT_SENSOR_MAXLOADS, 
	float* SSN_CURRENT_SENSOR_VOLTAGE_SCALARS) {
    
    // 2. Check CONFIG in EEPROM only if MAC was valid
    // Check EEPROM for current sensor configurations
    EEPROM_Read_Array(EEPROM_CONFIG_BLOCK, EEPROM_CONFIG_LOC, SSN_CONFIG, EEPROM_CONFIG_SIZE);
    uint8_t valid_CONFIG_in_EEPROM = is_Valid_CONFIG(SSN_CONFIG);
    int i; if (!valid_CONFIG_in_EEPROM) {
        // we don't have a valid config, need to send Get_Config message to SSN Server
        printf("[LOG] Invalid CONFIG in EEPROM. Getting CONFIG from SSN_SERVER now...\n");
        return NO_CONFIG_STATE;
    } 
    else {
        // Copy from the configurations, the sensor ratings, thresholds and maximum load values to our variables
		for (i = 0; i < NO_OF_MACHINES; i++) {
			/* Get the parameters from the Configurations */
			SSN_CURRENT_SENSOR_RATINGS[i]       = SSN_CONFIG[4*i+0];
			SSN_CURRENT_SENSOR_THRESHOLDS[i]    = SSN_CONFIG[4*i+1] / 10.0f;
			SSN_CURRENT_SENSOR_MAXLOADS[i]      = SSN_CONFIG[4*i+2];
			if (SSN_CONFIG[4*i+3] == 0) {
				SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[i] = 0.333;
			} else if (SSN_CONFIG[4*i+3] == 1) {
				SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[i] = 1.00;
			} else if (SSN_CONFIG[4*i+3] == 2) {
				SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[i] = 1.65;
			}
		}
		*TEMPERATURE_MIN_THRESHOLD	= SSN_CONFIG[16];
		*TEMPERATURE_MAX_THRESHOLD	= SSN_CONFIG[17];
		*HUMIDITY_MIN_THRESHOLD		= SSN_CONFIG[18];
		*HUMIDITY_MAX_THRESHOLD		= SSN_CONFIG[19];
		// save new reporting interval
		*SSN_REPORT_INTERVAL = SSN_CONFIG[20];
		printf("[LOG] Found Saved Current Sensor Configuration in EEPROM: \n"
			"     >> S1-Rating: %03d Arms | S1-Scalar: %.3f Vrms | M1-Threshold: %.3f Arms | M1-Maxload: %03d Arms |\n"
			"     >> S2-Rating: %03d Arms | S2-Scalar: %.3f Vrms | M2-Threshold: %.3f Arms | M2-Maxload: %03d Arms |\n"
			"     >> S3-Rating: %03d Arms | S3-Scalar: %.3f Vrms | M3-Threshold: %.3f Arms | M3-Maxload: %03d Arms |\n"
			"     >> S4-Rating: %03d Arms | S4-Scalar: %.3f Vrms | M4-Threshold: %.3f Arms | M4-Maxload: %03d Arms |\n"
			"     >> MIN TEMP : %03d C    | MAX TEMP : %03d C    |\n"
			"     >> MIN RH   : %03d %    | MIN RH   : %03d %    |\n"
			"     >> Report   : %d seconds\n", 
			SSN_CURRENT_SENSOR_RATINGS[0], SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[0], SSN_CURRENT_SENSOR_THRESHOLDS[0], SSN_CURRENT_SENSOR_MAXLOADS[0],
			SSN_CURRENT_SENSOR_RATINGS[1], SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[1], SSN_CURRENT_SENSOR_THRESHOLDS[1], SSN_CURRENT_SENSOR_MAXLOADS[1],
			SSN_CURRENT_SENSOR_RATINGS[2], SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[2], SSN_CURRENT_SENSOR_THRESHOLDS[2], SSN_CURRENT_SENSOR_MAXLOADS[2],
			SSN_CURRENT_SENSOR_RATINGS[3], SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[3], SSN_CURRENT_SENSOR_THRESHOLDS[3], SSN_CURRENT_SENSOR_MAXLOADS[3], 
			*TEMPERATURE_MIN_THRESHOLD, *TEMPERATURE_MAX_THRESHOLD, 
			*HUMIDITY_MIN_THRESHOLD, *HUMIDITY_MAX_THRESHOLD,
			*SSN_REPORT_INTERVAL);
        // We have our MAC address and the sensor configurations, only thing remaining is the time of day
        return NO_TIMEOFDAY_STATE;
    }
}