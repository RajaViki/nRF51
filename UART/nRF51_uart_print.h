 /*
 * Arduino like functions for serial printing of numbers and strings
 * Created by: Domen Jurkovic, 25.12.2015
 * 
 */

#ifndef NRF51_UART_PRINT_H
#define NRF51_UART_PRINT_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>	

#include "nrf.h"
#include "app_uart.h"
#include "app_error.h"
#include "app_fifo.h"

//uint8_t base:
#define	DEC	10
#define BIN 2
#define HEX 16
#define OCT 8

/****************************************************************************************/
/* UART INIT FUNCTIONS - change in .c file accordingly to your HW */
/****************************************************************************************/
// initialize UART
uint32_t UART_Init(uint8_t rx_pin, uint8_t tx_pin, uint32_t baud_rate);
void uart_error_handle(app_uart_evt_t * p_event);

uint32_t uart_send_byte(uint8_t byte);
uint8_t uart_receive_byte(void);


/****************************************************************************************/
/* PRINT/WRITE FUNCTIONS */
/****************************************************************************************/
//print WITHOUT new line and carriage return
void printString(char *data);	//send/print string overserial port UART.
void printNumber(int32_t number, uint8_t base);	//send/print SINGED/UNSIGNED int32_t number over serial port UART.

//print WITH new line and carriage return
void printStringLn(char *data);	//send/print string overserial port UART.
void printNumberLn(int32_t number, uint8_t base);	//send/print SINGED/UNSIGNED number over serial port UART.
void printLn(void);	//print new line and carriage return

//send raw data, any type.
void writeData(void *data, uint8_t dataSize);

//"private" function. Can be used if needed.
void printUnsignedNumber(uint32_t n, uint8_t base);	//send/print UNSIGNED uint32_t over serial port UART.

void printFloat(float number);
void printFloatLn(float number);

#endif

