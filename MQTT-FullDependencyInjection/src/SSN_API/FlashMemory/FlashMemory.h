#ifndef __FlashMemory_h__
#define __FlashMemory_h__

#include "../global.h"
#include "../Drivers/UART/uart.h"
#include "../Drivers/EEPROM/eeprom.h"
#include "../Drivers/MESSAGES/messages.h"
#include "../Communication/Communication.h"

/** 
 * Finds MAC address in EEPROM
 * @param SSN_MAC_ADDRESS Six byte array containing the six bytes of SSN MAC address
 * @param SSN_DEFAULT_MAC Six byte array containing the six bytes of default SSN MAC address in case nothing in found in EEPROM
 * @return <b>NO_CONFIG_STATE</b> if MAC address is found in EEPROM; else <b>NO_MAC_STATE</b>
 */
uint8_t FindMACInFlashMemory(uint8_t* ssn_mac_address, uint8_t* ssn_default_mac, char* node_exclusive_channel);

/** 
 * Finds Current Sensor Configurations in EEPROM
 * @param SSN_CONFIG Byte array in which current sensor configurations will be written
 * @param SSN_REPORT_INTERVAL Pointer to byte variable containing SSN status update interval (period of SSN status updates, e.g., 1 sec)
 * @param SSN_CURRENT_SENSOR_RATINGS Byte array in which current sensor ratings will be written
 * @param SSN_CURRENT_SENSOR_THRESHOLDS Byte array in which machine threshold currents will be written to decide IDLE state for machines
 * @param SSN_CURRENT_SENSOR_MAXLOADS Byte array in which machine maximum load currents will be written to calculate load percentages
 * @return <b>NO_TIMEOFDAY_STATE</b> if sensor configurations are found in EEPROM; else <b>NO_CONFIG_STATE</b>
 */
uint8_t FindSensorConfigurationsInFlashMemory(uint8_t* ssn_config, uint8_t* ssn_report_interval, uint8_t* temperature_min_threshold, uint8_t* temperature_max_threshold, 
	uint8_t* humidity_min_threshold, uint8_t* humidity_max_threshold, uint8_t* ssn_current_sensor_ratings, float* ssn_current_sensor_thresholds,  uint8_t* ssn_current_sensor_maxloads, 
	float* ssn_current_sensor_voltage_scalars);

#endif

