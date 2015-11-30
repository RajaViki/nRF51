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
 * Updated/customised by Domen Jurkovic, 30.11.2015
 * Initialize with: 
	#define RX_PIN_NUMBER 17	// UART RX pin number.
	#define TX_PIN_NUMBER 18	// UART TX pin number.
	#define CTS_PIN_NUMBER 20	// UART Clear To Send pin number. Not used if HWFC is set to false. 
	#define RTS_PIN_NUMBER 19	// UART Request To Send pin number. Not used if HWFC is set to false. 
	#define HWFC	false				// UART hardware flow control.
	simple_uart_config(RTS_PIN_NUMBER, TX_PIN_NUMBER, CTS_PIN_NUMBER, RX_PIN_NUMBER, HWFC);
 */

#include <stdint.h>

#include "nrf.h"
#include "simple_uart.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

uint8_t simple_uart_get(void)
{
    while (NRF_UART0->EVENTS_RXDRDY != 1)
    {
        // Wait for RXD data to be received
    }

    NRF_UART0->EVENTS_RXDRDY = 0;
    return (uint8_t)NRF_UART0->RXD;
}


bool simple_uart_get_with_timeout(int32_t timeout_ms, uint8_t * rx_data)
{
    bool ret = true;

    while (NRF_UART0->EVENTS_RXDRDY != 1)
    {
        if (timeout_ms-- >= 0)
        {
            // wait in 1ms chunk before checking for status.
            nrf_delay_us(1000);
        }
        else
        {
            ret = false;
            break;
        }
    } // Wait for RXD data to be received.

    if (timeout_ms >= 0)
    {
        // clear the event and set rx_data with received byte.
        NRF_UART0->EVENTS_RXDRDY = 0;
        *rx_data                 = (uint8_t)NRF_UART0->RXD;
    }

    return ret;
}

void simple_uart_put(uint8_t cr)
{
    NRF_UART0->TXD = (uint8_t)cr;

    while (NRF_UART0->EVENTS_TXDRDY != 1)
    {
        // Wait for TXD data to be sent.
    }

    NRF_UART0->EVENTS_TXDRDY = 0;
}


void simple_uart_putstring(const uint8_t * str)
{
    uint_fast8_t i  = 0;
    uint8_t      ch = str[i++];

    while (ch != '\0')
    {
        simple_uart_put(ch);
        ch = str[i++];
    }
}

void simple_uart_config(uint8_t rts_pin_number,
                        uint8_t txd_pin_number,
                        uint8_t cts_pin_number,
                        uint8_t rxd_pin_number,
                        bool    hwfc)
{
/** @snippet [Configure UART RX and TX pin] */
    nrf_gpio_cfg_output(txd_pin_number);
    nrf_gpio_cfg_input(rxd_pin_number, NRF_GPIO_PIN_NOPULL);

    NRF_UART0->PSELTXD = txd_pin_number;
    NRF_UART0->PSELRXD = rxd_pin_number;
/** @snippet [Configure UART RX and TX pin] */
    if (hwfc)
    {
        nrf_gpio_cfg_output(rts_pin_number);
        nrf_gpio_cfg_input(cts_pin_number, NRF_GPIO_PIN_NOPULL);
        NRF_UART0->PSELCTS = cts_pin_number;
        NRF_UART0->PSELRTS = rts_pin_number;
        NRF_UART0->CONFIG  = (UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);
    }

    NRF_UART0->BAUDRATE      = (UART_BAUDRATE_BAUDRATE_Baud38400 << UART_BAUDRATE_BAUDRATE_Pos);
    NRF_UART0->ENABLE        = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
    NRF_UART0->TASKS_STARTTX = 1;
    NRF_UART0->TASKS_STARTRX = 1;
    NRF_UART0->EVENTS_RXDRDY = 0;
}

/****************************************************************************************/
/* CUSTOM FUNCTIONS */
/****************************************************************************************/

/*
	Send/print character/string over USART1. 
	Printable data for viewing on terminal.
	Call this function:		printString("test data");
	Data must be string.
*/
void printString(char *data){
	uint16_t i;
	uint16_t str_length = strlen(data);
	for(i = 0; i < str_length; i++)
	{
		simple_uart_put(data[i]);
	}
}


/*
	Send/print unsigned or signed number over USART1. 
	Printable data for viewing on terminal.
	Call this function:		printNumber(USART1, number, DEC);		printNumber(2246, DEC);	
	Base: DEC, HEX, OCT, BIN
	Data must be number, int32_t.
*/
void printNumber(int32_t number, uint8_t base){
	if (number < 0) 
	{
		printString("-");
		number = -number;
		printUnsignedNumber(number, base);
	}
	else 
	{
		printUnsignedNumber(number, base);
	}
}
//print WITH new line and carriage return
void printStringLn(char *data){
	uint16_t i;
	uint16_t str_length = strlen(data);
	for(i = 0; i < str_length; i++)
	{
		simple_uart_put(data[i]);
	}
	printLn();
}

/*
	Send/print unsigned or signed number over USART1. 
	Printable data for viewing on terminal.
	Call this function:		printNumber(USART1, number, DEC);		printNumber(USART1, 2246, DEC);	
	Base: DEC, HEX, OCT, BIN
	Data must be number, int32_t.
*/
void printNumberLn(int32_t number, uint8_t base){
	if (number < 0) 
	{
		printString("-");
		number = -number;
		printUnsignedNumber(number, base);
	}
	else 
	{
		printUnsignedNumber(number, base);
	}
	printLn();
}

void printLn(){
	printString("\n\r");
}

/*
	Send raw data over USART1. 
	Not "printable" data.
	Call this function:		writeData(USARTx, &data, sizeof(data));
	Data can be any type.
*/
void writeData(void *data, uint8_t dataSize){
  uint8_t i, d;
  
  d = dataSize/2;
 
  for(i = 0; i < d; i++)
  {
    simple_uart_put(*( ((uint16_t *)data) + i ) );
  }
 
}

/*
	This is "private" function. It is used by other functions like: printNumber(int32_t number, uint8_t base). 
	However, it can be used by user.
	Send/print unsigned number over USART1. 
	Printable data for viewing on terminal.
	Call this function:		printUnsignedNumber(number, DEC);		printUnsignedNumber(USART1, 2246, DEC);	
	Base: DEC, HEX, OCT, BIN
	Data must be number, int32_t.
*/
void printUnsignedNumber(uint32_t n, uint8_t base){
	char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
	char *str = &buf[sizeof(buf) - 1];
	unsigned long m;
	char c;
  *str = '\0';

  //prevent crash if called with base == 1
  if (base < 2) base = 10;

  do 
	{
    m = n;
    n /= base;
    c = m - base * n;
    *--str = c < 10 ? c + '0' : c + 'A' - 10;
	} while(n);

	printString(str); 
}

void printFloat(float number){
	int32_t integer_part = (int32_t) number;
	float decimal_part =  (number - (float)integer_part) * 10000;
	
	if((integer_part == 0) && (decimal_part < 0)){
		printString("-");
	}
	printNumber(integer_part, DEC);
	printString(".");
	
	if(decimal_part < 0){
		decimal_part = - decimal_part;
	}
	if(decimal_part < 1000){
			if(decimal_part < 100)	printNumber(0, DEC);
			if(decimal_part < 10)	printNumber(0, DEC);
			printNumber(0, DEC);
	}
	printNumber((uint32_t)decimal_part, DEC);
}

void printFloatLn(float number){
	printFloat(number);
	printLn();
}
