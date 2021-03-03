#ifndef __global_h__
#define __global_h__

#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

#include <xc.h>
#include <p32xxxx.h>
#include <plib.h>
#include <stdint.h>
#include <stdbool.h>
#include "Drivers/PSEUDO_RTCC/pseudo_rtcc.h"

#define SYSTEM_CLK                  60000000
#define PERIPH_CLK                  30000000
#define SSN_DEFAULT_PORT            8888

/** Red led pin on SSN */
#define RED_LED                     BIT_3
/** Green led pin on SSN */
#define GREEN_LED                   BIT_2

/** Simple always True variable for loops */
#define SSN_IS_ALIVE                100

/** States of our SSN */
#define SELF_TEST_FAILED_STATE      0
#define NO_CURRENT_SENSOR_STATE     1
#define NO_ETHERNET_STATE           2
#define NO_MAC_STATE                3
#define NO_CONFIG_STATE             4   
#define ACK_CONFIG_STATE            5
#define NO_TIMEOFDAY_STATE          6
#define ABNORMAL_ACTIVITY_STATE     7
#define NORMAL_ACTIVITY_STATE       8

/* EEPROM Read/Write Position for MAC address */
#define EEPROM_MAC_LOC              0
/* EEPROM Read/Write Position for current sensor configurations */
#define EEPROM_CONFIG_LOC           12

/* EEPROM MAC address size */
#define EEPROM_MAC_SIZE             6
/* EEPROM current sensor configurations size */
#define EEPROM_CONFIG_SIZE          21
/* EEPROM time of day size */
#define TIME_Of_DAY_SIZE            4
/* Max number of machines that SSN can monitor */
#define NO_OF_MACHINES              4

/* Global MACRO Definitions for MQTT */
//#define MQTT_TCP_SOCKET             2
#define MQTT_MAX_LEN                100
#define MQTT_BUFFER_SIZE            2048
#define MQTT_Port                   1883

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
uint32_t fault_count;

// Debugging macros for each module. Enable them for printing status
//#define _SMARTSENSE_DEBUG_
//#define _UART_DEBUG_
//#define _EEPROM_DEBUG_
//#define _CURRENTSENSOR_DEBUG_
//#define _TEMPSENSOR_DEBUG_
//#define _NETWORK_DEBUG_

#define TH_AM2320
//#define TH_DHT22

/** 
 * A simple loop count based delay 
 * @param counter The number of empty loop iterations to wait for
 */
static inline void delay(uint32_t counter) {
    while (counter--);
}

/**
 * Converts a given uint16_t to 2 byte array
 * @param integer The number to convert
 * @param bytes The byte array to fill
 */
static inline void get_bytes_from_uint16(uint16_t integer, uint8_t* bytes) {
    bytes[0] = (integer & 0xFF00) >> 8;
    bytes[1] = (integer & 0x00FF);
}

/**
 * Converts a given uint32_t to 4 byte array
 * @param integer The number to convert
 * @param bytes The byte array to fill
 */
static inline void get_bytes_from_uint32(uint32_t integer, uint8_t* bytes) {
    bytes[0] = (integer & 0xFF000000) >> 24;
    bytes[1] = (integer & 0x00FF0000) >> 16;
    bytes[2] = (integer & 0x0000FF00) >> 8;
    bytes[3] = (integer & 0x000000FF);
}

/**
 * Converts a given byte array to uint32_t
 * @param bytes The byte array containing the bytes to combine into an integer
 * @return The uint32_t combined from the byte array
 */
static inline uint32_t get_uint32_from_bytes(uint8_t* bytes) {
    return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

/**
 * Rounds a given floating point to 2-decimal number
 * @param float_val
 * @return 2-decimal place rounded-off floating point number
 */
static inline float round_float_to_2_decimal_place(float float_val) {
    // https://www.geeksforgeeks.org/rounding-floating-point-number-two-decimal-places-c-c/
    // 37.66666 * 100 =3766.66 
    // 3766.66 + .5 =3767.16    for rounding off value 
    // then type cast to int so value is 3767 
    // then divided by 100 so the value converted into 37.67 
    int rounded_float = (int) (float_val * 100.0 + 0.5);
    return (float) rounded_float / 100.0f;
}

/** 
 * Sleep for microseconds 
 * @param us The number of microseconds to sleep for
 */
static inline void sleep_for_microseconds(unsigned int us) {
    T5CON = 0x8000; // enable Timer5, source PBCLK, 1:1 prescaler
    // delay 100us per loop until less than 100us remain
    while (us >= 100) {
        us -= 100;
        TMR5 = 0;
        while (TMR5 < PERIPH_CLK / 10000);
    }
    // delay 10us per loop until less than 10us remain
    while (us >= 10) {
        us -= 10;
        TMR5 = 0;
        while (TMR5 < PERIPH_CLK / 100000);
    }
    // delay 1us per loop until finished
    while (us > 0) {
        us--;
        TMR5 = 0;
        while (TMR5 < PERIPH_CLK / 1000000);
    }
    // turn off Timer5 so function is self-contained
    T5CONCLR = 0x8000;
}

static inline void EnableWatchdog() {
    WDTCONbits.WDTCLR = 1;
    WDTCONbits.ON = 1;
}

static inline void ServiceWatchdog() {
    WDTCONbits.WDTCLR = 1;
}

static inline void sleep_for_microseconds_and_clear_watchdog(uint32_t delay_us) {
    while (delay_us > 1000000) {
        sleep_for_microseconds(1000000);
        ServiceWatchdog();
        delay_us -= 1000000;
    }
    sleep_for_microseconds(delay_us);
}

/** 
 * enable multivectored interrupts for PIC32MX
 */
static inline void setup_Interrupts() {
    // multivectored mode of system interrupts
    INTEnableSystemMultiVectoredInt();
}

static inline void start_ms_timer_with_interrupt() {
    IEC0CLR = 0x0200;       // disable timer 2 interrupt, IEC0<9>
    IFS0CLR = 0x0200;       // clear timer 2 int flag, IFS0<9>
    IEC0SET = 0x0200;       // enable timer 2 int, IEC0<9>
}

static inline void stop_ms_timer_with_interrupt() {
    IEC0CLR = 0x0200;       // disable timer 2 interrupt, IEC0<9>
    IFS0CLR = 0x0200;       // clear timer 2 int flag, IFS0<9>
    IEC0SET = 0x0000;       // disable timer 2 int, IEC0<9>
}

static inline void setup_millisecond_timer_with_interrupt() {
    IEC0CLR = 0x0200;                   // disable timer 2 interrupt, IEC0<9>
    IFS0CLR = 0x0200;                   // clear timer 2 int flag, IFS0<9>
    IPC2CLR = 0x001f;                   // clear timer 2 priority/subpriority fields 
    IPC2SET = 0x0010;                   // set timer 2 int priority = 4, IPC2<4:2>
    IEC0SET = 0x0200;                   // enable timer 2 int, IEC0<9>
	// Turn on 16-bit Timer2, set prescaler to 1:256 (frequency is Pbclk / 256)
    T2CON   = 0x8060;                   // this prescaler reduces the input clock frequency by 64    
    PR2 = (0.001*PERIPH_CLK/64);	// 1 millisecond timer interrupt
    stop_ms_timer_with_interrupt();
}

static inline void EnableGlobalHalfSecondInterrupt() {
    IEC0bits.T1IE = 0x01;   // enable timer 1 int, IEC0<4>
}

static inline void DisableGlobalHalfSecondInterrupt() {
    IEC0bits.T1IE = 0x00;   // disable timer 1 interrupt, IEC0<4>
}

static inline void setup_Global_Clock_And_Half_Second_Interrupt(uint32_t PERIPH_CLOCK) {
    // initialize the SSN global clock 
    ssn_uptime_in_seconds = 0;
    // enable timer-1 interrupt
    IEC0bits.T1IE = 0x00;			// disable timer 1 interrupt, IEC0<4>
    IFS0CLR = 0x0010;				// clear timer 1 int flag, IFS0<4>
    IPC1CLR = 0x001f;				// clear timer 1 priority/subpriority fields 
    IPC1SET = 0x0010;				// set timer 1 int priority = 4, IPC1<4:2>
    IEC0bits.T1IE = 0x01;			// enable timer 1 int, IEC0<4>
    T1CON = 0x8030;					// this prescaler reduces the input clock frequency by 256
    PR1 = (0.5*PERIPH_CLOCK/256);	// half second timer interrupt
}

/** 
 * Clears LED indicator by turning off red/green lights
 */
static inline void Clear_LED_INDICATOR() {
    PORTSetBits(IOPORT_A, RED_LED);
    PORTSetBits(IOPORT_A, GREEN_LED);
}

/** 
 * Sets up LED indicator for SSN
 */
static inline void setup_LED_Indicator() {
    PORTSetPinsDigitalOut(IOPORT_A, RED_LED);
    PORTSetPinsDigitalOut(IOPORT_A, GREEN_LED);
    Clear_LED_INDICATOR();
}

/** 
 * Indicates node not configured; Either no MAC or no configurations
 */
static inline void Node_Up_Not_Configured_LED_INDICATE() {
    /* Solid Red + Solid Green = Solid Orange */
    PORTClearBits(IOPORT_A, RED_LED);
    PORTClearBits(IOPORT_A, GREEN_LED);
}

/** 
 * Indicates no ethernet connection available
 */
static inline void No_Ethernet_LED_INDICATE() {
    /* Toggle Red + Toggle Green = Toggle Orange */
    PORTToggleBits(IOPORT_A, RED_LED);
    PORTToggleBits(IOPORT_A, GREEN_LED);
}

/** 
 * Indicates failure in self tests; could be EEPROM, temperature sensor or Ethernet failure
 */
static inline void Self_Test_Failed_LED_INDICATE() {
    /* Solid Red + Green OFF = Solid Red */
    PORTClearBits(IOPORT_A, RED_LED);
    PORTSetBits(IOPORT_A, GREEN_LED);
}

/** 
 * Indicates normal mode operation after all tests and message communication successful
 */
static inline void Normal_Operation_LED_INDICATE() {
    /* Red OFF + Solid Green = Solid Green */
    PORTSetBits(IOPORT_A, RED_LED);
    PORTClearBits(IOPORT_A, GREEN_LED);
}

/** 
 * Indicates either no current sensors are connected to SSN or they are reading zero
 */
static inline void Current_Sensors_Disconnected_LED_INDICATE() {
    /* Red OFF + Green Toggle = Green Toggle */
    PORTSetBits(IOPORT_A, RED_LED);
    PORTToggleBits(IOPORT_A, GREEN_LED);
}

/** 
 * Indicates Abnormal activity sensed in ambient temperature or relative humidity readings
 */
static inline void Abnormal_Activity_LED_INDICATE() {
    /* Red Toggle + Green OFF = Red Toggle */
    PORTToggleBits(IOPORT_A, RED_LED);
    PORTSetBits(IOPORT_A, GREEN_LED);
}

/** 
 * Indicates SSN state from LED
 * @param this_state A variable indicating the current state of SSN 
 */
static inline void SSN_LED_INDICATE(uint8_t this_state) {
    /* This one function will determine the state of SSN and give the proper LED heartbeat indication */
    switch (this_state) {
        /* No mac address? */
        case NO_MAC_STATE:
            Node_Up_Not_Configured_LED_INDICATE();
            break;
        /* No configuration? */
        case NO_CONFIG_STATE:
            Node_Up_Not_Configured_LED_INDICATE();
            break;
        /* No time of day? */
        case NO_TIMEOFDAY_STATE:
            Node_Up_Not_Configured_LED_INDICATE();
            break;
        /* CONFIG received so ack config? */
        case ACK_CONFIG_STATE:
            Node_Up_Not_Configured_LED_INDICATE();
            break;
        /* there is no ethernet ? */
        case NO_ETHERNET_STATE:
            No_Ethernet_LED_INDICATE();
            break;
        /* self tests have failed, so... */
        case SELF_TEST_FAILED_STATE:
            Self_Test_Failed_LED_INDICATE();
            break;
        /* there is no ethernet ? */
        case NO_CURRENT_SENSOR_STATE:
            Current_Sensors_Disconnected_LED_INDICATE();
            break;
        /* self tests have failed, so... */
        case ABNORMAL_ACTIVITY_STATE:
            Abnormal_Activity_LED_INDICATE();
            break;
        /* status update message? */
        case NORMAL_ACTIVITY_STATE:
            Normal_Operation_LED_INDICATE();
            break;
        default:
            break;
    }
}

static inline void SetPinForMicroseconds(uint32_t port, uint32_t pin, uint32_t us) {
    PORTSetBits(port, pin);
    sleep_for_microseconds(us);
}

static inline void ClearPinForMicroseconds(uint32_t port, uint32_t pin, uint32_t us) {
    PORTClearBits(port, pin);
    sleep_for_microseconds(us);
}

//// not used. PWM sweep function in case we ever need that kind of led blink
//static inline void PWM_Sweep() {
//    RPA2R = 0x0005; // OC4 output on Pin-A2
//    RPA3R = 0x0005; // OC3 output on Pin-A3
//    // Configure standard PWM mode for output compare module 4 and 3
//    OC4CON = 0x0006; // OC4 on, timer 2 is clock source
//    OC3CON = 0x000D; // OC3 on, timer 3 is clock source
//    // A write to PRy configures the PWM frequency
//    // PR = [FPB / (PWM Frequency * TMR Prescale Value)] ? 1
//    // : note the TMR Prescaler is 1 and is thus ignored
//    PR2 = (PERIPH_CLK / 16000) - 1;
//    PR3 = (PERIPH_CLK / 16000) - 1;
//    // A write to OCxRS configures the duty cycle
//    // : OCxRS / PRy = duty cycle
//    OC4RS = (PR2 + 1) * ((float)10 / 100);
//    OC3RS = (PR3 + 1) * ((float)10 / 100);
//    T2CONSET = 0x8000;      // Enable Timer2, prescaler 1:1
//    T3CONSET = 0x8000;      // Enable Timer2, prescaler 1:1
//    OC4CONSET = 0x8000;     // Enable Output Compare Module 1
//    OC3CONSET = 0x8000;     // Enable Output Compare Module 1
//    
//    uint8_t duty_cycle = 0, counter = 0;
//    bool increasing = true;
//    
//    for (duty_cycle = 1; duty_cycle < 100; duty_cycle++) {
//            OC4RS = (PR2+1)*((float)duty_cycle/100);
//            OC3RS = (PR3+1)*((float)duty_cycle/100);
//            PORTSetBits(IOPORT_A, BIT_2); // check
//            PORTSetBits(IOPORT_A, BIT_3); // check        
//            delay_us(10000);            
//        }
//        for (duty_cycle = 100; duty_cycle > 0; duty_cycle--) {
//            OC4RS = (PR2+1)*((float)duty_cycle/100);
//            OC3RS = (PR3+1)*((float)duty_cycle/100);
//            PORTSetBits(IOPORT_A, BIT_2); // check
//            PORTSetBits(IOPORT_A, BIT_3); // check        
//            delay_us(10000);            
//        }
//        for (duty_cycle = 1; duty_cycle < 100; duty_cycle++) {
//            OC4RS = (PR2+1)*((float)duty_cycle/100);
//            OC3RS = (PR3+1)*((float)(100-duty_cycle)/100);
//            PORTSetBits(IOPORT_A, BIT_2); // check
//            PORTSetBits(IOPORT_A, BIT_3); // check        
//            delay_us(10000);            
//        }
//        for (duty_cycle = 1; duty_cycle < 100; duty_cycle++) {
//            OC4RS = (PR2+1)*((float)(100-duty_cycle)/100);
//            OC3RS = (PR3+1)*((float)duty_cycle/100);
//            PORTSetBits(IOPORT_A, BIT_2); // check
//            PORTSetBits(IOPORT_A, BIT_3); // check        
//            delay_us(10000);            
//        }
//}

//int EEPROM_checker_code() {
//    uint8_t default_MAC[] = {0xAA, 0xBB, 0xBB, 0xAA, 0xAA, 0xBB};
//    uint8_t check_MAC[6];
//    setup_printf(19200);
//    setup_EEPROM();
////    uint8_t clear_status = EEPROM_Clear();
////    if (clear_status == 1)
////        printf("Cleared Successfully\n");
////    while (1);
//    
//    uint8_t i, write_good;
//    EEPROM_Write_Array(EEPROM_BLOCK_0, 0x91, default_MAC, 6);
//    printf("Write Complete\n");
//
//    uint8_t read_good, read_value, check = 0;
//    EEPROM_Read_Array(EEPROM_BLOCK_0, 0x91, check_MAC, 6);
//    sleep_for_microseconds(10000); // delay 10ms
//    
//    for (i = 0; i < 6; i++) {
//        if (check_MAC[i] == default_MAC[i])
//            check++;
//        printf("%d %d\n", check_MAC[i], default_MAC[i]);
//    }
//    if (check == 6)
//        printf("Check OK\n");
//    else 
//        printf("Check BAD: %d\n", check);
//    while(1);
//}

#endif 
