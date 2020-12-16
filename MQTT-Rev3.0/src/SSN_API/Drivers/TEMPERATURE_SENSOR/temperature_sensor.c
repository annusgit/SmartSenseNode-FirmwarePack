#include "temperature_sensor.h"

void open_I2C2() {
	// setup the peripheral at 100KHz
	I2C2CON = 0x00; // turn off the I2C2 module
	I2C2CONbits.DISSLW = 1; // Disable slew rate for 100kHz
	I2C2BRG = 0x091; // 100KHz operation
	I2C2CONbits.ON = 1; // turn on the I2C2 module
	// setup the required pins for this peripheral
	PORTSetPinsDigitalOut(IOPORT_A, BIT_2);
	PORTSetPinsDigitalOut(IOPORT_B, BIT_2);
	PORTSetPinsDigitalOut(IOPORT_B, BIT_3);
	PORTSetBits(IOPORT_B, BIT_2);
	PORTSetBits(IOPORT_B, BIT_3);
}

void setup_Temperature_Humidity_Sensor() {
#ifdef TH_AM2320
	open_I2C2();
	PORTSetPinsDigitalOut(IOPORT_A, BIT_4);
	PORTSetBits(IOPORT_A, BIT_4);
#endif
#ifdef TH_DHT22
	//         Peripheral pin selection for the output pin
	//        RPB2R = 0x00;
#endif
}

bool I2C2_wait_while_busy() {
	// returns 1 if runs normally, returns 0 in case of timeout
	uint16_t wait = 0;
	while (I2C2CON & 0x1F || I2C2STATbits.TRSTAT) {
		if (wait++ >= wait_loop_count) {
			return 0;
		}
	}
#ifdef _TEMPSENSOR_DEBUG_
	printf("I2C2_wait_while_busy loop count: %d\n", wait);
#endif
	return 1;
}

bool I2C2_transmit_start_bit() {
	// returns 1 if runs normally, returns 0 in case of timeout
	uint16_t wait = 0;
	I2C2CONbits.SEN = 1;
	while (I2C2CONbits.SEN == 1) {
		if (wait++ >= wait_loop_count) {
			return 0;
		}
	}
#ifdef _TEMPSENSOR_DEBUG_
	printf("I2C2_transmit_start_bit loop count: %d\n", wait);
#endif
	return 1;
}

bool I2C2_transmit_stop_bit() {
	// returns 1 if runs normally, returns 0 in case of timeout
	uint16_t wait = 0;
	I2C2CONbits.PEN = 1;
	while (I2C2CONbits.PEN == 1) {
		if (wait++ >= wait_loop_count) {
			return 0;
		}
	}
#ifdef _TEMPSENSOR_DEBUG_
	printf("I2C2_transmit_stop_bit loop count: %d\n", wait);
#endif
	return 1;
}

bool I2C2_transmit_restart_bit() {
	uint16_t wait = 0;
	I2C2CONbits.RSEN = 1;
	while (I2C2CONbits.RSEN == 1) {
		if (wait++ >= wait_loop_count) {
			return 0;
		}
	}
#ifdef _TEMPSENSOR_DEBUG_
	printf("I2C2_transmit_restart_bit loop count: %d\n", wait);
#endif
	return 1;
}

void I2C2_transmit_byte(uint8_t byte) {
	I2C2TRN = byte;
}

uint8_t I2C2_receive_byte() {
	uint16_t wait = 0;
	I2C2CONbits.RCEN = 1; // Receive enable
	while (I2C2CONbits.RCEN || !I2C2STATbits.RBF) { // Wait until RCEN is cleared (automatic)  
		if (wait++ >= wait_loop_count) {
			return 0;
		}
	}
#ifdef _TEMPSENSOR_DEBUG_
	printf("I2C2_receive_byte loop count: %d\n", wait);
#endif
	return (0xFF & I2C2RCV);
}

bool I2C2_is_byte_received() {
	uint16_t wait = 0;
	I2C2CONbits.RCEN = 1; // Receive enable
	while (I2C2CONbits.RCEN || !I2C2STATbits.RBF) { // Wait until RCEN is cleared (automatic)  
		if (wait++ >= wait_loop_count) {
			return 0;
		}
	}
#ifdef _TEMPSENSOR_DEBUG_
	printf("I2C2_is_byte_received loop count: %d\n", wait);
#endif
	return 1;
}

bool I2C2_ack(void) {
	uint16_t wait = 0;
	I2C2_wait_while_busy();
	I2C2CONbits.ACKDT = 0; // Set hardware to send ACK bit
	I2C2CONbits.ACKEN = 1; // Send ACK bit, will be automatically cleared by hardware when sent  
	while (I2C2CONbits.ACKEN) { // Wait until ACKEN bit is cleared, meaning ACK bit has been sent
		if (wait++ >= wait_loop_count) {
			return 0;
		}
	}
#ifdef _TEMPSENSOR_DEBUG_
	printf("I2C2_ack loop count: %d\n", wait);
#endif
	return 1;
}

bool AM2320_I2C2_Read_Temp_and_Humidity() {
	// startup sequence, wake up the AM2320
	// initiate start condition
	if (!I2C2_transmit_start_bit()) return 0;
	if (!I2C2_wait_while_busy()) return 0;
	// Address the temperature sensor
	I2C2_transmit_byte((AM2320_I2C_Address | 0));
	if (!I2C2_wait_while_busy()) return 0;
	// stop bit
	if (!I2C2_transmit_stop_bit()) return 0;
	if (!I2C2_wait_while_busy()) return 0;
	sleep_for_microseconds(10000);

	// read register sequence
	// initiate start condition
	if (!I2C2_transmit_start_bit()) return 0;
	if (!I2C2_wait_while_busy()) return 0;
	// Address the temperature sensor
	I2C2_transmit_byte((AM2320_I2C_Address | 0));
	if (!I2C2_wait_while_busy()) return 0;

	// functional code for reading registers
	I2C2_transmit_byte(AM2320_Read_Function_Code);
	if (!I2C2_wait_while_busy()) return 0;

	// starting address to read from
	I2C2_transmit_byte(AM2320_Starting_Address);
	if (!I2C2_wait_while_busy()) return 0;

	// number of bytes to be read
	I2C2_transmit_byte(AM2320_Num_Bytes_Requested);
	if (!I2C2_wait_while_busy()) return 0;

	// stop bit
	if (!I2C2_transmit_stop_bit()) return 0;
	if (!I2C2_wait_while_busy()) return 0;
	sleep_for_microseconds(2000);

	// receive the output of the sensor in its specific format
	// initiate start condition
	if (!I2C2_transmit_start_bit()) return 0;
	if (!I2C2_wait_while_busy()) return 0;
	// Address the temperature sensor
	I2C2_transmit_byte((AM2320_I2C_Address | 1));
	if (!I2C2_wait_while_busy()) return 0;

	// Check if we have received an I2C byte
	if (!I2C2_is_byte_received()) return 0;
	recv_data[0] = (0xFF & I2C2RCV);
	if (!I2C2_wait_while_busy()) return 0;
	if (!I2C2_ack()) return 0;
	if (!I2C2_wait_while_busy()) return 0;
	sleep_for_microseconds(3);
	if (recv_data[0] == AM2320_Read_Function_Code)
#ifdef _TEMPSENSOR_DEBUG_
		printf(">> Control byte received\n");
#endif

	// Check if we have received an I2C byte
	if (!I2C2_is_byte_received()) return 0;
	recv_data[1] = (0xFF & I2C2RCV);
	if (!I2C2_ack()) return 0;
	if (!I2C2_wait_while_busy()) return 0;
	sleep_for_microseconds(3);
	if (recv_data[1] == AM2320_Num_Bytes_Requested)
#ifdef _TEMPSENSOR_DEBUG_
		printf(">> Number byte received\n");
#endif

	// Check if we have received an I2C byte
	if (!I2C2_is_byte_received()) return 0;
	recv_data[2] = (0xFF & I2C2RCV);
	if (!I2C2_wait_while_busy()) return 0;
	if (!I2C2_ack()) return 0;
	if (!I2C2_wait_while_busy()) return 0;
	sleep_for_microseconds(3);

	// Check if we have received an I2C byte
	if (!I2C2_is_byte_received()) return 0;
	recv_data[3] = (0xFF & I2C2RCV);
	if (!I2C2_wait_while_busy()) return 0;
	if (!I2C2_ack()) return 0;
	if (!I2C2_wait_while_busy()) return 0;
	sleep_for_microseconds(3);

	// Check if we have received an I2C byte
	if (!I2C2_is_byte_received()) return 0;
	recv_data[4] = (0xFF & I2C2RCV);
	if (!I2C2_wait_while_busy()) return 0;
	if (!I2C2_ack()) return 0;
	if (!I2C2_wait_while_busy()) return 0;
	sleep_for_microseconds(3);

	// Check if we have received an I2C byte
	if (!I2C2_is_byte_received()) return 0;
	recv_data[5] = (0xFF & I2C2RCV);
	if (!I2C2_wait_while_busy()) return 0;
	if (!I2C2_ack()) return 0;
	if (!I2C2_wait_while_busy()) return 0;
	sleep_for_microseconds(3);

	// Check if we have received an I2C byte
	if (!I2C2_is_byte_received()) return 0;
	recv_data[6] = (0xFF & I2C2RCV);
	if (!I2C2_wait_while_busy()) return 0;
	if (!I2C2_ack()) return 0;
	if (!I2C2_wait_while_busy()) return 0;
	sleep_for_microseconds(3);

	// Check if we have received an I2C byte
	if (!I2C2_is_byte_received()) return 0;
	recv_data[7] = (0xFF & I2C2RCV);
	if (!I2C2_wait_while_busy()) return 0;

	// transmit final stop bit
	if (!I2C2_transmit_stop_bit()) return 0;
	if (!I2C2_wait_while_busy()) return 0;
	return 1;
}

uint16_t convert_bytes_to_word(int8_t high_byte, int8_t low_byte) {
	return ((high_byte & 0xFF) << 8) | (low_byte & 0xFF);
}

unsigned short crc16(unsigned char *ptr, unsigned char len) {
	unsigned short crc = 0xFFFF;
	unsigned char i;
	while (len--) {
		crc ^= *ptr++;
		for (i = 0; i < 8; i++) {
			if (crc & 0x01) {
				crc >>= 1;
				crc ^= 0xA001;
			} else crc >>= 1;
		}
	}
	return crc;
}

bool CRC_check() {
	unsigned short CRC_calculated = crc16(recv_data, 6);
	unsigned short CRC_received = ((recv_data[7] & 0xFF) << 8) | (recv_data[6] & 0xFF);
	if (CRC_calculated == CRC_received)
		return 1;
	return 0;
}

uint8_t sample_Temperature_Humidity(uint16_t *temperature, uint16_t* relative_humidity) {
	uint16_t temp_t, humid_t; // temporary variables
	AM2320_I2C2_Read_Temp_and_Humidity();
	temp_t = convert_bytes_to_word(recv_data[4], recv_data[5]);
	humid_t = convert_bytes_to_word(recv_data[2], recv_data[3]);
	if (CRC_check()) {
		// CRC check confirms correct readings
		*temperature = temp_t;
		*relative_humidity = humid_t;
		return 1;
	}
	return 0;
}

uint16_t ExpectState(uint8_t level) {
	uint16_t count = 0;
	T5CON = 0x8000; // enable Timer5, source PBCLK, 1:1 prescaler
	// wait for a certain number of counts or return TIMEOUT if more than 1ms
	while (PORTBbits.RB2 == level) {
		if (TMR5 < PERIPH_CLK / 1000) {
			count++;
		} else {
			// break since we have a timeout at 1ms
			T5CONCLR = 0x8000; // disable Timer5
			return ONE_WIRE_TIMEOUT;
		}
	}
	T5CONCLR = 0x8000; // disable Timer5
	return count;
}

bool DHT22_OW_Read_Temp_and_Humidity() {
	uint16_t counts_used, count;
	PORTSetPinsDigitalOut(IOPORT_B, BIT_2);
	/* Reset the DHT22 Custom OW */
	ClearPinForMicroseconds(IOPORT_B, BIT_2, 1100); // Reset
	SetPinForMicroseconds(IOPORT_B, BIT_2, 25); // bring it back to high/idle
	/* Wait for the device to show presence ~185us */
	PORTSetPinsDigitalIn(IOPORT_B, BIT_2); // Set pin for digital input
	counts_used = 0;
	// Capture the presence state with 0/1 alternating levels
	count = ExpectState(0x00);
	if (count == ONE_WIRE_TIMEOUT) return 0;
	sample_counter[counts_used++] = count;
	count = ExpectState(0x01);
	if (count == ONE_WIRE_TIMEOUT) return 0;
	sample_counter[counts_used++] = count;
	// capture the rest of the 40 bits
	uint8_t i;
	for (i = 0; i < 40; i++) {
		if (i % 8 == 0) {
			sleep_for_microseconds(1);
		}
		count = ExpectState(0x00);
		if (count == ONE_WIRE_TIMEOUT) return 0;
		sample_counter[counts_used++] = count;
		count = ExpectState(0x01);
		if (count == ONE_WIRE_TIMEOUT) return 0;
		sample_counter[counts_used++] = count;
		// generate bit value from captured signal
		recv_data[i / 8] <<= 1;
		if (sample_counter[counts_used - 1] >= sample_counter[counts_used - 2]) {
			recv_data[i / 8] |= 1;
		} else {
			// do nothing
		}
	}
#ifdef TH_DHT22_DEBUG
	printf("Captured %d bits: ", counts_used);
	uint16_t j;
	for (j = 0; j < counts_used; j++) {
		printf("%d ", sample_counter[j]);
	}
	uint8_t k;
	for (k = 0; k < 5; k++) {
		printf("%d ", recv_data[k]);
	}
	float temperature = (float) (((recv_data[2] & 0x7F) << 8) | recv_data[3]) / 10.0f;
	float relative_humidity = (float) ((recv_data[0] << 8) | recv_data[1]) / 10.0f;
	printf("%.2f %.2f\n", temperature, relative_humidity);
#endif
	return 1; // return 1 only if everything went fine
}

bool CheckSum() {
	// Check we read 40 bits and that the checksum matches.
	if (recv_data[4] == ((recv_data[0] + recv_data[1] + recv_data[2] + recv_data[3]) & 0xFF)) {
		return true;
	}
	return false;
}

int8_t sample_Temperature_Humidity_bytes_using_AM2320(uint8_t* temperature_bytes, uint8_t* relative_humidity_bytes) {
	bool read_ok = AM2320_I2C2_Read_Temp_and_Humidity();
	if (!read_ok) {
		// reset sensor's power source
//		PORTClearBits(IOPORT_A, BIT_4);
//		sleep_for_microseconds(1000);
//		PORTSetBits(IOPORT_A, BIT_4);
		fault_count++; // remember 
		return SENSOR_READ_ERROR;
	}
	if (!CRC_check()) {
		// reset sensor's power source
//		PORTClearBits(IOPORT_A, BIT_4);
//		sleep_for_microseconds(1000);
//		PORTSetBits(IOPORT_A, BIT_4);
		fault_count++; // remember 
		return SENSOR_CRC_ERROR;
	}
	temperature_bytes[0] = recv_data[4];
	temperature_bytes[1] = recv_data[5];
	relative_humidity_bytes[0] = recv_data[2];
	relative_humidity_bytes[1] = recv_data[3];
	return SENSOR_READ_SUCCESSFUL;
}

int8_t sample_Temperature_Humidity_bytes_using_DHT22(uint8_t* temperature_bytes, uint8_t* relative_humidity_bytes) {
	bool read_ok = DHT22_OW_Read_Temp_and_Humidity();
	if (!read_ok) return SENSOR_READ_ERROR;
	if (!CheckSum()) return SENSOR_CheckSum_ERROR;
	temperature_bytes[0] = recv_data[2];
	temperature_bytes[1] = recv_data[3];
	relative_humidity_bytes[0] = recv_data[0];
	relative_humidity_bytes[1] = recv_data[1];
	return SENSOR_READ_SUCCESSFUL;
}

uint8_t ambient_condition_status() {
	float temperature, relative_humidity;
#ifdef TH_AM2320
	temperature = (float) ((recv_data[4] << 8) | recv_data[5]) / 10.0f;
	relative_humidity = (float) ((recv_data[2] << 8) | recv_data[3]) / 10.0f;
#endif
#ifdef TH_DHT22
	temperature = (float) (((recv_data[2] & 0x7F) << 8) | recv_data[3]) / 10.0f;
	relative_humidity = (float) ((recv_data[0] << 8) | recv_data[1]) / 10.0f;
#endif
	//printf("Temperature: %.2f, Humidity: %.2f\n", temperature, relative_humidity);
	// Normal condition, return 0
	if (temperature < MAX_NORMAL_TEMPERATURE && temperature > MIN_NORMAL_TEMPERATURE)
		if (relative_humidity < MAX_NORMAL_RELATIVE_HUMIDITY && relative_humidity > MIN_NORMAL_RELATIVE_HUMIDITY)
			return NORMAL_AMBIENT_CONDITION;
	// This shouldn't be returned, 1
	return ABNORMAL_AMBIENT_CONDITION;
}

