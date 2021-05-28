#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

#include "SSN_API.h"

/** SSN Server Address */
uint8_t SSN_SERVER_IP[] = {192, 168, 0, 110};
//uint8_t SSN_SERVER_IP[] = {};//34, 87, 92, 5};
uint8_t DEFAULT_SERVER_IP[] = {34, 87, 92, 5};//{192, 168, 0, 110};
//uint8_t SSN_SERVER_IP[] = {115, 186, 183, 129};
unsigned char MQTT_SERVER_DNS[40] = "mqtt.wisermachines.com";//"maryum";

/** SSN Server PORT */
//uint16_t SSN_SERVER_PORT = 36000;
//uint8_t SSN_SERVER_IP[] = {192, 168, 0, 120};
///** SSN Server PORT */
//uint16_t SSN_SERVER_PORT = 36000;

//uint8_t DNS_ADDRESS[4] = {8, 8, 8, 8};

/** Static IP Assignment */
uint8_t SSN_STATIC_IP[4]		= {172, 16, 41, 131};
uint8_t SSN_SUBNET_MASK[4]		= {255, 255, 0, 0};
uint8_t SSN_GATWAY_ADDRESS[4]	= {172, 16, 5, 1};
uint8_t SSN_DNS_ADDRESS[4]		= {172, 16, 14, 12};

/** A counter to maintain how many messages have been sent from SSN to Server since wakeup */
uint32_t SSN_SENT_MESSAGES_COUNTER = 0;
/** Boolean variable for Interrupt Enabled or not */
bool InterruptEnabled = false;
/** Counter variable for interrupts per second */
uint8_t interrupts_per_second = 2;
/** Counter variable for counting half seconds per second */
uint8_t half_second_counter = 0, delays_per_second_counter = 0;
/** Counter variable for counting after how many intervals to send the status update */
uint8_t report_counter = 0;
bool report_now = false;
/** Current State of the SSN. There is no state machine of the SSN but we still use this variable to keep track at some instances */
uint8_t SSN_CURRENT_STATE = NO_MAC_STATE, SSN_PREV_STATE;
/** Report Interval of SSN set according to the configurations passed to the SSN */
uint8_t SSN_REPORT_INTERVAL = 1;
/** SSN current sensor configurations */
uint8_t SSN_CONFIG[EEPROM_CONFIG_SIZE];
/** Flags used to indicate if we have received configurations */
bool CONFIG_received = false, TimeOfDay_received = false;
/** SSN current sensor relative scalar for voltage output */
float SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[NO_OF_MACHINES];
/** SSN current sensor ratings */
uint8_t SSN_CURRENT_SENSOR_RATINGS[4];
/** SSN machine maximum loads for calculating percentage loads on machines */
uint8_t SSN_CURRENT_SENSOR_MAXLOADS[4];
/** SSN machine thresholds for deciding IDLE state */
float SSN_CURRENT_SENSOR_THRESHOLDS[4];
/** SSN Temperature and Humidity Sensor Thresholds */
uint8_t TEMPERATURE_MIN_THRESHOLD, TEMPERATURE_MAX_THRESHOLD;
uint8_t RELATIVE_HUMIDITY_MIN_THRESHOLD, RELATIVE_HUMIDITY_MAX_THRESHOLD;
/** SSN machine load currents array */
float Machine_load_currents[NO_OF_MACHINES] = {0};
/** SSN machine load percentages array */
uint8_t Machine_load_percentages[NO_OF_MACHINES] = {0};
/** SSN machine status array initialized to a OFF state */
uint8_t Machine_status[NO_OF_MACHINES] = {SENSOR_NOT_CONNECTED, SENSOR_NOT_CONNECTED, SENSOR_NOT_CONNECTED, SENSOR_NOT_CONNECTED};
/** SSN machine status tracker array */
uint8_t Machine_prev_status[NO_OF_MACHINES] = {SENSOR_NOT_CONNECTED, SENSOR_NOT_CONNECTED, SENSOR_NOT_CONNECTED, SENSOR_NOT_CONNECTED};
/** SSN machine status flag array that tells if the machine status changed */
uint8_t Machine_status_flag = 0;
/** SSN machine timestamps for recording since when the machines have been in the current states */
uint32_t Machine_status_timestamp[NO_OF_MACHINES] = {0};
/** SSN machine status duration array for holding the number of seconds for which the machines have been in the current state */
uint32_t Machine_status_duration[NO_OF_MACHINES] = {0};
/** Machine status change flag. It will be used for resending status update out of sync with the reporting interval for accurate timing */
bool machine_status_change_flag = false;
/** SSN UDP socket number */
uint8_t SSN_UDP_SOCKET_NUM = 4;
/** SSN default MAC address. This is the same for all SSNs */
uint8_t SSN_DEFAULT_MAC[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
/** SSN current MAC address. May hold the default MAC or the one received from SSN Server. The last two bytes are the SSN Identity */
uint8_t SSN_MAC_ADDRESS[6] = {0};
/** SSN temperature sensor reading bytes */
uint8_t temperature_bytes[2];
/** SSN relative humidity reading bytes */
uint8_t relative_humidity_bytes[2];
/** SSN temperature and humidity reading successful/unsuccessful status bit */
int8_t temp_humidity_recv_status;
/** SSN abnormal activity byte */
uint8_t abnormal_activity;
/** A variable to maintain a count of how many messages we have sent */
uint32_t message_count = 0;
int message_publish_status = 0;
uint8_t mqtt_failure_counts = 0;
uint8_t MQTTallowedfailureCount;
/** SSN loop variable */
uint8_t i;
uint8_t currentconfig[100];


void SSN_Setup() {
	// Setup calls for all our peripherals/devices
	setup_printf(115200);
	// open_UART1(9600);
	setup_EEPROM();
	setup_Ethernet(5000000);
	setup_Current_Sensors();
//	setup_Temperature_Humidity_Sensor();
//	setup_IR_Temperature_Sensor_And_Laser();
	setup_LED_Indicator();
	setup_Interrupts();
    setup_millisecond_timer_with_interrupt();
	fault_count = 0;
}

void SSN_COPY_MAC_FROM_MEMORY() {
	SSN_PREV_STATE = SSN_CURRENT_STATE;
	SSN_CURRENT_STATE = FindMACInFlashMemory(SSN_MAC_ADDRESS, SSN_DEFAULT_MAC);
	if (SSN_PREV_STATE != SSN_CURRENT_STATE) {
		Clear_LED_INDICATOR();
	}
}

void SSN_GET_MAC() {
	uint16_t SendAfter = 0;
	// When we will receive a MAC address (if we didn't have it), we will reset the controller
	while (SSN_CURRENT_STATE == NO_MAC_STATE) {
//		SSN_CHECK_ETHERNET_CONNECTION();
		SSN_PREV_STATE = SSN_CURRENT_STATE;
		SSN_CURRENT_STATE = NO_MAC_STATE;
		if (SSN_PREV_STATE != SSN_CURRENT_STATE) {
			Clear_LED_INDICATOR();
		}
		// request a MAC address after every 5 seconds
		if (SendAfter % 50 == 0) {
			message_publish_status = Send_GETMAC_Message(SSN_MAC_ADDRESS);
            if (message_publish_status != SUCCESSS) {
                message_publish_status = FAILURE;
//                printf("message publish status %d\n\n",message_publish_status);
//                return message_publish_status;   
            }
		}
		// Give LED indication every second
		if (SendAfter % 10 == 0) {
			SSN_LED_INDICATE(SSN_CURRENT_STATE);
		}
		SendAfter++;
		ServiceWatchdog();
        // MQTT process handler
		start_ms_timer_with_interrupt();
		MQTTYield(&Client_MQTT, 50);
		message_publish_status = SSN_Check_Connection_And_Reconnect(message_publish_status);

		stop_ms_timer_with_interrupt();
		// 100 milliseconds
		sleep_for_microseconds(100000);
	}
//	return message_publish_status;
}

void SSN_GET_CONFIG() {
	// Find configurations in EEPROM
	SSN_PREV_STATE = SSN_CURRENT_STATE;
	SSN_CURRENT_STATE = FindSensorConfigurationsInFlashMemory(SSN_CONFIG, &SSN_REPORT_INTERVAL, &TEMPERATURE_MIN_THRESHOLD, &TEMPERATURE_MAX_THRESHOLD, &RELATIVE_HUMIDITY_MIN_THRESHOLD,
		&RELATIVE_HUMIDITY_MAX_THRESHOLD, SSN_CURRENT_SENSOR_RATINGS, SSN_CURRENT_SENSOR_THRESHOLDS, SSN_CURRENT_SENSOR_MAXLOADS, SSN_CURRENT_SENSOR_VOLTAGE_SCALARS);
	uint16_t SendAfter = 0;
	while (SSN_CURRENT_STATE == NO_CONFIG_STATE) {
//		SSN_CHECK_ETHERNET_CONNECTION();
		SSN_PREV_STATE = SSN_CURRENT_STATE;
		SSN_CURRENT_STATE = NO_CONFIG_STATE;
		if (SSN_PREV_STATE != SSN_CURRENT_STATE) {
			Clear_LED_INDICATOR();
		}
		if (CONFIG_received) {
			break;
		}
		// request a Configuration after every 5 seconds
		if (SendAfter % 50 == 0) {
			message_publish_status = Send_GETCONFIG_Message(SSN_MAC_ADDRESS);
            if (message_publish_status != SUCCESSS) {
                message_publish_status = FAILURE;   
//                return message_publish_status;   
            }
		}
		// Give LED indication every second
		if (SendAfter % 10 == 0) {
			SSN_LED_INDICATE(SSN_CURRENT_STATE);
		}
		SendAfter++;
		ServiceWatchdog();
		// MQTT process handler
		start_ms_timer_with_interrupt();
		MQTTYield(&Client_MQTT, 50);
		message_publish_status = SSN_Check_Connection_And_Reconnect(message_publish_status);
		stop_ms_timer_with_interrupt();
		// 100 milliseconds
		sleep_for_microseconds(100000);
	}
//	return message_publish_status;
}

void SSN_GET_TIMEOFDAY() {
    TimeOfDay_received = false;
	uint16_t SendAfter = 0;
	while (1) {
		SSN_CHECK_ETHERNET_CONNECTION();
		SSN_PREV_STATE = SSN_CURRENT_STATE;
		SSN_CURRENT_STATE = NO_CONFIG_STATE;
		if (SSN_PREV_STATE != SSN_CURRENT_STATE) {
			Clear_LED_INDICATOR();
		}
		if (TimeOfDay_received) {
			// initialize SSN's uptime
			ssn_uptime_in_seconds = 0;
			SSN_PREV_STATE = SSN_CURRENT_STATE;
			SSN_CURRENT_STATE = NORMAL_ACTIVITY_STATE;
			if (SSN_PREV_STATE != SSN_CURRENT_STATE) {
				Clear_LED_INDICATOR();
			}
			break;
		}
		// request time of day after every 5 seconds
		if (SendAfter % 50 == 0 && !TimeOfDay_received) {
			message_publish_status = Send_GETTimeOfDay_Message(SSN_MAC_ADDRESS);		
            if (message_publish_status != SUCCESSS) {
                message_publish_status = FAILURE;   
//                return message_publish_status;   
            }
		}
		// Give LED indication every second
		if (SendAfter % 10 == 0) {
			SSN_LED_INDICATE(SSN_CURRENT_STATE);
		}
		SendAfter++;
		ServiceWatchdog();
		// MQTT process handler
		start_ms_timer_with_interrupt();
		MQTTYield(&Client_MQTT, 50);
		message_publish_status = SSN_Check_Connection_And_Reconnect(message_publish_status);
        stop_ms_timer_with_interrupt();
		// 100 milliseconds
		sleep_for_microseconds(100000);
	}
//	return message_publish_status;
}

int SSN_Check_Connection_And_Reconnect(int return_code) {
    int rc;
    if (return_code != SUCCESSS) {
        printf("[**MQTT**] Bad Return Code: %d\n", return_code);
        SSN_CHECK_ETHERNET_CONNECTION();
        CloseMQTTClientConnectionAndSocket(&Client_MQTT, TCP_SOCKET);
        rc = SetupMQTTClientConnection(&MQTT_Network, &Client_MQTT, &MQTTOptions, SSN_SERVER_IP, NodeExclusiveChannel, SSN_RECEIVE_ASYNC_MESSAGE_OVER_MQTT, &UDP_message, ssn_dynamic_clock);   
//        printf("rc %d\n",rc);
        return rc;
    }   
    return SUCCESSS;
}

void SSN_RECEIVE_ASYNC_MESSAGE_OVER_MQTT(MessageData* md) {
    start_ms_timer_with_interrupt();
    MQTTYield(&Client_MQTT, 100);
    stop_ms_timer_with_interrupt();
	unsigned char testbuffer[BUFFER_SIZE];
	MQTTMessage* message = md->message;
	// printf("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	clear_array(testbuffer, 100);
	if (MQTTOptions.showtopics) {
		memcpy(testbuffer, (char*) message->payload, (int) message->payloadlen);
		*(testbuffer + (int) (message->payloadlen) + 1) = "\n";

		uint8_t received_message_id;
		uint32_t TimeOFDayTick;
		received_message_id = decipher_received_message(testbuffer, params);

		// based on which message was received (received_message_id), we extract and save the data
		switch (received_message_id) {
			case SET_MAC_MESSAGE_ID:
				printf("<- SET_MAC MESSAGE RECEIVED: %X:%X:%X:%X:%X:%X\n", params[0], params[1], params[2], params[3], params[4], params[5]);
				printf("Resetting Controller Now...\n");
				// write the new MAC addresses to designated location in EEPROM
				EEPROM_Write_Array(EEPROM_BLOCK_0, EEPROM_MAC_LOC, params, EEPROM_MAC_SIZE);
				// reset the SSN from software
				SoftReset();
				while (1);
				break;

			case SET_TIMEOFDAY_MESSAGE_ID:
				if(!TimeOfDay_received) {
					TimeOFDayTick = get_uint32_from_bytes(params);
					printf("<- SET_TIMEOFDAY MESSAGE RECEIVED: %d\n", TimeOFDayTick);
					// assign incoming clock time to SSN Global Clock (Pseudo Clock because we don't have an RTCC)
					set_ssn_time(TimeOFDayTick);
					ssn_uptime_in_seconds = 0;
					TimeOfDay_received = true;					
				}
				break;

			case SET_CONFIG_MESSAGE_ID:
				// write the new config to designated location in EEPROM
				EEPROM_Write_Array(EEPROM_BLOCK_0, EEPROM_CONFIG_LOC, params, EEPROM_CONFIG_SIZE);
				// Copy received configurations to the SSN_CONFIG array
				int i;
				for (i = 0; i < EEPROM_CONFIG_SIZE; i++) {
					SSN_CONFIG[i] = params[i];
				}
				// Copy from the configurations, the sensor ratings, thresholds and maximum load values to our variables
				for (i = 0; i < NO_OF_MACHINES; i++) {
					/* Get the parameters from the Configurations */
					SSN_CURRENT_SENSOR_RATINGS[i] = SSN_CONFIG[4 * i + 0];
					SSN_CURRENT_SENSOR_THRESHOLDS[i] = SSN_CONFIG[4 * i + 1] / 10.0f;
					SSN_CURRENT_SENSOR_MAXLOADS[i] = SSN_CONFIG[4 * i + 2];
					if (SSN_CONFIG[4 * i + 3] == 0) {
						SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[i] = 1.0;
					} else if (SSN_CONFIG[4 * i + 3] == 1) {
						SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[i] = 0.333;
					} else if (SSN_CONFIG[4 * i + 3] == 2) {
						SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[i] = 1.65;
                    }
				}
				TEMPERATURE_MIN_THRESHOLD = SSN_CONFIG[16];
				TEMPERATURE_MAX_THRESHOLD = SSN_CONFIG[17];
				RELATIVE_HUMIDITY_MIN_THRESHOLD = SSN_CONFIG[18];
				RELATIVE_HUMIDITY_MAX_THRESHOLD = SSN_CONFIG[19];
				// save new reporting interval
				SSN_REPORT_INTERVAL = SSN_CONFIG[20];
				printf("[LOG] Received New Current Sensor Configuration from SSN Server: \n"
					"     >> S1-Rating: %03d Arms | M1-Threshold: %.3f Arms | M1-Maxload: %03d Arms | S1-Scalar: %.3f Vrms | \n"
					"     >> S2-Rating: %03d Arms | M2-Threshold: %.3f Arms | M2-Maxload: %03d Arms | S2-Scalar: %.3f Vrms |\n"
					"     >> S3-Rating: %03d Arms | M3-Threshold: %.3f Arms | M3-Maxload: %03d Arms | S3-Scalar: %.3f Vrms |\n"
					"     >> S4-Rating: %03d Arms | M4-Threshold: %.3f Arms | M4-Maxload: %03d Arms | S4-Scalar: %.3f Vrms |\n"
					"     >> MIN TEMP : %03d C    | MAX TEMP : %03d C    |\n"
					"     >> MIN RH   : %03d %    | MIN RH   : %03d %    |\n"
					"     >> Report   : %d seconds\n",
					SSN_CURRENT_SENSOR_RATINGS[0], SSN_CURRENT_SENSOR_THRESHOLDS[0], SSN_CURRENT_SENSOR_MAXLOADS[0], SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[0],
					SSN_CURRENT_SENSOR_RATINGS[1], SSN_CURRENT_SENSOR_THRESHOLDS[1], SSN_CURRENT_SENSOR_MAXLOADS[1], SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[1],
					SSN_CURRENT_SENSOR_RATINGS[2], SSN_CURRENT_SENSOR_THRESHOLDS[2], SSN_CURRENT_SENSOR_MAXLOADS[2], SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[2],
					SSN_CURRENT_SENSOR_RATINGS[3], SSN_CURRENT_SENSOR_THRESHOLDS[3], SSN_CURRENT_SENSOR_MAXLOADS[3], SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[3],
					TEMPERATURE_MIN_THRESHOLD, TEMPERATURE_MAX_THRESHOLD, RELATIVE_HUMIDITY_MIN_THRESHOLD, RELATIVE_HUMIDITY_MAX_THRESHOLD, SSN_REPORT_INTERVAL);
				// Reset Machine States 
				for (i = 0; i < NO_OF_MACHINES; i++) {
					Machine_status[i] = SENSOR_NOT_CONNECTED;
				}
                MQTTallowedfailureCount = MQTTallowedfailureCounts(SSN_REPORT_INTERVAL);
				CONFIG_received = true;
//                printf("[LOG] Node Configurations Updated. Restarting now...\n");
//                CloseMQTTClientConnectionAndSocket(&Client_MQTT, TCP_SOCKET);
//                SoftReset();
				break;

				// Only for debugging, will be removed
				// This message will clear the EEPROM of our SSN
			case DEBUG_EEPROM_CLEAR_MESSAGE_ID:
				// stop the global timer
				stop_Global_Clock();
				printf("[DEBUG] Clearing EEPROM Now...\n");
				// Clear EEPROM and reset node
				EEPROM_Clear();
				// reset the SSN
				printf("[DEBUG] Resetting Controller Now...\n");
				SoftReset();
				while (1);
				break;

				// Only for debugging, will be removed
				// This message will reset our SSN
			case DEBUG_RESET_SSN_MESSAGE_ID:
				// stop the global timer
				stop_Global_Clock();
				// reset the SSN
				printf("[DEBUG] Resetting Controller Now...\n");
				sleep_for_microseconds(1000000);
				SoftReset();
				while (1);
				break;
            case RETRIEVE_CURRENT_CONFIG:
                printf("<- RETRIEVE CURRENT CONFIG MESSAGE RECEIVED\n");
				for (i = 0; i < NO_OF_MACHINES; i++) {
					currentconfig[4 * i + 0] = SSN_CURRENT_SENSOR_RATINGS[i];
					currentconfig[4 * i + 1] = (int) (10.0 * SSN_CURRENT_SENSOR_THRESHOLDS[i]);
					currentconfig[4 * i + 2] = SSN_CURRENT_SENSOR_MAXLOADS[i];
                    SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[i] = 0.333;
					if (SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[i] == 1.0) {
						currentconfig[4 * i + 3] = 0;
					} else if (SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[i] == 0.333) {
						currentconfig[4 * i + 3] = 1;
					}
				}
				currentconfig[16] = TEMPERATURE_MIN_THRESHOLD;
				currentconfig[17] = TEMPERATURE_MAX_THRESHOLD;
				currentconfig[18] = RELATIVE_HUMIDITY_MIN_THRESHOLD;
				currentconfig[19] = RELATIVE_HUMIDITY_MAX_THRESHOLD;
				// save new reporting interval
				currentconfig[20] = SSN_REPORT_INTERVAL;
				printf("[LOG] Sending Sensor Configuration to SSN Server: \n"
					"     >> S1-Rating: %d Arms | M1-Threshold: %.2f Arms | M1-Maxload: %d Arms | S1-Scalar: %d Vrms | \n"
					"     >> S2-Rating: %d Arms | M2-Threshold: %.2f Arms | M2-Maxload: %d Arms | S2-Scalar: %d Vrms | \n"
					"     >> S3-Rating: %d Arms | M3-Threshold: %.2f Arms | M3-Maxload: %d Arms | S3-Scalar: %d Vrms | \n"
					"     >> S4-Rating: %d Arms | M4-Threshold: %.2f Arms | M4-Maxload: %d Arms | S4-Scalar: %d Vrms | \n"
					"     >> MIN TEMP : %03d C    | MAX TEMP : %03d C    |\n"
					"     >> MIN RH   : %03d %    | MIN RH   : %03d %    |\n"
					"     >> Report   : %d seconds\n",
					currentconfig[0], (float)currentconfig[1]/10.0f, currentconfig[2], currentconfig[3],
					currentconfig[4], (float)currentconfig[5]/10.0f, currentconfig[6], currentconfig[7],
					currentconfig[8], (float)currentconfig[9]/10.0f, currentconfig[10], currentconfig[11],
					currentconfig[12], (float)currentconfig[13]/10.0f, currentconfig[14], currentconfig[15],
					currentconfig[16], currentconfig[17], currentconfig[18], currentconfig[19], currentconfig[20]);
                Send_RETRIEVECONFIG_Message(SSN_MAC_ADDRESS,currentconfig);
				break;
            default:
				break;
		}
		clear_array(testbuffer, 100);
	}
}

void SSN_CHECK_ETHERNET_CONNECTION() {
	bool LinkWasOkay = true;
	// Check Ethernet Physical Link Status before sending message
	while (Ethernet_get_physical_link_status() == PHY_LINK_OFF) {
		SSN_PREV_STATE = SSN_CURRENT_STATE;
		SSN_CURRENT_STATE = NO_ETHERNET_STATE;
		if (SSN_PREV_STATE != SSN_CURRENT_STATE) {
			LinkWasOkay = false;
			Clear_LED_INDICATOR();
		}
		printf("[LOG] No Ethernet Physical Link Connection :/ \n");
        SSN_LED_INDICATE(SSN_CURRENT_STATE);
		// Service the watchdog timer to make sure we don't reset 
		ServiceWatchdog();
		sleep_for_microseconds(500000);
	}
	if (!LinkWasOkay) {
		SSN_PREV_STATE = SSN_CURRENT_STATE;
		SSN_CURRENT_STATE = NORMAL_ACTIVITY_STATE;
		if (SSN_PREV_STATE != SSN_CURRENT_STATE) {
			LinkWasOkay = false;
			Clear_LED_INDICATOR();
		}
		printf("[LOG] Ethernet Physical Link Recovered :)\n");
		// reconnect to MQTT broker after breaking previous socket connection
		// close(TCP_SOCKET);
		// SetupMQTTClientConnection(&MQTT_Network, &Client_MQTT, &MQTTOptions, SSN_SERVER_IP, NodeExclusiveChannel, SSN_RECEIVE_ASYNC_MESSAGE_OVER_MQTT);
	}
	return;
}

void SSN_GET_AMBIENT_CONDITION(uint8_t TEMPERATURE_MIN_THRESHOLD, uint8_t TEMPERATURE_MAX_THRESHOLD, uint8_t RELATIVE_HUMIDITY_MIN_THRESHOLD, uint8_t RELATIVE_HUMIDITY_MAX_THRESHOLD) {
#ifdef TH_AM2320
	temp_humidity_recv_status = sample_Temperature_Humidity_bytes_using_AM2320(temperature_bytes, relative_humidity_bytes);
#endif
#ifdef TH_DHT22
	temp_humidity_recv_status = sample_Temperature_Humidity_bytes_using_DHT22(temperature_bytes, relative_humidity_bytes);
#endif
	if (temp_humidity_recv_status == SENSOR_READ_ERROR) {
		abnormal_activity = TEMP_SENSOR_READ_ERROR_CONDITION;
#ifdef TH_DHT22_DEBUG
		printf("DHT22 Read Timeout Occurred\n");
#endif
		return;
	}
	if (temp_humidity_recv_status == SENSOR_CRC_ERROR) {
		abnormal_activity = TEMP_SENSOR_CRC_ERROR_CONDITION;
#ifdef TH_DHT22_DEBUG
		printf("DHT22 CheckSum Error Occurred\n");
#endif
		return;
	}
	abnormal_activity = ambient_condition_status(TEMPERATURE_MIN_THRESHOLD, TEMPERATURE_MAX_THRESHOLD, RELATIVE_HUMIDITY_MIN_THRESHOLD, RELATIVE_HUMIDITY_MAX_THRESHOLD);
	if (abnormal_activity == NORMAL_AMBIENT_CONDITION) {
		SSN_PREV_STATE = SSN_CURRENT_STATE;
		SSN_CURRENT_STATE = NORMAL_AMBIENT_CONDITION;
		if (SSN_PREV_STATE != SSN_CURRENT_STATE) {
			Clear_LED_INDICATOR();
		}
	} else {
		SSN_PREV_STATE = SSN_CURRENT_STATE;
		SSN_CURRENT_STATE = NORMAL_AMBIENT_CONDITION;
		if (SSN_PREV_STATE != SSN_CURRENT_STATE) {
			Clear_LED_INDICATOR();
		}
	}
	return;
}

void SSN_GET_OBJECT_TEMPERATURE_CONDITION_IR(uint8_t TEMPERATURE_MIN_THRESHOLD, uint8_t TEMPERATURE_MAX_THRESHOLD) {
	// printf("here\n");
	float object_celcius_temperature = MLX90614_Read_Temperature_Object_Celcius();
	int integer_temperature;
	if (object_celcius_temperature == MLX90614_COMM_ERROR_CODE) {
		abnormal_activity = TEMP_SENSOR_READ_ERROR_CONDITION;
	} else if (object_celcius_temperature > TEMPERATURE_MIN_THRESHOLD && object_celcius_temperature < TEMPERATURE_MAX_THRESHOLD){
		abnormal_activity = NORMAL_AMBIENT_CONDITION;
	} else {
		abnormal_activity = ABNORMAL_AMBIENT_CONDITION;
	}
	/* convert to special bytes for backend */
	integer_temperature = (int)(object_celcius_temperature*10);
	MLX90614_special_bytes[0] = ((0xFF00 & integer_temperature) >> 8);
	MLX90614_special_bytes[1] = (0x00FF & integer_temperature);
	if (abnormal_activity == NORMAL_AMBIENT_CONDITION) {
		SSN_PREV_STATE = SSN_CURRENT_STATE;
		SSN_CURRENT_STATE = NORMAL_ACTIVITY_STATE;
		if (SSN_PREV_STATE != SSN_CURRENT_STATE) {
			Clear_LED_INDICATOR();
		}
	} else {
		SSN_PREV_STATE = SSN_CURRENT_STATE;
		SSN_CURRENT_STATE = ABNORMAL_ACTIVITY_STATE;
		if (SSN_PREV_STATE != SSN_CURRENT_STATE) {
			Clear_LED_INDICATOR();
		}
	}
}

void SSN_GET_OBJECT_TEMPERATURE_CONDITION_Thermistor(uint8_t TEMPERATURE_MIN_THRESHOLD, uint8_t TEMPERATURE_MAX_THRESHOLD) {
	// printf("there\n");
	float object_celcius_temperature = Thermistor_NTC_4092_50k_Get_Object_Temperature_In_Celcius();
	// printf("Thermistor Reading: %.2f\n", object_celcius_temperature);
	int integer_temperature;
	if (object_celcius_temperature > TEMPERATURE_MIN_THRESHOLD && object_celcius_temperature < TEMPERATURE_MAX_THRESHOLD){
		abnormal_activity = NORMAL_AMBIENT_CONDITION;
	} else {
		abnormal_activity = ABNORMAL_AMBIENT_CONDITION;
	}
	/* convert to special bytes for backend */
	integer_temperature = (int)(object_celcius_temperature*10);
	NTC_Thermistor_4092_50k_special_bytes[0] = ((0xFF00 & integer_temperature) >> 8);
	NTC_Thermistor_4092_50k_special_bytes[1] = (0x00FF & integer_temperature);
	if (abnormal_activity == NORMAL_AMBIENT_CONDITION) {
		SSN_PREV_STATE = SSN_CURRENT_STATE;
		SSN_CURRENT_STATE = NORMAL_AMBIENT_CONDITION;
		if (SSN_PREV_STATE != SSN_CURRENT_STATE) {
			Clear_LED_INDICATOR();
		}
	} else {
		SSN_PREV_STATE = SSN_CURRENT_STATE;
		SSN_CURRENT_STATE = ABNORMAL_AMBIENT_CONDITION;
		if (SSN_PREV_STATE != SSN_CURRENT_STATE) {
			Clear_LED_INDICATOR();
		}
	}
}

void SSN_RESET_AFTER_N_SECONDS(uint32_t seconds) {
	/* Check if we should reset (after given minutes when machines are not ON) */
	if (ssn_uptime_in_seconds > seconds) {
		printf("[LOG] Closing MQTT TCP Socket\n");
		close(TCP_SOCKET);
		printf("[LOG] Time to have some rest... I'll wake up in about 5 seconds...\n");
		SoftReset();
		while (1);
	}
	return;
}

void SSN_RESET_AFTER_N_SECONDS_IF_NO_MACHINE_ON(uint32_t seconds) {
	/* Check if we should reset (after given minutes when machines are not ON) */
	if (ssn_uptime_in_seconds > seconds && Machine_status[0] != MACHINE_ON && Machine_status[1] != MACHINE_ON && Machine_status[2] != MACHINE_ON && Machine_status[3] != MACHINE_ON) {
		printf("Time to have some rest... I'll wake up in about 5 seconds...\n");
		SoftReset();
		while (1);
	}
	return;
}

void SSN_REQUEST_Time_of_Day_AFTER_N_SECONDS(uint32_t seconds) {
	/* Check if we should reset (after given minutes when machines are not ON) */
	if (ssn_uptime_in_seconds % seconds==0 && ssn_uptime_in_seconds!=0) {
		printf("[LOG] Requesting Time of Day for Routing Syncing...\n");
		SSN_GET_TIMEOFDAY();
	}
}

void SSN_REQUEST_IP_From_DHCP_AFTER_N_SECONDS(uint32_t seconds) {
    /* Check if we should reset (after given minutes when machines are not ON) */
	if (ssn_uptime_in_seconds % seconds==0 && ssn_uptime_in_seconds!=0) {
		printf("[LOG] Requesting IP from DHCP After Lease Time Expired...\n");
        // close all existing sockets
        printf("[LOG] Closing all existing connections and sockets...\n");
        CloseMQTTClientConnectionAndSocket(&Client_MQTT, TCP_SOCKET);
        // get new IP from DHCP
       	SetupConnectionWithDHCP(SSN_MAC_ADDRESS);
        // create sockets and connections all over again
        SetupMQTTClientConnection(&MQTT_Network, &Client_MQTT, &MQTTOptions, SSN_SERVER_IP, NodeExclusiveChannel, SSN_RECEIVE_ASYNC_MESSAGE_OVER_MQTT);
	}
	return;
}

void SSN_RESET_IF_SOCKET_CORRUPTED(bool socket_is_fine) {
	if (!socket_is_fine) {
		SSN_PREV_STATE = SSN_CURRENT_STATE;
		SSN_CURRENT_STATE = NO_ETHERNET_STATE;
		if (SSN_PREV_STATE != SSN_CURRENT_STATE) {
			Clear_LED_INDICATOR();
		}
		printf("-> Socket Corrupted. Reinitializing SSN..\n");
		SoftReset();
	}
}

void led_blink_test() {
	SSN_Setup();
	Clear_LED_INDICATOR();
	while (true) {
		No_Ethernet_LED_INDICATE();
		printf("Trying to show a sign of life...\n");
		// sleep for a second
		sleep_for_microseconds(1000000);
	}
	// we should never get to this point
	return;
}

void current_test() {
	SSN_Setup();
	SSN_CURRENT_SENSOR_RATINGS[0] = 100;
	SSN_CURRENT_SENSOR_RATINGS[1] = 000;
	SSN_CURRENT_SENSOR_RATINGS[2] = 030;
	SSN_CURRENT_SENSOR_RATINGS[3] = 000;
	while (true) {
		Calculate_RMS_Current_On_All_Channels(SSN_CURRENT_SENSOR_RATINGS, 400, Machine_load_currents);
		// sleep for a second
		sleep_for_microseconds(1000000);
	}
	// we should never get to this point
	return;
}

//void network_test() {
//	SSN_Setup();
//	SSN_UDP_SOCKET = SetupConnectionWithStaticIPAndReturnSocket(SSN_UDP_SOCKET_NUM, SSN_MAC_ADDRESS, SSN_STATIC_IP, SSN_SUBNET_MASK, SSN_GATWAY_ADDRESS, SSN_DNS_ADDRESS);
//	uint8_t test_message_array[100] = "I am Annus Zulfiqar and I am trying to test this network";
//	uint8_t test_message_size = 56;
//	while (true) {
//		message_publish_status = SendMessage(SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT, test_message_array, test_message_size);
//		if (message_publish_status != SUCCESSS) {
//			printf("Socket Corrupted. Reinitializing..\n");
//			setup_Ethernet(5000000);
//			SSN_UDP_SOCKET = SetupConnectionWithStaticIPAndReturnSocket(SSN_UDP_SOCKET_NUM, SSN_MAC_ADDRESS, SSN_STATIC_IP, SSN_SUBNET_MASK, SSN_GATWAY_ADDRESS, SSN_DNS_ADDRESS);
//			printf("Reinitialization Successful.\n");
//		}
//		sleep_for_microseconds(1000000);
//	}
//	// we should never get to this point
//	return;
//}

void watchdog_test() {
	SSN_Setup();
	printf("################# Testing Watchdog #################\n");
	EnableWatchdog();
	int seconds = 1;
	while (true) {
		ServiceWatchdog();
		printf("Sleeping for %d seconds\n", seconds);
		sleep_for_microseconds(seconds * 1000000);
		seconds++;
	}
	return;
}

int DHT22_Sensor_Test() {
	SSN_Setup();
	while (true) {
		SSN_GET_AMBIENT_CONDITION(0, 100, 0, 100);
		ambient_condition_status(0, 100, 0, 100);
		sleep_for_microseconds(2000000);
	}
	return 1;
}

//int main() {
//	// Setup Smart Sense Node
//	SSN_Setup();
//	while(1) {
//		SSN_GET_OBJECT_TEMPERATURE_CONDITION_Thermistor(0, 100);
//		sleep_for_microseconds(2000000);
//	}
//	return 0;
//}

//unsigned char tempBuffer[BUFFER_SIZE] = {};
//unsigned char TargetName[40] = "m11.cloudmqtt.com";
//uint8_t DNS_ADDRESS[4] = {8, 8, 8, 8};
//uint8_t MQTT_IP[4];
//
//
//int main() {
//        // Basic setup for our SSN to work    
//        SSN_Setup();
//        SSN_COPY_MAC_FROM_MEMORY();
//        Ethernet_Register_MAC(SSN_MAC_ADDRESS);
//        Ethernet_set_Static_IP(SSN_STATIC_IP, SSN_SUBNET_MASK, SSN_GATWAY_ADDRESS);
//        printf("HELLOWORLD\n");
//        printf("***%d.%d.%d.%d\n", MQTT_IP[0], MQTT_IP[1], MQTT_IP[2], MQTT_IP[3]);
//
//        Network n;
//        n.my_socket = 0;
//        DNS_init(1, tempBuffer);
//
//        while(1) {
//                T5CON = 0x8000;
//                TMR5 = 0;
//                strcpy(TargetName, "www.carepvtltd.com");
//                while ((DNS_run(DNS_ADDRESS, TargetName, MQTT_IP) == 0) && (TMR5 < PERIPH_CLK));
//                TMR5 = 0;
//                T5CONCLR = 0x8000;
//                printf("***%s\n", TargetName);
//                printf("***%d.%d.%d.%d\n", MQTT_IP[0], MQTT_IP[1], MQTT_IP[2], MQTT_IP[3]);
//                sleep_for_microseconds(1000000);
//        }
//        
//        return 1;
//}


//#define TCP_SOCKET  0
//#define MAX_LEN     10
//#define BUFFER_SIZE	2048
//#define targetPort  1883    // mqtt server port
//
//unsigned char targetIP[4] = {192, 168, 0, 120}; // mqtt server IP
//unsigned char tempBuffer[BUFFER_SIZE] = {};
//const char* cliendId = "4C:DF";
//
//typedef struct opts_struct {
//	char clientid[MAX_LEN];
//	int nodelimiter;
//	char delimiter[MAX_LEN];
//	enum QoS qos;
//	char username[MAX_LEN];
//	char password[MAX_LEN];
//	char host[4]; // this is an ip
//	int port;
//	int showtopics;
//} opts_struct;
//
//opts_struct opts = { 
//    .clientid="annusman", 
//    .nodelimiter=0, 
//    .delimiter="\n", 
//    .qos=QOS0, 
//    .username=NULL, 
//    .password=NULL, 
//    .host={192, 168, 0, 120},
//    .port=targetPort, 
//    .showtopics=0
//};
//
//// @brief messageArrived callback function
//void messageArrived(MessageData* md) {
//	unsigned char testbuffer[100];
//	MQTTMessage* message = md->message;
//
//	if (opts.showtopics) {
//		memcpy(testbuffer,(char*)message->payload,(int)message->payloadlen);
//		*(testbuffer + (int)(message->payloadlen) + 1) = "\n";
//		printf("%s\r\n",testbuffer);
//	}
//
//	if (opts.nodelimiter)
//		printf("%.*s", (int)message->payloadlen, (char*)message->payload);
//	else
//		printf("%.*s%s", (int)message->payloadlen, (char*)message->payload, opts.delimiter);
//}
//
//int MQTT_Skeleton() {
//    // Basic setup for our SSN to work
//    SSN_Setup();
//    SSN_COPY_MAC_FROM_MEMORY();
//    Ethernet_set_Static_IP(SSN_STATIC_IP, SSN_SUBNET_MASK, SSN_GATWAY_ADDRESS);
//
//    int rc = 0;
//	unsigned char buf[100];
//    Network n;
//	MQTTClient c;
//
//	NewNetwork(&n, TCP_SOCKET);
//	ConnectNetwork(&n, targetIP, targetPort);
//	MQTTClientInit(&c, &n, 1000, buf, 100, tempBuffer, 2048);
//
//	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
//	data.willFlag = 0;
//	data.MQTTVersion = 3;
//	data.clientID.cstring = opts.clientid;
//	data.username.cstring = opts.username;
//	data.password.cstring = opts.password;
//
//	data.keepAliveInterval = 60;
//	data.cleansession = 1;
//
//	rc = MQTTConnect(&c, &data);
//	printf("Connected %d\r\n", rc);
//	opts.showtopics = 1;
//
//	printf("Subscribing to %s\r\n", "annus");
//	rc = MQTTSubscribe(&c, "annus", opts.qos, messageArrived);
//	printf("Subscribed %d\r\n", rc);
//
//    while(1) {
//    	MQTTYield(&c, data.keepAliveInterval);
//    }
//    while(true);
//    return 1;
//}


//int main() {
//    setup_printf(19200);
//    PORTSetPinsDigitalOut(IOPORT_A, BIT_2);
//    while(1) {
//        /* Reset */
//        ClearPinForMicroseconds(IOPORT_A, BIT_2, 1100);  // Reset
//        SetPinForMicroseconds(IOPORT_A, BIT_2, 40);      // bring it back to high/idle
//        /* Wait for the device to show presence ~185us */
//        PORTSetPinsDigitalIn(IOPORT_A, BIT_2);           // Set pin for digital input
//        sleep_for_microseconds(185); 
//        
//        /* implement desi one-wire read here */
//        // it takes 5 milliseconds for read to complete
//        uint16_t total_read_time = 50;
//        uint16_t arr[5000];
//        uint32_t counts = 0;
//        T5CON = 0x8000; // enable Timer5, source PBCLK, 1:1 prescaler
//        while(TMR5 < PERIPH_CLK/5000) {
////            uint16_t count = 0;
////            while(PORTReadBits(IOPORT_A, BIT_2) > 0) {
////                count++;
////            }
////            arr[counts++] = count;
//            unsigned int sample = PORTReadBits(IOPORT_A, BIT_2);
//            counts++;
//        }
//        // turn off Timer5 so function is self-contained
//        T5CONCLR = 0x8000;
//
////        uint16_t chase = 0;
////        while(chase++ < counts) {
////            printf("%d ", arr[chase]);
////            sleep_for_microseconds(100);
////        }
//        printf("Sampled Collected: %d\n", counts);
//
//        sleep_for_microseconds(5000); // it takes 5 milliseconds to return readings from sensor
//        // bring it back to idle state
//        PORTSetPinsDigitalOut(IOPORT_A, BIT_2);
//        PORTSetBits(IOPORT_A, BIT_2);
//        SetPinForMicroseconds(IOPORT_A, BIT_2, 2000000);
//    }
//    return 1;
//}
//

//int main() {
//	// direct stdout to uart1. This will make printf() use UART1
//	setup_printf(115200);
//	open_UART1(115200);
////	__XC_UART = 1;
//	printf("Starting now...\n");
//	int i, num_char;
//	const int count = 20;
//	char buffer[count];
//	for (i=0; i<count; i++) {
//		buffer[i] = '\0';
//	}
//	uint8_t command[6] = {0x01, 0x03, 0x00, 0x2F, 0x00, 0x01};
//	while(1) {
//		printf("<< 0X0103002f0001\n");
//		SerialTransmit_UART1(command);
//		sleep_for_microseconds(1000);
//		// clear reception array
//		for (i=0; i<count; i++) {
//			buffer[i] = '\0';
//		}
//		num_char = SerialReceive_UART1(buffer, count);
//		if (num_char > 0) {
//			printf(">> ");
//			for(i=0; i<num_char; i++) {
//				printf("%x ", buffer[i]);
//			}
//			printf("\n");			
//		}
//		sleep_for_microseconds(500000);
//	}
//	return 0;
//}

//int main2() {
//	__XC_UART = 1;
////	setup_printf(115200);
//	open_UART1(115200);
//	open_UART2(115200);
//	printf("Starting now...\n");
//	// create and initiate the MODBUS MASTER object
//	ModbusMaster master;
//	if (modbusMasterInit(&master) != MODBUS_OK) {
//		printf("[LOG]: Can't initiate Master Object!\n");
//	}
//	unsigned int max_len, recv_len; 
//	uint8_t frame[256];
//	ModbusError error;
//	while (1) {
//		// create MODBUS request for reading holding registers
//		error = modbusBuildRequest03(&master, 23, 1, 1);
//		if (error != MODBUS_OK) {
//			printf("[LOG]: Can't build request using Master Object!\n");
//		}
//		// transmit function request to MODBUS slave over UART2
//		SerialTransmit_UART2("hello world\r\n"); // master.request.frame); // 
//		printf("Transmission Successful: ");
//		printf(master.request.frame);
//		printf("\n");
//		// receive response from the slave
//		recv_len = SerialReceive_UART2((char*)&frame, max_len);
//		if (recv_len > 0) {
//			printf("Reception Successful!: [%d bytes]", recv_len);
//			uint8_t i; for(i=0; i<recv_len; i++) {
//				printf("%d", frame[i]);
//			}
//			printf("\n");			
//		} else {
//			printf("Reception Failed!\n");
//		}
////		//Pass the frame to the library
////		master.response.frame = frame;
////		master.response.length = recv_len;
////		error = modbusParseResponse(&master);
////		if (error == MODBUS_OK) {
////			//Use the data
////			//see ModbusMaster::data
////			printf("MODBUS Data Received: ");
////			uint8_t i; for(i=0; i<recv_len; i++) {
////				printf("%d", frame[i]);
////			}
////			printf("\n");
////		} else if (error == MODBUS_ERROR_EXCEPTION) {
////			//Use the exception information
////			printf("slave threw an exception - %d\n", master.exception.code);
////		} else {
////			//Handle the other errors
////			//see ModbusError
////		}
//		sleep_for_microseconds(1000000);
//	}
//	return 0;
//}



