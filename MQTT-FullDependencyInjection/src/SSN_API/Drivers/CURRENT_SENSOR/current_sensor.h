#ifndef __current_sensor_h__
#define __current_sensor_h__

#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

#include "../../global.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <plib.h>

/** Number of samples from ADC channel to calculate the Root Mean Square Current */
#define NUM_OF_ADC_SAMPLES_FOR_IRMS             400
/** An implementation specific scalar value for voltage output current sensors */
#define VOLTAGE_OUTPUT_CURRENT_SENSOR_SCALAR    1
/** An implementation specific scalar value for current output current sensors */
#define CURRENT_OUTPUT_CURRENT_SENSOR_SCALAR    2

/** Enumeration for listing the possible machine states */
enum Machine_Status {MACHINE_OFF=0, MACHINE_IDLE, MACHINE_ON, SENSOR_NOT_CONNECTED};

/** Implementation specific machine status time markers for keeping state timestamps */
extern uint32_t MACHINES_STATE_TIME_MARKERS[NO_OF_MACHINES];
/** Implementation specific machine status durations for keeping edge-case state timestamps */
extern uint32_t MACHINES_STATE_TIME_DURATION_UPON_STATE_CHANGE[NO_OF_MACHINES];

/** Machine load RMS averages */
#define n_for_rms_averaging             8
#define n_for_rms_status_assignment     50
#define state_change_criteria           (int)(0.9*n_for_rms_status_assignment)
#define off_current_threshold           0.5

extern uint8_t rms_averaging_sample_count, rms_status_assignment_sample_count;
extern float RMS_buffer[NO_OF_MACHINES*n_for_rms_averaging];
extern float RMS_long_buffer[NO_OF_MACHINES*n_for_rms_status_assignment];

/** Setup up the ADC peripheral */
void open_ADC();

/** 
 * Sets up Current Sensors for SSN
 */
void setup_Current_Sensors(); 

/** 
 * Samples specific ADC channel
 * @param channel Channel number to sample; expected 1-4
 * @return 16-bit ADC sample value
 */
uint16_t sample_Current_Sensor_channel(uint8_t channel);

/** 
 * Calculates RMS value of current on one ADC channel
 * @param channel Channel number to sample; expected 1-4
 * @param num_samples Number of samples to read for calculating RMS value
 * @param SENSOR_RATING Sensor rating to consider for calculating RMS value
 * @param SENSOR_TYPE_SCALAR Sensor type scalar varies with type of sensor, either voltage output or current output
 * @return Single byte integer RMS value of current at this channel
 */
unsigned char CurrentSensor_Read_RMS(uint8_t channel, uint16_t num_samples, uint8_t SENSOR_RATING, float SENSOR_TYPE_SCALAR);

/** 
 * Calculates RMS value of current on all ADC channels
 * @param SENSOR_RATINGS Array of sensor ratings to consider for calculating RMS values
 * @param num_samples Number of samples to read on each channel for calculating RMS values
 * @param single_byte_RMS_CURRENTS Byte array to hold single byte integer RMS currents for each channel
 */
void Calculate_RMS_Current_On_All_Channels(uint8_t* SENSOR_RATINGS, uint16_t num_samples, float* RMS_CURRENTS);

/** 
 * Calculates True RMS value of current on all ADC channels
 * @param SENSOR_RATINGS Array of sensor ratings to consider for calculating RMS values
 * @param num_samples Number of samples to read on each channel for calculating RMS values
 * @param single_byte_RMS_CURRENTS Byte array to hold single byte integer RMS currents for each channel
 */
void Calculate_True_RMS_Current_On_All_Channels(uint8_t* SENSOR_RATINGS, float* SSN_CURRENT_SENSOR_VOLTAGE_SCALARS, uint16_t num_samples, float* RMS_CURRENTS);

/** 
 * Calculates RMS value of current for a voltage output current sensor. Expects the ADC channel has already been sampled
 * @param channel Which channel to sample
 * @param current_samples_array Array holding the ADC samples
 * @param num_samples Number of samples in the ADC samples array
 * @param sensor_max_value Rating of the current sensor for which RMS is to be computed
 * @return RMS value of current
 */
float Current_VSensor_Read_RMS(uint8_t channel, uint16_t* current_samples_array, uint16_t num_samples, uint8_t sensor_max_value);

/** 
 * Calculates RMS value of current for a current output current sensor. Expects the ADC channel has already been sampled
 * @param channel Which channel to sample
 * @param current_samples_array Array holding the ADC samples
 * @param num_samples Number of samples in the ADC samples array
 * @param sensor_max_value Rating of the current sensor for which RMS is to be computed
 * @return RMS value of current
 */
float Current_CSensor_Read_RMS(uint8_t channel, uint16_t* current_samples_array, uint16_t num_samples, uint8_t sensor_max_value);

/** 
 * Calculates RMS values of current for each ADC channel along with the load percentages, machine status, timestamps and duration in state
 * @param SSN_CURRENT_SENSOR_RATINGS Byte array for saving current sensor ratings
 * @param SSN_CURRENT_SENSOR_THRESHOLDS Byte array for saving machine thresholds for deciding IDLE state of machines
 * @param SSN_CURRENT_SENSOR_MAXLOADS Byte array for saving machine maximum loads for calculating percentage loads
 * @param Machine_load_currents A byte array of machine load currents calculated by SSN
 * @param Machine_load_percentages A byte array of machine load percentages calculated by SSN
 * @param Machine_status A byte array of machine status (ON/OFF/IDLE) calculated by SSN
 * @param Machine_status_duration An array of machine status duration indicating for how long the machines have been in current state
 * @param Machine_status_timestamp An array of machine status timestamp indicating since when the machines have been in current state
 * @return Status_change_flag a flag that will indicate if the state of any machine has changed
 */
bool Get_Machines_Status_Update(uint8_t* SSN_CURRENT_SENSOR_RATINGS, float* SSN_CURRENT_SENSOR_VOLTAGE_SCALARS, float* SSN_CURRENT_SENSOR_THRESHOLDS, uint8_t* SSN_CURRENT_SENSOR_MAXLOADS, 
    float* Machine_load_currents, uint8_t* Machine_load_percentages, uint8_t* Machine_status, uint8_t* Machine_prev_status, uint8_t* Machine_status_flag, uint32_t* Machine_status_duration, 
    uint32_t* Machine_status_timestamp, uint32_t ssn_current_dynamic_clock_val);

int8_t Get_Machine_Status(uint8_t machine_number, float idle_threshold, uint8_t prev_state);

void Clear_Machine_Status_flag (uint8_t* Machine_status_flag);

#endif