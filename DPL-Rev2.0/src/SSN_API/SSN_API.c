#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

#include "SSN_API.h"

/** Our SSN UDP communication socket */
SOCKET SSN_UDP_SOCKET;
/** SSN Server Address */
uint8_t SSN_SERVER_IP[] = {192, 168, 0, 130};
/** SSN Server PORT */
uint16_t SSN_SERVER_PORT = 9999;

/** Static IP Assignment */
uint8_t SSN_STATIC_IP[4] = {192, 168, 0, 164};
uint8_t SSN_SUBNET_MASK[4]      = {255, 255, 255, 0};
uint8_t SSN_GATWAY_ADDRESS[4]   = {192, 168, 0, 1};

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
/** Current State of the SSN. There is no state machine of the SSN but we still use this variable to keep track at some instances */
uint8_t SSN_CURRENT_STATE = NO_MAC_STATE, SSN_PREV_STATE;
/** Report Interval of SSN set according to the configurations passed to the SSN */
uint8_t SSN_REPORT_INTERVAL = 1;
/** SSN current sensor configurations */
uint8_t SSN_CONFIG[EEPROM_CONFIG_SIZE];
/** SSN current sensor ratings */
uint8_t SSN_CURRENT_SENSOR_RATINGS[4];
/** SSN machine thresholds for deciding IDLE state */
uint8_t SSN_CURRENT_SENSOR_THRESHOLDS[4];
/** SSN machine maximum loads for calculating percentage loads on machines */
uint8_t SSN_CURRENT_SENSOR_MAXLOADS[4];
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
bool socket_ok = true;
/** SSN loop variable */
uint8_t i;

void SSN_Setup() {
    // Setup calls for all our peripherals/devices
    setup_printf(19200);
    setup_EEPROM();
    setup_Ethernet(5000000);
    setup_Current_Sensors();
    setup_Temperature_Humidity_Sensor();
    setup_LED_Indicator();
    setup_Interrupts();
}

void SSN_COPY_MAC_FROM_MEMORY() {
    SSN_PREV_STATE = SSN_CURRENT_STATE;
    SSN_CURRENT_STATE = FindMACInFlashMemory(SSN_MAC_ADDRESS, SSN_DEFAULT_MAC);
    if(SSN_PREV_STATE!=SSN_CURRENT_STATE) {
        Clear_LED_INDICATOR();
    }
}

void SSN_GET_MAC() {
    uint16_t SendAfter = 0;
    // When we will receive a MAC address (if we didn't have it), we will reset the controller
    while (SSN_CURRENT_STATE == NO_MAC_STATE) {
        SSN_CHECK_ETHERNET_CONNECTION();
        SSN_PREV_STATE = SSN_CURRENT_STATE;
        SSN_CURRENT_STATE = NO_MAC_STATE;
        if(SSN_PREV_STATE!=SSN_CURRENT_STATE) {
            Clear_LED_INDICATOR();
        }
        // request a MAC address after every 5 seconds
        if (SendAfter % 50 == 0) {
            Send_GETMAC_Message(SSN_MAC_ADDRESS, SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT);
        }
        // Try to receive a message every 100 milliseconds
        Receive_MAC(SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT);
        // Give LED indication every second
        if (SendAfter % 10 == 0) {
            SSN_LED_INDICATE(SSN_CURRENT_STATE);
        }
        SendAfter++;       
        ServiceWatchdog();
        // 100 milliseconds
        sleep_for_microseconds(100000);
    }
    return;
}

void SSN_GET_CONFIG() {
    // Find configurations in EEPROM
    SSN_PREV_STATE = SSN_CURRENT_STATE;
    SSN_CURRENT_STATE = FindSensorConfigurationsInFlashMemory(SSN_CONFIG, &SSN_REPORT_INTERVAL, SSN_CURRENT_SENSOR_RATINGS, SSN_CURRENT_SENSOR_THRESHOLDS, SSN_CURRENT_SENSOR_MAXLOADS);
    uint16_t SendAfter = 0; 
    while (SSN_CURRENT_STATE == NO_CONFIG_STATE) {
        SSN_CHECK_ETHERNET_CONNECTION();
        SSN_PREV_STATE = SSN_CURRENT_STATE;
        SSN_CURRENT_STATE = NO_CONFIG_STATE;
        if(SSN_PREV_STATE!=SSN_CURRENT_STATE) {
            Clear_LED_INDICATOR();
        }
        // request a Configuration after every 5 seconds
        if (SendAfter % 50 == 0) {
            Send_GETCONFIG_Message(SSN_MAC_ADDRESS, SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT);
        }
        // Try to receive a message every 100 milliseconds
        if (Receive_CONFIG(SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT, SSN_CONFIG, &SSN_REPORT_INTERVAL, SSN_CURRENT_SENSOR_RATINGS, SSN_CURRENT_SENSOR_THRESHOLDS, 
                SSN_CURRENT_SENSOR_MAXLOADS, Machine_status)) {
            break;
        }
        // Give LED indication every second
        if (SendAfter % 10 == 0) {
            SSN_LED_INDICATE(SSN_CURRENT_STATE);
        }
        SendAfter++;
        ServiceWatchdog();
        // 100 milliseconds
        sleep_for_microseconds(100000);
    }
    return;
}

void SSN_GET_CONFIG_WITH_5_SECONDS_HALT() {
    // Wait for new configurations for five seconds
    printf("LOG: Waiting for updated configurations from Server...\n");
    // Notify the server you are waiting for configurations
    Send_GETCONFIG_Message(SSN_MAC_ADDRESS, SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT);
    uint16_t SendAfter = 0; 
    bool NewConfigsReceived = false;        
    while (SendAfter < 50) {
        SSN_CHECK_ETHERNET_CONNECTION();
        SSN_PREV_STATE = SSN_CURRENT_STATE;
        SSN_CURRENT_STATE = NO_CONFIG_STATE;
        if(SSN_PREV_STATE!=SSN_CURRENT_STATE) {
            Clear_LED_INDICATOR();
        }
        // Try to receive a message every 100 milliseconds
        if (Receive_CONFIG(SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT, SSN_CONFIG, &SSN_REPORT_INTERVAL, SSN_CURRENT_SENSOR_RATINGS, SSN_CURRENT_SENSOR_THRESHOLDS, 
                SSN_CURRENT_SENSOR_MAXLOADS, Machine_status)) {
            NewConfigsReceived = true;
            break;
        }
        // Give LED indication every second
        if (SendAfter % 10 == 0) {
            SSN_LED_INDICATE(NO_CONFIG_STATE);
        }
        SendAfter++;
        ServiceWatchdog();
        // 100 milliseconds
        sleep_for_microseconds(100000);
    }
    if (!NewConfigsReceived) {
        // Find configurations in EEPROM
        SSN_PREV_STATE = SSN_CURRENT_STATE;
        SSN_CURRENT_STATE = FindSensorConfigurationsInFlashMemory(SSN_CONFIG, &SSN_REPORT_INTERVAL, SSN_CURRENT_SENSOR_RATINGS, SSN_CURRENT_SENSOR_THRESHOLDS, SSN_CURRENT_SENSOR_MAXLOADS);
        SendAfter = 0;
        while (SSN_CURRENT_STATE == NO_CONFIG_STATE) {
            SSN_CHECK_ETHERNET_CONNECTION();
            SSN_PREV_STATE = SSN_CURRENT_STATE;
            SSN_CURRENT_STATE = NO_CONFIG_STATE;
            if(SSN_PREV_STATE!=SSN_CURRENT_STATE) {
                Clear_LED_INDICATOR();
            }
            // request a MAC address after every 5 seconds
            if (SendAfter % 50 == 0) {
                Send_GETCONFIG_Message(SSN_MAC_ADDRESS, SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT);
            }
            // Try to receive a message every 100 milliseconds
            if (Receive_CONFIG(SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT, SSN_CONFIG, &SSN_REPORT_INTERVAL, SSN_CURRENT_SENSOR_RATINGS, SSN_CURRENT_SENSOR_THRESHOLDS, 
                    SSN_CURRENT_SENSOR_MAXLOADS, Machine_status)) {
                break;
            }
            // Give LED indication every second
            if (SendAfter % 10 == 0) {
                SSN_LED_INDICATE(SSN_CURRENT_STATE);
            }
            SendAfter++;
            ServiceWatchdog();
            // 100 milliseconds
            sleep_for_microseconds(100000);
        }
    }
    return;
}

void SSN_GET_TIMEOFDAY() {
    uint16_t SendAfter = 0;
    while(1) {
        SSN_CHECK_ETHERNET_CONNECTION();
        SSN_PREV_STATE = SSN_CURRENT_STATE;
        SSN_CURRENT_STATE = NO_CONFIG_STATE;
        if(SSN_PREV_STATE!=SSN_CURRENT_STATE) {
            Clear_LED_INDICATOR();
        }
        // request a MAC address after every 5 seconds
        if (SendAfter % 50 == 0) {
            Send_GETTimeOfDay_Message(SSN_MAC_ADDRESS, SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT);
        }
        // Try to receive a message every 100 milliseconds
        if (Receive_TimeOfDay(SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT)) {
            // initialize SSN's uptime
            ssn_uptime_in_seconds = 0;
            SSN_PREV_STATE = SSN_CURRENT_STATE;
            SSN_CURRENT_STATE = NORMAL_ACTIVITY_STATE;
            if(SSN_PREV_STATE!=SSN_CURRENT_STATE) {
                Clear_LED_INDICATOR();
            }
            break;   
        }
        // Give LED indication every second
        if (SendAfter % 10 == 0) {
            SSN_LED_INDICATE(SSN_CURRENT_STATE);
        }
        SendAfter++;       
        ServiceWatchdog();
        // 100 milliseconds
        sleep_for_microseconds(100000);
    }
    return;
}

void SSN_RECEIVE_ASYNC_MESSAGE() {
    // We can receive configurations and time of day on the fly
    Receive_CONFIG(SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT, SSN_CONFIG, &SSN_REPORT_INTERVAL, SSN_CURRENT_SENSOR_RATINGS, SSN_CURRENT_SENSOR_THRESHOLDS, SSN_CURRENT_SENSOR_MAXLOADS, 
            Machine_status);
    Receive_TimeOfDay(SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT);
}

void SSN_CHECK_ETHERNET_CONNECTION() {    
    bool LinkWasOkay = true;
    // Check Ethernet Physical Link Status before sending message
    while(Ethernet_get_physical_link_status() == PHY_LINK_OFF) {
        SSN_PREV_STATE = SSN_CURRENT_STATE;
        SSN_CURRENT_STATE = NO_ETHERNET_STATE;
        if(SSN_PREV_STATE!=SSN_CURRENT_STATE) {
            LinkWasOkay = false;
            Clear_LED_INDICATOR();
        }
        printf("LOG: I Am Stuck :/ Ethernet Physical Link BAD...\n");
        SSN_LED_INDICATE(SSN_CURRENT_STATE);
        // Service the watchdog timer to make sure we don't reset 
        ServiceWatchdog();
        sleep_for_microseconds(500000);
    }
    if(!LinkWasOkay) {
        SSN_PREV_STATE = SSN_CURRENT_STATE;
        SSN_CURRENT_STATE = NORMAL_ACTIVITY_STATE;
        if(SSN_PREV_STATE!=SSN_CURRENT_STATE) {
            LinkWasOkay = false;
            Clear_LED_INDICATOR();
        }
        printf("LOG: Ethernet Physical Link Recovered :)\n");
    }
    return;
}

void SSN_GET_AMBIENT_CONDITION() {
    // sample sensors and do calculations
    temp_humidity_recv_status = sample_Temperature_Humidity_bytes(temperature_bytes, relative_humidity_bytes);
    if(temp_humidity_recv_status==SENSOR_READ_ERROR) {
        abnormal_activity = TEMP_SENSOR_READ_ERROR_CONDITION;
        return;
    }
    if(temp_humidity_recv_status==SENSOR_READ_CRC_ERROR) {
        abnormal_activity = TEMP_SENSOR_CRC_ERROR_CONDITION;
        return;
    }
    abnormal_activity = ambient_condition_status();
    if (abnormal_activity == NORMAL_AMBIENT_CONDITION) {
        SSN_PREV_STATE = SSN_CURRENT_STATE;
        SSN_CURRENT_STATE = NORMAL_ACTIVITY_STATE;
        if(SSN_PREV_STATE!=SSN_CURRENT_STATE) {
            Clear_LED_INDICATOR();
        }
    }
    else {
        SSN_PREV_STATE = SSN_CURRENT_STATE;
        SSN_CURRENT_STATE = ABNORMAL_ACTIVITY_STATE;
        if(SSN_PREV_STATE!=SSN_CURRENT_STATE) {
            Clear_LED_INDICATOR();
        }
    }
    return;
}

void SSN_RESET_AFTER_N_SECONDS(uint32_t seconds) {       
    /* Check if we should reset (after given minutes when machines are not ON) */
    if(ssn_uptime_in_seconds > seconds) {
        printf("Time to have some rest... I'll wake up in about 5 seconds...\n");
        SoftReset();
        while(1);
    }
    return;
}

void SSN_RESET_AFTER_N_SECONDS_IF_NO_MACHINE_ON(uint32_t seconds) {       
    /* Check if we should reset (after given minutes when machines are not ON) */
    if(ssn_uptime_in_seconds > seconds && Machine_status[0] != MACHINE_ON && Machine_status[1] != MACHINE_ON && Machine_status[2] != MACHINE_ON && Machine_status[3] != MACHINE_ON) {
        printf("Time to have some rest... I'll wake up in about 5 seconds...\n");
        SoftReset();
        while(1);
    }
    return;
}

void SSN_RESET_IF_SOCKET_CORRUPTED() {
    if(!socket_ok) {
        SSN_PREV_STATE = SSN_CURRENT_STATE;
        SSN_CURRENT_STATE = NO_ETHERNET_STATE;
        if(SSN_PREV_STATE!=SSN_CURRENT_STATE) {
            Clear_LED_INDICATOR();
        }
        printf("-> Socket Corrupted. Reinitializing SSN..\n");
        SoftReset();
    }
}

void led_blink_test() {
    SSN_Setup();
    Clear_LED_INDICATOR();
    while(true) {
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
    while(true) {
        printf("In here\n");
        Calculate_RMS_Current_On_All_Channels(SSN_CURRENT_SENSOR_RATINGS, 400, Machine_load_currents);
        // sleep for a second
        sleep_for_microseconds(1000000);
    }
    // we should never get to this point
    return;
}

int AM2320_test() {
        SSN_Setup();
        printf("Start\n");
        while (1) {
                printf("Working\n");
                // Read temperature and humidity sensor
                SSN_GET_AMBIENT_CONDITION();
                sleep_for_microseconds(2000000);
        }
        return 0;
}

void network_test() {
    SSN_Setup();
    SSN_UDP_SOCKET = SetupConnectionWithStaticIP(SSN_UDP_SOCKET_NUM, SSN_MAC_ADDRESS, SSN_STATIC_IP, SSN_SUBNET_MASK, SSN_GATWAY_ADDRESS);
    uint8_t test_message_array[100] = "I am Annus Zulfiqar and I am trying to test this network";
    uint8_t test_message_size = 56;
    while(true) {
        socket_ok = SendMessage(SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT, test_message_array, test_message_size);
        if(!socket_ok) {
            printf("Socket Corrupted. Reinitializing..\n");
            setup_Ethernet(5000000);
            SSN_UDP_SOCKET = SetupConnectionWithStaticIP(SSN_UDP_SOCKET_NUM, SSN_MAC_ADDRESS, SSN_STATIC_IP, SSN_SUBNET_MASK, SSN_GATWAY_ADDRESS);
            printf("Reinitialization Successful.\n");
        }
        sleep_for_microseconds(1000000);
    }
    // we should never get to this point
    return;
}

void watchdog_test() {
    SSN_Setup();
    printf("################# Testing Watchdog #################\n");
    EnableWatchdog();
    int seconds = 1;
    while(true) {
        ServiceWatchdog();
        printf("Sleeping for %d seconds\n", seconds);
        sleep_for_microseconds(seconds*1000000);
        seconds++;
    }
    return;
}


