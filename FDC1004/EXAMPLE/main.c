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
#define LED 22
#define RX_PIN_NUMBER 17	// UART RX pin number.
#define TX_PIN_NUMBER 18	// UART TX pin number.
#define BAUD_RATE			UART_BAUDRATE_BAUDRATE_Baud19200

void connect(void);
void FDC1004_start_measurement(void);
void FDC1004_stop_measurement(void);

FDC1004_results_t FDC1004_results;
uint8_t meas_control_bit = 0;

void system_setup(){
	// LED
	nrf_gpio_cfg_output(LED);	
	
	// UART
	UART_Init(RX_PIN_NUMBER, TX_PIN_NUMBER, BAUD_RATE);
	printStringLn("-----------------------------------------------");
	// TWI
	if(!twi_master_init()) printStringLn("I2C init error!");
	
	// FDC
	if(FDC1004_timer_setup()) printStringLn("FDC timer error!");
	if(FDC1004_init()) printStringLn("FDC init error!");
	
}

int main(void){
	uint8_t received_data = 0;
	uint8_t fdc_status = 0;
	system_setup();
	printStringLn("SYSTEM INITIALIZED!");
		
	//fdc_status = FDC1004_capdac_setup();
	//fdc_status = FDC1004_elimintate_offset();
	//if(fdc_status){ printStringLn("capdac error!"); printStringLn("offset error!");}
	
	while(1){
		if(meas_control_bit == START_MEASUREMENT){ //start new measurement
			fdc_status = FCD1004_measure(&FDC1004_results, SAMPLE_RATE_100, AV_RATE_N);
			//fdc_status = FCD1004_measure_single_cap(&FDC1004_results, SAMPLE_RATE_100, CAP4, AV_RATE_N);
			
			//meas_control_bit = STOP_MEASUREMENT;
			
			if(!fdc_status){
//				printStringLn("\t\t\t............");
//				printString("CALC result_f:\t");
//				printFloat(RESULT_INT_TO_FLOAT(FDC1004_results.ch4));	printString("\t");
//				printFloat(RESULT_INT_TO_FLOAT(FDC1004_results.ch2));	printString("\t");
//				printFloat(RESULT_INT_TO_FLOAT(FDC1004_results.ch3));	printString("\t");
//				printFloatLn(RESULT_INT_TO_FLOAT(FDC1004_results.ch4));
				
				printString("RAW results:\t");
				printNumber(FDC1004_results.ch1, DEC);	printString("\t");
				printNumber(FDC1004_results.ch2, DEC);	printString("\t");
				printNumber(FDC1004_results.ch3, DEC);	printString("\t");
				printNumber(FDC1004_results.ch4, DEC);	printString("\t");
				printString("\tstatus:\t"); printNumberLn(fdc_status, DEC);
				
				/* No error handling implemented jet */
				
			}
			else{
				printString("MEAS ERROR: ");
				printNumberLn(fdc_status, BIN);
			}
			
		}

		received_data = uart_receive_byte();
		if(received_data != 0){
			switch(received_data){
				case I_CONNECT: 
					connect(); 
					break;
				case I_ELIMINATE_OFFSET: 
					if(FDC1004_elimintate_offset()) printStringLn("offset error!"); 
					break;
				case I_START_MEAS: 
					FDC1004_start_measurement(); 
					break;
				case I_STOP_MEAS: 
					FDC1004_stop_measurement(); 
					break;
				case I_RESET: 
					if(FDC1004_init()) printStringLn("init error!"); 
					break;
				default: break;
			}			
		}
		received_data = 0;
		
		nrf_delay_ms(250);
	}
}

void connect(void){
	#ifdef PRINTOUT
		printStringLn("\tConnected.");
	#else
		
	#endif
}

void FDC1004_start_measurement(void)
{
	meas_control_bit = START_MEASUREMENT;
	
	#ifdef PRINTOUT
		printStringLn("\tMeasurement started.");
	#else
		//printString(S_MEASUREMENT_STARTED);
	#endif
}

void FDC1004_stop_measurement(void)
{
	meas_control_bit = STOP_MEASUREMENT;
    //clear variables and other shit

	#ifdef PRINTOUT
		printStringLn("\tMeasurement stopped.");
	#else
		//printString(S_MEASUREMENT_STOPPED);
	#endif
}
