/* HX711 library
 * Created by Domen Jurkovic 25.12.2015
 * Modified 14.1.2016 for Froc chair
 *
 * Check example project.
 * Library use TIMER1.
 */
 
#include <stdio.h> 
#include "compiler_abstraction.h"
#include "nrf.h"

#include "nrf_hx711.h"

const nrf_drv_timer_t timer1_instance = NRF_DRV_TIMER_INSTANCE(1);	//ENABLE TIMER1 in nrf_drv_uart.c
bool timer1_int = false;

// define clock and data pin, channel, and gain factor
uint8_t HX711_init(HX711_pin_typedef *hx711_pin_struct){
	nrf_drv_timer_config_t timer1_config;
	uint32_t meas_time_ms = HX_MEAS_TIMEOUT; 
	uint32_t meas_time_ticks = nrf_drv_timer_ms_to_ticks(&timer1_instance, meas_time_ms);
	uint32_t err_code = NRF_SUCCESS;
	
	//HX gpio init
	nrf_gpio_cfg_output(hx711_pin_struct->pd_sck_pin);	// power down / serial clock
	nrf_gpio_cfg_output(hx711_pin_struct->rate_pin);		// rate
	nrf_gpio_cfg_input(hx711_pin_struct->dout_pin, NRF_GPIO_PIN_NOPULL);		//data out from hx711
	
	nrf_gpio_pin_clear(hx711_pin_struct->pd_sck_pin);	// normal mode, power on
	nrf_gpio_pin_clear(hx711_pin_struct->rate_pin);	// HX_RATE_10
		
	//TIMER1 init
	timer1_config.frequency = NRF_TIMER_FREQ_125kHz;
	timer1_config.mode = NRF_TIMER_MODE_TIMER;
	timer1_config.bit_width = NRF_TIMER_BIT_WIDTH_16;
	timer1_config.interrupt_priority = APP_IRQ_PRIORITY_LOW;	
	timer1_config.p_context = NULL;
	//err_code = nrf_drv_timer_init(&timer1_instance, NULL, timer1_event_handler);
	err_code = nrf_drv_timer_init(&timer1_instance, &timer1_config, timer1_event_handler);
	nrf_drv_timer_extended_compare(&timer1_instance, NRF_TIMER_CC_CHANNEL0, meas_time_ticks, NRF_TIMER_SHORT_COMPARE1_STOP_MASK, true);
	nrf_drv_timer_compare_int_enable(&timer1_instance, NRF_TIMER_CC_CHANNEL0);
	
	// initialise HX and databus
	HX711_power_down(hx711_pin_struct);	
	HX711_power_up(hx711_pin_struct);
	
	return err_code;
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

// measure CHA/B according to GAIN setting in HX711_data_typedef  *hx711_data_struct
// returns raw data from HX711
int32_t HX711_measure_raw(HX711_pin_typedef *hx711_pin_struct, HX711_data_typedef *hx711_data_struct){
	uint8_t num_of_pulses=0;
	int32_t result = 0;
	int32_t temp_result = 0;
	uint8_t i, j = 0;
	uint8_t bit_shift = 0;
	uint32_t meas_time_ticks = nrf_drv_timer_ms_to_ticks(&timer1_instance, HX_MEAS_TIMEOUT);

	nrf_drv_timer_extended_compare(&timer1_instance, NRF_TIMER_CC_CHANNEL0, meas_time_ticks, NRF_TIMER_SHORT_COMPARE1_STOP_MASK, true);	
	hx711_data_struct->status = HX_MEAS_OK;
	hx711_data_struct->result = 0;	// clear previous result stored
	
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
		// wait while HX is not ready, report error if timeout. 
		start_timer1();
		while (HX711_is_ready(hx711_pin_struct) == HX_STAT_BUSY){
			if(timer1_int == true){	// measure timeout
				stop_timer1();	
				result = 0;
				timer1_int = false;
				hx711_data_struct->status = HX_MEAS_ERROR;	// meas timeout
				hx711_data_struct->result = result;
				return 0;
			}
		}
		stop_timer1();

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
		
		// output data is in two's complement. DATASHEET:	When input differential signal goes out of 
		// the 24 bit range, the output data will be saturated at 800000h (MIN) or 7FFFFFh (MAX).
		// To printout and keep two's complement of 24bit data in 32bit variable, shift it left and than right. 
		// the sign of 24bit initial value will remain as it was originally.
		temp_result = temp_result << 8;	
		temp_result = temp_result >> 8;
		//OR
		//temp_result ^= 0x00800000;
		//temp_result -= 8388608;
		
		temp_result = temp_result >> bit_shift;		
		
		result += temp_result;	// add temporary result to result
		temp_result = 0;	// clear temporary result
	}
	result /= hx711_data_struct->av_rate;	// average
	hx711_data_struct->result = result;
	return result;
}

// convert raw data to mass
int32_t raw_to_mass(int32_t raw_data, int32_t offset, int16_t slope_coeff){
	int32_t mass = 0;
	float slope_cofficient = ((float) slope_coeff) / 10000;	// KAKO BO FORMATIRAN KOEFICIENT?!?
	mass = slope_cofficient * (raw_data - offset);
	
	return mass;
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

void timer1_event_handler(nrf_timer_event_t event_type, void* p_context)
{
	switch(event_type)
	{
		case NRF_TIMER_EVENT_COMPARE0:
			timer1_int = true;
			nrf_drv_timer_clear(&timer1_instance);
			break;
		default: break;
	}    
}

void start_timer1(void){
	timer1_int = false;
	nrf_drv_timer_clear(&timer1_instance);
	nrf_drv_timer_enable(&timer1_instance);
}

void stop_timer1(void){
	nrf_drv_timer_disable(&timer1_instance);
	nrf_drv_timer_clear(&timer1_instance);
}


