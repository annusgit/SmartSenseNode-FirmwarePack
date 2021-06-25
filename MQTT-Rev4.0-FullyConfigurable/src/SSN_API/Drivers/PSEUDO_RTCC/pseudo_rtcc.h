
#ifndef __pseudo_rtcc_h__
#define __pseudo_rtcc_h__


#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

#include <xc.h>
#include <p32xxxx.h>
#include <plib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/** A structure for maintaining the global clock of Smart Sense Node */
typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} pseudo_clock;

/** This is our global SSN clock variable */
uint32_t ssn_static_clock, ssn_dynamic_clock;

/** This is our global SSN clock in number of seconds, for how many seconds has the SSN been awake? */
uint32_t ssn_uptime_in_seconds;

///**
// * Sets up the global clock and half-second interrupt for SSN
// * @param PERIPH_CLOCK Peripheral clock of SSN
// */
//void setup_Global_Clock_And_Half_Second_Interrupt(uint32_t PERIPH_CLOCK);
//
///**
// * Starts half-second interrupt from Timer-1
// */
//void EnableGlobalHalfSecondInterrupt();
//
///**
// * Stops half-second interrupt from Timer-1
// */
//void DisableGlobalHalfSecondInterrupt();

/**
 * Stops the global SSN clock
 */
void stop_Global_Clock();

/**
 * Sets the SSN global time to give time
 * @param this_time Byte array containing hours, minutes, seconds, day, month and year of current time
 */
void set_ssn_time(uint32_t this_time);

/**
 * Increments given clock time by one tick or one second
 * @param this_clock The uint32_t variable clock (in tick seconds) to increment
 */
void increment_pseudo_clock_time(pseudo_clock* this_clock);

/**
 * Increments given clock time by one tick or one second
 * @param this_clock The byte array representing clock to increment
 */
void increment_this_clock_time(uint8_t* this_clock);


#endif
