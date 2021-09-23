
#ifndef __SystemTests_h__
#define __SystemTests_h__

#include "../global.h"
#include "../Drivers/UART/uart.h"
#include "../Drivers/EEPROM/eeprom.h"
#include "../Drivers/NETWORK/network.h"
#include "../Drivers/TEMPERATURE_SENSOR/temperature_sensor.h"

/** Temperature reading bytes for internal testing only */
uint8_t temperature_bytes[2]; 
/** Relative humidity reading bytes for internal testing only */
uint8_t relative_humidity_bytes[2];

/** 
 * Runs system diagnostic tests for checking if important peripherals are functioning properly; does not return if found faulty. 
 * Checks the following peripherals in that order.
 * - EEPROM Read/Write
 * - Temperature and Humidity Sensor
 * - Ethernet Physical Connection
 */
void RunSystemTests();

#endif
