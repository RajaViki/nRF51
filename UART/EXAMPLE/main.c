/*
	Created by Domen Jurkovic
	v1.0, 9-Feb-2016
	
	To use Noridc drivers, set up nrf_drv_config.h file
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nRF51_uart_print.h"


// USER MUST DEFINE:
#define RX_PIN_NUMBER 17	// UART RX pin number.
#define TX_PIN_NUMBER 18	// UART TX pin number.
#define BAUD_RATE			UART_BAUDRATE_BAUDRATE_Baud19200

int main(void){
	uint8_t received_data = 0;
	
	// UART
	UART_Init(RX_PIN_NUMBER, TX_PIN_NUMBER, BAUD_RATE);
	printStringLn("-----------------------------------------------");
	
	while (true)
	{
		printString("Test string");
		printLn();
		printStringLn("Test string with new line");
		printNumberLn(1050,DEC);
		printFloatLn(1050.012);
		
		received_data = uart_receive_byte();
		if(received_data == 'x'){
			printStringLn("X received!");
		}
		received_data = 0;
		
		nrf_delay_ms(250);
		
	}
}
