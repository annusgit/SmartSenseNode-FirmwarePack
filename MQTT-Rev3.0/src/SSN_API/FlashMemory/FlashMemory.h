
#ifndef __FlashMemory_h__
#define __FlashMemory_h__

#include "../global.h"
#include "../Drivers/UART/uart.h"
#include "../Drivers/EEPROM/eeprom.h"

/** 
 * Finds MAC address in EEPROM
 * @param SSN_MAC_ADDRESS Six byte array containing the six bytes of SSN MAC address
 * @param SSN_DEFAULT_MAC Six byte array containing the six bytes of default SSN MAC address in case nothing in found in EEPROM
 * @return <b>NO_CONFIG_STATE</b> if MAC address is found in EEPROM; else <b>NO_MAC_STATE</b>
 */
uint8_t FindMACInFlashMemory(uint8_t* SSN_MAC_ADDRESS, uint8_t* SSN_DEFAULT_MAC);

/** 
 * Finds Current Sensor Configurations in EEPROM
 * @param SSN_CONFIG Byte array in which current sensor configurations will be written
 * @param SSN_REPORT_INTERVAL Pointer to byte variable containing SSN status update interval (period of SSN status updates, e.g., 1 sec)
 * @param SSN_CURRENT_SENSOR_RATINGS Byte array in which current sensor ratings will be written
 * @param SSN_CURRENT_SENSOR_THRESHOLDS Byte array in which machine threshold currents will be written to decide IDLE state for machines
 * @param SSN_CURRENT_SENSOR_MAXLOADS Byte array in which machine maximum load currents will be written to calculate load percentages
 * @return <b>NO_TIMEOFDAY_STATE</b> if sensor configurations are found in EEPROM; else <b>NO_CONFIG_STATE</b>
 */
uint8_t FindSensorConfigurationsInFlashMemory(uint8_t* SSN_CONFIG, uint8_t* SSN_REPORT_INTERVAL, uint8_t* TEMPERATURE_MIN_THRESHOLD,  uint8_t* TEMPERATURE_MAX_THRESHOLD, 
    uint8_t* HUMIDITY_MIN_THRESHOLD, uint8_t* HUMIDITY_MAX_THRESHOLD, uint8_t* SSN_CURRENT_SENSOR_RATINGS, float* SSN_CURRENT_SENSOR_THRESHOLDS,  uint8_t* SSN_CURRENT_SENSOR_MAXLOADS, 
    float* SSN_CURRENT_SENSOR_VOLTAGE_SCALARS, uint8_t* THERMISTOR_CONFIG);

#endif

