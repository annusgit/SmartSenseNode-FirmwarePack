
#ifndef __SSN_API_h__    /* Guard against multiple inclusion */
#define __SSN_API_h__

#include <plib.h>
#include "SystemTests/SystemTests.h"
#include "FlashMemory/FlashMemory.h"
#include "Connection/Connection.h"
#include "Communication/Communication.h"

/** 
 * \mainpage 
 * <A NAME="Contents"></A>
 * @section contents_sec Table of Contents
 * <A HREF="#Introduction">Introduction</A><br>
 * <A HREF="#API">API</A><br>
 * <A HREF="#CodingStandard">Coding Standard</A><br>
 * <A HREF="#VersionLog">Version Log</A><br>
 * <A HREF="#Acronyms">Acronyms</A><br>
 * <A HREF="#Documentation">Documentation</A><br>
 * <A HREF="#Requirements">Requirements</A><br>
 * <A HREF="#Tools">Tools</A><br>
 *
 * <HR> 
 *
 *  <A NAME="Introduction"></A>
 * @section intro_sec Introduction
 * This document summarizes the core functions, variables and APIs used for the development of SSN firmware.
 *
 * \image html SSNv1.2.jpeg "Smart Sense Node v1.2"
 *
 * Based on the design problem for an IoT-based Smart Sense Node Network, our design is built around the following hardware/components.
 *  - PIC32MX170F256B Microcontroller with the following on-chip resources
 *      -# 256KB Program Memory (Flash) and 64KB Data Memory (SRAM)
 *      -# Multiple ADC channels with upto a Million Samples Per Second and 10-bit resolution
 *      -# Support for I2C and SPI peripherals
 *  - 24LC08 1KB EEPROM with I2C interface
 *  - W5500 Ethernet Offload Chip with SPI interface and integrated MAC and PHY
 *  - AM2320 Temperature and humidity sensor with I2C interface
 * <br><A HREF="#Contents">Table of Contents</A><br>
 *
 * <HR> 
 *
 *  <A NAME="API"></A>
 * @section api_sec API
 * The SSN API is a High-level API that deals with devices such as Flash Memory and Ethernet at an abstract level hiding the peripheral level 
 * details of individual devices such as the protocols being used to communicate and single byte transactions between the MCU and peripherals. 
 * This API is itself dependant on a Driver API for each peripheral/device in use.
 * 
 * <br><A HREF="#Contents">Table of Contents</A><br>
 *
 * <HR> 
 *
 *  <A NAME="Coding Standard"></A>
 * @section CodingStandard_sec CodingStandard
 * Most of the API functions are designed on the basis of dependancy injection. For example, consider the following function definition
 * @code
 * void Calculate_RMS_Current_On_All_Channels(uint8_t* SENSOR_RATINGS, uint16_t num_samples, unsigned char* single_byte_RMS_CURRENTS) {
 *	    uint32_t count = 0, ADC_raw_samples[NO_OF_MACHINES] = {0}, max_ADC_raw_sample[NO_OF_MACHINES] = {0}, ADC_raw_non_zero_sum[NO_OF_MACHINES] = {0}, ADC_raw_non_zero_count[NO_OF_MACHINES] = {0};
 *	    uint32_t MAX_SAMPLE_BASED_CURRENT_RMS_value[NO_OF_MACHINES] = {0}, AVERAGE_SAMPLE_BASED_CURRENT_RMS_value[NO_OF_MACHINES] = {0}, CURRENT_RMS_VALUE[NO_OF_MACHINES] = {0};
 *	    float SENSOR_TYPE_SCALAR;
 *	    uint8_t i;
 *	    
 *	    while(count < num_samples) {
 *	        
 *	        for (i = 0; i < NO_OF_MACHINES; i++) {
 *	            // Sample one value from ith channel
 *	            ADC_raw_samples[i] = sample_Current_Sensor_channel(i);
 *	            // record the maximum value in this sample space for MAX Sample based RMS calculation for ith channel
 *	            if (ADC_raw_samples[i] > max_ADC_raw_sample[i])
 *	                max_ADC_raw_sample[i] = ADC_raw_samples[i];
 *	            // record every non-zero value in this sample space for AVERAGE Sample based RMS calculation for ith channel
 *	            if (ADC_raw_samples[i] > 0) {
 *	                ADC_raw_non_zero_sum[i] += ADC_raw_samples[i];
 *	                ADC_raw_non_zero_count[i]++;
 *	            }
 *	        }
 *	        count++;
 *	        // pick 200 samples per wave cycle of AC Sine Wave @ 50Hz => 100us sampling period
 *	        sleep_for_microseconds(100); 
 *	    }
 *
 *	    // Calculate the RMS Current Values using two methods and average them
 *	    for (i = 0; i < NO_OF_MACHINES; i++) {
 *	        SENSOR_TYPE_SCALAR = VOLTAGE_OUTPUT_CURRENT_SENSOR_SCALAR;
 *	        if (SENSOR_RATINGS[i] == 100)
 *	            SENSOR_TYPE_SCALAR = CURRENT_OUTPUT_CURRENT_SENSOR_SCALAR;
 *	        MAX_SAMPLE_BASED_CURRENT_RMS_value[i] = (SENSOR_RATINGS[i] / 724.07) * SENSOR_TYPE_SCALAR * (0.707 * (float)max_ADC_raw_sample[i]);
 *	        AVERAGE_SAMPLE_BASED_CURRENT_RMS_value[i] = (SENSOR_RATINGS[i] / 718.89) * SENSOR_TYPE_SCALAR * (1.1 * (float)ADC_raw_non_zero_sum[i]/ADC_raw_non_zero_count[i]);
 *	        CURRENT_RMS_VALUE[i] = (float)(MAX_SAMPLE_BASED_CURRENT_RMS_value[i] + AVERAGE_SAMPLE_BASED_CURRENT_RMS_value[i]) / 2;
 *	        single_byte_RMS_CURRENTS[i] = (unsigned char)CURRENT_RMS_VALUE[i];
 *	    }
 *	}
 * @endcode
 *
 * We use this function to calculate RMS value of currents for all current transformers by sampling all ADC channels but this function expects to 
 * receive the current ratings of these sensors from where ever this routine is invoked. It also requires the number of samples to take before making 
 * this calculation. Therefore the dependancies for this function are passed as parameters of this function call. Most of the functions in the SSN API
 * are written in a similar way.
 *
 * <br><A HREF="#Contents">Table of Contents</A><br>
 *
 * <HR> 
 *
 *  <A NAME="Version Log"></A>
 * @section VersionLog_sec VersionLog
 * 
 * The current state of SSN firmware is at <b>Version 1.0</b>
 *
 * <br><A HREF="#Contents">Table of Contents</A><br>
 *
 * <HR> 
 *
 *  <A NAME="Acronyms"></A>
 * @section Acronyms_sec Acronyms
 * 
 * <b>SSN -> Smart Sense Node</b> 
 *
 * <br><A HREF="#Contents">Table of Contents</A><br>
 *
 * <HR> 
 *
 *  <A NAME="Documentation"></A>
 * @section Documentation_sec Documentation
 * 
 * The documentation is mostly presented in the Files tab. For viewing individual code files with documentation for functions and variables, 
 * go to <b>Files->File List</b> and click on the file symbols next to file names. Clicking on the names directly will show the source code 
 * inside those files.
 * 
 * <br><A HREF="#Contents">Table of Contents</A><br>
 *
 * <HR> 
 *
 *  <A NAME="Requirements"></A>
 * @section Requirements_sec Requirements
 * 
 * This firmware code relies on two dependencies
 * 	- Legacy Peripheral library for PIC32MX series MCUs available <a href="https://www.microchip.com/SWLibraryWeb/product.aspx?product=PIC32%20Peripheral%20Library">here</a> 
 * 	- Wiznet W5500 ioLibrary driver available <a href="http://wizwiki.net/wiki/doku.php/products:w5500:driver">here</a> 
 * 
 * <br><A HREF="#Contents">Table of Contents</A><br>
 *
 * <HR> 
 *
 *  <A NAME="Tools"></A>
 * @section Tools_sec Tools
 * 
 * The entire firmware for this code has been written in <b>C</b> using <b>MPLABX IDE</b> and <b>XC32 compiler v1.40</b> 
 * available <a href="https://www.microchip.com/development-tools/pic-and-dspic-downloads-archive">here</a>. The Peripheral library must be installed 
 * inside the XC32 compiler folder for correct inclusion in the source code. 
 * 
 * <br><A HREF="#Contents">Table of Contents</A><br>
 */

/**
 * Our global variables for SSN
 */
/** Our SSN UDP communication socket */
extern SOCKET SSN_UDP_SOCKET;
/** SSN Server Address */
extern uint8_t SSN_SERVER_IP[4];
/** SSN Server PORT */
extern uint16_t SSN_SERVER_PORT;

/** Static IP Assignment */
extern uint8_t SSN_STATIC_IP[4];
extern uint8_t SSN_SUBNET_MASK[4];
extern uint8_t SSN_GATWAY_ADDRESS[4];
extern uint8_t SSN_DNS_ADDRESS[4];

/** A counter to maintain how many messages have been sent from SSN to Server since wakeup */
extern uint32_t SSN_SENT_MESSAGES_COUNTER;
/** Boolean variable for Interrupt Enabled or not */
extern bool InterruptEnabled;
/** Counter variable for interrupts per second */
extern uint8_t interrupts_per_second;
/** Counter variable for counting half seconds per second */
extern uint8_t half_second_counter, delays_per_second_counter; 
/** Counter variable for counting after how many intervals to send the status update */
extern uint8_t report_counter;
/** Current State of the SSN. There is no state machine of the SSN but we still use this variable to keep track at some instances */
extern uint8_t SSN_CURRENT_STATE, SSN_PREV_STATE;
/** Report Interval of SSN set according to the configurations passed to the SSN */
extern uint8_t SSN_REPORT_INTERVAL;
/** SSN current sensor configurations */
extern uint8_t SSN_CONFIG[EEPROM_CONFIG_SIZE];
/** Flags used to indicate if we have received configurations */
extern bool CONFIG_received, TimeOfDay_received;
/** SSN current sensor relative scalar for voltage output */
extern float SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[NO_OF_MACHINES];
/** SSN current sensor ratings */
extern uint8_t SSN_CURRENT_SENSOR_RATINGS[4];
/** SSN machine maximum loads for calculating percentage loads on machines */
extern uint8_t SSN_CURRENT_SENSOR_MAXLOADS[4];
/** SSN machine thresholds for deciding IDLE state */
extern float SSN_CURRENT_SENSOR_THRESHOLDS[4];
/** SSN Temperature and Humidity Sensor Thresholds */
extern uint8_t TEMPERATURE_MIN_THRESHOLD, TEMPERATURE_MAX_THRESHOLD;
extern uint8_t RELATIVE_HUMIDITY_MIN_THRESHOLD, RELATIVE_HUMIDITY_MAX_THRESHOLD;
/** SSN machine load currents array */
extern float Machine_load_currents[NO_OF_MACHINES];
/** SSN machine load percentages array */
extern uint8_t Machine_load_percentages[NO_OF_MACHINES];
/** SSN machine status array */
extern uint8_t Machine_status[NO_OF_MACHINES];
/** SSN machine status tracker array */
extern uint8_t Machine_prev_status[NO_OF_MACHINES];
/** SSN machine status flag array that tells if the machine status changed */
extern uint8_t Machine_status_flag;
/** SSN machine timestamps for recording since when the machines have been in the current states */
extern uint32_t Machine_status_timestamp[NO_OF_MACHINES];
/** SSN machine status duration array for holding the number of seconds for which the machines have been in the current state */
extern uint32_t Machine_status_duration[NO_OF_MACHINES];
/** Machine status change flag. It will be used for resending status update out of sync with the reporting interval for accurate timing */
bool machine_status_change_flag;
/** SSN UDP socket number */
extern uint8_t SSN_UDP_SOCKET_NUM;
/** SSN default MAC address. This is the same for all SSNs */
extern uint8_t SSN_DEFAULT_MAC[];
/** SSN current MAC address. May hold the default MAC or the one received from SSN Server. The last two bytes are the SSN Identity */
extern uint8_t SSN_MAC_ADDRESS[6];
/** SSN temperature sensor reading bytes */
extern uint8_t temperature_bytes[2];
/** SSN relative humidity reading bytes */
extern uint8_t relative_humidity_bytes[2];
/** SSN temperature and humidity reading successful/unsuccessful status bit */
extern int8_t temp_humidity_recv_status; 
/** SSN abnormal activity bit */
extern uint8_t abnormal_activity;
/** A variable to maintain a count of how many messages we have sent */
extern uint32_t message_count; 
/** Socket health check variable */
extern bool socket_ok;
/** SSN loop variable */
extern uint8_t i;

/** 
 *  Includes are needed peripherals and APIs for SSN functionality
 */
void SSN_Setup();
void SSN_COPY_MAC_FROM_MEMORY();
void SSN_GET_MAC();
void SSN_GET_CONFIG();
void SSN_GET_CONFIG_WITH_5_SECONDS_HALT();
void SSN_GET_TIMEOFDAY();
void SSN_RECEIVE_ASYNC_MESSAGE();
void SSN_RECEIVE_ASYNC_MESSAGE_OVER_MQTT(MessageData* md) ;
void SSN_CHECK_ETHERNET_CONNECTION();
void SSN_GET_AMBIENT_CONDITION();
void SSN_RESET_AFTER_N_SECONDS(uint32_t seconds);
void SSN_RESET_AFTER_N_SECONDS_IF_NO_MACHINE_ON(uint32_t seconds);
void SSN_RESET_IF_SOCKET_CORRUPTED();
/**
 * Peripheral testing and debugging functions
 */
void led_blink_test();
void current_test();
void network_test();
void watchdog_test();

#endif /* _EXAMPLE_FILE_NAME_H */

