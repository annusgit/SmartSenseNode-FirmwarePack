#include "pseudo_rtcc.h"

uint8_t Days_in_a_Month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void set_ssn_time(uint32_t this_time) {
    // assign both clocks the same time
    ssn_static_clock = this_time;
    ssn_dynamic_clock = this_time;
}

void stop_Global_Clock() {
    IEC0CLR = 0x0010;       // disable timer 1 interrupt, IEC0<4>
    IFS0CLR = 0x0010;       // clear timer 1 int flag, IFS0<4>
    IPC1CLR = 0x001f;       // clear timer 1 priority/subpriority fields 
}

void increment_pseudo_clock_time(pseudo_clock* this_clock) {
    (*this_clock).seconds++;
    if ((*this_clock).seconds >= 60) {
        (*this_clock).seconds = 0;
        (*this_clock).minutes++;
        if ((*this_clock).minutes >= 60) {
            (*this_clock).minutes = 0;
            (*this_clock).hours++;
            if ((*this_clock).hours >= 24) {
                (*this_clock).hours = 0;
                (*this_clock).day++;
                if ((*this_clock).day >= Days_in_a_Month[(*this_clock).month-1]) {
                    (*this_clock).day = 1;
                    (*this_clock).month++;
                    if ((*this_clock).month >= 13) {
                        (*this_clock).month = 1;
                        (*this_clock).year++;
                        if ((*this_clock).year >= 100) {
                            (*this_clock).year = 0;                
                        }
                    }
                }
            }
        }
    }
}


void increment_this_clock_time(uint8_t* this_clock) {
    /* this_clock: [0:hours, 1:minutes, 2:seconds, 3: day, 4:month, 5:year]*/
    pseudo_clock temp;
    temp.hours   = this_clock[0];
    temp.minutes = this_clock[1];
    temp.seconds = this_clock[2];
    temp.day     = this_clock[3];
    temp.month   = this_clock[4];
    temp.year    = this_clock[5];

    increment_pseudo_clock_time(&temp);
    
    /* Now assign it back to the original clock pointed variable */
    this_clock[0] = temp.hours;
    this_clock[1] = temp.minutes;
    this_clock[2] = temp.seconds;
    this_clock[3] = temp.day;
    this_clock[4] = temp.month;
    this_clock[5] = temp.year;    
}



