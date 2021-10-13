#include "FlashMemory.h"


uint8_t FindMACInFlashMemory(uint8_t* ssn_mac_address, uint8_t* ssn_default_mac, char* node_exclusive_channel) {
	printf("[EEPROM] Retrieving MAC address from EEPROM\n");
    // 1. Check EEPROM for MAC address
    EEPROM_Read_Array(EEPROM_MAC_BLOCK, EEPROM_MAC_LOC, ssn_mac_address, EEPROM_MAC_SIZE);
    int i, valid_MAC_in_EEPROM = is_Valid_MAC(ssn_mac_address);
    if (!valid_MAC_in_EEPROM) {
        // we don't have a valid MAC address, use default MAC instead
        printf("[EEPROM] Invalid MAC in EEPROM. Using Default MAC (Bytes): [%X:%X:%X:%X:%X:%X]\n", ssn_default_mac[0], ssn_default_mac[1], ssn_default_mac[2], ssn_default_mac[3], ssn_default_mac[4], 
                ssn_default_mac[5]);
        for (i = 0; i < EEPROM_MAC_SIZE; i++) {
			ssn_mac_address[i] = ssn_default_mac[i];	
		}
        // Since we have no custom MAC address with us, set state to ask for MAC address with Get_MAC message
        return NO_MAC_STATE;
    }
    else {
        printf("[EEPROM] Found MAC in EEPROM (Bytes): [%X:%X:%X:%X:%X:%X]\n", ssn_mac_address[0], ssn_mac_address[1], ssn_mac_address[2], ssn_mac_address[3], ssn_mac_address[4], 
                ssn_mac_address[5]);
        EEPROM_Read_Array(EEPROM_MAC_STRING_BLOCK, EEPROM_MAC_STRING_LOC, node_exclusive_channel, EEPROM_MAC_STRING_SIZE);
        int valid_MAC_string_in_EEPROM = is_Valid_MAC_String(node_exclusive_channel);
        if (!valid_MAC_string_in_EEPROM) {
            printf("[EEPROM] Acquired INVALID Node Exclusive Channel from EEPROM (ASCII): %s\n", node_exclusive_channel);
            printf("[EEPROM] Creating new valid EEPROM string...\n");
            sprintf(node_exclusive_channel, "%02X:%02X:%02X:%02X:%02X:%02X", ssn_mac_address[0], ssn_mac_address[1], ssn_mac_address[2], ssn_mac_address[3], ssn_mac_address[4], ssn_mac_address[5]);
            EEPROM_Write_Array(EEPROM_BLOCK_0, EEPROM_MAC_STRING_LOC, node_exclusive_channel, EEPROM_MAC_STRING_SIZE);
            printf("[EEPROM] Written new valid EEPROM string in EEPROM (ASCII): %s\n", node_exclusive_channel);
        } else {
            printf("[EEPROM] Acquired Valid Node Exclusive Channel from EEPROM (ASCII): %s\n", node_exclusive_channel);
        }
        return NO_CONFIG_STATE;
    }
}


uint8_t FindSensorConfigurationsInFlashMemory(uint8_t* ssn_config, uint8_t* ssn_report_interval, uint8_t* temperature_min_threshold, uint8_t* temperature_max_threshold, 
	uint8_t* humidity_min_threshold, uint8_t* humidity_max_threshold, uint8_t* ssn_current_sensor_ratings, float* ssn_current_sensor_thresholds,  uint8_t* ssn_current_sensor_maxloads, 
	float* ssn_current_sensor_voltage_scalars) {
    // 2. Check CONFIG in EEPROM only if MAC was valid
    // Check EEPROM for current sensor configurations
    EEPROM_Read_Array(EEPROM_CONFIG_BLOCK, EEPROM_CONFIG_LOC, ssn_config, EEPROM_CONFIG_SIZE);
    uint8_t valid_CONFIG_in_EEPROM = is_Valid_CONFIG(ssn_config);
    int i; if (!valid_CONFIG_in_EEPROM) {
        // we don't have a valid config, need to send Get_Config message to SSN Server
        printf("[EEPROM] Invalid CONFIG in EEPROM. Getting CONFIG from SSN_SERVER now...\n");
        return NO_CONFIG_STATE;
    } 
    else {
        // Copy from the configurations, the sensor ratings, thresholds and maximum load values to our variables
		for (i = 0; i < NO_OF_MACHINES; i++) {
			/* Get the parameters from the Configurations */
			ssn_current_sensor_ratings[i]       = ssn_config[4*i+0];
			ssn_current_sensor_thresholds[i]    = ssn_config[4*i+1] / 10.0f;
			ssn_current_sensor_maxloads[i]      = ssn_config[4*i+2];
			if (ssn_config[4*i+3] == 0) {
				ssn_current_sensor_voltage_scalars[i] = 0.333;
			} else if (ssn_config[4*i+3] == 1) {
				ssn_current_sensor_voltage_scalars[i] = 1.00;
			} else if (ssn_config[4*i+3] == 2) {
				ssn_current_sensor_voltage_scalars[i] = 1.65;
			}
		}
		*temperature_min_threshold	= ssn_config[16];
		*temperature_max_threshold	= ssn_config[17];
		*humidity_min_threshold		= ssn_config[18];
		*humidity_max_threshold		= ssn_config[19];
		// save new reporting interval
		*ssn_report_interval = ssn_config[20];
		printf("[EEPROM] Found Saved Current Sensor Configuration in EEPROM: \n"
			"     >> S1-Rating: %03d Arms | S1-Scalar: %.3f Vrms | M1-Threshold: %.3f Arms | M1-Maxload: %03d Arms |\n"
			"     >> S2-Rating: %03d Arms | S2-Scalar: %.3f Vrms | M2-Threshold: %.3f Arms | M2-Maxload: %03d Arms |\n"
			"     >> S3-Rating: %03d Arms | S3-Scalar: %.3f Vrms | M3-Threshold: %.3f Arms | M3-Maxload: %03d Arms |\n"
			"     >> S4-Rating: %03d Arms | S4-Scalar: %.3f Vrms | M4-Threshold: %.3f Arms | M4-Maxload: %03d Arms |\n"
			"     >> MIN TEMP : %03d C    | MAX TEMP : %03d C    |\n"
			"     >> MIN RH   : %03d %    | MIN RH   : %03d %    |\n"
			"     >> Report   : %d seconds\n", 
			ssn_current_sensor_ratings[0], ssn_current_sensor_voltage_scalars[0], ssn_current_sensor_thresholds[0], ssn_current_sensor_maxloads[0],
			ssn_current_sensor_ratings[1], ssn_current_sensor_voltage_scalars[1], ssn_current_sensor_thresholds[1], ssn_current_sensor_maxloads[1],
			ssn_current_sensor_ratings[2], ssn_current_sensor_voltage_scalars[2], ssn_current_sensor_thresholds[2], ssn_current_sensor_maxloads[2],
			ssn_current_sensor_ratings[3], ssn_current_sensor_voltage_scalars[3], ssn_current_sensor_thresholds[3], ssn_current_sensor_maxloads[3], 
			*temperature_min_threshold, *temperature_max_threshold, 
			*humidity_min_threshold, *humidity_max_threshold,
			*ssn_report_interval);
        // We have our MAC address and the sensor configurations, only thing remaining is the time of day
        return NO_TIMEOFDAY_STATE;
    }
}