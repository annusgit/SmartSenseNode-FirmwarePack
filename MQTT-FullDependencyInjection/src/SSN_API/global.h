#ifndef __global_h__
#define __global_h__

#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

#include <xc.h>
#include <p32xxxx.h>
#include <plib.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "Drivers/PSEUDO_RTCC/pseudo_rtcc.h"

#define SYSTEM_CLK                          60000000
#define PERIPH_CLK                          30000000
#define SSN_DEFAULT_PORT                    8888

/** Red led pin on SSN */
#define RED_LED                             BIT_3
/** Green led pin on SSN */
#define GREEN_LED                           BIT_2

/** Simple always True variable for loops */
#define SSN_IS_ALIVE                        100

/** States of our SSN */
#define SELF_TEST_FAILED_STATE              0
#define NO_CURRENT_SENSOR_STATE             1
#define NO_ETHERNET_STATE                   2
#define NO_MAC_STATE                        3
#define NO_CONFIG_STATE                     4
#define ACK_CONFIG_STATE                    5
#define NO_TIMEOFDAY_STATE                  6
#define ABNORMAL_ACTIVITY_STATE             7
#define NORMAL_ACTIVITY_STATE               8
// some additional states while we try to connect to the network
#define GETTING_IP_FROM_DHCP                9
#define LOOKING_UP_DNS                      10
#define ESTABLISHING_MQTT_CONNECTION        11

/* EEPROM Read/Write Position for MAC address */
#define EEPROM_MAC_BLOCK                    0
#define EEPROM_MAC_LOC                      100
#define EEPROM_MAC_SIZE                     6
/* EEPROM Read/Write Position for MAC address string */
#define EEPROM_MAC_STRING_BLOCK             1
#define EEPROM_MAC_STRING_LOC               100
#define EEPROM_MAC_STRING_SIZE              17
/* EEPROM Read/Write Position for current sensor configurations */
#define EEPROM_CONFIG_BLOCK                 2
#define EEPROM_CONFIG_LOC                   100
#define EEPROM_CONFIG_SIZE                  21

#define TIME_Of_DAY_SIZE                    4
/* Max number of machines that SSN can monitor */
#define NO_OF_MACHINES                      4

/* Global MACRO Definitions for MQTT */
//#define MQTT_TCP_SOCKET                   2
#define MQTT_MAX_LEN                        100
#define MQTT_BUFFER_SIZE                    2048
#define MQTT_Port                           1883

//#define __UDP_COMMUNICATION
#define __MQTT_COMMUNICATION

/** SSN DEBUG MESSAGES OVER UDP */
#define MQTT_Publication_Failed             0
#define TCP_Socket_Error                    1
#define Ethernet_just_recovered             2
#define DHCP_IP_Time_Received               3
#define SSN_just_Restarted                  4
#define MQTT_Client_Reconnected             5
#define TimeofDay_Received_for_syncing      6
#define TCP_Socket_Conn_Failed              7
#define MQTT_Connection_failed_Restarting   8

#define stringtosendsize 100

/** SSN Fault Count Variable */
extern uint32_t fault_count;

// Debugging macros for each module. Enable them for printing status
//#define _SMARTSENSE_DEBUG_
//#define _UART_DEBUG_
//#define _EEPROM_DEBUG_
//#define _CURRENTSENSOR_DEBUG_
//#define _TEMPSENSOR_DEBUG_
//#define _NETWORK_DEBUG_

#define TIME_RESYNC_AFTER_HOURS                 4
#define TEMPERATURE_SENSOR_READ_AFTER_SECONDS   2

#define TH_AM2320
//#define TH_DHT22
//#define NTC_Thermistor
//#define OTS_LS_MLX90614

//#define DHCPIP
#define STATICIP

//#define USE_DNS
#define USE_STATIC_SERVER_IP

/** 
 * A simple loop count based delay 
 * @param counter The number of empty loop iterations to wait for
 */
void delay(uint32_t counter);
/**
 * Converts a given uint16_t to 2 byte array
 * @param integer The number to convert
 * @param bytes The byte array to fill
 */
void get_bytes_from_uint16(uint16_t integer, uint8_t* bytes);
/**
 * Converts a given uint32_t to 4 byte array
 * @param integer The number to convert
 * @param bytes The byte array to fill
 */
void get_bytes_from_uint32(uint32_t integer, uint8_t* bytes);
/**
 * Converts a given byte array to uint32_t
 * @param bytes The byte array containing the bytes to combine into an integer
 * @return The uint32_t combined from the byte array
 */
uint32_t get_uint32_from_bytes(uint8_t* bytes);
/**
 * Rounds a given floating point to 2-decimal number
 * @param float_val
 * @return 2-decimal place rounded-off floating point number
 */
float round_float_to_2_decimal_place(float float_val);
/** 
 * Sleep for microseconds 
 * @param us The number of microseconds to sleep for
 */
void sleep_for_microseconds(unsigned int us);
void EnableWatchdog();
void ServiceWatchdog();
void sleep_for_microseconds_and_clear_watchdog(uint32_t delay_us);
/** 
 * enable multivectored interrupts for PIC32MX
 */
void setup_Interrupts();
void start_ms_timer_with_interrupt();
void stop_ms_timer_with_interrupt();
void setup_millisecond_timer_with_interrupt();
void EnableGlobalHalfSecondInterrupt();
void DisableGlobalHalfSecondInterrupt();
void setup_Global_Clock_And_Half_Second_Interrupt(uint32_t PERIPH_CLOCK, uint32_t* ssn_uptime_in_seconds_varible);
/** 
 * Clears LED indicator by turning off red/green lights
 */
void Clear_LED_INDICATOR();
/** 
 * Sets up LED indicator for SSN
 */
void setup_LED_Indicator();
/** 
 * Indicates node not configured; Either no MAC or no configurations
 */
void Node_Up_Not_Configured_LED_INDICATE();
/** 
 * Indicates no ethernet connection available
 */
void No_Ethernet_LED_INDICATE();
/** 
 * Indicates failure in self tests; could be EEPROM, temperature sensor or Ethernet failure
 */
void Self_Test_Failed_LED_INDICATE();
/** 
 * Indicates normal mode operation after all tests and message communication successful
 */
void Normal_Operation_LED_INDICATE();
/** 
 * Indicates either no current sensors are connected to SSN or they are reading zero
 */
void Current_Sensors_Disconnected_LED_INDICATE();
/** 
 * Indicates Abnormal activity sensed in ambient temperature or relative humidity readings
 */
void Abnormal_Activity_LED_INDICATE();
/** 
 * Indicates trying to connect to a network
 */
void establishing_connection_LED_INDICATE();
/** 
 * Indicates SSN state from LED
 * @param this_state A variable indicating the current state of SSN 
 */
void SSN_LED_INDICATE(uint8_t this_state);
void SetPinForMicroseconds(uint32_t port, uint32_t pin, uint32_t us);
void ClearPinForMicroseconds(uint32_t port, uint32_t pin, uint32_t us);


#endif 
