#include "current_sensor.h"

void open_ADC() {
	// configure and enable the ADC
	CloseADC10(); // ensure the ADC is off before setting the configuration

	// define setup parameters for OpenADC10
	// Turn module on | output in integer | trigger mode auto | enable  autosample
#define PARAM1  ADC_MODULE_ON | ADC_FORMAT_INTG | ADC_CLK_AUTO | ADC_AUTO_SAMPLING_ON

	// define setup parameters for OpenADC10
	// ADC ref external    | disable offset test    | enable scan mode | perform 2 samples | use one buffer | use MUXA mode
	// note: to read X number of pins you must set ADC_SAMPLES_PER_INT_X
#define PARAM2  ADC_VREF_AVDD_AVSS | ADC_OFFSET_CAL_DISABLE | ADC_SCAN_ON | ADC_SAMPLES_PER_INT_4 | ADC_ALT_BUF_OFF | ADC_ALT_INPUT_OFF

	// define setup parameters for OpenADC10
	// 				  use ADC internal clock | set sample time
#define PARAM3  ADC_CONV_CLK_INTERNAL_RC | ADC_SAMPLE_TIME_15

	// define setup parameters for OpenADC10
	// set AN4 and AN5
#define PARAM4	ENABLE_AN0_ANA | ENABLE_AN1_ANA | ENABLE_AN2_ANA | ENABLE_AN3_ANA

	// define setup parameters for OpenADC10
	// do not assign channels to scan
#define PARAM5  SKIP_SCAN_AN4 | SKIP_SCAN_AN5 | SKIP_SCAN_AN6 | SKIP_SCAN_AN7 | SKIP_SCAN_AN8 | SKIP_SCAN_AN9 | SKIP_SCAN_AN10 | SKIP_SCAN_AN11 | SKIP_SCAN_AN12 | SKIP_SCAN_AN13 | SKIP_SCAN_AN14 | SKIP_SCAN_AN15

	// use ground as neg ref for A 
	SetChanADC10(ADC_CH0_NEG_SAMPLEA_NVREF); // use ground as the negative reference
	OpenADC10(PARAM1, PARAM2, PARAM3, PARAM4, PARAM5); // configure ADC using parameter define above

	// Enable the ADC
	EnableADC10();

	// wait for the first conversion to complete so there will be valid data in ADC result registers
	while (!mAD1GetIntFlag());

	//    ANSELAbits.ANSA0 = 1;   // set RA0 (AN1) to analog
	//    ANSELAbits.ANSA1 = 1;   // set RA1 (AN2) to analog
	//    ANSELBbits.ANSB0 = 1;   // set RB0 (AN4) to analog
	//    ANSELBbits.ANSB1 = 1;   // set RB1 (AN3) to analog
	//    TRISAbits.TRISA0 = 1;   // set RA0 as an input
	//    TRISAbits.TRISA1 = 1;   // set RA1 as an input
	//    TRISBbits.TRISB0 = 1;   // set RB0 as an input
	//    TRISBbits.TRISB1 = 1;   // set RB1 as an input
}

void setup_Current_Sensors() {
	open_ADC();
}

uint16_t sample_Current_Sensor_channel(uint8_t channel) {
	// will return raw value
	switch (channel) {
		case 0:
			return ADC1BUF0;
			break;
		case 1:
			return ADC1BUF1;
			break;
		case 2:
			return ADC1BUF2;
			break;
		case 3:
			return ADC1BUF3;
			break;
		default:
			return 2020; // wrong channel; return a value larger than 1024
			break;
	}
}

unsigned char CurrentSensor_Read_RMS(uint8_t channel, uint16_t num_samples, uint8_t SENSOR_RATING, float SENSOR_TYPE_SCALAR) {

	uint32_t count = 0, ADC_raw_sample = 0, max_ADC_raw_sample = 0, ADC_raw_non_zero_sum = 0, ADC_raw_non_zero_count = 0;
	uint32_t MAX_SAMPLE_BASED_CURRENT_RMS_value = 0, AVERAGE_SAMPLE_BASED_CURRENT_RMS_value = 0, CURRENT_RMS_VALUE = 0;
	char single_byte_current_RMS_value;

	while (count < num_samples) {
		ADC_raw_sample = sample_Current_Sensor_channel(channel);
		// record the maximum value in this sample space for MAX Sample based RMS calculation
		if (ADC_raw_sample > max_ADC_raw_sample)
			max_ADC_raw_sample = ADC_raw_sample;
		// record every non-zero value in this sample space for AVERAGE Sample based RMS calculation
		if (ADC_raw_sample > 0) {
			ADC_raw_non_zero_sum += ADC_raw_sample;
			ADC_raw_non_zero_count++;
		}
		count++;
		// pick 200 samples per wave cycle of AC Sine Wave @ 50Hz => 100us sampling period
		sleep_for_microseconds(100);
	}

	MAX_SAMPLE_BASED_CURRENT_RMS_value = (SENSOR_RATING / 724.07) * SENSOR_TYPE_SCALAR * (0.707 * (float) max_ADC_raw_sample);
	AVERAGE_SAMPLE_BASED_CURRENT_RMS_value = (SENSOR_RATING / 718.89) * SENSOR_TYPE_SCALAR * (1.1 * (float) ADC_raw_non_zero_sum / ADC_raw_non_zero_count);
	CURRENT_RMS_VALUE = (float) (MAX_SAMPLE_BASED_CURRENT_RMS_value + AVERAGE_SAMPLE_BASED_CURRENT_RMS_value) / 2;
	single_byte_current_RMS_value = (unsigned char) CURRENT_RMS_VALUE;
	return single_byte_current_RMS_value;
}

void Calculate_RMS_Current_On_All_Channels(uint8_t* SENSOR_RATINGS, uint16_t num_samples, float* RMS_CURRENTS) {
	//    printf("Calculating...\n");
	uint32_t count = 0, ADC_raw_samples[NO_OF_MACHINES] = {0}, max_ADC_raw_sample[NO_OF_MACHINES] = {0}, ADC_raw_non_zero_sum[NO_OF_MACHINES] = {0}, ADC_raw_non_zero_count[NO_OF_MACHINES] = {0};
	float MAX_SAMPLE_BASED_CURRENT_RMS_value[NO_OF_MACHINES] = {0}, AVERAGE_SAMPLE_BASED_CURRENT_RMS_value[NO_OF_MACHINES] = {0}, CURRENT_RMS_VALUE[NO_OF_MACHINES] = {0};
	float SENSOR_TYPE_SCALAR;
	uint8_t i;
	while (count < num_samples) {
		for (i = 0; i < NO_OF_MACHINES; i++) {
			// Sample one value from ith channel
			ADC_raw_samples[i] = sample_Current_Sensor_channel(i);
			// record the maximum value in this sample space for MAX Sample based RMS calculation for ith channel
			if (ADC_raw_samples[i] > max_ADC_raw_sample[i]) {
				max_ADC_raw_sample[i] = ADC_raw_samples[i];
				//                printf("%d\n", max_ADC_raw_sample[i]);
			}
			// record every non-zero value in this sample space for AVERAGE Sample based RMS calculation for ith channel
			if (ADC_raw_samples[i] > 0) {
				ADC_raw_non_zero_sum[i] += ADC_raw_samples[i];
				ADC_raw_non_zero_count[i]++;
			}
		}
		count++;
		// pick 200 samples per wave cycle of AC Sine Wave @ 50Hz => 100us sampling period
		// 100us * 400 = 40ms => 2 Time periods of a 50Hz Sine Wave, so we capture two waves with 200 samples in each
		sleep_for_microseconds(100);
	}
	/* Calculate the RMS Current Values using two methods and average them */
	for (i = 0; i < NO_OF_MACHINES; i++) {
		// printf("Current-%d: %d %d %d\n", i+1, max_ADC_raw_sample[i], ADC_raw_non_zero_sum[i], ADC_raw_non_zero_count[i]);
		// RMS = 1.1 Average Value && RMS = 0.707 * Max Value
		float measured_maximum_voltage = 3.3 * (float) max_ADC_raw_sample[i] / 1024;
		float measured_average_voltage;
		if (ADC_raw_non_zero_count[i] != 0) {
			measured_average_voltage = 3.3 * ((float) ADC_raw_non_zero_sum[i] / ADC_raw_non_zero_count[i]) / 1024;
		} else {
			measured_average_voltage = measured_maximum_voltage * 0.707 / 1.1;
		}
		if (SENSOR_RATINGS[i] == 100) {
			// this is a current output current sensor, will output 1.65Vrms = 2.33 Vmax at any rating
			MAX_SAMPLE_BASED_CURRENT_RMS_value[i] = (float) SENSOR_RATINGS[i] * measured_maximum_voltage / 2.33;
			AVERAGE_SAMPLE_BASED_CURRENT_RMS_value[i] = 1.1 * (float) SENSOR_RATINGS[i] * measured_average_voltage / 1.5;
		} else {
			// this is a voltage output current sensor, can't output more than 1Vrms = 1.414Vmax
			MAX_SAMPLE_BASED_CURRENT_RMS_value[i] = (float) SENSOR_RATINGS[i] * measured_maximum_voltage / 1.414;
			AVERAGE_SAMPLE_BASED_CURRENT_RMS_value[i] = 1.1 * (float) SENSOR_RATINGS[i] * measured_average_voltage / 0.909;
		}
		CURRENT_RMS_VALUE[i] = MAX_SAMPLE_BASED_CURRENT_RMS_value[i]; // ((float)MAX_SAMPLE_BASED_CURRENT_RMS_value[i]+(float)AVERAGE_SAMPLE_BASED_CURRENT_RMS_value[i])/2;
		RMS_CURRENTS[i] = CURRENT_RMS_VALUE[i];
		//printf("%d >> %.2f %.2f %d\n", i+1, measured_maximum_voltage, measured_average_voltage, single_byte_RMS_CURRENTS[i]);
	}
}

void Calculate_True_RMS_Current_On_All_Channels(uint8_t* SENSOR_RATINGS, float* SSN_CURRENT_SENSOR_VOLTAGE_SCALARS, uint16_t num_samples, float* RMS_CURRENTS) {
	// sensor_relative_voltage means we are looking at the max output voltage of our sensors
	// e.g voltage-output 30Amp sensor would give 1Vrms -> 30Arms
	// and current-output 100Amp sensor would give max 1.65Vrms -> 100Arms
	float sensor_relative_voltage[NO_OF_MACHINES] = {0}, non_zero_voltage_squared_running_sum[NO_OF_MACHINES] = {0}, non_zero_voltage_count[NO_OF_MACHINES] = {0}, CURRENT_RMS_VALUE[NO_OF_MACHINES] = {0};
	float sensor_relative_scalar;
	uint32_t i, count = 0;
	while (count < num_samples) {
		for (i = 0; i < NO_OF_MACHINES; i++) {
			sensor_relative_scalar = SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[i];
			// Sample one value from i-th channel
			uint16_t adc_raw_sample = sample_Current_Sensor_channel(i);
			// pick 200 samples per wave cycle of AC Sine Wave @ 50Hz => 100us sampling period
			// but we found out that having 0 and non-0 samples have different time periods. so they are adjusted separately
			if (adc_raw_sample == 0) {
				sleep_for_microseconds(93);
			} else {
				sensor_relative_voltage[i] = (3.3 * (float) adc_raw_sample / 1024) / sensor_relative_scalar;
				non_zero_voltage_squared_running_sum[i] += sensor_relative_voltage[i] * sensor_relative_voltage[i];
				non_zero_voltage_count[i]++;
				sleep_for_microseconds(84);
			}
		}
		count++;
	}
	/* Calculate the True RMS Current Values on all 4 channels */
	for (i = 0; i < NO_OF_MACHINES; i++) {
		if (non_zero_voltage_count[i] < 1) {
			CURRENT_RMS_VALUE[i] = 0;
		} else {
			CURRENT_RMS_VALUE[i] = (float) SENSOR_RATINGS[i] * sqrt(non_zero_voltage_squared_running_sum[i] / non_zero_voltage_count[i]);
			// empirical error compensation => error = 0.01444*CURRENT_RMS_VALUE[i] + 0.055555;
			CURRENT_RMS_VALUE[i] += 0.01444 * CURRENT_RMS_VALUE[i] + 0.055555; //linear compensation on pure sine wave using function generator
		}
		// fill the new current value into the RMS buffer for RMS Averaging
		RMS_buffer[i * n_for_rms_averaging + rms_averaging_sample_count] = CURRENT_RMS_VALUE[i];
		// fill the new current value into the long RMS buffer for lagging state assignment
		RMS_long_buffer[i * n_for_rms_status_assignment + rms_status_assignment_sample_count] = CURRENT_RMS_VALUE[i];
		if (i == NO_OF_MACHINES - 1) {
			// increment counters only at the last machine
			rms_averaging_sample_count++;
			rms_status_assignment_sample_count++;
		}
		if (rms_averaging_sample_count >= n_for_rms_averaging) {
			rms_averaging_sample_count = 0;
		}
		if (rms_status_assignment_sample_count >= n_for_rms_status_assignment) {
			rms_status_assignment_sample_count = 0;
		}
		// calculate the average RMS current value over the buffered RMS values
		float buffer_sum = 0;
		int j = 0; for (j = 0; j < n_for_rms_averaging; j++) {
			buffer_sum += RMS_buffer[i * n_for_rms_averaging + j];
		}
		RMS_CURRENTS[i] = buffer_sum / n_for_rms_averaging;
	}
}

float Current_VSensor_Read_RMS(uint8_t channel, uint16_t* adc_samples_array, uint16_t num_samples, uint8_t sensor_max_value) {
	uint32_t max_ADC_raw_sample = 0, ADC_raw_sample = 0, ADC_raw_RMS_value = 0, count = 0;
	char single_byte_raw_RMS_value;
	while (count < num_samples) {
		ADC_raw_sample = sample_Current_Sensor_channel(channel);
		adc_samples_array[count] = ADC_raw_sample;
		if (ADC_raw_sample > max_ADC_raw_sample)
			max_ADC_raw_sample = ADC_raw_sample;
		count++;
		sleep_for_microseconds(100);
	}
	// max ADC value * 0.707 = RMS value
	ADC_raw_RMS_value = sensor_max_value * 0.707 * (3.3 * (float) max_ADC_raw_sample / 1024);
	single_byte_raw_RMS_value = (unsigned char) ADC_raw_RMS_value;
	return single_byte_raw_RMS_value;
}

float Current_CSensor_Read_RMS(uint8_t channel, uint16_t* adc_samples_array, uint16_t num_samples, uint8_t sensor_max_value) {
	uint32_t max_ADC_raw_sample = 0, ADC_raw_sample = 0, ADC_raw_RMS_value = 0, count = 0;
	char single_byte_raw_RMS_value;
	while (count < num_samples) {
		ADC_raw_sample = sample_Current_Sensor_channel(channel);
		adc_samples_array[count] = ADC_raw_sample;
		if (ADC_raw_sample > max_ADC_raw_sample)
			max_ADC_raw_sample = ADC_raw_sample;
		sleep_for_microseconds(100);
		count++;
	}
	// max ADC value * 0.707 = RMS value
	ADC_raw_RMS_value = sensor_max_value * 0.707 * (1.65 * (float) max_ADC_raw_sample / 1024);
	single_byte_raw_RMS_value = (unsigned char) ADC_raw_RMS_value;
	return single_byte_raw_RMS_value;
}

bool Get_Machines_Status_Update(uint8_t* SSN_CURRENT_SENSOR_RATINGS, float* SSN_CURRENT_SENSOR_VOLTAGE_SCALARS, float* SSN_CURRENT_SENSOR_THRESHOLDS, uint8_t* SSN_CURRENT_SENSOR_MAXLOADS,
	float* Machine_load_currents, uint8_t* Machine_load_percentages, uint8_t* Machine_status, uint8_t* Machine_prev_status, uint8_t* Machine_status_flag, uint32_t* Machine_status_duration,
	uint32_t* Machine_status_timestamp) {
	// This function will calculate the load currents, load percentages and machine on/off status for all four machines
	// It will also calculate the time-in-state in SSN-Seconds and assign a timestamp to the state as well
	// Off(0)/Idle(1)/On(2)
	// All Sensor Ratings in SSN_CONFIG are at: 1, 4, 7, 10             =>  SSN_CONFIG[3*i+1]
	// All Sensor Threshold Currents in SSN_CONFIG are at: 2, 5, 8, 11  =>  SSN_CONFIG[3*i+2]
	// All Sensor Max Load Currents in SSN_CONFIG are at: 3, 6, 9, 12   =>  SSN_CONFIG[3*i+3]
	bool status_change_flag = false;
	/* Sample all channels and record their respective RMS currents before proceeding */
	Calculate_True_RMS_Current_On_All_Channels(SSN_CURRENT_SENSOR_RATINGS, SSN_CURRENT_SENSOR_VOLTAGE_SCALARS, 150, Machine_load_currents);
	/* Decide the states of these machines based on current values and assign them timestamps */
	uint8_t this_machine_rating, this_machine_maxload;
	float this_machine_threshold;
	int i;
	for (i = 0; i < NO_OF_MACHINES; i++) {
		// Round-off machine currents to 2-decimal places
		Machine_load_currents[i] = round_float_to_2_decimal_place(Machine_load_currents[i]);
		/* Get the parameters from the Configurations */
		this_machine_rating = SSN_CURRENT_SENSOR_RATINGS[i];
		this_machine_threshold = SSN_CURRENT_SENSOR_THRESHOLDS[i]; // the thresholds 
		this_machine_maxload = SSN_CURRENT_SENSOR_MAXLOADS[i];
		// printf("Machine-%d: %d %d %d\n", i, this_machine_rating, this_machine_threshold, this_machine_maxload);
		// if the sensor is rated 0, it simply means no sensor attached, so everything is 0
		if (this_machine_rating == 0) {
			// load current is 0
			Machine_load_currents[i] = 0;
			// load percentage is 0
			Machine_load_percentages[i] = 0;
			// Machine Status is "OFF"
			Machine_status[i] = MACHINE_OFF;
			// Machine is off, so no duration
			Machine_status_duration[i] = 0;
			// assign a completely zero timestamp
			Machine_status_timestamp[i] = 0;
			// no need to proceed from here
			continue;
		}
		if (Machine_status[i] == SENSOR_NOT_CONNECTED) {
			Machine_prev_status[i] = SENSOR_NOT_CONNECTED;
			Machine_status[i] = MACHINE_OFF;
			// assign the current SSN Clock timestamp
			Machine_status_timestamp[i] = ssn_dynamic_clock;
			MACHINES_STATE_TIME_DURATION_UPON_STATE_CHANGE[i] = 0; // because it was just turned on
			Machine_status_duration[i] = 0; // because it just its state
			*Machine_status_flag = (*Machine_status_flag) | (1 << i); // status flag assignment at the right bit location
			status_change_flag = true; // set the flag to true
			continue;
		}

		// Calculate the load percentage on the machine based on the maximum rated load and load current
		Machine_load_percentages[i] = (unsigned char) (100 * Machine_load_currents[i] / this_machine_maxload);
		// Assign Machine Status based on RMS Load Current and threshold current for this machine
		// Also check previous state and decide how to update the machine status duration and timestamp
		Machine_prev_status[i] = Machine_status[i];
		// Lagging State Assignment, happens only if we see consistent pattern of a state
		// will remain persistent on noisy signals
		int8_t updated_status = Get_Machine_Status(i, this_machine_threshold, Machine_prev_status[i]);
		if (updated_status != -1) {
			Machine_status[i] = updated_status;
		}
		/* Has the machine status changed just now? */
		if (Machine_status[i] != Machine_prev_status[i]) {
			// assign the current SSN Clock timestamp
			Machine_status_timestamp[i] = ssn_dynamic_clock; // update the timestamp
			MACHINES_STATE_TIME_DURATION_UPON_STATE_CHANGE[i] = Machine_status_duration[i]; // save the max duration for which the machine remained in the previous state
			Machine_status_duration[i] = 0; // because it just its state
			*Machine_status_flag = (*Machine_status_flag) | (1 << i); // status flag assignment at the right bit location
			//printf("Machine Status Flag: %d\n", *Machine_status_flag);
			status_change_flag = true; // set the flag to true
		} else {
			/* Else the machine is sustaining its state */
			// printf(">>>>>>>>>>>>>>>>>> States %d %d\n", Machine_status[i], this_machine_prev_status);
			MACHINES_STATE_TIME_DURATION_UPON_STATE_CHANGE[i] = Machine_status_duration[i]; // save the max duration before updating it
			Machine_status_duration[i] = ssn_dynamic_clock - Machine_status_timestamp[i];
		}
	}
	return status_change_flag;
}

int8_t Get_Machine_Status(uint8_t machine_number, float idle_threshold, uint8_t prev_state) {
	uint8_t j, OFF_count = 0, IDLE_count = 0, ON_count = 0;
	for (j = 0; j < n_for_rms_status_assignment; j++) {
		if (RMS_long_buffer[machine_number * n_for_rms_status_assignment + j] >= 0 && RMS_long_buffer[machine_number * n_for_rms_status_assignment + j] <= off_current_threshold) {
			OFF_count++;
		}
		else if (RMS_long_buffer[machine_number * n_for_rms_status_assignment + j] > off_current_threshold && RMS_long_buffer[machine_number * n_for_rms_status_assignment + j] <= idle_threshold) {
			IDLE_count++;
		} else {
			ON_count++;
		}
	}
	if (OFF_count >= state_change_criteria) {
		return MACHINE_OFF;
	} else if (IDLE_count >= state_change_criteria) {
		return MACHINE_IDLE;
	} else if (ON_count >= state_change_criteria) {
		return MACHINE_ON;
	} else {
		if (prev_state == MACHINE_ON && OFF_count > ON_count && IDLE_count > ON_count) {
			return MACHINE_OFF;
		} else if (prev_state == MACHINE_OFF && ON_count > OFF_count && IDLE_count > OFF_count) {
			return MACHINE_ON;
		}
		return -1; // this indicates that we don't need to change our state
	}
}

void Clear_Machine_Status_flag(uint8_t* Machine_status_flag) {
	*Machine_status_flag = 0x00;
}

