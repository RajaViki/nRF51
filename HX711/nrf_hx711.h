/* HX711 library
 * Created by Domen Jurkovic 30.11.2015
 * Modified 4.1.2016 for Froc chair
 *
*/


#ifndef _NRF_HX711_H
#define _NRF_HX711_H

#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_drv_timer.h"

#include "app_util_platform.h"

// USER CAN CHANGE:
#define HX_NORMAL_AV_RATE	10
#define HX_OFFSET_AV_RATE	50
#define HX_SHIFT_BITS_CHA	7
#define HX_SHIFT_BITS_CHB	0

// Output settling time (datasheet, p3 :Settling time refers to the time 
//	from power up, reset, input channel change and gain changeto valid stable output data
#define HX_MEAS_TIMEOUT		500	//[ms];

// defines
#define HX_GAIN_32	32		//Channel B
#define HX_GAIN_64	64		//Channel A
#define HX_GAIN_128 128		//Channel A

#define HX_RATE_10 0
#define HX_RATE_80 1

#define HX_STAT_READY		0
#define HX_STAT_BUSY		1

#define HX_MEAS_OK			0
#define HX_MEAS_ERROR		1

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
	uint8_t status;
}HX711_data_typedef;


uint8_t HX711_init(HX711_pin_typedef *hx711_pin_struct);

uint8_t HX711_is_ready(HX711_pin_typedef *hx711_pin_struct);

int32_t HX711_measure_raw(HX711_pin_typedef *hx711_pin_struct, HX711_data_typedef *hx711_data_struct);

int32_t raw_to_mass(int32_t raw_data, int32_t offset, int16_t slope_coeff);	// KAKO BO FORMATIRAN KOEFICIENT?!?

void HX711_power_down(HX711_pin_typedef *hx711_pin_struct);
void HX711_power_up(HX711_pin_typedef *hx711_pin_struct);

void timer1_event_handler(nrf_timer_event_t event_type, void* p_context);
void start_timer1(void);
void stop_timer1(void);

#endif /* nrf_hx711_h */

