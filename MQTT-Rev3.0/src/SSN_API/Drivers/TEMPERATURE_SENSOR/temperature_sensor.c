#include "temperature_sensor.h"

float TEMPERATURE_READINGS_ARRAY[N_TEMPERATURE_READINGS];
int current_temperature_reading_index = 0;
bool first_N_temperature_readings_acquired = false;

//float NTC_Thermistor_4092_50k_LUT_Resistance[114] = {
//	1956240, 1812199, 1679700, 1557748, 1445439, 1341952, 1246540, 1158525, 1077290, 1001621, 932353, 868317, 809086, 754271, 703517, 656499, 612919, 572506, 534686, 
//	499905, 467604, 437592, 409692, 383745, 359601, 337126, 316194, 296522, 278353, 261408, 245599, 230842, 217062, 204189, 192156, 180906, 170291, 160449, 151235, 
//	142605, 134519, 126941, 119834, 113168, 106912, 100988, 95475, 90296, 85428, 80852, 76547, 72497, 68685, 65095, 61685, 58500, 55499, 52669, 50000, 47481, 45104, 
//	42859, 40739, 38718, 36826, 35037, 33345, 31745, 30230, 28796, 27438, 26152, 24923, 23768, 22674, 21635, 20651, 19716, 18829, 17987, 17187, 16421, 15699, 15013, 
//	14360, 13740, 13150, 12588, 12053, 11544, 11055, 10593, 10154, 9734, 9335, 8954, 8590, 8243, 7912, 7593, 7292, 7004, 6729, 6466, 6215, 5975, 5745, 5526, 5314, 5113, 
//	4921, 4737, 4561, 4392};
//float NTC_Thermistor_4092_50k_LUT_Temperature_Celcius[114] = {
//	-39.44, -38.33, -37.22, -36.11, -35.0, -33.89, -32.78, -31.67, -30.56, -29.44, -28.33, -27.22, -26.11, -25.0, -23.89, -22.78, -21.67, -20.56, -19.44, -18.33, -17.22, 
//	-16.11, -15.0, -13.89, -12.78, -11.67, -10.56, -9.44, -8.33, -7.22, -6.11, -5.0, -3.89, -2.78, -1.67, -0.56, 0.56, 1.67, 2.78, 3.89, 5.0, 6.11, 7.22, 8.33, 9.44, 
//	10.56, 11.67, 12.78, 13.89, 15.0, 16.11, 17.22, 18.33, 19.44, 20.56, 21.67, 22.78, 23.89, 25.0, 26.11, 27.22, 28.33, 29.44, 30.56, 31.67, 32.78, 33.89, 35.0, 36.11, 
//	37.22, 38.33, 39.44, 40.56, 41.67, 42.78, 43.89, 45.0, 46.11, 47.22, 48.33, 49.44, 50.56, 51.67, 52.78, 53.89, 55.0, 56.11, 57.22, 58.33, 59.44, 60.56, 61.67, 62.78,
//	63.89, 65.0, 66.11, 67.22, 68.33, 69.44, 70.56, 71.67, 72.78, 73.89, 75.0, 76.11, 77.22, 78.33, 79.44, 80.56, 81.67, 82.78, 83.89, 85.0, 86.11};

float NTC_Thermistor_4092_50k_LUT_Resistance[THERMISTOR_LUT_SIZE] = {
	4870621.06, 4487453.98, 4137440.56, 3817468.21, 3524738.25, 3256732.21, 3011181.86, 2786042.73, 2579470.54, 2389800.29, 2215527.65, 2055292.43, 1907863.82, 1772127.23, 1647072.59, 1531783.86, 
	1425429.68, 1327255.02, 1236573.64, 1152761.43, 1075250.38, 1003523.17, 937108.35, 875575.97, 818533.67, 765623.17, 716517.08, 670916.08, 628546.3, 589157.02, 552518.6, 518420.5, 486669.68, 
	457088.95, 429515.62, 403800.22, 379805.34, 357404.59, 336481.64, 316929.38, 298649.11, 281549.84, 265547.68, 250565.16, 236530.8, 223378.52, 211047.27, 199480.6, 188626.25, 178435.88, 
	168864.68, 159871.17, 151416.86, 143466.08, 135985.7, 128945.0, 122315.41, 116070.4, 110185.31, 104637.2, 99404.72, 94467.99, 89808.52, 85409.06, 81253.54, 77326.97, 73615.36, 70105.67, 
	66785.72, 63644.12, 60670.25, 57854.15, 55186.54, 52658.71, 50262.53, 47990.38, 45835.12, 43790.05, 41848.91, 40005.83, 38255.28, 36592.1, 35011.42, 33508.69, 32079.63, 30720.19, 29426.61, 
	28195.32, 27022.97, 25906.42, 24842.69, 23829.0, 22862.72, 21941.37, 21062.61, 20224.24, 19424.18, 18660.48, 17931.29, 17234.85, 16569.52, 15933.75, 15326.06, 14745.06, 14189.44, 13657.95, 
	13149.42, 12662.74, 12196.85, 11750.74, 11323.49, 10914.2, 10522.0, 10146.11, 9785.76, 9440.23, 9108.83, 8790.91, 8485.85, 8193.07, 7912.01, 7642.14, 7382.96, 7134.0, 6894.79, 6664.91, 6443.94,
	6231.5, 6027.22, 5830.73, 5641.71, 5459.83, 5284.79, 5116.3, 4954.08, 4797.87, 4647.41, 4502.47, 4362.81, 4228.22, 4098.49, 3973.42, 3852.82, 3736.51, 3624.31, 3516.07, 3411.62, 3310.81, 3213.5,
	3119.55, 3028.83, 2941.21, 2856.58, 2774.8, 2695.79, 2619.42, 2545.61, 2474.25, 2405.24, 2338.51, 2273.96, 2211.52, 2151.1, 2092.63, 2036.04, 1981.26, 1928.23, 1876.88, 1827.14, 1778.97, 1732.3,
	1687.09, 1643.27, 1600.81, 1559.65, 1519.74, 1481.05, 1443.53, 1407.14, 1371.84, 1337.6, 1304.37, 1272.13, 1240.84, 1210.46, 1180.98, 1152.35, 1124.54, 1097.54, 1071.32, 1045.84, 1021.09, 997.04,
	973.66, 950.94, 928.86, 907.39, 886.52, 866.22, 846.48
};

float NTC_Thermistor_4092_50k_LUT_Temperature_Celcius[THERMISTOR_LUT_SIZE] = {
	-50, -49, -48, -47, -46, -45, -44, -43, -42, -41, -40, -39, -38, -37, -36, -35, -34, -33, -32, -31, -30, -29, -28, -27, -26, -25, -24, -23, -22, -21, -20, -19, -18, -17, -16, -15, -14, -13, -12,
	-11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 
	89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 
	130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149
};


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

/* MLX90614 Functions */

void setup_IR_Temperature_Sensor_And_Laser() {
#ifdef OTS_LS_MLX90614
	open_I2C2();
	PORTSetPinsDigitalOut(IOPORT_A, BIT_4);
	PORTSetBits(IOPORT_A, BIT_4);
	PORTSetPinsDigitalOut(IOPORT_B, BIT_10);
	PORTSetBits(IOPORT_B, BIT_10);
#endif
}

bool read16(uint8_t this_addr) {
//	uint16_t ret;
//	Wire.beginTransmission(_addr); // start transmission to device
//	Wire.write(a); // sends register address to read from
//	Wire.endTransmission(false); // end transmission
//	Wire.requestFrom(_addr, (size_t) 3); // send data n-bytes read
//	ret = Wire.read(); // receive DATA
//	ret |= Wire.read() << 8; // receive DATA
//	uint8_t pec = Wire.read();
//	return ret;
	/* First Phase */
	// initiate start condition and wait
	if (!I2C2_transmit_start_bit()) return 0;
	if (!I2C2_wait_while_busy()) return 0;
	// Address the temperature sensor and wait
	I2C2_transmit_byte((MLX90614_I2CADDR | 0));
	if (!I2C2_wait_while_busy()) return 0;
	// Address the register we want to read from and wait
	I2C2_transmit_byte((this_addr));
	if (!I2C2_wait_while_busy()) return 0;
	// stop transmission and wait
//	if (!I2C2_transmit_stop_bit()) return 0;
//	if (!I2C2_wait_while_busy()) return 0;
	/* Second Phase */
	// initiate start condition
	if (!I2C2_transmit_restart_bit()) return 0;
	if (!I2C2_wait_while_busy()) return 0;
	// Address the temperature sensor again and wait
	I2C2_transmit_byte((MLX90614_I2CADDR | 1));
	if (!I2C2_wait_while_busy()) return 0;
	// Check if we have received an I2C byte
	if (!I2C2_is_byte_received()) return 0;
	MLX90614_data_bytes[0] = (0xFF & I2C2RCV);
	MLX90614_data = (0xFF & I2C2RCV);
	if (!I2C2_wait_while_busy()) return 0;
//	printf("First Byte: %d\n", data);
	if (!I2C2_ack()) return 0;
	if (!I2C2_wait_while_busy()) return 0;
	// Check if we have received an I2C byte
	if (!I2C2_is_byte_received()) return 0;
	MLX90614_data_bytes[1] = (0x7F & I2C2RCV);
	MLX90614_data |= ((0x7F & I2C2RCV) << 8);
	if (!I2C2_wait_while_busy()) return 0;
//	printf("Second Byte: %d\n", (0xFF & I2C2RCV));
	if (!I2C2_ack()) return 0;
	if (!I2C2_wait_while_busy()) return 0;
	// Check if we have received an I2C byte
	if (!I2C2_is_byte_received()) return 0;
	MLX90614_pec |= (0xFF & I2C2RCV);
	if (!I2C2_wait_while_busy()) return 0;
	if (!I2C2_ack()) return 0;
	if (!I2C2_wait_while_busy()) return 0;
	// stop transmission and wait
	if (!I2C2_transmit_stop_bit()) return 0;
	if (!I2C2_wait_while_busy()) return 0;
	return 1;
}

float readTemp(uint8_t reg) {
	read16(reg);
	return (float)MLX90614_data*0.02-273.15;
}

/**
 * @brief Get the current temperature of an object in degrees Farenheit
 *
 * @return double The temperature in degrees Farenheit
 */
float MLX90614_Read_Temperature_Object_Fahrenheit(void) {
	float celcius_reading = MLX90614_Read_Temperature_Object_Celcius();
	// The celcius_reading value can only be between -70 and +380, 
	// therefore a lesser or greater value means communication error
	if (celcius_reading > MLX90614_MIN_CELCIUS_LIMIT && celcius_reading < MLX90614_MAX_CELCIUS_LIMIT) {
		return (celcius_reading * 9 / 5) + 32;
	}
	return MLX90614_COMM_ERROR_CODE;
}

/**
 * @brief Get the current ambient temperature in degrees Farenheit
 *
 * @return double The temperature in degrees Farenheit
 */
float MLX90614_Read_Temperature_Ambient_Fahrenheit(void) {
	float celcius_reading = MLX90614_Read_Temperature_Ambient_Celcius();
	// The celcius_reading value can only be between -70 and +380, 
	// therefore a lesser or greater value means communication error
	if (celcius_reading > MLX90614_MIN_CELCIUS_LIMIT && celcius_reading < MLX90614_MAX_CELCIUS_LIMIT) {
		return (celcius_reading * 9 / 5) + 32;
	}
	return MLX90614_COMM_ERROR_CODE;
}

/**
 * @brief Get the current temperature of an object in degrees Celcius
 *
 * @return double The temperature in degrees Celcius
 */
float MLX90614_Read_Temperature_Object_Celcius(void) {
	float celcius_reading = readTemp(MLX90614_TOBJ1);
	// The celcius_reading value can only be between -70 and +380, 
	// therefore a lesser or greater value means communication error
	if (celcius_reading > MLX90614_MIN_CELCIUS_LIMIT && celcius_reading < MLX90614_MAX_CELCIUS_LIMIT) {
		return celcius_reading;
	}
	return MLX90614_COMM_ERROR_CODE;
}

/**
 * @brief Get the current ambient temperature in degrees Celcius
 *
 * @return double The temperature in degrees Celcius
 */
float MLX90614_Read_Temperature_Ambient_Celcius(void) {
	float celcius_reading = readTemp(MLX90614_TA);
	// The celcius_reading value can only be between -70 and +380, 
	// therefore a lesser or greater value means communication error
	if (celcius_reading > MLX90614_MIN_CELCIUS_LIMIT && celcius_reading < MLX90614_MAX_CELCIUS_LIMIT) {
		return celcius_reading;
	}
	return MLX90614_COMM_ERROR_CODE;
}

/* MLX90614 Functions */

/* NTC Thermistor 4092 50Kohm */
float Thermistor_NTC_4092_50k_Get_Object_Temperature_In_Celcius() {
	float reference_resistance = 3300, VDD = 3.3;
	float reference_voltage = 3.3 * (float)sample_Current_Sensor_channel(0) / 1024;
    float reference_current = reference_voltage / reference_resistance;
	float thermistor_voltage = VDD - reference_voltage;
	float thermistor_resistance = thermistor_voltage / reference_current; // they are both in series
	// printf("Thermistor Resistance: %.2f\n", thermistor_resistance);
	// binary search the Thermistor LUT to find the appropriate corresponding temperature for this value of resistance
	uint8_t first = 0, last = THERMISTOR_LUT_SIZE - 1, mid;
	bool found = false;
	// first check if this resistance value is between LUT Min and Max resistances that we already know
	// because if we don't check this, our binary search will fail
	// if resistance is out of range, just return some out of range temperature, say 200*C
	if (thermistor_resistance > NTC_Thermistor_4092_50k_LUT_Resistance[first] || thermistor_resistance < NTC_Thermistor_4092_50k_LUT_Resistance[last]) {
		return THERMISTOR_LUT_SIZE;
	}
	// printf("Binary Search Begin...\n");
	while(first <= last && !found) {
		mid = (first+last)/2;
		if (NTC_Thermistor_4092_50k_LUT_Resistance[mid] == thermistor_resistance) {
			found = true;
		} else if (NTC_Thermistor_4092_50k_LUT_Resistance[mid] > thermistor_resistance) {
			first = mid + 1;
		} else {
			last = mid - 1;
		}
	}
	// printf("Binary Search End...\n");
	if (found) {
//		printf("\nExact Resistance Value found in LUT!\n");
//		printf("Thermistor Resistance: %f\n", thermistor_resistance);	
//		printf("LUT Resistance: %f\n", NTC_Thermistor_4092_50k_LUT_Resistance[mid]);	
//		printf("LUT Temperature: %f\n", NTC_Thermistor_4092_50k_LUT_Temperature_Celcius[mid]);
		return NTC_Thermistor_4092_50k_LUT_Temperature_Celcius[mid];
	} else {
		// interpolation step here, we approximate a straight line and calculate slope using two points
		// first and last are the key indices
		float delta_c = NTC_Thermistor_4092_50k_LUT_Temperature_Celcius[last] - NTC_Thermistor_4092_50k_LUT_Temperature_Celcius[first];
		float delta_r = NTC_Thermistor_4092_50k_LUT_Resistance[last] - NTC_Thermistor_4092_50k_LUT_Resistance[first];
		float m = delta_c/delta_r;
		float calculated_temperature_in_celcius = m * (thermistor_resistance - NTC_Thermistor_4092_50k_LUT_Resistance[first]) + NTC_Thermistor_4092_50k_LUT_Temperature_Celcius[first];
//		printf("\nNo Resistance Value found in LUT. Interpolating to Approximate Temperature Value...\n");
//		printf("\nThermistor Resistance: %f\n", thermistor_resistance);	
//		printf("Reference Resistance - 1: %f\n", NTC_Thermistor_4092_50k_LUT_Resistance[first]);
//		printf("Reference Temperature - 1: %f\n", NTC_Thermistor_4092_50k_LUT_Temperature_Celcius[first]);
//		printf("Reference Resistance - 2: %f\n", NTC_Thermistor_4092_50k_LUT_Resistance[last]);
//		printf("Reference Temperature - 2: %f\n", NTC_Thermistor_4092_50k_LUT_Temperature_Celcius[last]);
//		printf("Interpolated Temperature: %f\n", calculated_temperature_in_celcius);
		return calculated_temperature_in_celcius;
	}
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

uint8_t ambient_condition_status(uint8_t TEMPERATURE_MIN_THRESHOLD, uint8_t TEMPERATURE_MAX_THRESHOLD, uint8_t RELATIVE_HUMIDITY_MIN_THRESHOLD, uint8_t RELATIVE_HUMIDITY_MAX_THRESHOLD) {
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
	if (temperature < TEMPERATURE_MAX_THRESHOLD && temperature > TEMPERATURE_MIN_THRESHOLD)
		if (relative_humidity < RELATIVE_HUMIDITY_MAX_THRESHOLD && relative_humidity > RELATIVE_HUMIDITY_MIN_THRESHOLD)
			return NORMAL_AMBIENT_CONDITION;
	// This shouldn't be returned, 1
	return ABNORMAL_AMBIENT_CONDITION;
}

float average_value_of_temperature(float current_temperature) {
    TEMPERATURE_READINGS_ARRAY[current_temperature_reading_index++] = current_temperature;
    if (current_temperature_reading_index >= N_TEMPERATURE_READINGS) {
        current_temperature_reading_index = 0;
        first_N_temperature_readings_acquired = true;
    } else {
        if (!first_N_temperature_readings_acquired) {
            printf("[LOG] Returning raw temperature reading. Will average after %d values have been acquired\n", N_TEMPERATURE_READINGS);
            return current_temperature;
        }
    }
    float sum = 0;
    uint8_t k; for (k=0; k<N_TEMPERATURE_READINGS; k++) {
        sum += TEMPERATURE_READINGS_ARRAY[k];
    }
    printf("[LOG] Returning average temperature reading\n");
    return sum/N_TEMPERATURE_READINGS;
}