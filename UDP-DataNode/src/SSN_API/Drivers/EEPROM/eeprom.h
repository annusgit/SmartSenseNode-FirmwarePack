
#ifndef __eeprom_h__
#define __eeprom_h__


#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

#include "../../global.h"
#include <stdint.h>
#include <stdio.h>
#include <plib.h>

/** EEPROM 24LC08 Address */
#define EEPROM_24LC08_BASE_ADDRESS  0xA0 // Append block number and R/W bit
/** EEPROM 24LC08 Read bit */
#define EEPROM_24LC08_READ_BIT      0x00
/** EEPROM 24LC08 Write bit */
#define EEPROM_24LC08_WRITE_BIT     0x01
/** EEPROM 24LC08 Read bit combined with address */
#define EEPROM_BYTE_READ            0xA1
/** EEPROM 24LC08 Write bit combined with address */
#define EEPROM_BYTE_WRITE           0xA0
/** EEPROM 24LC08 block size of 256 bytes */
#define EEPROM_BLOCK_SIZE           256
/** EEPROM 24LC08 test location for testing operation */
#define EEPROM_TEST_LOCATION        0x50
/** EEPROM 24LC08 test value */
#define EEPROM_TEST_VALUE           0x91
/** EEPROM 24LC08 clear value. All bytes must be set to 0xFF */
#define EEPROM_CLEAR_VALUE          0xFF
/** EEPROM 24LC08 test passed */
#define EEPROM_TEST_PASSED          1
/** EEPROM 24LC08 test failed */
#define EEPROM_TEST_FAILED          0


/** Our EEPROM chip has four blocks of memory, 256 bytes in each block */
enum EEPROM_24LC08_BLOCKS {EEPROM_BLOCK_0=0, EEPROM_BLOCK_1, EEPROM_BLOCK_2, EEPROM_BLOCK_3, EEPROM_BLOCK_COUNT};

/** 
 * Opens I2C1 peripheral
 */
void open_I2C1();

/** 
 * Waits while I2C1 is busy reading or writing
 */
void I2C1_wait_while_busy();

/** 
 * Transmit single bit for starting I2C communication
 */
void I2C1_transmit_start_bit();

/** 
 * Transmit single bit for stoping I2C communication
 */
void I2C1_transmit_stop_bit();

/** 
 * Transmit single bit for restarting I2C communication
 */
void I2C1_transmit_restart_bit();

/** 
 * Transmit single byte over I2C
 * @param byte Single byte to transmit
 */
void I2C1_transmit_byte(uint8_t byte);

/** 
 * Recieves byte over I2C1
 * @return Single byte read over I2C1
 */
uint8_t I2C1_receive_byte();

// API level functions for EEPROM
/** 
 * Sets up EEPROM using I2C1
 */
void setup_EEPROM();

/** 
 * Writes single byte into the EEPROM
 * @param block Block number to write into (1-4)
 * @param address Address inside the block to write into (0-255)
 * @param data The single byte to write at this location
 */
uint8_t EEPROM_Write_BYTE(uint8_t block, uint8_t address, uint8_t data);

/** 
 * Reads a single byte from the EEPROM
 * @param block Block number to read from (1-4)
 * @param address Address inside the block to read from (0-255)
 * @return The single byte read from this location
 */
uint8_t EEPROM_Read_BYTE(uint8_t block, uint8_t address);

/** 
 * Writes byte array into the EEPROM. Bytes are written at contiguous locations
 * @param block Block number to write into (1-4)
 * @param address Address inside the block to start writing from (0-255)
 * @param arr The byte array to write into the EEPROM
 * @param size The number of bytes to write
 */
uint8_t EEPROM_Write_Array(uint8_t block, uint8_t address, uint8_t* arr, uint8_t size);

/** 
 * Reads a byte array from the EEPROM. Bytes are read from contiguous locations
 * @param block Block number to read from (1-4)
 * @param address Address inside the block to start reading from (0-255)
 * @param arr The byte array to write the read values into
 * @param size The number of bytes to read 
 * @return 1 for read successful
 */
uint8_t EEPROM_Read_Array(uint8_t block, uint8_t address, uint8_t* arr, uint8_t size);

/** 
 * Clears EEPROM
 * @return 1 for successfully cleared
 */
uint8_t EEPROM_Clear();

/** 
 * Checks EEPROM read/write by writing and reading a single byte at a pre-defined location
 * @return <b>EEPROM_TEST_PASSED</b> for successful test; <b>EEPROM_TEST_FAILED</b> for failure
 */
uint8_t EEPROM_Check();


#endif