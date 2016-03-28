#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

#include "nRF51_uart_print.h"
#include "twi_master.h"
#include "twi_master_config.h"

#include "nrf_fdc1004.h"
#include "communication.h"

 
// USER MUST DEFINE PINS
#define REED_RELAY 25
#define RX_PIN_NUMBER 17	// UART RX pin number.
#define TX_PIN_NUMBER 18	// UART TX pin number.
#define BAUD_RATE			UART_BAUDRATE_BAUDRATE_Baud38400

static FDC_results_t FDC1004_results;
uint8_t results_control_bit = IGNORE_RESULTS;

void system_setup(){
	nrf_gpio_cfg_input(REED_RELAY, NRF_GPIO_PIN_PULLUP);
	
	// UART
	UART_Init(RX_PIN_NUMBER, TX_PIN_NUMBER, BAUD_RATE);
	printStringLn("-----------------------------------------------");
		
	/********************** FDC init & setup *********************/
	if(I2C_init()){
		// handle I2C init error
		printStringLn("I2C init error!");
	}
	
	if(FDC1004_timer_setup()){
		// handle timer error
		printStringLn("Timer init error!");
	}
	
	if(FDC1004_init()){
		printStringLn("FDC init error!");
	}
	
	
}

int main(void){
	uint8_t received_data = 0;
	uint8_t fdc_status = 0;
	uint8_t reed_relay_status = 0;
	
	system_setup();
	printStringLn("SYSTEM INITIALIZED!");
		
	while(1){
		received_data = uart_receive_byte();
		if(received_data != 0){
			switch(received_data){
				case I_CONNECT: 
					printStringLn("C!"); 
					break;
				
				case I_ELIMINATE_OFFSET:
					printString("O: ");
					if(FDC1004_elimintate_offset()){
						printStringLn("FDC offset elimination error!");
					}
					else printStringLn("FDC offset elimination OK!");
					break;
				
				case I_START_MEAS: 
					printStringLn("M!");
					fdc_status = FCD1004_start_repeated_measurement(SAMPLE_RATE_400);
					break;
				
				case I_STOP_MEAS: 
					printStringLn("S!");
					fdc_status = FCD1004_stop_repeated_measurement();
					break;
				
				case I_GET_RESULTS:
					printStringLn("G!");
					results_control_bit = GET_RESULTS;
					break;
				
				case I_RESET:
					if(FDC1004_init()){ 
						printStringLn("FDC init error!");
					}
					else printStringLn("FDC init OK!");
					results_control_bit = IGNORE_RESULTS;
					
					break;
					
				case I_NORDIC_RESET:
					NVIC_SystemReset();
					break;
				default: break;
			}			
		}
		received_data = 0;
		
		if(results_control_bit == GET_RESULTS){
			fdc_status |= FCD1004_get_results(&FDC1004_results, 1);	// av_rate = 1
			reed_relay_status = !nrf_gpio_pin_read(REED_RELAY);
					
			printString("R:\t");
			printNumber(FDC1004_results.ch1, DEC);	printString("\t");
			printNumber(FDC1004_results.ch2, DEC);	printString("\t");
			printNumber(FDC1004_results.ch3, DEC);	printString("\t");
			//printNumber(FDC1004_results.ch4, DEC);	printString("\t");
			printNumber(reed_relay_status , DEC);	printString("\t");
			printNumberLn(fdc_status, DEC);
			
			results_control_bit = IGNORE_RESULTS;
		}
		
	}
}

