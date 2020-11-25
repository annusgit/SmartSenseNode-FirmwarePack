#ifndef __temperature_sensor_h__
#define __temperature_sensor_h__

#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

#include "../../global.h"
#include <stdint.h>
#include <plib.h>

#define AM2320_I2C_Address              0xB8
#define AM2320_Read_Function_Code       0x03
#define AM2320_Starting_Address         0x00
#define AM2320_Num_Bytes_Requested      0x04

/** These four variables are obselete now */
/** Minimum normal ambient temperature */
#define MIN_NORMAL_TEMPERATURE          0
/** Maximum normal ambient temperature */
#define MAX_NORMAL_TEMPERATURE          60
/** Minimum normal ambient relative humidity */
#define MIN_NORMAL_RELATIVE_HUMIDITY    0
/** Maximum normal ambient relative humidity */
#define MAX_NORMAL_RELATIVE_HUMIDITY    100

#define SENSOR_READ_ERROR               -1
#define SENSOR_READ_CRC_ERROR           0
#define SENSOR_READ_SUCCESSFUL          1

#define NORMAL_AMBIENT_CONDITION            0
#define ABNORMAL_AMBIENT_CONDITION          1
#define TEMP_SENSOR_READ_ERROR_CONDITION    2
#define TEMP_SENSOR_CRC_ERROR_CONDITION     3

#define I2C_NORMAL_OP_WAIT_LOOP_COUNT   500  // after testing, 143 is the max number of loops required for reading from I2C
#define I2C_TEST_OP_WAIT_LOOP_COUNT     500  // startup operation requires more time

/** The data received from the temperature sensor AM2320, i.e., Control byte, number of bytes' byte, 4 data bytes, 2 CRC bytes */ 
uint8_t recv_data[8];

/** A wait loop variable to make sure our I2C read functions always exit */
static uint32_t wait_loop_count = I2C_TEST_OP_WAIT_LOOP_COUNT;

// Device level functions
/** 
 * Opens I2C2 peripheral
 */
void open_I2C2();

/** 
 * Waits while I2C2 is busy reading or writing
 */
bool I2C2_wait_while_busy();

/** 
 * Transmit single bit for starting I2C communication
 */
bool I2C2_transmit_start_bit();

/** 
 * Transmit single bit for stoping I2C communication
 */
bool I2C2_transmit_stop_bit();

/** 
 * Transmit single bit for restarting I2C communication
 */
bool I2C2_transmit_restart_bit();

/** 
 * Transmit single byte over I2C
 * @param byte Single byte to transmit
 */
void I2C2_transmit_byte(uint8_t byte);

/** 
 * Recieves byte over I2C2
 * @return Single byte read over I2C2 
 */
uint8_t I2C2_receive_byte();

/** 
 * Confirms if a byte has been received in the RX buffer of I2C2
 * @return bool 
 */
bool I2C2_is_byte_received();

/** 
 * Acknowledges the received data over I2C via a single bit transmission
 */
bool I2C2_ack(void);

/** 
 * Reads the temperature and humidity bytes from AM2320 sensor using I2C. Read values are written into recv_data array
 */
bool AM2320_I2C2_Read_Temp_and_Humidity();

/** 
 * Performs CRC check on received data
 * @param ptr Byte array containing received data
 * @param len Length of data to check using CRC
 * @return 1 if CRC check OK; 0 otherwise
 */
unsigned short crc16(unsigned char *ptr, unsigned char len);

/**
 * Performs CRC check utilizing the crc16 function
 */
uint8_t CRC_check();

/**
 * Converts bytes to word by combining high and low bytes
 * @param high_byte High byte of word
 * @param low_byte Low byte of word
 * @return 16-bit Word 
 */
uint16_t convert_bytes_to_word(int8_t high_byte, int8_t low_byte);

// API higher-level functions
/**
 * Sets up the AM2320 temperature sensor
 */
void setup_Temperature_Humidity_Sensor();

/**
 * Samples temperature and humidity readings from sensor
 * @param temperature Pointer to 16-bit word to save temperature reading
 * @param relative_humidity Pointer to 16-bit word to save relative humidity reading
 * @return 1 if CRC check was OK; 0 otherwise
 */
uint8_t sample_Temperature_Humidity(uint16_t *temperature, uint16_t* relative_humidity);

/**
 * Samples temperature and humidity readings from sensor
 * @param temperature_bytes Pointer to 8-bit byte array to save temperature reading as bytes
 * @param relative_humidity_bytes Pointer to 8-bit byte array to save relative humidity reading as bytes
 * @return -1 if reading corruption, 0 if CRC check failed; 0 if successful
 */
int8_t sample_Temperature_Humidity_bytes(uint8_t* temperature_bytes, uint8_t* relative_humidity_bytes);

/**
 * Gets ambient condition status
 * @return <b>NORMAL_AMBIENT_CONDITION</b> if normal; <b>ABNORMAL_AMBIENT_CONDITION</b> otherwise.
 */
uint8_t ambient_condition_status(uint8_t temp_min, uint8_t temp_max, uint8_t rh_min, uint8_t rh_max);

#endif

