#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

#include "SSN_API.h"

/** DNS Variables */
uint8_t DEFAULT_SERVER_IP[4]	= {172, 16, 5, 100};
char MQTT_SERVER_DNS_STRING[22] = "mqtt.wisermachines.com";
/** SSN Server Address (in case DNS fails) */
uint8_t SSN_SERVER_IP[4]		= {172, 16, 5, 100}; // {34, 87, 92, 5}; // 
/** Static IP Assignment */
uint8_t SSN_STATIC_IP[4]		= {172, 16, 41, 129};
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
/** A boolean to flag that an MQTT message is to be sent now */
bool report_now = false;
/** Current State of the SSN. There is no state machine of the SSN but we still use this variable to keep track at some instances */
uint8_t SSN_CURRENT_STATE = NO_MAC_STATE, SSN_PREV_STATE;
/** Report Interval of SSN set according to the configurations passed to the SSN */
uint8_t SSN_REPORT_INTERVAL = 5;
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
/** SSN Temperature Thresholds */
uint8_t TEMPERATURE_MIN_THRESHOLD, TEMPERATURE_MAX_THRESHOLD;
/** SSN Humidity Thresholds */
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
uint8_t SSN_DEFAULT_MAC[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
/** SSN current MAC address. May hold the default MAC or the one received from SSN Server. The last two bytes are the SSN Identity */
uint8_t SSN_MAC_ADDRESS[6] = {0};
/** SSN temperature sensor reading bytes */
uint8_t temperature_bytes[2] = {0};
/** SSN relative humidity reading bytes */
uint8_t relative_humidity_bytes[2]= {0};
/** SSN abnormal activity byte */
uint8_t abnormal_activity;
/** A variable to maintain a count of how many messages we have sent */
uint32_t message_count = 0;
/** MQTT failure counts */
uint8_t mqtt_failure_counts;
/** Maximum allowed MQTT failure counts */
uint8_t MQTTallowedfailureCount;


void SSN_Setup() {
	// Setup calls for all our peripherals/devices
	setup_printf(115200);
	SSN_Show_Message();
	setup_EEPROM();
	setup_Ethernet(5000000);
	setup_Current_Sensors();
	setup_Temperature_Humidity_Sensor();
	setup_LED_Indicator();
	setup_Interrupts();
	setup_millisecond_timer_with_interrupt();
	fault_count = 0;
	// seed the random number generator using ADC reading on 4th input channel
	srand(ADC1BUF3);
}

void SSN_Show_Message() {
	printf("\n\n\n");
	printf("###############################################################################################################\n");
	printf("###############################################################################################################\n");
	printf("###############################################################################################################\n");
	printf("###############################################################################################################\n");
	printf("##                            __    _                              \n");
	printf("##                       _wr''        '-q__                        \n");
	printf("##                    _dP                 9m_                      \n");
	printf("##                  _#P                     9#_                    \n");
	printf("##                 d#@                       9#m                   \n");
	printf("##                d##                         ###                  \n");
	printf("##               J###                         ###L                 Authors: Annus Zulfiqar, Maryum Aamer\n");
	printf("##               {###K                       J###K                 Program Memory Used:  137KB/256KB (53%c)\n", 37);
	printf("##               ]####K      ___aaa___      J####F                 Data Memory Used:     13KB/64KB (21%c)\n", 37);
	printf("##           __gmM######_  w#P""   ""9#m  _d#####Mmw__                 Last Firmware Update: June 22nd, 2021\n");
	printf("##        _g##############mZ_         __g##############m_          www.wisermachines.com\n");
	printf("##      _d####M@PPPP@@M#######Mmp gm#########@@PPP9@M####m_        \n");
	printf("##     a###''          ,Z'#####@'' '######'/g          ''M#m       \n");
	printf("##    J#@'             0L  '*##     ##@'  J#              *#K      \n");
	printf("##    #'               '#    '_gmwgm_~    dF               `#_     \n");
	printf("##   7F                 '#_   ]#####F   _dK                 JE     \n");
	printf("##   ]                    *m__ ##### __g@'                   F     \n");
	printf("##                          'PJ#####LP'                            \n");
	printf("##    `                       0######_                      '      \n");
	printf("##                          _0########_                            \n");
	printf("##        .               _d#####^#####m__              ,          \n");
	printf("##         '*w_________am#####P'   ~9#####mw_________w*'           \n");
	printf("##             ''9@#####@M''           ''P@#####@M''               \n");
	printf("###############################################################################################################\n");
	printf("###############################################################################################################\n");
	printf("###############################################################################################################\n");
	printf("###############################################################################################################\n");
	printf("[LOG] Boot up in progress...\n");
}

void SSN_COPY_MAC_FROM_MEMORY(uint8_t* ssn_mac_address, uint8_t* ssn_default_mac, uint8_t* ssn_prev_state, uint8_t* ssn_current_state, char* node_exclusive_channel) {
	*ssn_prev_state = *ssn_current_state;
	*ssn_current_state = FindMACInFlashMemory(ssn_mac_address, ssn_default_mac, node_exclusive_channel);
	if (*ssn_prev_state != *ssn_current_state) {
		Clear_LED_INDICATOR();
	}
}

void SSN_GET_MAC(uint8_t* ssn_mac_address, uint8_t* ssn_prev_state, uint8_t* ssn_current_state, uint8_t allowed_mqtt_failure_counts, Network* net, MQTTClient* mqtt_client, 
	opts_struct* mqtt_options, MQTTPacket_connectData* mqtt_datapacket, uint8_t *ssn_server_ip, char* cliendId_node_exclusive_channel, void* messageArrivedoverMQTT) {
	uint16_t SendAfter = 0, mqtt_failed_count = 0;
	int message_publish_status;
	// When we will receive a MAC address (if we didn't have it), we will reset the controller
	while (*ssn_current_state == NO_MAC_STATE) {
		*ssn_prev_state = *ssn_current_state;
		*ssn_current_state = NO_MAC_STATE;
		if (*ssn_prev_state != *ssn_current_state) {
			Clear_LED_INDICATOR();
		}
		// request a MAC address after every 5 seconds
		if (SendAfter % 50 == 0) {
			message_publish_status = Send_GETMAC_Message(ssn_mac_address);
			if (message_publish_status != SUCCESSS) {
				message_publish_status = FAILURE;
                mqtt_failed_count++;
                printf("[ERROR] Message Publication to MQTT Broker Failed (Count = %d/%d)\n", mqtt_failed_count, allowed_mqtt_failure_counts);
            } 
            else {
                mqtt_failed_count = 0;
            }
		}
		// Give LED indication every second
		if (SendAfter % 10 == 0) {
			SSN_LED_INDICATE(*ssn_current_state);
		}
		SendAfter++;
		ServiceWatchdog();
		/** Replacing this with the next few lines of code */
//		// MQTT process handler
//		start_ms_timer_with_interrupt();
//		MQTTYield(&Client_MQTT, 50);
//		message_publish_status = SSN_Check_Connection_And_Reconnect(ssn_prev_state, ssn_current_state, message_publish_status, net, mqtt_client, mqtt_options, mqtt_datapacket, 
//			ssn_server_ip, cliendId_node_exclusive_channel, messageArrivedoverMQTT);
//		stop_ms_timer_with_interrupt();
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// MQTT background handler
		start_ms_timer_with_interrupt();
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Check if we failed to publish, check connections and reconnect if necessary
        		// check how many failure counts have we encountered and reconnect to broker if necessary
		if (mqtt_failed_count >= allowed_mqtt_failure_counts) {
			// assume our connection has broken, we'll reconnect at this point
			printf("[MQTT] SSN Message Publication to MQTT Broker Failed and Retry Exceeded...\n");
			message_publish_status = SSN_Check_Connection_And_Reconnect(ssn_prev_state, ssn_current_state, message_publish_status, net, mqtt_client, mqtt_options, mqtt_datapacket, 
				ssn_server_ip, cliendId_node_exclusive_channel, messageArrivedoverMQTT);			
			// Reset current state back to no MAC state
			*ssn_current_state = NO_MAC_STATE;
			// connection re-established, exit fault code and reset failure counts
			mqtt_failed_count = 0;
		}
    	MQTTYield(&Client_MQTT, 100);
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		stop_ms_timer_with_interrupt();
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 100 milliseconds
		sleep_for_microseconds(100000);
	}
	//	return message_publish_status;
}

void SSN_GET_CONFIG(uint8_t* ssn_mac_address, uint8_t* ssn_config, uint8_t* ssn_report_interval, uint8_t* temperature_min_threshold, uint8_t* temperature_max_threshold, 
	uint8_t* humidity_min_threshold, uint8_t* humidity_max_threshold, uint8_t* ssn_current_sensor_ratings, float* ssn_current_sensor_thresholds,  uint8_t* ssn_current_sensor_maxloads, 
	float* ssn_current_sensor_voltage_scalars, uint8_t* ssn_prev_state, uint8_t* ssn_current_state, bool* sensor_configurations_received, uint8_t allowed_mqtt_failure_counts, Network* net, 
	MQTTClient* mqtt_client, opts_struct* mqtt_options, MQTTPacket_connectData* mqtt_datapacket, uint8_t *ssn_server_ip, char* cliendId_node_exclusive_channel, void* messageArrivedoverMQTT) {
	int message_publish_status;
	// Find configurations in EEPROM
	*ssn_prev_state = *ssn_current_state;
	*ssn_current_state = FindSensorConfigurationsInFlashMemory(ssn_config, ssn_report_interval, temperature_min_threshold, temperature_max_threshold, humidity_min_threshold,
		humidity_max_threshold, ssn_current_sensor_ratings, ssn_current_sensor_thresholds, ssn_current_sensor_maxloads, ssn_current_sensor_voltage_scalars);
	uint16_t SendAfter = 0, mqtt_failed_count = 0;
	while (*ssn_current_state == NO_CONFIG_STATE) {
		*ssn_prev_state = *ssn_current_state;
		*ssn_current_state = NO_CONFIG_STATE;
		if (*ssn_prev_state != *ssn_current_state) {
			Clear_LED_INDICATOR();
		}
		if (*sensor_configurations_received) { break; }
		// request a Configuration after every 5 seconds
		if (SendAfter % 50 == 0) {
			message_publish_status = Send_GETCONFIG_Message(ssn_mac_address);
			if (message_publish_status != SUCCESSS) {
				message_publish_status = FAILURE;
                mqtt_failed_count++;
                printf("[ERROR] Message Publication to MQTT Broker Failed (Count = %d/%d)\n", mqtt_failed_count, allowed_mqtt_failure_counts);
            } 
            else {
                mqtt_failed_count = 0;
            }
		}
		// Give LED indication every second
		if (SendAfter % 10 == 0) {
			SSN_LED_INDICATE(*ssn_current_state);
		}
		SendAfter++;
		ServiceWatchdog();
		/** Replacing this with the next few lines of code */
//		// MQTT process handler
//		start_ms_timer_with_interrupt();
//		MQTTYield(&Client_MQTT, 50);
//		message_publish_status = SSN_Check_Connection_And_Reconnect(ssn_prev_state, ssn_current_state, message_publish_status, net, mqtt_client, mqtt_options, mqtt_datapacket, 
//			ssn_server_ip, cliendId_node_exclusive_channel, messageArrivedoverMQTT);
//		stop_ms_timer_with_interrupt();
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// MQTT background handler
		start_ms_timer_with_interrupt();
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Check if we failed to publish, check connections and reconnect if necessary
		// check how many failure counts have we encountered and reconnect to broker if necessary
		if (mqtt_failed_count >= allowed_mqtt_failure_counts) {
			// assume our connection has broken, we'll reconnect at this point
			printf("[MQTT] SSN Message Publication to MQTT Broker Failed and Retry Exceeded...\n");
			message_publish_status = SSN_Check_Connection_And_Reconnect(ssn_prev_state, ssn_current_state, message_publish_status, net, mqtt_client, mqtt_options, mqtt_datapacket, 
				ssn_server_ip, cliendId_node_exclusive_channel, messageArrivedoverMQTT);
			// connection re-established, exit fault code and reset failure counts
			mqtt_failed_count = 0;
		}
    	MQTTYield(&Client_MQTT, 100);
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		stop_ms_timer_with_interrupt();
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 100 milliseconds
		sleep_for_microseconds(100000);
	}
	//	return message_publish_status;
}

void SSN_GET_TIMEOFDAY(uint8_t* ssn_mac_address, uint8_t* ssn_prev_state, uint8_t* ssn_current_state, bool* timeofday_received, uint8_t allowed_mqtt_failure_counts, Network* net, 
	MQTTClient* mqtt_client, opts_struct* mqtt_options, MQTTPacket_connectData* mqtt_datapacket, uint8_t *ssn_server_ip, char* cliendId_node_exclusive_channel, void* messageArrivedoverMQTT) {
	*timeofday_received = false;
	uint16_t SendAfter = 0, mqtt_failed_count = 0;
	int message_publish_status;
	while (1) {
		*ssn_prev_state = *ssn_current_state;
		*ssn_current_state = NO_CONFIG_STATE;
		if (*ssn_prev_state != *ssn_current_state) {
			Clear_LED_INDICATOR();
		}
		if (*timeofday_received) {
			// initialize SSN's uptime
			ssn_uptime_in_seconds = 0;
			*ssn_prev_state = *ssn_current_state;
			*ssn_current_state = NORMAL_ACTIVITY_STATE;
			if (*ssn_prev_state != *ssn_current_state) {
				Clear_LED_INDICATOR();
			}
			break;
		}
		// request time of day after every 5 seconds
		if (SendAfter % 50 == 0 && !(*timeofday_received)) {
			message_publish_status = Send_GETTimeOfDay_Message(ssn_mac_address);
			if (message_publish_status != SUCCESSS) {
				message_publish_status = FAILURE;
                mqtt_failed_count++;
                printf("[ERROR] Message Publication to MQTT Broker Failed (Count = %d/%d)\n", mqtt_failed_count, allowed_mqtt_failure_counts);
            } 
            else {
                mqtt_failed_count = 0;
            }
		}
		// Give LED indication every second
		if (SendAfter % 10 == 0) {
			SSN_LED_INDICATE(*ssn_current_state);
		}
		SendAfter++;
		ServiceWatchdog();
		/** Replacing this with the next few lines of code */
//		// MQTT process handler
//		start_ms_timer_with_interrupt();
//		MQTTYield(&Client_MQTT, 50);
		message_publish_status = SSN_Check_Connection_And_Reconnect(ssn_prev_state, ssn_current_state, message_publish_status, net, mqtt_client, mqtt_options, mqtt_datapacket, 
			ssn_server_ip, cliendId_node_exclusive_channel, messageArrivedoverMQTT);
//		stop_ms_timer_with_interrupt();
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// MQTT background handler
		start_ms_timer_with_interrupt();
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Check if we failed to publish, check connections and reconnect if necessary
		// check how many failure counts have we encountered and reconnect to broker if necessary
		if (mqtt_failed_count >= allowed_mqtt_failure_counts) {
			// assume our connection has broken, we'll reconnect at this point
			printf("[MQTT] SSN Message Publication to MQTT Broker Failed and Retry Exceeded...\n");
            message_publish_status = SSN_Check_Connection_And_Reconnect(ssn_prev_state, ssn_current_state, message_publish_status, net, mqtt_client, mqtt_options, mqtt_datapacket, 
				ssn_server_ip, cliendId_node_exclusive_channel, messageArrivedoverMQTT);
			// connection re-established, exit fault code and reset failure counts
			mqtt_failed_count = 0;
		}
    	MQTTYield(&Client_MQTT, 100);
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		stop_ms_timer_with_interrupt();
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 100 milliseconds
		sleep_for_microseconds(100000);
	}
	//	return message_publish_status;
}

int SSN_Check_Connection_And_Reconnect(uint8_t* ssn_prev_state, uint8_t* ssn_current_state, int return_code, Network* net, MQTTClient* mqtt_client, opts_struct* mqtt_options, MQTTPacket_connectData* mqtt_datapacket, uint8_t *ssn_server_ip, 
	char* cliendId_node_exclusive_channel, void* messageArrivedoverMQTT) {
	int rc;
	if (return_code != SUCCESSS) {
		printf("[**MQTT**] Bad Return Code: %d\n", return_code);
		SSN_CHECK_ETHERNET_CONNECTION(ssn_prev_state, ssn_current_state);
		CloseMQTTClientConnectionAndSocket(mqtt_client, TCP_SOCKET);
		rc = SetupMQTTClientConnection(net, mqtt_client, mqtt_options, mqtt_datapacket, ssn_server_ip, cliendId_node_exclusive_channel, messageArrivedoverMQTT);
		// printf("rc %d\n", rc);
		return rc;
	}
	return SUCCESSS;
}

void SSN_RECEIVE_ASYNC_MESSAGE_OVER_MQTT(MessageData* md) {
	unsigned char testbuffer[BUFFER_SIZE];
	MQTTMessage* message = md->message;
	// printf("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	clear_array(testbuffer, 100);
	/** Culprit */
	start_ms_timer_with_interrupt();
    MQTTYield(&Client_MQTT, 100);
    stop_ms_timer_with_interrupt();
	/** Culprit */
	if (MQTTOptions.showtopics) {
		memcpy(testbuffer, (char*) message->payload, (int) message->payloadlen);
		*(testbuffer + (int) (message->payloadlen) + 1) = '\n';
		uint8_t received_message_id;
		uint32_t TimeOFDayTick;
		received_message_id = decipher_received_message(testbuffer, params);
		// based on which message was received (received_message_id), we extract and save the data
		switch (received_message_id) {
			case SET_MAC_MESSAGE_ID:
				printf("<- SET_MAC MESSAGE RECEIVED: %X:%X:%X:%X:%X:%X\n", params[0], params[1], params[2], params[3], params[4], params[5]);
				printf("Resetting Controller Now...\n");
				// write the new MAC addresses to designated location in EEPROM
				EEPROM_Write_Array(EEPROM_MAC_BLOCK, EEPROM_MAC_LOC, params, EEPROM_MAC_SIZE);
				// convert and write the string of MAC address to designated location in EEPROM. 
				// That will be used by MQTT API to connect to special channels for this node
				sprintf(NodeExclusiveChannel, "%02X:%02X:%02X:%02X:%02X:%02X", params[0], params[1], params[2], params[3], params[4], params[5]);
				EEPROM_Write_Array(EEPROM_MAC_STRING_BLOCK, EEPROM_MAC_STRING_LOC, NodeExclusiveChannel, EEPROM_MAC_STRING_SIZE);
				// close MQTT connection
				CloseMQTTClientConnectionAndSocket(&Client_MQTT, TCP_SOCKET);
				// reset the SSN from software
				SoftReset();
				while (1);
				break;

			case SET_TIMEOFDAY_MESSAGE_ID:
				if (!TimeOfDay_received) {
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
				EEPROM_Write_Array(EEPROM_CONFIG_BLOCK, EEPROM_CONFIG_LOC, params, EEPROM_CONFIG_SIZE);
				// Copy received configurations to the SSN_CONFIG array
				int i; for (i = 0; i < EEPROM_CONFIG_SIZE; i++) {
					SSN_CONFIG[i] = params[i];
				}
				// Copy from the configurations, the sensor ratings, thresholds and maximum load values to our variables
				for (i = 0; i < NO_OF_MACHINES; i++) {
					/* Get the parameters from the Configurations */
					SSN_CURRENT_SENSOR_RATINGS[i] = SSN_CONFIG[4 * i + 0];
					SSN_CURRENT_SENSOR_THRESHOLDS[i] = SSN_CONFIG[4 * i + 1] / 10.0f;
					SSN_CURRENT_SENSOR_MAXLOADS[i] = SSN_CONFIG[4 * i + 2];
					if (SSN_CONFIG[4 * i + 3] == 0) {
						SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[i] = 0.333;
					} else if (SSN_CONFIG[4 * i + 3] == 1) {
						SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[i] = 1.00;
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
					"     >> S1-Rating: %03d Arms | S1-Scalar: %.3f Vrms | M1-Threshold: %.3f Arms | M1-Maxload: %03d Arms |\n"
					"     >> S2-Rating: %03d Arms | S2-Scalar: %.3f Vrms | M2-Threshold: %.3f Arms | M2-Maxload: %03d Arms |\n"
					"     >> S3-Rating: %03d Arms | S3-Scalar: %.3f Vrms | M3-Threshold: %.3f Arms | M3-Maxload: %03d Arms |\n"
					"     >> S4-Rating: %03d Arms | S4-Scalar: %.3f Vrms | M4-Threshold: %.3f Arms | M4-Maxload: %03d Arms |\n"
					"     >> MIN TEMP : %03d C    | MAX TEMP : %03d C    |\n"
					"     >> MIN RH   : %03d %    | MIN RH   : %03d %    |\n"
					"     >> Report   : %d seconds\n",
					SSN_CURRENT_SENSOR_RATINGS[0], SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[0], SSN_CURRENT_SENSOR_THRESHOLDS[0], SSN_CURRENT_SENSOR_MAXLOADS[0],
					SSN_CURRENT_SENSOR_RATINGS[1], SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[1], SSN_CURRENT_SENSOR_THRESHOLDS[1], SSN_CURRENT_SENSOR_MAXLOADS[1],
					SSN_CURRENT_SENSOR_RATINGS[2], SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[2], SSN_CURRENT_SENSOR_THRESHOLDS[2], SSN_CURRENT_SENSOR_MAXLOADS[2],
					SSN_CURRENT_SENSOR_RATINGS[3], SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[3], SSN_CURRENT_SENSOR_THRESHOLDS[3], SSN_CURRENT_SENSOR_MAXLOADS[3],
					TEMPERATURE_MIN_THRESHOLD, TEMPERATURE_MAX_THRESHOLD, RELATIVE_HUMIDITY_MIN_THRESHOLD, RELATIVE_HUMIDITY_MAX_THRESHOLD, SSN_REPORT_INTERVAL);
				// Reset Machine States 
				for (i = 0; i < NO_OF_MACHINES; i++) {
					Machine_status[i] = SENSOR_NOT_CONNECTED;
				}
				MQTTallowedfailureCount = MQTTallowedfailureCounts(SSN_REPORT_INTERVAL);
				CONFIG_received = true;
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
				// Close MQTT connection, TCP socket connection with the broker as well as the socket
				CloseMQTTClientConnectionAndSocket(&Client_MQTT, TCP_SOCKET);
				// reset the SSN
				printf("[DEBUG] Resetting Controller Now...\n");
				sleep_for_microseconds(1000000);
				SoftReset();
				while (1);
				break;
				
            case RETRIEVE_CURRENT_CONFIG_MESSAGE_ID:
				printf("<- RETRIEVE CURRENT CONFIG MESSAGE RECEIVED\n");
				printf("[LOG] Sending Sensor Configuration to SSN Server: \n"
					"     >> S1-Rating: %d Arms | M1-Threshold: %.2f Arms | M1-Maxload: %d Arms | S1-Scalar: %d Vrms | \n"
					"     >> S2-Rating: %d Arms | M2-Threshold: %.2f Arms | M2-Maxload: %d Arms | S2-Scalar: %d Vrms | \n"
					"     >> S3-Rating: %d Arms | M3-Threshold: %.2f Arms | M3-Maxload: %d Arms | S3-Scalar: %d Vrms | \n"
					"     >> S4-Rating: %d Arms | M4-Threshold: %.2f Arms | M4-Maxload: %d Arms | S4-Scalar: %d Vrms | \n"
					"     >> MIN TEMP : %03d C    | MAX TEMP : %03d C    |\n"
					"     >> MIN RH   : %03d %    | MIN RH   : %03d %    |\n"
					"     >> Report   : %d seconds\n",
					SSN_CONFIG[0], SSN_CONFIG[1], SSN_CONFIG[2], SSN_CONFIG[3],
					SSN_CONFIG[4], SSN_CONFIG[5], SSN_CONFIG[6], SSN_CONFIG[7],
					SSN_CONFIG[8], SSN_CONFIG[9], SSN_CONFIG[10], SSN_CONFIG[11],
					SSN_CONFIG[12], SSN_CONFIG[13], SSN_CONFIG[14], SSN_CONFIG[15],
					SSN_CONFIG[16], SSN_CONFIG[17], SSN_CONFIG[18], SSN_CONFIG[19], SSN_CONFIG[20]);
                Send_RETRIEVECONFIG_Message(SSN_MAC_ADDRESS, SSN_CONFIG);
				break;
				
            default:
				break;
		}
		clear_array(testbuffer, 100);
	}
}

void SSN_CHECK_ETHERNET_CONNECTION(uint8_t* ssn_prev_state, uint8_t* ssn_current_state) {
	bool LinkWasOkay = true;
	// Check Ethernet Physical Link Status before sending message
	while (Ethernet_get_physical_link_status() == PHY_LINK_OFF) {
		*ssn_prev_state = *ssn_current_state;
		*ssn_current_state = NO_ETHERNET_STATE;
		if (*ssn_prev_state != *ssn_current_state) {
			LinkWasOkay = false;
			Clear_LED_INDICATOR();
		}
		printf("[LOG] No Ethernet Physical Link Connection :/ \n");
		SSN_LED_INDICATE(*ssn_current_state);
		// Service the watchdog timer to make sure we don't reset 
		ServiceWatchdog();
		sleep_for_microseconds(500000);
	}
	if (!LinkWasOkay) {
		*ssn_prev_state = *ssn_current_state;
		*ssn_current_state = NORMAL_ACTIVITY_STATE;
		if (*ssn_prev_state != *ssn_current_state) {
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

void SSN_GET_AMBIENT_CONDITION(uint8_t* temperature_byte_array, uint8_t* humidity_byte_array, uint8_t temperature_min_threshold, uint8_t temperature_max_threshold, uint8_t humidity_min_threshold, 
	uint8_t humidity_max_threshold, uint8_t* ssn_prev_state, uint8_t* ssn_current_state, uint8_t* abnormal_activity_flag) {
	/** SSN temperature and humidity reading successful/unsuccessful status bit */
	int8_t temp_humidity_recv_status;
	clear_array(temperature_byte_array, 2);
	clear_array(humidity_byte_array, 2);
#ifdef TH_AM2320
	temp_humidity_recv_status = sample_Temperature_Humidity_bytes_using_AM2320(temperature_byte_array, humidity_byte_array);
	printf("Temperature = %.2f Degrees Celcius, Humidity = %.2f Percent\n", (float)((temperature_byte_array[0] << 8) | temperature_bytes[1])/10.0f, 
		(float)((humidity_byte_array[0] << 8) | humidity_byte_array[1])/10.0f);
#endif
#ifdef TH_DHT22
	temp_humidity_recv_status = sample_Temperature_Humidity_bytes_using_DHT22(temperature_byte_array, humidity_byte_array);
#endif
	if (temp_humidity_recv_status == SENSOR_READ_ERROR) {
		*abnormal_activity_flag = TEMP_SENSOR_READ_ERROR_CONDITION;
#ifdef TH_DHT22_DEBUG
		printf("DHT22 Read Timeout Occurred\n");
#endif
		return;
	}
	if (temp_humidity_recv_status == SENSOR_CRC_ERROR) {
		*abnormal_activity_flag = TEMP_SENSOR_CRC_ERROR_CONDITION;
#ifdef TH_DHT22_DEBUG
		printf("DHT22 CheckSum Error Occurred\n");
#endif
		return;
	}
	*abnormal_activity_flag = ambient_condition_status(temperature_min_threshold, temperature_max_threshold, humidity_min_threshold, humidity_max_threshold);
	if (*abnormal_activity_flag == NORMAL_AMBIENT_CONDITION) {
		*ssn_prev_state = *ssn_current_state;
		*ssn_current_state = NORMAL_ACTIVITY_STATE;
		if (*ssn_prev_state != *ssn_current_state) {
			Clear_LED_INDICATOR();
		}
	} else {
		*ssn_prev_state = *ssn_current_state;
		*ssn_current_state = NORMAL_ACTIVITY_STATE;
		if (*ssn_prev_state != *ssn_current_state) {
			Clear_LED_INDICATOR();
		}
	}
	return;
}

void SSN_GET_OBJECT_TEMPERATURE_CONDITION_IR(uint8_t* temperature_byte_array, uint8_t temperature_min_threshold, uint8_t temperature_max_threshold, uint8_t* ssn_prev_state, 
	uint8_t* ssn_current_state, uint8_t* abnormal_activity_flag) {
	// printf("here\n");
	float object_celcius_temperature = MLX90614_Read_Temperature_Object_Celcius();
	int integer_temperature;
	if (object_celcius_temperature == MLX90614_COMM_ERROR_CODE) {
		*abnormal_activity_flag = TEMP_SENSOR_READ_ERROR_CONDITION;
	} else if (object_celcius_temperature > temperature_min_threshold && object_celcius_temperature < temperature_max_threshold) {
		*abnormal_activity_flag = NORMAL_AMBIENT_CONDITION;
	} else {
		*abnormal_activity_flag = ABNORMAL_AMBIENT_CONDITION;
	}
	/* convert to special bytes for backend */
	integer_temperature = (int) (object_celcius_temperature * 10);
	temperature_byte_array[0] = ((0xFF00 & integer_temperature) >> 8);
	temperature_byte_array[1] = (0x00FF & integer_temperature);
	if (*abnormal_activity_flag == NORMAL_AMBIENT_CONDITION) {
		*ssn_prev_state = *ssn_current_state;
		*ssn_current_state = NORMAL_ACTIVITY_STATE;
		if (*ssn_prev_state != *ssn_current_state) {
			Clear_LED_INDICATOR();
		}
	} else {
		*ssn_prev_state = *ssn_current_state;
		*ssn_current_state = ABNORMAL_ACTIVITY_STATE;
		if (*ssn_prev_state != *ssn_current_state) {
			Clear_LED_INDICATOR();
		}
	}
}

void SSN_GET_OBJECT_TEMPERATURE_CONDITION_Thermistor(uint8_t* temperature_byte_array, uint8_t temperature_min_threshold, uint8_t temperature_max_threshold, uint8_t* ssn_prev_state, 
	uint8_t* ssn_current_state, uint8_t* abnormal_activity_flag) {
	float object_celcius_temperature = Thermistor_NTC_4092_50k_Get_Object_Temperature_In_Celcius();
    // get the average value of last N temperature readings
    object_celcius_temperature = average_value_of_temperature(object_celcius_temperature); 
    printf("Thermistor Reading: %.2f\n", object_celcius_temperature);
    int integer_temperature;
	if (object_celcius_temperature > temperature_min_threshold && object_celcius_temperature < temperature_max_threshold){
		*abnormal_activity_flag = NORMAL_AMBIENT_CONDITION;
	} else {
		*abnormal_activity_flag = ABNORMAL_AMBIENT_CONDITION;
	}
	/* convert to special bytes for backend */
	integer_temperature = (int)(object_celcius_temperature*10);
	temperature_byte_array[0] = ((0xFF00 & integer_temperature) >> 8);
	temperature_byte_array[1] = (0x00FF & integer_temperature);
	if (*abnormal_activity_flag == NORMAL_AMBIENT_CONDITION) {
		*ssn_prev_state = *ssn_current_state;
		*ssn_current_state = NORMAL_ACTIVITY_STATE;
		if (*ssn_prev_state != *ssn_current_state) {
			Clear_LED_INDICATOR();
		}
	} else {
		*ssn_prev_state = *ssn_current_state;
		*ssn_current_state = ABNORMAL_ACTIVITY_STATE;
		if (*ssn_prev_state != *ssn_current_state) {
			Clear_LED_INDICATOR();
		}
	}
}

void SSN_RESET_AFTER_N_SECONDS(uint32_t ssn_uptime_right_now, uint32_t seconds) {
	/* Check if we should reset (after given minutes when machines are not ON) */
	if (ssn_uptime_right_now > seconds) {
		printf("[LOG] Closing MQTT TCP Socket\n");
		close(TCP_SOCKET);
		printf("[LOG] Time to have some rest... I'll wake up in about 5 seconds...\n");
		SoftReset();
		while (1);
	}
	return;
}

void SSN_RESET_AFTER_N_SECONDS_IF_NO_MACHINE_ON(uint32_t ssn_uptime_right_now, uint32_t seconds) {
	/* Check if we should reset (after given minutes when machines are not ON) */
	if (ssn_uptime_right_now > seconds && Machine_status[0] != MACHINE_ON && Machine_status[1] != MACHINE_ON && Machine_status[2] != MACHINE_ON && Machine_status[3] != MACHINE_ON) {
		printf("Time to have some rest... I'll wake up in about 5 seconds...\n");
		SoftReset();
		while (1);
	}
	return;
}

void SSN_REQUEST_Time_of_Day_AFTER_N_SECONDS(uint32_t ssn_uptime_right_now, uint32_t seconds, uint8_t allowed_mqtt_failure_counts, uint8_t* ssn_mac_address, uint8_t* ssn_prev_state, uint8_t* ssn_current_state, 
	bool* timeofday_received, Network* net, MQTTClient* mqtt_client, opts_struct* mqtt_options, MQTTPacket_connectData* mqtt_datapacket, uint8_t *ssn_server_ip, 
	char* cliendId_node_exclusive_channel, void* messageArrivedoverMQTT) {
	/* Check if we should reset (after given minutes when machines are not ON) */
	if ((ssn_uptime_right_now % seconds) == 0 && ssn_uptime_right_now != 0) {
		printf("[LOG] Requesting Time of Day for Routing Syncing...\n");
		SSN_GET_TIMEOFDAY(ssn_mac_address, ssn_prev_state, ssn_current_state, timeofday_received, allowed_mqtt_failure_counts, net, mqtt_client, mqtt_options, mqtt_datapacket, ssn_server_ip, 
			cliendId_node_exclusive_channel, messageArrivedoverMQTT);
	}
}

void SSN_REQUEST_IP_From_DHCP_AFTER_N_SECONDS(uint8_t* ssn_mac_address, uint32_t ssn_uptime_right_now, uint32_t seconds, char* node_exclusive_channel, 
	wiz_NetInfo* wiznet_network_information_object, Network* net, MQTTClient* mqtt_client, opts_struct* mqtt_options, MQTTPacket_connectData* mqtt_datapacket, uint8_t *ssn_server_ip, 
	char* cliendId_node_exclusive_channel, void* messageArrivedoverMQTT) {
	/* Check if we should reset (after given minutes when machines are not ON) */
	if (ssn_uptime_right_now != 0 && (ssn_uptime_right_now % seconds) == 0) {
		printf("[LOG] Requesting IP from DHCP After Lease Time Expired...\n");
		// close all existing sockets
		printf("[LOG] Closing all existing connections and sockets...\n");
		CloseMQTTClientConnectionAndSocket(mqtt_client, TCP_SOCKET);
		// get new IP from DHCP
		SetupConnectionWithDHCP(ssn_mac_address, node_exclusive_channel, wiznet_network_information_object);
		// create sockets and connections all over again
		SetupMQTTClientConnection(net, mqtt_client, mqtt_options, mqtt_datapacket, ssn_server_ip, cliendId_node_exclusive_channel, messageArrivedoverMQTT);
	}
	return;
}

void SSN_RESET_IF_SOCKET_CORRUPTED(bool socket_is_fine, uint8_t* ssn_current_state, uint8_t* ssn_prev_state) {
	if (!socket_is_fine) {
		*ssn_prev_state = *ssn_current_state;
		*ssn_current_state = NO_ETHERNET_STATE;
		if (*ssn_prev_state != *ssn_current_state) {
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
//
//void current_test() {
//	SSN_Setup();
//	SSN_CURRENT_SENSOR_RATINGS[0] = 100;
//	SSN_CURRENT_SENSOR_RATINGS[1] = 000;
//	SSN_CURRENT_SENSOR_RATINGS[2] = 030;
//	SSN_CURRENT_SENSOR_RATINGS[3] = 000;
//	while (true) {
//		Calculate_RMS_Current_On_All_Channels(SSN_CURRENT_SENSOR_RATINGS, 400, Machine_load_currents);
//		// sleep for a second
//		sleep_for_microseconds(1000000);
//	}
//	// we should never get to this point
//	return;
//}

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
//
//void watchdog_test() {
//	SSN_Setup();
//	printf("################# Testing Watchdog #################\n");
//	EnableWatchdog();
//	int seconds = 1;
//	while (true) {
//		ServiceWatchdog();
//		printf("Sleeping for %d seconds\n", seconds);
//		sleep_for_microseconds(seconds * 1000000);
//		seconds++;
//	}
//	return;
//}
//
//int DHT22_Sensor_Test() {
//	SSN_Setup();
//	while (true) {
//		SSN_GET_AMBIENT_CONDITION(0, 100, 0, 100);
//		ambient_condition_status(0, 100, 0, 100);
//		sleep_for_microseconds(2000000);
//	}
//	return 1;
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



