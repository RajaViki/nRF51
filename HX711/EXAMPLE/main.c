/*
 *	Created by: Domen Jurkovic, 14.1.2016
 *	Example project for HX711 and uart print out
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nRF51_uart_print.h"
#include "nrf_hx711.h"


// USER MUST DEFINE PINS
#define RX_PIN_NUMBER 17	// UART RX pin number.
#define TX_PIN_NUMBER 18	// UART TX pin number.

HX711_pin_typedef	HX711_pin_structure;
HX711_data_typedef	HX711_data_structure_chA;
HX711_data_typedef	HX711_data_structure_chB;

void HX711_setup(void){
	HX711_pin_structure.dout_pin	=	9;
	HX711_pin_structure.pd_sck_pin	=	10;
	HX711_pin_structure.rate_pin	=	8;
		
	if(HX711_init(&HX711_pin_structure) != NRF_SUCCESS){	// init pins
		// handle this error
		printStringLn("HX init error");
	}
	
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
	HX711_measure_raw(&HX711_pin_structure, &HX711_data_structure_chA);
	HX711_measure_raw(&HX711_pin_structure, &HX711_data_structure_chB);
	
	HX711_data_structure_chA.result = 0;
	HX711_data_structure_chB.result = 0;
}


int main(void)
{
	int32_t bridge_offset = 0;
	int32_t raw_result = 0;
	int16_t slope_coeff = 0;	// KAKO BO FORMATIRAN KOEFICIENT?!?
	int32_t calculated_mass = 0;
		
	// UART
	printNumberLn(UART_Init(RX_PIN_NUMBER, TX_PIN_NUMBER, UART_BAUDRATE_BAUDRATE_Baud19200), DEC);
	printStringLn("-----------------------------------------------");
	printStringLn("System initialised");
	
	/* ------------------------------------------------------------- */
	/* 					HX functions example						 */
	/* ------------------------------------------------------------- */
	
	// 1.
	// Initialize HX711 pins and data structures
	HX711_setup();
	
	// 2.
	// Eliminate offset - tara
	HX711_data_structure_chA.av_rate = HX_OFFSET_AV_RATE;	// set high averaging for offset elimination
	HX711_data_structure_chA.rate = HX_RATE_10;						// set slow mode (10 SPS)
	bridge_offset = HX711_measure_raw(&HX711_pin_structure, &HX711_data_structure_chA);	// measure
	if(HX711_data_structure_chA.status != HX_MEAS_OK){	// check for error (meas timeout)
			printNumberLn(HX_MEAS_ERROR, DEC);// error!
	}
	/* write raw offset value (bridge_offset) to EEPROM: bridge_offset->EEPROM  */
	HX711_data_structure_chA.av_rate = HX_NORMAL_AV_RATE;	// set average back to normal
	
	// 3. 
	// Calibration	- send raw values for 30kg and 60kg
	raw_result = HX711_measure_raw(&HX711_pin_structure, &HX711_data_structure_chA);	// raw result (without offset elimination)
	if(HX711_data_structure_chA.status != HX_MEAS_OK){	// check for error (meas timeout)
			printNumberLn(HX_MEAS_ERROR, DEC);// error!
		}
	/* send for 30kg and 60kg; get slope coefficient form app and store it in EEPROM: slope_coeff->EEPROM */
	
	// 4. 
	// Final measurement (with eliminated offset and slope coefficient
	raw_result = HX711_measure_raw(&HX711_pin_structure, &HX711_data_structure_chA);	// raw result (without offset elimination)
	if(HX711_data_structure_chA.status != HX_MEAS_OK){	// check for error (meas timeout)
		printNumberLn(HX_MEAS_ERROR, DEC);	// error!
	}
	/* get slope coefficient from EEPROM: EEPROM->slope_coeff */
	/* get offset value from EEPROM: EEPROM->bridge_offset */
	calculated_mass = raw_to_mass(raw_result, bridge_offset, slope_coeff);	// convert raw data to mass

	// 5.
	// HX711 power down and wake up.
	HX711_power_down(&HX711_pin_structure);
	HX711_power_up(&HX711_pin_structure);
		
	while (true){
		HX711_power_up(&HX711_pin_structure);
				
		printNumberLn(HX711_measure_raw(&HX711_pin_structure, &HX711_data_structure_chA) - bridge_offset, DEC);

		if(HX711_data_structure_chA.status != HX_MEAS_OK){	// check for error (meas timeout)
			printNumberLn(HX_MEAS_ERROR, DEC);// error!
		}

		HX711_power_down(&HX711_pin_structure);
		printLn();
		nrf_delay_ms(100);
			
	}
}
