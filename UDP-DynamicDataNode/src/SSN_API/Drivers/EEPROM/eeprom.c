
#include "eeprom.h"

void open_I2C1(){
    I2C1CON = 0x00;             // turn off the I2C1 module
    I2C1CONbits.DISSLW = 1;     // Disable slew rate for 100kHz
    I2C1BRG = 0x091;            // 100KHz operation
    I2C1CON = 0x00009910;       // turn on the I2C1 module
}

void setup_EEPROM() {
    open_I2C1();
}

void I2C1_wait_while_busy() {
    while(I2C1CON & 0x1F || I2C1STATbits.TRSTAT); // idle
}

void I2C1_transmit_start_bit() {
    I2C1CONbits.SEN = 1;
    while (I2C1CONbits.SEN == 1);    
}

void I2C1_transmit_stop_bit() {
	I2C1CONbits.PEN = 1;
    while (I2C1CONbits.PEN == 1);  
}

void I2C1_transmit_restart_bit() {
    I2C1CONbits.RSEN = 1;
    while (I2C1CONbits.RSEN == 1);    
}

void I2C1_transmit_byte(uint8_t byte) {
    I2C1TRN = byte;
}

uint8_t I2C1_receive_byte() {
    I2C1CONbits.RCEN = 1;                           // Receive enable
    while (I2C1CONbits.RCEN || !I2C1STATbits.RBF);  // Wait until RCEN is cleared (automatic)  
    return I2C1RCV;
}

uint8_t EEPROM_Write_BYTE(uint8_t block, uint8_t address, uint8_t data){
    // 1. initiate start condition
    I2C1_transmit_start_bit();
    I2C1_wait_while_busy();
    // 2. write the control byte for 24LC08
    I2C1_transmit_byte((EEPROM_BYTE_WRITE | (block << 1)));
    I2C1_wait_while_busy();
    // 3. write the address to be accessed
    I2C1_transmit_byte(address);
    I2C1_wait_while_busy();
    // 4. write the data
    I2C1_transmit_byte(data);
    I2C1_wait_while_busy();
    // 5. initiate the stop condition
    I2C1_transmit_stop_bit();
    I2C1_wait_while_busy();
    return 1;
}

uint8_t EEPROM_Read_BYTE(uint8_t block, uint8_t address){
    // 1. initiate start condition
    I2C1_transmit_start_bit();
    I2C1_wait_while_busy();
    // 2. write the control byte for 24LC08 write
    I2C1_transmit_byte((EEPROM_BYTE_WRITE | (block << 1)));
    I2C1_wait_while_busy();
    // 3. write the address to be accessed
    I2C1_transmit_byte(address);
    I2C1_wait_while_busy();
    // 4. initiate restart condition
    I2C1_transmit_restart_bit();
    I2C1_wait_while_busy();
    // 5. write the control byte for 24LC08 read
    I2C1_transmit_byte((EEPROM_BYTE_READ | (block << 1)));
    I2C1_wait_while_busy();    
    // 6. read the data from receive buffer
    uint32_t recv_word = I2C1_receive_byte();
    I2C1_wait_while_busy();
    // 7. initiate the stop condition
    I2C1_transmit_stop_bit();
    I2C1_wait_while_busy();
    return recv_word;
}

uint8_t EEPROM_Write_Array(uint8_t block, uint8_t address, uint8_t* arr, uint8_t size) {
    uint16_t count = 0;
    // write
    while(count < size) {
        EEPROM_Write_BYTE(block, address+count, arr[count]);
        // let it write for 5ms
        sleep_for_microseconds(5000);
        count++;
    }
    // check write
    count = 0;
    while(count < size) {
        if (EEPROM_Read_BYTE(block, address+count) != arr[count])
            return 0;
        count++;
    }
    return 1;
}

uint8_t EEPROM_Read_Array(uint8_t block, uint8_t address, uint8_t* arr, uint8_t size) {
    unsigned int count = 0;
    while(count < size) {
        arr[count] = EEPROM_Read_BYTE(block, address+count);
        count++;
    }
    return 1;
}

uint8_t EEPROM_Clear() {
    printf("LOG: Wait for 5 seconds...\n");
    uint16_t block_count, memory_count, CLEAR = 0xFF;    
    for (block_count = 0; block_count < EEPROM_BLOCK_COUNT; block_count++) {
        for (memory_count = 0; memory_count < EEPROM_BLOCK_SIZE; memory_count++) {
            EEPROM_Write_BYTE(block_count, memory_count, CLEAR);
            // wait for 5ms
            sleep_for_microseconds(5000);
        }
    }
    
    // check
    for (block_count = 0; block_count < EEPROM_BLOCK_COUNT; block_count++) {
        for (memory_count = 0; memory_count < EEPROM_BLOCK_SIZE; memory_count++) {
            if (EEPROM_Read_BYTE(block_count, memory_count) != CLEAR)
                return 0;
        }
    }
    return 1;
}

uint8_t EEPROM_Check() {
    // First clear a certain location and check if it works
    EEPROM_Write_BYTE(EEPROM_BLOCK_2, EEPROM_TEST_LOCATION, EEPROM_CLEAR_VALUE);
    sleep_for_microseconds(5000);
    // Clear successful?
    if (EEPROM_Read_BYTE(EEPROM_BLOCK_2, EEPROM_TEST_LOCATION) != EEPROM_CLEAR_VALUE)
        return EEPROM_TEST_FAILED;
    // write to a default location and check if it works
    EEPROM_Write_BYTE(EEPROM_BLOCK_2, EEPROM_TEST_LOCATION, EEPROM_TEST_VALUE);
    sleep_for_microseconds(5000);
    // Test successful?
    if (EEPROM_Read_BYTE(EEPROM_BLOCK_2, EEPROM_TEST_LOCATION) != EEPROM_TEST_VALUE)
        return EEPROM_TEST_FAILED;
    return EEPROM_TEST_PASSED;
}




