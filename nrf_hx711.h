/* HX711 library
 * Created by Domen Jurkovic 30.11.2015
 *
*/


#ifndef _NRF_HX711_H
#define _NRF_HX711_H

#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

#define HX_GAIN_32	32		//Channel B
#define HX_GAIN_64	64		//Channel A
#define HX_GAIN_128 128	//Channel A

#define HX_RATE_10 0
#define HX_RATE_80 1

#define HX_STAT_READY		0
#define HX_STAT_BUSY		1

typedef struct{
	uint32_t pd_sck_pin;
	uint32_t dout_pin;
	uint32_t rate_pin;	
}HX711_pin_typedef;

typedef struct{
	uint8_t gain;
	uint8_t rate;
	uint8_t av_rate;
	int32_t result;
	int32_t offset;
}HX711_data_typedef;

void HX711_init(HX711_pin_typedef *hx711_pin_struct, HX711_data_typedef *hx711_data_struct);

uint8_t HX711_is_ready(HX711_pin_typedef *hx711_pin_struct);	
int32_t HX711_measure(HX711_pin_typedef *hx711_pin_struct, HX711_data_typedef *hx711_data_struct);
void HX711_eliminate_offset(HX711_pin_typedef *hx711_pin_struct, HX711_data_typedef *hx711_data_struct);	//tara

void HX711_power_down(HX711_pin_typedef *hx711_pin_struct);
void HX711_power_up(HX711_pin_typedef *hx711_pin_struct);


#endif /* nrf_hx711_h */

