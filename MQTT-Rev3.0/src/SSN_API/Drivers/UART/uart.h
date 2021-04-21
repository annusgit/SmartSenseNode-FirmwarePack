#ifndef __uart_h__
#define __uart_h__

#include "../../global.h"

/** 
 * Opens UART peripheral for communication 
 * @param baudrate
 */
void open_UART1(unsigned int baudrate);  // MODBUS
void open_UART2(unsigned int baudrate);  // printf

/** 
 * Transmits a byte array over serial interface
 * @param buffer A pointer to a byte array to send over the UART
 * @return 0 if successful
 */
int SerialTransmit_UART2(const char *buffer);

/** 
 * Receives a byte array over serial interface
 * @param buffer A pointer to a byte array to write the received message into
 * @param max_size Maximum number of bytes expected to be received over serial interface
 * @return 0 if successful
 */
unsigned int SerialReceive_UART2(char *buffer, unsigned int max_size);

// API high-level functions
/**
 * Sets up the print function at a specific baudrate
 * @param baudrate
 */
void setup_printf(unsigned int baudrate);


#endif

