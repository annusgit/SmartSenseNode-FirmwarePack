#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

#pragma config FWDTEN		= OFF			// Turn off watchdog timer
#pragma config WDTPS		= PS32768		// Watchdog timer period
#pragma config FSOSCEN		= OFF			// Secondary Oscillator Enable (Disabled)
#pragma config FNOSC        = FRCPLL		// Select 8MHz internal Fast RC (FRC) oscillator with PLL
#pragma config FPLLIDIV     = DIV_2         // Divide PLL input (FRC) -> 4MHz
#pragma config FPLLMUL		= MUL_15		// PLL Multiplier -> 60MHz
#pragma config FPLLODIV		= DIV_1         // PLL Output Divider -> 60MHz
#pragma config FPBDIV       = DIV_2         // Peripheral Clock divisor -> 30MHz
#pragma config ICESEL       = ICS_PGx2		// ICE/ICD Comm Channel Select (Communicate on PGEC2/PGED2)
#pragma config JTAGEN		= OFF           // JTAG Enable (Disabled)

#include "SSN_API/SSN_API.h"

/** A millisecond timer interrupt required for DHCP and MQTT Yielding functions */
void __ISR(_TIMER_2_VECTOR, IPL4SOFT) Timer2IntHandler(void) {
	// clear timer 2 interrupt flag
	IFS0bits.T2IF = 0x00;
	// millisecond ticks for DHCP
	msTicks++; /* increment counter necessary in Delay()*/
	// millisecond ticks for MQTT
	MilliTimer_Handler();
	////////////////////////////////////////////////////////
	// SHOULD BE Added DHCP Timer Handler your 1s tick timer
	if (msTicks % 1000 == 0) {
		DHCP_time_handler();
		/* Give the Ethernet Indication */
		// No_Ethernet_LED_INDICATE();
		// printf("\n(LOG): One Second Passed in Millisecond Interrupt Handler\n");
	}
	//////////////////////////////////////////////////////
}

/** Half-Second interrupt that controls our send message routine of the SSN. Half-second and not one second is because we can not set an interrupt of up to 1 second with the current clock of the SSN. 
 * We only start this interrupt service once we have Ethernet configured and all self-tests are successful. The message to be sent is constructed every half a second in the main function and only reported 
 * to the server after every "SSN_REPORT_INTERVAL" seconds. */
void __ISR(_TIMER_1_VECTOR, IPL4SOFT) Timer1IntHandler_SSN_Hearbeat(void) {
	// clear timer 1 interrupt flag, IFS0<4>
	IFS0bits.T1IF = 0x00;
	// Indicate the status of SSN from the SSN LED after every half second
	SSN_LED_INDICATE(SSN_CURRENT_STATE);
	// check of we have reached one second interval (because two half-seconds make one second)
	half_second_counter++;
	if (half_second_counter >= interrupts_per_second) {
		// reset half second counter
		half_second_counter = 0;
		// add a second to report counter
		report_counter++;
		// increment global uptime in seconds
		ssn_uptime_in_seconds++;
		ssn_dynamic_clock++;
		// Is it time to report?
		if (report_counter >= SSN_REPORT_INTERVAL) {
			// Reset the reporting counter
			report_counter = 0;
			report_now = true;
		}
	}
}

/** 
 * The main loop of SSN operation. It calls the following functions in order:
 *      - Sets up all required peripherals
 *      - Runs system tests for checking:
 *          -# EEPROM Read/Write
 *          -# Temperature and Humidity Sensor
 *          -# Ethernet Physical Connection
 *      - Finds MAC address in EEPROM; if available, assigns it to SSN; if not available, assigns default MAC address to SSN
 *      - Sets up Ethernet connection using which ever MAC address was selected
 *      - If using default MAC address, SSN sends periodic GET_MAC requests to SSN Server until it successfully retrieves one and resets self
 *      - Waits for five seconds for new Current Sensor Configurations from SSN Server. These cannot be reprogrammed after this five seconds window
 *      - If new configurations received, assigns them to SSN, writes them to EEPROM and proceeds
 *      - If new configurations not received, finds Current Sensor Configurations in EEPROM; if available, assigns them to SSN; 
		  if not available SSN sends periodic GET_CONFIG requests to SSN Server until it successfully retrieves one and writes them to EEPROM and proceeds
 *      - SSN sends periodic GET_TimeOfDay requests to SSN Server until it successfully retrieves it and proceeds
 *      - SSN starts global clock and half-second periodic interrupt 
 *      - SSN calculates machine status update and ambient conditions every 100 milliseconds. 
		  The ISR sends the status update after every ${SSN_REPORT_INTERVAL} seconds
 */

int main() {
	// Setup Smart Sense Node
	SSN_Setup(115200);
	// Check the EEPROM, temperature sensor and network connection before proceeding
	RunSystemTests(0, 100);
	// We need a watchdog to make sure we don't get stuck forever
	EnableWatchdog();
	// First find MAC in flash memory or assign default MAC address
	SSN_COPY_MAC_FROM_MEMORY(SSN_MAC_ADDRESS, SSN_DEFAULT_MAC, &SSN_PREV_STATE, &SSN_CURRENT_STATE, NodeExclusiveChannel);
	// Choose STATIC or Dynamic IP assignment
#ifdef DHCPIP
	SetupConnectionWithDHCP(SSN_MAC_ADDRESS, NodeExclusiveChannel, &WIZ5500_network_information);
#endif
#ifdef STATICIP
	// Setup Static IP for SSN to join existing network
	SetupConnectionWithStaticIP(SSN_MAC_ADDRESS, NodeExclusiveChannel, &WIZ5500_network_information, SSN_STATIC_IP, SSN_SUBNET_MASK, SSN_GATWAY_ADDRESS, SSN_DNS_ADDRESS);
#endif	 
#ifdef USE_DNS
	GetServerIP_UsingDNS(WIZ5500_network_information.dns, DEFAULT_SERVER_IP, MQTT_SERVER_DNS_STRING, SSN_SERVER_IP);
#endif
#ifdef USE_STATIC_SERVER_IP
	printf("[LOG]: Connecting to Static MQTT Server IP: %d.%d.%d.%d\n", SSN_SERVER_IP[0], SSN_SERVER_IP[1], SSN_SERVER_IP[2],SSN_SERVER_IP[3]);
#endif
	// Setup MQTT connection for SSN communication with broker
	SetupMQTTClientConnection(&MQTT_Network, &Client_MQTT, &MQTTOptions, &MQTT_DataPacket, SSN_SERVER_IP, NodeExclusiveChannel, SSN_RECEIVE_ASYNC_MESSAGE_OVER_MQTT);
	// MQTT get allowed failure counts to determine when a reconnection is necessary
	MQTTallowedfailureCount = MQTTallowedfailureCounts(SSN_REPORT_INTERVAL);
	printf("[MQTT] By Default, Number of Failures-to-Publish Before Reconnect = %d\n", MQTTallowedfailureCount);
	// Get MAC address for SSN if we didn't have one already
	SSN_GET_MAC(SSN_MAC_ADDRESS, &SSN_PREV_STATE, &SSN_CURRENT_STATE, MQTTallowedfailureCount, &MQTT_Network, &Client_MQTT, &MQTTOptions, &MQTT_DataPacket, SSN_SERVER_IP, 
		NodeExclusiveChannel, SSN_RECEIVE_ASYNC_MESSAGE_OVER_MQTT);
	// Get SSN configurations for SSN or pick from EEPROM if already assigned
	SSN_GET_CONFIG(SSN_MAC_ADDRESS, SSN_CONFIG, &SSN_REPORT_INTERVAL, &TEMPERATURE_MIN_THRESHOLD, &TEMPERATURE_MAX_THRESHOLD, &RELATIVE_HUMIDITY_MIN_THRESHOLD, &RELATIVE_HUMIDITY_MAX_THRESHOLD, 
		SSN_CURRENT_SENSOR_RATINGS, SSN_CURRENT_SENSOR_THRESHOLDS,  SSN_CURRENT_SENSOR_MAXLOADS, SSN_CURRENT_SENSOR_VOLTAGE_SCALARS, &SSN_PREV_STATE, &SSN_CURRENT_STATE, &CONFIG_received, 
		MQTTallowedfailureCount, &MQTT_Network, &Client_MQTT, &MQTTOptions, &MQTT_DataPacket, SSN_SERVER_IP, NodeExclusiveChannel, SSN_RECEIVE_ASYNC_MESSAGE_OVER_MQTT);
	// Receive time of day from the server for accurate timestamps
	SSN_GET_TIMEOFDAY(SSN_MAC_ADDRESS, &SSN_PREV_STATE, &SSN_CURRENT_STATE, &TimeOfDay_received, MQTTallowedfailureCount, &MQTT_Network, &Client_MQTT, &MQTTOptions, &MQTT_DataPacket, SSN_SERVER_IP, 
			NodeExclusiveChannel, SSN_RECEIVE_ASYNC_MESSAGE_OVER_MQTT);
	// MQTT get allowed failure counts to determine when a reconnection is necessary
	MQTTallowedfailureCount = MQTTallowedfailureCounts(SSN_REPORT_INTERVAL);
	printf("[MQTT] By Configuration, Number of Failures-to-Publish Before Reconnect = %d\n", MQTTallowedfailureCount);
	// Clear the watchdog
	ServiceWatchdog();
	// Start the global clock that will trigger a response each half of a second through our half-second interrupt defined above Main function
	setup_Global_Clock_And_Half_Second_Interrupt(PERIPH_CLK, &ssn_uptime_in_seconds);
	// a counter to check how many 100 millisecond intervals have passed
	uint8_t ms_100_counter = 0;
	// publication successful flag
	int message_publish_status;
	// SSN heartbeat loop
	while (SSN_IS_ALIVE) {
		// Network critical section begins here. Disable global half second interrupt
		DisableGlobalHalfSecondInterrupt();
		// Re-Sync Time of Day after every 4 hours (time in seconds)
		SSN_REQUEST_Time_of_Day_AFTER_N_SECONDS(ssn_uptime_in_seconds, TIME_RESYNC_AFTER_HOURS * 3600, MQTTallowedfailureCount, SSN_MAC_ADDRESS, &SSN_PREV_STATE, &SSN_CURRENT_STATE, 
			&TimeOfDay_received, &MQTT_Network, &Client_MQTT, &MQTTOptions, &MQTT_DataPacket, SSN_SERVER_IP, NodeExclusiveChannel, SSN_RECEIVE_ASYNC_MESSAGE_OVER_MQTT);
#ifdef DHCPIP
		// Request IP from DHCP after it expires based on lease time
		SSN_REQUEST_IP_From_DHCP_AFTER_N_SECONDS(SSN_MAC_ADDRESS, ssn_uptime_in_seconds, getDHCPLeasetime(), NodeExclusiveChannel, &WIZ5500_network_information, 
			&MQTT_Network, &Client_MQTT, &MQTTOptions, &MQTT_DataPacket, SSN_SERVER_IP, NodeExclusiveChannel, SSN_RECEIVE_ASYNC_MESSAGE_OVER_MQTT);
#endif
		// Read time of day from one of the sensors after N seconds (10 is the number of 100 milliseconds in 1 second)
		if (ms_100_counter >= TEMPERATURE_SENSOR_READ_AFTER_SECONDS * 10) {
#ifdef TH_AM2320
			// Read ambient temperature and humidity sensor
			SSN_GET_AMBIENT_CONDITION(temperature_bytes, relative_humidity_bytes, TEMPERATURE_MIN_THRESHOLD, TEMPERATURE_MAX_THRESHOLD, RELATIVE_HUMIDITY_MIN_THRESHOLD, 
                RELATIVE_HUMIDITY_MAX_THRESHOLD, &SSN_PREV_STATE, &SSN_CURRENT_STATE, &abnormal_activity);
#endif
#ifdef NTC_Thermistor
			// Read thermistor and assign temperature bytes its value
			SSN_GET_OBJECT_TEMPERATURE_CONDITION_Thermistor(NTC_Thermistor_4092_50k_special_bytes, TEMPERATURE_MIN_THRESHOLD, TEMPERATURE_MAX_THRESHOLD, &SSN_PREV_STATE, &SSN_CURRENT_STATE, 
				&abnormal_activity);
			temperature_bytes[0] = NTC_Thermistor_4092_50k_special_bytes[0];
			temperature_bytes[1] = NTC_Thermistor_4092_50k_special_bytes[1];
#endif
#ifdef OTS_LS_MLX90614
			SSN_GET_OBJECT_TEMPERATURE_CONDITION_IR(MLX90614_special_bytes, TEMPERATURE_MIN_THRESHOLD, TEMPERATURE_MAX_THRESHOLD, &SSN_PREV_STATE, &SSN_CURRENT_STATE, &abnormal_activity);
			temperature_bytes[0] = MLX90614_special_bytes[0];
			temperature_bytes[1] = MLX90614_special_bytes[1];
#endif
			// reset 100 millisecond counter
			ms_100_counter = 0;
		}
		// Get load currents and status of machines
		machine_status_change_flag = Get_Machines_Status_Update(SSN_CURRENT_SENSOR_RATINGS, SSN_CURRENT_SENSOR_VOLTAGE_SCALARS, SSN_CURRENT_SENSOR_THRESHOLDS, SSN_CURRENT_SENSOR_MAXLOADS, 
            Machine_load_currents, Machine_load_percentages, Machine_status, Machine_prev_status, &Machine_status_flag, Machine_status_duration, Machine_status_timestamp, ssn_dynamic_clock);
		// Clear the watchdog
		ServiceWatchdog();
		// we will report our status update out of sync with reporting interval if a state changes, this will allow us for accurate timing measurements
		if (machine_status_change_flag == true) {
			message_count++;
			message_publish_status = Send_STATUSUPDATE_Message(SSN_MAC_ADDRESS, temperature_bytes, relative_humidity_bytes, Machine_load_currents, Machine_load_percentages,
				Machine_prev_status, Machine_status_flag, MACHINES_STATE_TIME_DURATION_UPON_STATE_CHANGE, Machine_status_timestamp, ssn_static_clock, abnormal_activity);
			Clear_Machine_Status_flag(&Machine_status_flag);
			if (message_publish_status != SUCCESSS) {
				mqtt_failure_counts++;
				printf("[ERROR] Message Publication to MQTT Broker Failed (Count = %d/%d)\n", mqtt_failure_counts, MQTTallowedfailureCount);
			}
			else {
				mqtt_failure_counts = 0;
			}
		}
		if (report_now == true) {
			message_count++;
			report_now = false; // reset report flag
			message_publish_status = Send_STATUSUPDATE_Message(SSN_MAC_ADDRESS, temperature_bytes, relative_humidity_bytes, Machine_load_currents, Machine_load_percentages,
				Machine_prev_status, Machine_status_flag, MACHINES_STATE_TIME_DURATION_UPON_STATE_CHANGE, Machine_status_timestamp, ssn_static_clock, abnormal_activity);
			Clear_Machine_Status_flag(&Machine_status_flag);
			if (message_publish_status != SUCCESSS) {
				mqtt_failure_counts++;
				printf("[ERROR] Message Publication to MQTT Broker Failed (Count = %d/%d)\n", mqtt_failure_counts, MQTTallowedfailureCount);
			}
			else {
				mqtt_failure_counts = 0;
			}
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// MQTT background handler
		start_ms_timer_with_interrupt();
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Check if we failed to publish, check connections and reconnect if necessary
		// check how many failure counts have we encountered and reconnect to broker if necessary
		if (mqtt_failure_counts >= MQTTallowedfailureCount) {
			// assume our connection has broken, we'll reconnect at this point
			printf("[MQTT] SSN Message Publication to MQTT Broker Failed and Retry Exceeded...\n");
			message_publish_status = SSN_Check_Connection_And_Reconnect(&SSN_PREV_STATE, &SSN_CURRENT_STATE, message_publish_status, &MQTT_Network, &Client_MQTT, &MQTTOptions, &MQTT_DataPacket, 
				SSN_SERVER_IP, NodeExclusiveChannel, SSN_RECEIVE_ASYNC_MESSAGE_OVER_MQTT);
			// connection re-established, exit fault code and reset failure counts
			mqtt_failure_counts = 0;
		}
		MQTTYield(&Client_MQTT, 100);
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		stop_ms_timer_with_interrupt();
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Network critical section ends here. Enable global half second interrupt
		EnableGlobalHalfSecondInterrupt();
		// sleep for 100 milliseconds
		sleep_for_microseconds(100000);
		ms_100_counter++;
	}
	// we should never reach this point
	return 0;
}
