/* HX711 library
 * Created by Domen Jurkovic 30.11.2015
 *
 * initialise with:
 * Check example project.
 */
 
#include <stdio.h> 
#include "compiler_abstraction.h"
#include "nrf.h"

#include "nrf_hx711.h"

// define clock and data pin, channel, and gain factor
void HX711_init(HX711_pin_typedef *hx711_pin_struct){
	nrf_gpio_cfg_output(hx711_pin_struct->pd_sck_pin);	// power down / serial clock
	nrf_gpio_cfg_output(hx711_pin_struct->rate_pin);		// rate
	
	nrf_gpio_cfg_input(hx711_pin_struct->dout_pin, NRF_GPIO_PIN_NOPULL);		//data out from hx711
	
	nrf_gpio_pin_clear(hx711_pin_struct->pd_sck_pin);	// normal mode, power on
	nrf_gpio_pin_clear(hx711_pin_struct->rate_pin);	// HX_RATE_10
		
	// initialise HX and databus
	HX711_power_down(hx711_pin_struct);	
	HX711_power_up(hx711_pin_struct);
}

// check if HX711 is ready
// from the datasheet: When output data is not ready for retrieval, digital output pin DOUT is high. Serial clock
// input PD_SCK should be low. When DOUT goes to low, it indicates data is ready for retrieval.
uint8_t HX711_is_ready(HX711_pin_typedef *hx711_pin_struct){
	if(nrf_gpio_pin_read(hx711_pin_struct->dout_pin) == 0){
		return HX_STAT_READY;
	}
	else{
		return HX_STAT_BUSY;
	}
}

// sets gain and start new measurement, while read previous result 
int32_t HX711_measure(HX711_pin_typedef *hx711_pin_struct, HX711_data_typedef *hx711_data_struct){
	uint8_t num_of_pulses=0;
	int32_t result = 0;
	int32_t temp_result = 0;
	uint8_t i, j = 0;
	uint8_t bit_shift = 0;
	
	switch (hx711_data_struct->gain) {
		case HX_GAIN_128:		// channel A, gain factor 128
			num_of_pulses = 25;
			bit_shift = HX_SHIFT_BITS_CHA;
			break;
		case HX_GAIN_64:		// channel A, gain factor 64
			num_of_pulses = 27;
			bit_shift = HX_SHIFT_BITS_CHB;
			break;
		case HX_GAIN_32:		// channel B, fixed gain factor 32
			num_of_pulses = 26;
			bit_shift = HX_SHIFT_BITS_CHA;
			break;
	}
	
	//sets RATE according to datasheet
	if(hx711_data_struct->rate == HX_RATE_10){
		nrf_gpio_pin_clear(hx711_pin_struct->rate_pin);	// RATE_10
	}
	else{
		nrf_gpio_pin_set(hx711_pin_struct->rate_pin);	// HX_RATE_80
	}
	
	// set the channel and the gain factor for the next measurement, ignore results from previous measurement
	for (j = 0; j < num_of_pulses; j++) {
		nrf_gpio_pin_set(hx711_pin_struct->pd_sck_pin);		
			nrf_delay_us(2); 	//delay
		nrf_gpio_pin_clear(hx711_pin_struct->pd_sck_pin);	
			nrf_delay_us(2); 	//delay
	}
	
	// make av_rate measurements
	for(i = 0; i < hx711_data_struct->av_rate; i++){
		
		// wait while HX is not ready. 
		while (HX711_is_ready(hx711_pin_struct) == HX_STAT_BUSY){
		}
			
		
		for (j = 0; j < num_of_pulses; j++) {
			nrf_gpio_pin_set(hx711_pin_struct->pd_sck_pin);		
			nrf_delay_us(2); 	//delay
			if(j < 24){	// read 24 bits from dout, 25 bits since first one is also shifted
				temp_result |= nrf_gpio_pin_read(hx711_pin_struct->dout_pin) & 1;
				temp_result = temp_result << 1;
			}
			
			nrf_gpio_pin_clear(hx711_pin_struct->pd_sck_pin);	
				nrf_delay_us(2); 	//delay
		}
		temp_result = temp_result >> 1;	//25 bits to 24 bits
		temp_result = temp_result & 0x00FFFFFF;	// clear unvalid bits (24 - 32 bit)
		
		// output data is in two's complement. DATAHSEET:	When input differential signal goes out of 
		// the 24 bit range, the output data will be saturated at 800000h (MIN) or 7FFFFFh (MAX).
		// To printout and keep two's complement of 24bit data in 32bit variable, shift it left and than right. 
		// the sign of 24bit initial value will remain as it was originaly.
		temp_result = temp_result << 8;	
		temp_result = temp_result >> 8;
		//OR
		//temp_result ^= 0x00800000;
		//temp_result -= 8388608;
		
		temp_result = temp_result >> bit_shift;		
		
		result += (temp_result - hx711_data_struct->offset);	// add temporary result to result, substract offset value. 
		temp_result = 0;	// clear temporary result
	}
	result /= hx711_data_struct->av_rate;	// average
	hx711_data_struct->result = result;
	return result;
}

// measure with high average (HX_OFFSET_AV_RATE) at 10SPS = 5 seconds for (HX_OFFSET_AV_RATE = 50) and write offset in corresponding structure
void HX711_eliminate_offset(HX711_pin_typedef *hx711_pin_struct, HX711_data_typedef *hx711_data_struct){
	uint8_t av_rate = hx711_data_struct->av_rate;
	uint8_t sps = hx711_data_struct->rate;
	
	hx711_data_struct->result = 0;
	hx711_data_struct->offset = 0;
	hx711_data_struct->rate = HX_RATE_10;
	hx711_data_struct->av_rate = HX_OFFSET_AV_RATE;
	
	hx711_data_struct->offset = HX711_measure(hx711_pin_struct, hx711_data_struct);
	
	hx711_data_struct->rate = sps;
	hx711_data_struct->av_rate = av_rate;
}

// puts the chip into power down mode
void HX711_power_down(HX711_pin_typedef *hx711_pin_struct){
	nrf_gpio_pin_set(hx711_pin_struct->pd_sck_pin);	
	nrf_delay_us(100);	// 60us by datasheet
}

// wakes up the chip after power down mode
void HX711_power_up(HX711_pin_typedef *hx711_pin_struct){
	nrf_gpio_pin_clear(hx711_pin_struct->pd_sck_pin);
	nrf_delay_us(2);
}


