#ifndef __temperature_sensor_h__
#define __temperature_sensor_h__

#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

#include "../../global.h"
#include "../CURRENT_SENSOR/current_sensor.h"
#include <stdint.h>
#include <plib.h>

#define AM2320_I2C_Address              0xB8
#define AM2320_Read_Function_Code       0x03
#define AM2320_Starting_Address         0x00
#define AM2320_Num_Bytes_Requested      0x04

/** Minimum normal ambient temperature */
#define MIN_NORMAL_TEMPERATURE          0
/** Maximum normal ambient temperature */
#define MAX_NORMAL_TEMPERATURE          60
/** Minimum normal ambient relative humidity */
#define MIN_NORMAL_RELATIVE_HUMIDITY    0
/** Maximum normal ambient relative humidity */
#define MAX_NORMAL_RELATIVE_HUMIDITY    100

#define SENSOR_READ_ERROR               -1
#define SENSOR_CRC_ERROR                0
#define SENSOR_CheckSum_ERROR           0
#define SENSOR_READ_SUCCESSFUL          1

#define NORMAL_AMBIENT_CONDITION            0
#define ABNORMAL_AMBIENT_CONDITION          1
#define TEMP_SENSOR_READ_ERROR_CONDITION    2
#define TEMP_SENSOR_CRC_ERROR_CONDITION     3

#define I2C_NORMAL_OP_WAIT_LOOP_COUNT   500  // after testing, 143 is the max number of loops required for reading from I2C
#define I2C_TEST_OP_WAIT_LOOP_COUNT     500  // startup operation requires more time

#define ONE_WIRE_TIMEOUT                0xFFFF
#define ONE_WIRE_MIN_SAMPLES            50

/* MLX90614 IR Temperater Sensor Variables */
#define MLX90614_I2CADDR                0xB4

// RAM
#define MLX90614_RAWIR1                 0x04
#define MLX90614_RAWIR2                 0x05
#define MLX90614_TA                     0x06
#define MLX90614_TOBJ1                  0x07
#define MLX90614_TOBJ2                  0x08

// EEPROM
#define MLX90614_TOMAX                  0x20
#define MLX90614_TOMIN                  0x21
#define MLX90614_PWMCTRL                0x22
#define MLX90614_TARANGE                0x23
#define MLX90614_EMISS                  0x24
#define MLX90614_CONFIG                 0x25
#define MLX90614_ADDR                   0x2E
#define MLX90614_ID1                    0x3C
#define MLX90614_ID2                    0x3D
#define MLX90614_ID3                    0x3E
#define MLX90614_ID4                    0x3F
#define MLX90614_COMM_ERROR_CODE        1000
#define MLX90614_MIN_CELCIUS_LIMIT      -70
#define MLX90614_MAX_CELCIUS_LIMIT      380

uint16_t MLX90614_data;
uint8_t MLX90614_data_bytes[2], MLX90614_special_bytes[2], MLX90614_pec;

/* MLX90614 IR Temperater Sensor Variables */

/* NTC Thermistor 4092 50k Temperater Sensor Variables */
#define THERMISTOR_LUT_SIZE             200
float NTC_Thermistor_4092_50k_LUT_Resistance[THERMISTOR_LUT_SIZE], NTC_Thermistor_4092_50k_LUT_Temperature_Celcius[THERMISTOR_LUT_SIZE];
uint8_t NTC_Thermistor_4092_50k_special_bytes[2];
/* NTC Thermistor 4092 50k Temperater Sensor Variables */

//#define TH_DHT22_DEBUG
//#define _TEMPSENSOR_DEBUG_
#define N_TEMPERATURE_READINGS      10
float TEMPERATURE_READINGS_ARRAY_0[N_TEMPERATURE_READINGS];
float TEMPERATURE_READINGS_ARRAY_1[N_TEMPERATURE_READINGS];
int current_temperature_reading_index_0;
int current_temperature_reading_index_1;
bool first_N_temperature_readings_acquired;
/** The data received from the temperature sensor AM2320, i.e., Control byte, number of bytes' byte, 4 data bytes, 2 CRC bytes */ 
/** It is used by DHT22 if that is our active sensor */
uint8_t recv_data[8];

/** This is a digital read sample counter for DHT22 One-Wire protocol */
uint32_t sample_counter[82];

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

void setup_IR_Temperature_Sensor_And_Laser();
float MLX90614_Read_Temperature_Ambient_Celcius();
float MLX90614_Read_Temperature_Object_Celcius();
float MLX90614_Read_Temperature_Ambient_Fahrenheit();
float MLX90614_Read_Temperature_Object_Fahrenheit();

float Thermistor_NTC_4092_50k_Get_Object_Temperature_In_Celcius(uint8_t adc_channel);

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
bool CRC_check();

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
 * @return -1 if reading corruption, 0 if CRC check failed; 1 if successful
 */
int8_t sample_Temperature_Humidity_bytes_using_AM2320(uint8_t* temperature_bytes, uint8_t* relative_humidity_bytes);

uint16_t ExpectState(uint8_t level);
bool DHT22_OW_Read_Temp_and_Humidity();
bool CheckSum();

/**
 * Samples temperature and humidity readings from DHT-22 sensor
 * @param temperature_bytes Pointer to 8-bit byte array to save temperature reading as bytes
 * @param relative_humidity_bytes Pointer to 8-bit byte array to save relative humidity reading as bytes
 * @return -1 if reading corruption, 0 if CRC check failed; 1 if successful
 */
int8_t sample_Temperature_Humidity_bytes_using_DHT22(uint8_t* temperature_bytes, uint8_t* relative_humidity_bytes);

/**
 * Gets ambient condition status
 * @return <b>NORMAL_AMBIENT_CONDITION</b> if normal; <b>ABNORMAL_AMBIENT_CONDITION</b> otherwise.
 */
uint8_t ambient_condition_status(uint8_t TEMPERATURE_MIN_THRESHOLD, uint8_t TEMPERATURE_MAX_THRESHOLD, uint8_t RELATIVE_HUMIDITY_MIN_THRESHOLD, uint8_t RELATIVE_HUMIDITY_MAX_THRESHOLD);
float average_value_of_temperature(uint8_t thermistor_channel, float current_temperature);

#endif

