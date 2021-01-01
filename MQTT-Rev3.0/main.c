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
void __ISR(_TIMER_2_VECTOR, IPL4SOFT) Timer2IntHandler(void){
	// clear timer 2 interrupt flag
	IFS0bits.T2IF = 0x00;
    // millisecond ticks for DHCP
    msTicks++; /* increment counter necessary in Delay()*/
	// millisecond ticks for MQTT 
	MilliTimer_Handler();
	////////////////////////////////////////////////////////
	// SHOULD BE Added DHCP Timer Handler your 1s tick timer
	if(msTicks % 1000 == 0)	{
        DHCP_time_handler();
        /* Give the Ethernet Indication */
        // No_Ethernet_LED_INDICATE();
		printf("\n(LOG): One Second Passed in Millisecond Interrupt Handler\n");
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
			message_count++;
			//printf("Connection Status: %d\n", Client_MQTT.isconnected);
			socket_ok = Send_STATUSUPDATE_Message(SSN_MAC_ADDRESS, SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT, temperature_bytes, relative_humidity_bytes, Machine_load_currents,
				Machine_load_percentages, Machine_status, Machine_status_flag, Machine_status_duration, Machine_status_timestamp, ssn_static_clock, abnormal_activity);
			Clear_Machine_Status_flag(&Machine_status_flag);
			SSN_RESET_IF_SOCKET_CORRUPTED();
		}
		//SSN_RESET_AFTER_N_SECONDS(2*3600); // Testing
		SSN_RESET_AFTER_N_SECONDS(8 * 3600);
		//SSN_RESET_AFTER_N_SECONDS_IF_NO_MACHINE_ON(8*3600);
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
	SSN_Setup();
	// Check the EEPROM, temperature sensor and network connection before proceeding
	RunSystemTests();
	// We need a watchdog to make sure we don't get stuck forever
	EnableWatchdog();
	// First find MAC in flash memory or assign default MAC address
	SSN_COPY_MAC_FROM_MEMORY();
	// We can chose two ways to operate over UDP; static or dynamic IP
	SetupConnectionWithDHCP(SSN_MAC_ADDRESS, SSN_UDP_SOCKET_NUM);
	// Setup Static IP
	// SetupConnectionWithStaticIP(SSN_MAC_ADDRESS, SSN_STATIC_IP, SSN_SUBNET_MASK, SSN_GATWAY_ADDRESS, SSN_DNS_ADDRESS);
	// MQTT connection
	SetupMQTTClientConnection(&MQTT_Network, &Client_MQTT, &MQTTOptions, SSN_SERVER_IP, NodeExclusiveChannel, SSN_RECEIVE_ASYNC_MESSAGE_OVER_MQTT);
	// Get MAC address for SSN if we didn't have one already
	SSN_GET_MAC();
	// Get SSN configurations for SSN or pick from EEPROM if already assigned
	SSN_GET_CONFIG();
	// Receive time of day from the server for accurate timestamps
	SSN_GET_TIMEOFDAY();
	// Clear the watchdog
	ServiceWatchdog();
	// Start the global clock that will trigger a response each half of a second through our half-second interrupt defined above Main function
	setup_Global_Clock_And_Half_Second_Interrupt(PERIPH_CLK);
	//InterruptEnabled = true;
	uint8_t ms_100_counter = 0;
	while (SSN_IS_ALIVE) {
		// Network critical section begins here. Disable global half second interrupt
		DisableGlobalHalfSecondInterrupt();
		if (ms_100_counter >= 20) {
			// Read temperature and humidity sensor
			SSN_GET_AMBIENT_CONDITION();
			ms_100_counter = 0;
		}
		// Make sure Ethernet is working fine (blocking if no physical link available)
		SSN_CHECK_ETHERNET_CONNECTION();
		// Reset node if we have been running for more than 8 hours
		SSN_RESET_AFTER_N_SECONDS(8 * 3600);
		// Get load currents and status of machines
		machine_status_change_flag = Get_Machines_Status_Update(SSN_CURRENT_SENSOR_RATINGS, SSN_CURRENT_SENSOR_VOLTAGE_SCALARS, SSN_CURRENT_SENSOR_THRESHOLDS, SSN_CURRENT_SENSOR_MAXLOADS, 
			Machine_load_currents, Machine_load_percentages, Machine_status, &Machine_status_flag, Machine_status_duration, Machine_status_timestamp);
		// we will report our status update out of sync with reporting interval if a state changes, this will allow us for accurate timing measurements
		if (machine_status_change_flag == true) {
			message_count++;
			// printf("Connection Status: %d\n", Client_MQTT.isconnected);
			socket_ok = Send_STATUSUPDATE_Message(SSN_MAC_ADDRESS, SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT, temperature_bytes, relative_humidity_bytes, Machine_load_currents,
				Machine_load_percentages, Machine_prev_status, Machine_status_flag, MACHINES_STATE_TIME_DURATION_UPON_STATE_CHANGE, Machine_status_timestamp, ssn_static_clock,
				abnormal_activity);
			Clear_Machine_Status_flag(&Machine_status_flag);
		}
		// Clear the watchdog
		ServiceWatchdog();
		// MQTT process handler
		start_ms_timer_with_interrupt();
    	MQTTYield(&Client_MQTT, 50);
		stop_ms_timer_with_interrupt();
		// Network critical section ends here. Enable global half second interrupt
		EnableGlobalHalfSecondInterrupt();
		// sleep for 100 milliseconds
		sleep_for_microseconds(100000);
		ms_100_counter++;
	}
	// we should never reach this point
	return 0;
}

//int main() {
//	// Setup Smart Sense Node
//	SSN_Setup();
////	char test[50];
//	while(1) {
//		PORTToggleBits(IOPORT_A, GREEN_LED);
////		SerialTransmit_UART2("Who is this?\n");
////		SerialReceive_UART2(test, 50);
////		printf("(SSN) Got this: %s", test);
//		sleep_for_microseconds(1000000);
//	}
//	return 0;
//}