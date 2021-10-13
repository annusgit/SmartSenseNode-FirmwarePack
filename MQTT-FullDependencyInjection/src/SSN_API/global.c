#include "global.h"

uint32_t fault_count = 0;


/** 
 * A simple loop count based delay 
 * @param counter The number of empty loop iterations to wait for
 */
void delay(uint32_t counter) {
    while (counter--);
}

/**
 * Converts a given uint16_t to 2 byte array
 * @param integer The number to convert
 * @param bytes The byte array to fill
 */
void get_bytes_from_uint16(uint16_t integer, uint8_t* bytes) {
    bytes[0] = (integer & 0xFF00) >> 8;
    bytes[1] = (integer & 0x00FF);
}

/**
 * Converts a given uint32_t to 4 byte array
 * @param integer The number to convert
 * @param bytes The byte array to fill
 */
void get_bytes_from_uint32(uint32_t integer, uint8_t* bytes) {
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
uint32_t get_uint32_from_bytes(uint8_t* bytes) {
    return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

/**
 * Rounds a given floating point to 2-decimal number
 * @param float_val
 * @return 2-decimal place rounded-off floating point number
 */
float round_float_to_2_decimal_place(float float_val) {
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
void sleep_for_microseconds(unsigned int us) {
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

void EnableWatchdog() {
    WDTCONbits.WDTCLR = 1;
    WDTCONbits.ON = 1;
}

void ServiceWatchdog() {
    WDTCONbits.WDTCLR = 1;
}

void sleep_for_microseconds_and_clear_watchdog(uint32_t delay_us) {
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
void setup_Interrupts() {
    // multivectored mode of system interrupts
    INTEnableSystemMultiVectoredInt();
}

void start_ms_timer_with_interrupt() {
    IEC0CLR = 0x0200;       // disable timer 2 interrupt, IEC0<9>
    IFS0CLR = 0x0200;       // clear timer 2 int flag, IFS0<9>
    IEC0SET = 0x0200;       // enable timer 2 int, IEC0<9>
}

void stop_ms_timer_with_interrupt() {
    IEC0CLR = 0x0200;       // disable timer 2 interrupt, IEC0<9>
    IFS0CLR = 0x0200;       // clear timer 2 int flag, IFS0<9>
    IEC0SET = 0x0000;       // disable timer 2 int, IEC0<9>
}

void setup_millisecond_timer_with_interrupt() {
    IEC0CLR = 0x0200;                   // disable timer 2 interrupt, IEC0<9>
    IFS0CLR = 0x0200;                   // clear timer 2 int flag, IFS0<9>
    IPC2CLR = 0x001f;                   // clear timer 2 priority/subpriority fields 
    IPC2SET = 0x0010;                   // set timer 2 int priority = 4, IPC2<4:2>
    IEC0SET = 0x0200;                   // enable timer 2 int, IEC0<9>
	// Turn on 16-bit Timer2, set prescaler to 1:256 (frequency is Pbclk / 256)
    T2CON   = 0x8060;                   // this prescaler reduces the input clock frequency by 64    
    PR2     = (0.001*PERIPH_CLK/64);	// 1 millisecond timer interrupt
    stop_ms_timer_with_interrupt();
}

void EnableGlobalHalfSecondInterrupt() {
    IEC0bits.T1IE = 0x01;   // enable timer 1 int, IEC0<4>
}

void DisableGlobalHalfSecondInterrupt() {
    IEC0bits.T1IE = 0x00;   // disable timer 1 interrupt, IEC0<4>
}

void setup_Global_Clock_And_Half_Second_Interrupt(uint32_t PERIPH_CLOCK, uint32_t* ssn_uptime_in_seconds_varible) {
    // initialize the SSN global clock 
    *ssn_uptime_in_seconds_varible = 0;
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
void Clear_LED_INDICATOR() {
    PORTSetBits(IOPORT_A, RED_LED);
    PORTSetBits(IOPORT_A, GREEN_LED);
}

/** 
 * Sets up LED indicator for SSN
 */
void setup_LED_Indicator() {
    PORTSetPinsDigitalOut(IOPORT_A, RED_LED);
    PORTSetPinsDigitalOut(IOPORT_A, GREEN_LED);
    Clear_LED_INDICATOR();
}

/** 
 * Indicates node not configured; Either no MAC or no configurations
 */
void Node_Up_Not_Configured_LED_INDICATE() {
    /* Solid Red + Solid Green = Solid Orange */
    PORTClearBits(IOPORT_A, RED_LED);
    PORTClearBits(IOPORT_A, GREEN_LED);
}

/** 
 * Indicates no ethernet connection available
 */
void No_Ethernet_LED_INDICATE() {
    /* Toggle Red + Toggle Green = Toggle Orange */
    PORTToggleBits(IOPORT_A, RED_LED);
    PORTToggleBits(IOPORT_A, GREEN_LED);
}

/** 
 * Indicates failure in self tests; could be EEPROM, temperature sensor or Ethernet failure
 */
void Self_Test_Failed_LED_INDICATE() {
    /* Solid Red + Green OFF = Solid Red */
    PORTClearBits(IOPORT_A, RED_LED);
    PORTSetBits(IOPORT_A, GREEN_LED);
}

/** 
 * Indicates normal mode operation after all tests and message communication successful
 */
void Normal_Operation_LED_INDICATE() {
    /* Red OFF + Solid Green = Solid Green */
    PORTSetBits(IOPORT_A, RED_LED);
    PORTClearBits(IOPORT_A, GREEN_LED);
}

/** 
 * Indicates either no current sensors are connected to SSN or they are reading zero
 */
void Current_Sensors_Disconnected_LED_INDICATE() {
    /* Red OFF + Green Toggle = Green Toggle */
    PORTSetBits(IOPORT_A, RED_LED);
    PORTToggleBits(IOPORT_A, GREEN_LED);
}

/** 
 * Indicates Abnormal activity sensed in ambient temperature or relative humidity readings
 */
void Abnormal_Activity_LED_INDICATE() {
    /* Red Toggle + Green OFF = Red Toggle */
    PORTToggleBits(IOPORT_A, RED_LED);
    PORTSetBits(IOPORT_A, GREEN_LED);
}

/** 
 * Indicates trying to connect to a network
 */
void establishing_connection_LED_INDICATE() {
    /* Red OFF + Green Toggle = Green Toggle */
    PORTSetBits(IOPORT_A, RED_LED);
    PORTToggleBits(IOPORT_A, GREEN_LED);
}

/** 
 * Indicates SSN state from LED
 * @param this_state A variable indicating the current state of SSN 
 */
void SSN_LED_INDICATE(uint8_t this_state) {
    /* This one function will determine the state of SSN and give the proper LED heartbeat indication */
    switch (this_state) {
        /* No mac address? */
        case NO_MAC_STATE:
            Node_Up_Not_Configured_LED_INDICATE();
            break;
        case GETTING_IP_FROM_DHCP:
            establishing_connection_LED_INDICATE();
            break;
        case LOOKING_UP_DNS:
            establishing_connection_LED_INDICATE();
            break;
        case ESTABLISHING_MQTT_CONNECTION:
            establishing_connection_LED_INDICATE();
            break;        /* No configuration? */
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

void SetPinForMicroseconds(uint32_t port, uint32_t pin, uint32_t us) {
    PORTSetBits(port, pin);
    sleep_for_microseconds(us);
}

void ClearPinForMicroseconds(uint32_t port, uint32_t pin, uint32_t us) {
    PORTClearBits(port, pin);
    sleep_for_microseconds(us);
}
