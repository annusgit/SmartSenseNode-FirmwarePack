#include "uart.h"

void open_UART1(unsigned int baudrate) {
    /* To open and configure UART1 for printf function */
    RPB3R  = 0x0001;            // U1TX
    U1RXR  = 0x0001;            // U1RX
    U1MODE = 0x00000000;        // U1 configurations
    U1STA  = 0x1400;
    U1BRG  = (PERIPH_CLK/(16*baudrate))-1;
    U1MODESET = 0x00008000;     // enable UART1
}

void open_UART2(unsigned int baudrate) {
    /* To open and configure UART2 for printf function */
    RPB14R = 0x0002;            // U2TX
    U2RXR  = 0x0003;            // U2RX
    U2MODE = 0x00000000;        // U2 configurations
    U2STA  = 0x1400;
    U2BRG  = (PERIPH_CLK/(16*baudrate))-1;
    U2MODESET = 0x00008000;     // enable UART2
}

void setup_printf(unsigned int baudrate) {
    open_UART2(baudrate);
}

/* 
 * SerialTransmit() transmits a string to the UART2 TX pin MSB first
 * Inputs: *buffer = string to transmit */
int SerialTransmit_UART2(const char* buffer) {
    unsigned int size = strlen(buffer);
    while( size) {
        while( U2STAbits.UTXBF);    // wait while TX buffer full
        U2TXREG = *buffer;          // send single character to transmit buffer
 
        buffer++;                   // transmit next character on following loop
        size--;                     // loop until all characters sent (when size = 0)
    }
 
    while( !U2STAbits.TRMT);        // wait for last transmission to finish
 
    return 0;
}
 
/* SerialReceive() is a blocking function that waits for data on
 *  the UART2 RX buffer and then stores all incoming data into *buffer
 *
 * Note that when a carriage return '\r' is received, a nul character
 *  is appended signifying the strings end
 *
 * Inputs:  *buffer = Character array/pointer to store received data into
 *          max_size = number of bytes allocated to this pointer
 * Outputs: Number of characters received */
unsigned int SerialReceive_UART2(char *buffer, unsigned int max_size) {
    unsigned int num_char = 0;
    /* Wait for and store incoming data until either a carriage return is received
     *   or the number of received characters (num_chars) exceeds max_size */
    while(num_char < max_size) {
        while( !U2STAbits.URXDA);   // wait until data available in RX buffer
        *buffer = U2RXREG;          // empty contents of RX buffer into *buffer pointer 
        // insert nul character to indicate end of string
        if( *buffer == '\r') {
            *buffer = '\0';     
            break;
        }
        buffer++;
        num_char++;
    }
    return num_char;
} // END SerialReceive()