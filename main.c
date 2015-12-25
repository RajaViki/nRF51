/*
 *	Created by: Domen Jurkovic, 25.12.2015
 *	Example project for HX711 and simple_uart
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "simple_uart.h"
#include "nrf_hx711.h"


// USER MUST DEFINE PINS
#define RX_PIN_NUMBER 18	// UART RX pin number.
#define TX_PIN_NUMBER 17	// UART TX pin number.

HX711_pin_typedef		HX711_pin_structure;
HX711_data_typedef	HX711_data_structure_chA;
HX711_data_typedef	HX711_data_structure_chB;

void HX711_setup(void){
	HX711_pin_structure.dout_pin		=	9;
	HX711_pin_structure.pd_sck_pin	=	10;
	HX711_pin_structure.rate_pin		= 8;
		
	HX711_init(&HX711_pin_structure);	// init pins
	
	//channel A
	HX711_data_structure_chA.av_rate = 10;
	HX711_data_structure_chA.gain = HX_GAIN_128; 
	HX711_data_structure_chA.rate = HX_RATE_10;
	HX711_data_structure_chA.offset = 0;
	HX711_data_structure_chA.result = 0;
	
	//channel B
	HX711_data_structure_chB.av_rate = 10;
	HX711_data_structure_chB.gain = HX_GAIN_32; 
	HX711_data_structure_chB.rate = HX_RATE_10;
	HX711_data_structure_chB.offset = 0;
	HX711_data_structure_chB.result = 0;

	// dummy measurements
	while(HX711_is_ready(&HX711_pin_structure) != HX_STAT_READY){
	}
	HX711_measure(&HX711_pin_structure, &HX711_data_structure_chA);
	while(HX711_is_ready(&HX711_pin_structure) != HX_STAT_READY){
	}
	HX711_measure(&HX711_pin_structure, &HX711_data_structure_chB);
	
	HX711_data_structure_chA.result = 0;
	HX711_data_structure_chB.result = 0;
}


int main(void)
{
	// UART
	printNumberLn(uart_init(RX_PIN_NUMBER, TX_PIN_NUMBER, UART_BAUDRATE_BAUDRATE_Baud19200), DEC);
	printStringLn("-----------------------------------------------");
	
	HX711_setup();
		
	HX711_eliminate_offset(&HX711_pin_structure, &HX711_data_structure_chA);
	printStringLn("CH A offset eliminated.");
	
	/*
	//za merjenje baterije, popravi HX_SHIFT_BITS_CHB v nrf_hx711.h
	HX711_eliminate_offset(&HX711_pin_structure, &HX711_data_structure_chB);
	printStringLn("CH B offset eliminated.");
	*/
	
	printStringLn("System initialised");
	
	while (true){
		
		HX711_power_up(&HX711_pin_structure);
		printNumberLn(HX711_measure(&HX711_pin_structure, &HX711_data_structure_chA), DEC);
		printNumberLn(HX711_measure(&HX711_pin_structure, &HX711_data_structure_chB), DEC);
		HX711_power_down(&HX711_pin_structure);
		printLn();
		nrf_delay_ms(100);
		
		if(uart_receive_byte() == 'x'){
			printStringLn("x!");
		}
	}
}
