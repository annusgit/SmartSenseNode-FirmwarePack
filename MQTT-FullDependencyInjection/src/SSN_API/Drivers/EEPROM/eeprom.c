#include "eeprom.h"

uint32_t i2c_eeprom_wait_loop_count = I2C_EEPROM_OP_WAIT_LOOP_COUNT;

void open_I2C1() {
	I2C1CON = 0x00; // turn off the I2C1 module
	I2C1CONbits.DISSLW = 1; // Disable slew rate for 100kHz
	I2C1BRG = 0x091; // 100KHz operation
	I2C1CON = 0x00009910; // turn on the I2C1 module
}

void setup_EEPROM() {
	open_I2C1();
}

//void I2C1_wait_while_busy() {
//    while(I2C1CON & 0x1F || I2C1STATbits.TRSTAT); // idle
//}

bool I2C1_wait_while_busy() {
	// returns 1 if runs normally, returns 0 in case of timeout
	uint16_t wait = 0;
	while (I2C1CON & 0x1F || I2C1STATbits.TRSTAT) {
		if (wait++ >= i2c_eeprom_wait_loop_count) {
			return 0;
		}
	}
	return 1;
}

//void I2C1_transmit_start_bit() {
//    I2C1CONbits.SEN = 1;
//    while (I2C1CONbits.SEN == 1);    
//}

bool I2C1_transmit_start_bit() {
	// returns 1 if runs normally, returns 0 in case of timeout
	uint16_t wait = 0;
	I2C1CONbits.SEN = 1;
	while (I2C1CONbits.SEN == 1) {
		if (wait++ >= i2c_eeprom_wait_loop_count) {
			return 0;
		}
	}
	return 1;
}

//void I2C1_transmit_stop_bit() {
//	I2C1CONbits.PEN = 1;
//    while (I2C1CONbits.PEN == 1);  
//}

bool I2C1_transmit_stop_bit() {
	// returns 1 if runs normally, returns 0 in case of timeout
	uint16_t wait = 0;
	I2C1CONbits.PEN = 1;
	while (I2C1CONbits.PEN == 1) {
		if (wait++ >= i2c_eeprom_wait_loop_count) {
			return 0;
		}
	}
	return 1;
}

//void I2C1_transmit_restart_bit() {
//    I2C1CONbits.RSEN = 1;
//    while (I2C1CONbits.RSEN == 1);    
//}

bool I2C1_transmit_restart_bit() {
	uint16_t wait = 0;
	I2C1CONbits.RSEN = 1;
	while (I2C1CONbits.RSEN == 1) {
		if (wait++ >= i2c_eeprom_wait_loop_count) {
			return 0;
		}
	}
	return 1;
}

void I2C1_transmit_byte(uint8_t byte) {
	I2C1TRN = byte;
}

//uint8_t I2C1_receive_byte() {
//    I2C1CONbits.RCEN = 1;                           // Receive enable
//    while (I2C1CONbits.RCEN || !I2C1STATbits.RBF);  // Wait until RCEN is cleared (automatic)  
//    return I2C1RCV;
//}

uint8_t I2C1_receive_byte() {
	uint16_t wait = 0;
	I2C1CONbits.RCEN = 1; // Receive enable
	while (I2C1CONbits.RCEN || !I2C1STATbits.RBF) { // Wait until RCEN is cleared (automatic)  
		if (wait++ >= i2c_eeprom_wait_loop_count) {
			return 0;
		}
	}
	return (0xFF & I2C1RCV);
}

uint8_t EEPROM_Write_BYTE(uint8_t block, uint8_t address, uint8_t data) {
	// 1. initiate start condition
	if (!I2C1_transmit_start_bit()) return 0;
	if (!I2C1_wait_while_busy()) return 0;
	// 2. write the control byte for 24LC08
	I2C1_transmit_byte((EEPROM_BYTE_WRITE | (block << 1)));
	if (!I2C1_wait_while_busy()) return 0;
	// 3. write the address to be accessed
	I2C1_transmit_byte(address);
	if (!I2C1_wait_while_busy()) return 0;
	// 4. write the data
	I2C1_transmit_byte(data);
	if (!I2C1_wait_while_busy()) return 0;
	// 5. initiate the stop condition
	if (!I2C1_transmit_stop_bit()) return 0;
	if (!I2C1_wait_while_busy()) return 0;
	return 1;
}

uint8_t EEPROM_Read_BYTE(uint8_t block, uint8_t address) {
	// 1. initiate start condition
	if (!I2C1_transmit_start_bit()) return 0;
	if (!I2C1_wait_while_busy()) return 0;
	// 2. write the control byte for 24LC08 write
	I2C1_transmit_byte((EEPROM_BYTE_WRITE | (block << 1)));
	if (!I2C1_wait_while_busy()) return 0;
	// 3. write the address to be accessed
	I2C1_transmit_byte(address);
	if (!I2C1_wait_while_busy()) return 0;
	// 4. initiate restart condition
	if (!I2C1_transmit_restart_bit()) return 0;
	if (!I2C1_wait_while_busy()) return 0;
	// 5. write the control byte for 24LC08 read
	I2C1_transmit_byte((EEPROM_BYTE_READ | (block << 1)));
	if (!I2C1_wait_while_busy()) return 0;
	// 6. read the data from receive buffer
	uint32_t recv_word = I2C1_receive_byte();
	if (!I2C1_wait_while_busy()) return 0;
	// 7. initiate the stop condition
	if (!I2C1_transmit_stop_bit()) return 0;
	if (!I2C1_wait_while_busy()) return 0;
	return recv_word;
}

uint8_t EEPROM_Write_Array(uint8_t block, uint8_t address, uint8_t* arr, uint8_t size) {
	uint16_t count = 0;
	// write
	while (count < size) {
		EEPROM_Write_BYTE(block, address + count, arr[count]);
		// let it write for 5ms
		sleep_for_microseconds(5000);
		count++;
	}
	// check write
	count = 0;
	while (count < size) {
		if (EEPROM_Read_BYTE(block, address + count) != arr[count])
			return 0;
		count++;
	}
	return 1;
}

uint8_t EEPROM_Read_Array(uint8_t block, uint8_t address, uint8_t* arr, uint8_t size) {
	unsigned int count = 0;
	while (count < size) {
		arr[count] = EEPROM_Read_BYTE(block, address + count);
		count++;
	}
	return 1;
}

uint8_t EEPROM_Clear() {
	printf("[LOG] *Clearing all EEPROM Blocks and all their locations...\n");
	uint16_t memory_count;
	int block_count;
	for (block_count = 0; block_count < EEPROM_BLOCK_COUNT; block_count++) {
		printf("[LOG] Clearing Block-%d\n", block_count + 1);
		for (memory_count = 0; memory_count < EEPROM_BLOCK_SIZE; memory_count++) {
			EEPROM_Write_BYTE(block_count, memory_count, EEPROM_CLEAR_VALUE);
			// wait for 5ms
			sleep_for_microseconds(5000);
		}
	}
	// check that each location is cleared
	for (block_count = 0; block_count < EEPROM_BLOCK_COUNT; block_count++) {
		printf("[LOG] Rechecking Cleared Block-%d\n", block_count + 1);
		for (memory_count = 0; memory_count < EEPROM_BLOCK_SIZE; memory_count++) {
			if (EEPROM_Read_BYTE(block_count, memory_count) != EEPROM_CLEAR_VALUE) {
				return 0;
			}
		}
	}
	printf("[LOG] Done. All Memory Erased! New Life!!!\n");
	return 1;
}

uint8_t EEPROM_Check() {
	int test_loc, test_val, prev_val;
	// test five values; one in each block
	int this_block;
	for (this_block = 0; this_block < EEPROM_BLOCK_COUNT; this_block++) {
		printf("[LOG] -- Testing Block-%d --\n", this_block);
		int k;
		for (k = 1; k <= EEPROM_PER_BLOCK_TEST_COUNT; k++) {
			printf("[LOG]\t Test %d of %d\n", this_block * EEPROM_PER_BLOCK_TEST_COUNT + k, EEPROM_BLOCK_COUNT * EEPROM_PER_BLOCK_TEST_COUNT);
			// generate a random location between 0 and block size (255)
			// but make sure we don't test any used location. Just test other locations 
			test_loc = rand() % EEPROM_BLOCK_SIZE;
			while (test_loc >= EEPROM_CONFIG_LOC && test_loc <= (EEPROM_CONFIG_LOC + EEPROM_CONFIG_SIZE)) {
				printf("[LOG]\t\t Skipping Used Location %d...\n", test_loc);
				test_loc = rand() % EEPROM_BLOCK_SIZE;
			}
			// before proceeding, first recover previous value from this location
			prev_val = EEPROM_Read_BYTE(this_block, test_loc);
			printf("[LOG]\t\t Saved Previous Value = %d at Location %d\n", prev_val, test_loc);
			printf("[LOG]\t\t Clearing Location %d\n", test_loc);
			// First clear a certain location and check if it works
			EEPROM_Write_BYTE(this_block, test_loc, EEPROM_CLEAR_VALUE);
			sleep_for_microseconds(5000);
			// Clear successful?
			if (EEPROM_Read_BYTE(this_block, test_loc) != EEPROM_CLEAR_VALUE) {
				printf("[LOG]\t\t Clearing Test Failed at Location %d\n", test_loc);
				return EEPROM_TEST_FAILED;
			} else {
				printf("[LOG]\t\t Clearing Test Passed!\n", test_loc);
			}
			// write to a random location in this block and check if it works
			// the value must be one byte, so use same code as location (0-255)
			test_val = rand() % EEPROM_BLOCK_SIZE;
			printf("[LOG]\t\t Writing Random Value = %d at Location %d\n", test_val, test_loc);
			EEPROM_Write_BYTE(this_block, test_loc, test_val);
			sleep_for_microseconds(5000);
			// Test successful?
			if (EEPROM_Read_BYTE(this_block, test_loc) != test_val) {
				printf("[LOG]\t\t Random Write Test Failed at Location %d\n", test_loc);
				return EEPROM_TEST_FAILED;
			} else {
				printf("[LOG]\t\t Random Write Test Passed!\n", test_loc);
			}
			printf("[LOG]\t\t Returning previous value to Location %d...\n", test_loc);
			// after all tests have passed, rewrite the initial value back at this test location and check
			EEPROM_Write_BYTE(this_block, test_loc, prev_val);
			sleep_for_microseconds(5000);
			// Rewrite successful?
			if (EEPROM_Read_BYTE(this_block, test_loc) != prev_val) {
				printf("[LOG]\t\t Previous value = %d rewriting at Location %d procedure Failed!\n", prev_val, test_loc);
				return EEPROM_TEST_FAILED;
			} else {
				printf("[LOG]\t\t Previous value = %d rewritten at Location %d\n", prev_val, test_loc);
			}
		}
	}
	return EEPROM_TEST_PASSED;
}

