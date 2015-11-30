/*
	Example for simple_uart and nrf_hx711 library.
	Created by Domen Jurkovic, 30.11.2015
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf_delay.h"
#include "nrf_gpio.h"

#include "nrf_hx711.h"
#include "simple_uart.h"

#define LED 22 
#define RX_PIN_NUMBER 17	// UART RX pin number.
#define TX_PIN_NUMBER 18	// UART TX pin number.
#define CTS_PIN_NUMBER 20	// UART Clear To Send pin number. Not used if HWFC is set to false. 
#define RTS_PIN_NUMBER 19	// UART Request To Send pin number. Not used if HWFC is set to false. 
#define HWFC	false				// UART hardware flow control.

HX711_pin_typedef		HX711_pin_structure;
HX711_data_typedef	HX711_data_structure;


void system_setup(){
	nrf_gpio_cfg_output(LED);	//led setup on pin 22
}

void HX711_setup(void){
	HX711_pin_structure.dout_pin		=	23;
	HX711_pin_structure.pd_sck_pin	=	24;
	HX711_pin_structure.rate_pin		= 25;
	
	HX711_init(&HX711_pin_structure, &HX711_data_structure);
}

int main(void){
	system_setup();
		
	// UART
	simple_uart_config(RTS_PIN_NUMBER, TX_PIN_NUMBER, CTS_PIN_NUMBER, RX_PIN_NUMBER, HWFC);
	printStringLn("-----------------------------------------------");
	
	HX711_power_down(&HX711_pin_structure);
	HX711_power_up(&HX711_pin_structure);
	HX711_setup();
	
	if(HX711_is_ready(&HX711_pin_structure)){
		HX711_eliminate_offset(&HX711_pin_structure, &HX711_data_structure);
	}
	else{
		printStringLn("HX is not ready yet");
	}
	
	while (true)
	{
			printStringLn("---");
			/*nrf_gpio_pin_set(LED);
			nrf_delay_ms(500);
			nrf_gpio_pin_clear(LED);
			nrf_delay_ms(500);*/
			
		if(HX711_is_ready(&HX711_pin_structure)){
			printNumberLn(HX711_measure(&HX711_pin_structure, &HX711_data_structure), DEC);
		}
		else{
			printStringLn("HX is not ready yet");
		}	
	}
}

