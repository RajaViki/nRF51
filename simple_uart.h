 /* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 *	Updated/customised by Domen Jurkovic, 30.11.2015 
 */

#ifndef SIMPLE_UART_H
#define SIMPLE_UART_H

/*lint ++flb "Enter library region" */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>	

//uint8_t base:
#define	DEC	10
#define BIN 2
#define HEX 16
#define OCT 8

/** @file
* @brief Simple UART driver
*
*
* @defgroup nrf_drivers_simple_uart Simple UART driver
* @{
* @ingroup nrf_drivers
* @brief Simple UART driver
*/

/** @brief Function for reading a character from UART.
Execution is blocked until UART peripheral detects character has been received.
\return cr Received character.
*/
uint8_t simple_uart_get(void);

/** @brief Function for reading a character from UART with timeout on how long to wait for the byte to be received.
Execution is blocked until UART peripheral detects character has been received or until the timeout expires, which even occurs first
\return bool True, if byte is received before timeout, else returns False.
@param timeout_ms maximum time to wait for the data.
@param rx_data pointer to the memory where the received data is stored.
*/
bool simple_uart_get_with_timeout(int32_t timeout_ms, uint8_t *rx_data);

/** @brief Function for sending a character to UART.
Execution is blocked until UART peripheral reports character to have been send.
@param cr Character to send.
*/
void simple_uart_put(uint8_t cr);

/** @brief Function for sending a string to UART.
Execution is blocked until UART peripheral reports all characters to have been send.
Maximum string length is 254 characters including null character in the end.
@param str Null terminated string to send.
*/
void simple_uart_putstring(const uint8_t *str);

/** @brief Function for configuring UART to use 38400 baud rate.
@param rts_pin_number Chip pin number to be used for UART RTS
@param txd_pin_number Chip pin number to be used for UART TXD
@param cts_pin_number Chip pin number to be used for UART CTS
@param rxd_pin_number Chip pin number to be used for UART RXD
@param hwfc Enable hardware flow control
*/
void simple_uart_config(uint8_t rts_pin_number, uint8_t txd_pin_number, uint8_t cts_pin_number, uint8_t rxd_pin_number, bool hwfc);


/****************************************************************************************/
/* CUSTOM FUNCTIONS */
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



/**
 *@}
 **/

/*lint --flb "Leave library region" */
#endif
