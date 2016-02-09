/*
	Created by: Domen Jurkovic
	v1.3, 9-Feb-2016
*/

#include <stdbool.h>
#include <stdint.h>

#include "nrf_fdc1004.h"

// edit nrf_drv_config.h to enable TIMER0
// set up pins in twi_master.h
/*****************************************************************************
* I2C communication interface
*****************************************************************************/

FDC1004_offset_t FDC1004_offset;

uint8_t timeout_interrupt = 0;
const nrf_drv_timer_t timer0 = NRF_DRV_TIMER_INSTANCE(0);	// TIMER 0 in use

uint8_t FDC1004_init(void){
	FDC1004_t rw_struct;

	memset(&FDC1004_offset, 0, sizeof(FDC1004_offset));
	rw_struct.rw_status = FDC1004_SUCCESS;
	
	// software RESET
	rw_struct.register_pointer = FDC_CONF;	
	rw_struct.data[0] = 0x84;
	rw_struct.data[1] = 0x00;
	if(FDC1004_write_register(&rw_struct))	return (FDC1004_I2C_E | FDC1004_SETUP_E);
	nrf_delay_ms(100);
	
	// SETUP cap pins and initial state
	rw_struct.register_pointer = CONF_MEAS1;
	rw_struct.data[0] = 0x1C;
	rw_struct.data[1] = 0x00;
	if(FDC1004_write_register(&rw_struct)) return (FDC1004_I2C_E | FDC1004_SETUP_E);
		
	rw_struct.register_pointer = CONF_MEAS2;
	rw_struct.data[0] = 0x3C;
	rw_struct.data[1] = 0x00;
	if(FDC1004_write_register(&rw_struct)) return (FDC1004_I2C_E | FDC1004_SETUP_E);
		
	rw_struct.register_pointer = CONF_MEAS3;
	rw_struct.data[0] = 0x5C;
	rw_struct.data[1] = 0x00;
	if(FDC1004_write_register(&rw_struct)) return (FDC1004_I2C_E | FDC1004_SETUP_E);
	
	rw_struct.register_pointer = CONF_MEAS4;
	rw_struct.data[0] = 0x7C;
	rw_struct.data[1] = 0x00;
	if(FDC1004_write_register(&rw_struct)) return (FDC1004_I2C_E | FDC1004_SETUP_E);
		
	nrf_delay_ms(10);	
	#ifdef PRINTOUT
		printStringLn("INIT OK.");
	#else
	#endif
	
	/*
	rw_struct.register_pointer = MANUFACTURER_ID;
	if(FDC1004_read_register(&rw_struct))return (FDC1004_I2C_E | FDC1004_SETUP_E);	// read results for channel 1 - MSB
	printString("MANUFACTURER_ID: ");
	printNumber(rw_struct.data[0], HEX);
	printNumberLn(rw_struct.data[1], HEX);
	
	rw_struct.register_pointer = DEVICE_ID;
	if(FDC1004_read_register(&rw_struct))return (FDC1004_I2C_E | FDC1004_SETUP_E);	// read results for channel 1 - MSB
	printString("DEVICE_ID: ");
	printNumber(rw_struct.data[0], HEX);
	printNumberLn(rw_struct.data[1], HEX);
	*/
	
	return FDC1004_SUCCESS;
}

uint8_t FCD1004_measure(FDC1004_results_t * result_struct, uint8_t sample_rate, uint8_t av_rate)
{
	FDC1004_t rw_struct;
	uint8_t i = 0;
	uint32_t temp_res = 0;
	memset(result_struct, 0, sizeof(FDC1004_results_t));
	result_struct->measure_status = FDC1004_SUCCESS;
	
	// START repeated measurement
	rw_struct.register_pointer = FDC_CONF;
	switch (sample_rate){
		case SAMPLE_RATE_100: rw_struct.data[0] = 0x05; break;
		case SAMPLE_RATE_200: rw_struct.data[0] = 0x09; break;
		case SAMPLE_RATE_400: rw_struct.data[0] = 0x0D; break;
		default: return FDC1004_DATA_E;
	}
	rw_struct.data[1] = 0xF0;	
	if(FDC1004_write_register(&rw_struct))return FDC1004_I2C_E;	// start measuring all capacitors
	
	for(i=0; i<av_rate; i++){
		// WAIT for measurement completion
		rw_struct.register_pointer = FDC_CONF;
		start_timer();
		if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;	// read register
		while((rw_struct.data[1] & DONEX_MASK) != DONEX_MASK ){	// wait while DONE_1, 2, 3, 4 == 1
			if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;
			if(timeout_interrupt){ stop_timer(); return FDC1004_DATA_E;}
		}
						
		// READ RESULTS (spodaj pojasnjen ta dvojiski komplement. 
		// CH1
		rw_struct.register_pointer = MEAS1_MSB;
		if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;	// read results for channel 1 - MSB
		temp_res = rw_struct.data[0] << 16;
		temp_res |= rw_struct.data[1] << 8;
		rw_struct.register_pointer = MEAS1_LSB;
		if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;	// read results for channel 1 - LSB
		temp_res |= rw_struct.data[0];
		result_struct->ch1 += temp_res;
		temp_res = 0;
		// CH2
		rw_struct.register_pointer = MEAS2_MSB;
		if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;	// read results for channel 2 - MSB
		temp_res = rw_struct.data[0] << 16;
		temp_res |= rw_struct.data[1] << 8;
		rw_struct.register_pointer = MEAS2_LSB;
		if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;	// read results for channel 2 - LSB
		temp_res |= rw_struct.data[0];
		result_struct->ch2 += temp_res;
		temp_res = 0;
		// CH3
		rw_struct.register_pointer = MEAS3_MSB;
		if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;	// read results for channel 3 - MSB
		temp_res = rw_struct.data[0] << 16;
		temp_res |= rw_struct.data[1] << 8;
		rw_struct.register_pointer = MEAS3_LSB;
		if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;	// read results for channel 3 - LSB
		temp_res |= rw_struct.data[0];
		result_struct->ch3 += temp_res;
		temp_res = 0;
		// CH4
		rw_struct.register_pointer = MEAS4_MSB;
		if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;	// read results for channel 4 - MSB
		temp_res = rw_struct.data[0] << 16;
		temp_res |= rw_struct.data[1] << 8;
		rw_struct.register_pointer = MEAS4_LSB;
		if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;	// read results for channel 4 - LSB
		temp_res |= rw_struct.data[0];
		result_struct->ch4 += temp_res;
		temp_res = 0;
	}
	// stop repeated measurement
	rw_struct.register_pointer = FDC_CONF;
	rw_struct.data[0] = 0x04;	rw_struct.data[1] = 0x00;			
	if(FDC1004_write_register(&rw_struct))return FDC1004_I2C_E;		
	
	if(av_rate > 0){
		result_struct->ch1 /= av_rate;
		result_struct->ch2 /= av_rate;
		result_struct->ch3 /= av_rate;
		result_struct->ch4 /= av_rate;
	}
	
	if(FDC1004_offset.ch1 > 0){
		result_struct->ch1 -= FDC1004_offset.ch1;
		result_struct->ch1 >>= OFFSET_BIT_SHIFT;
	}
	if(FDC1004_offset.ch2 > 0){
		result_struct->ch2 -= FDC1004_offset.ch2;
		result_struct->ch2 >>= OFFSET_BIT_SHIFT;
	}
	if(FDC1004_offset.ch3 > 0){
		result_struct->ch3 -= FDC1004_offset.ch3;
		result_struct->ch3 >>= OFFSET_BIT_SHIFT;
	}
	if(FDC1004_offset.ch4 > 0){
		result_struct->ch4 -= FDC1004_offset.ch4;
		result_struct->ch4 >>= OFFSET_BIT_SHIFT;
	}

	return result_struct->measure_status;
}


uint8_t FCD1004_measure_single_cap(FDC1004_results_t * result_struct, uint8_t sample_rate, uint8_t select_cap, uint8_t av_rate)
{
	FDC1004_t rw_struct;
	uint8_t cap_donex_mask = 0;
	uint8_t i=0;
	int32_t temp_res = 0;
	
	memset(result_struct, 0, sizeof(FDC1004_results_t));
	result_struct->measure_status = FDC1004_SUCCESS;
		
	for(i=0; i<av_rate; i++)
	{
		switch (sample_rate){
			case SAMPLE_RATE_100: rw_struct.data[0] = 0x04; break;
			case SAMPLE_RATE_200: rw_struct.data[0] = 0x08; break;
			case SAMPLE_RATE_400: rw_struct.data[0] = 0x0C; break;
			default: return FDC1004_DATA_E;
		}
		switch (select_cap){
			case CAP1: rw_struct.data[1] = 0x80; cap_donex_mask = 0x08; break;
			case CAP2: rw_struct.data[1] = 0x40; cap_donex_mask = 0x04; break;
			case CAP3: rw_struct.data[1] = 0x20; cap_donex_mask = 0x02; break;
			case CAP4: rw_struct.data[1] = 0x10; cap_donex_mask = 0x01; break;
			default: return FDC1004_DATA_E;
		}
		
		// START single measurement
		rw_struct.register_pointer = FDC_CONF;
		if(FDC1004_write_register(&rw_struct))return FDC1004_I2C_E;		// start single measurement
		
		// WAIT for measurement completion
		start_timer();
		if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;			// read register
		while(( (rw_struct.data[1] & cap_donex_mask) == 0)){				// wait while DONE_x != 1
			if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;	// read register
			if(timeout_interrupt){ stop_timer(); return FDC1004_DATA_E;}
		}
				
		while( ((rw_struct.data[1] & DONEX_MASK) != DONEX_MASK) && (timeout_interrupt == 0) ){	// wait while DONE_1, 2, 3, 4 == 1
			if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;
		}
		
		
		switch (select_cap){
			case CAP1:
				rw_struct.register_pointer = MEAS1_MSB;
				if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;	// read results for channel 1 - MSB
				temp_res = rw_struct.data[0] << 16;
				temp_res |= rw_struct.data[1] << 8;
				rw_struct.register_pointer = MEAS1_LSB;
				if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;	// read results for channel 1 - LSB
				temp_res |= rw_struct.data[0];
				result_struct->ch1 += temp_res;
				temp_res = 0;
				break;
			case CAP2:
				rw_struct.register_pointer = MEAS2_MSB;
				if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;	// read results for channel 2 - MSB
				temp_res = rw_struct.data[0] << 16;
				temp_res |= rw_struct.data[1] << 8;
				rw_struct.register_pointer = MEAS2_LSB;
				if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;	// read results for channel 2 - LSB
				temp_res |= rw_struct.data[0];
				result_struct->ch2 += temp_res;
				temp_res = 0;
				break;
			case CAP3:
				rw_struct.register_pointer = MEAS3_MSB;
				if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;	// read results for channel 3 - MSB
				temp_res = rw_struct.data[0] << 16;
				temp_res |= rw_struct.data[1] << 8;
				rw_struct.register_pointer = MEAS3_LSB;
				if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;	// read results for channel 3 - LSB
				temp_res |= rw_struct.data[0];
				result_struct->ch3 += temp_res;
				temp_res = 0;
				break;
			case CAP4:
				rw_struct.register_pointer = MEAS4_MSB;
				if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;	// read results for channel 4 - MSB
				temp_res = rw_struct.data[0] << 16;
				temp_res |= rw_struct.data[1] << 8;
				rw_struct.register_pointer = MEAS4_LSB;
				if(FDC1004_read_register(&rw_struct))return FDC1004_I2C_E;	// read results for channel 4 - LSB
				temp_res |= rw_struct.data[0];
				result_struct->ch4 += temp_res;
				temp_res = 0;
				break;
			default: return FDC1004_DATA_E; 
		}
	}
	switch (select_cap){
			case CAP1:
				result_struct->ch1 /= av_rate;
				if(FDC1004_offset.ch1 > 0){
					result_struct->ch1 -= FDC1004_offset.ch1;
					result_struct->ch1 >>= OFFSET_BIT_SHIFT;	
				}
				break;
			case CAP2:
				result_struct->ch2 /= av_rate;
				if(FDC1004_offset.ch2 > 0){
					result_struct->ch2 -= FDC1004_offset.ch2;
					result_struct->ch2 >>= OFFSET_BIT_SHIFT;
				}
				break;
			case CAP3:
				result_struct->ch3 /= av_rate;
				if(FDC1004_offset.ch3 > 0){
					result_struct->ch3 -= FDC1004_offset.ch3;
					result_struct->ch3 >>= OFFSET_BIT_SHIFT;
				}
				break;
			case CAP4:
				result_struct->ch4 /= av_rate;
				if(FDC1004_offset.ch4 > 0){
					result_struct->ch4 -= FDC1004_offset.ch4;
					result_struct->ch4 >>= OFFSET_BIT_SHIFT;
				}
				break;
			default: break;
		}
	
	return FDC1004_SUCCESS;
}

uint8_t FDC1004_capdac_setup(void)
{
	FDC1004_results_t results;
	FDC1004_t rw_struct;
	int32_t *results_array[] = {&results.ch1, &results.ch2, &results.ch3, &results.ch4};
	uint8_t conf_measx_register_pointer[] = {CONF_MEAS1, CONF_MEAS2, CONF_MEAS3, CONF_MEAS4};
	uint16_t ch_setup_register_pointer[] = {CONF_MEAS1_CH_SETUP, CONF_MEAS2_CH_SETUP, CONF_MEAS3_CH_SETUP, CONF_MEAS4_CH_SETUP};
	
	float results_f = 0;
	uint32_t results_i = 0;
	float results_f_array[] = {0,0,0,0};
	float mid_point_result_f = ((FDC1004_RANGE * (float)PULL_VS_PUSH_FORCE))/10.0;	// point where push and pull forces corespond to PULL_VS_PUSH_FORCE ratio
	float diff_f = 0;
	uint16_t capdac_array[] = {0,0,0,0};
	uint16_t combined_register = 0;
	uint8_t i = 0;
	uint8_t measure_status = FDC1004_SUCCESS;

	
	#ifdef PRINTOUT
		printString("CAPDAC setup:");
		
		printString("\t Pull vs push ratio: ");
		printNumber(PULL_VS_PUSH_FORCE, DEC);
		printString("0% pull, ");
		printNumber(10 - PULL_VS_PUSH_FORCE, DEC);
		printStringLn("0% push, ");
		
		printString("\t Wanted midpoint of 15pF range: ");
		printFloatLn(mid_point_result_f);
		
	#else
	#endif
	measure_status = FDC1004_init();
	if(measure_status) return measure_status;
	
	// CAPDAC OFFSET ELIMINATION
	do{
		// MEASURE all capacitors
		measure_status = FCD1004_measure(&results, SAMPLE_RATE_400, AV_RATE_N);	// measure fast, only for rough range setup
		if(measure_status) return measure_status;	
		nrf_delay_ms(8);
		
		for(i=0; i<4; i++){
			results_f = (float) *results_array[i];
			results_f = results_f / DIVIDE_RESULTS;		// in pF, integer+decimal number. ?/ CAPDAc resolution
			results_f_array[i] = results_f;
			results_i = (uint32_t) results_f; 
			
			if(results_i >= FDC1004_RANGE)	
			{
				capdac_array[i] += (uint16_t)((FDC1004_RANGE - mid_point_result_f) / CAPDAC_RESOLUTION);
			}
			
			#ifdef PRINTOUT
				//printString("\t capdac: ");
				//printNumberLn(capdac_array[i], DEC);
			#endif
			
			// write CAPDAC value to register
			combined_register = ch_setup_register_pointer[i] | (capdac_array[i] << 5);
			rw_struct.register_pointer = conf_measx_register_pointer[i];
			rw_struct.data[0] = combined_register >> 8;
			rw_struct.data[1] = combined_register & 0x00FF;
			if(FDC1004_write_register(&rw_struct)) return (FDC1004_I2C_E | FDC1004_SETUP_E);
		}
	}while(!((results_f_array[0] < FDC1004_RANGE) && (results_f_array[1] < FDC1004_RANGE) && (results_f_array[2] < FDC1004_RANGE) && (results_f_array[3] < FDC1004_RANGE)));
	
	measure_status = FCD1004_measure(&results, SAMPLE_RATE_100, AV_RATE_H);	// measure accurate, for fine capdac setup
	if(measure_status) return measure_status;
	
	for(i=0; i<4; i++){
		results_f = (float) *results_array[i];
		results_f = results_f / DIVIDE_RESULTS;		// in pF, integer+decimal number. ?/ CAPDAc resolution
		results_f_array[i] = results_f;
		
		if(results_f_array[i] > (mid_point_result_f + CAPDAC_RESOLUTION)){
			diff_f = (results_f_array[i] - mid_point_result_f) / CAPDAC_RESOLUTION;
			if((diff_f - (uint16_t)diff_f) > 0.5){
				capdac_array[i] += ((uint16_t)diff_f) + 1;
			}
			else{
				capdac_array[i] += ((uint16_t)diff_f);
			}			
		}
		
		// write CAPDAC value to register
		combined_register = ch_setup_register_pointer[i] | (capdac_array[i] << 5);
		rw_struct.register_pointer = conf_measx_register_pointer[i];
		rw_struct.data[0] = combined_register >> 8;
		rw_struct.data[1] = combined_register & 0x00FF;
		if(FDC1004_write_register(&rw_struct)) return (FDC1004_I2C_E | FDC1004_SETUP_E);
	}
	nrf_delay_ms(8);
	#ifdef PRINTOUT
		printString("CAPDAC setup OK.");
	#else
	#endif
	
	return FDC1004_SUCCESS;
}

uint8_t FDC1004_elimintate_offset(void)
{
	FDC1004_results_t results;
	uint8_t status = FDC1004_SUCCESS;
	
	status = FDC1004_capdac_setup();
	if(status) return status;
	memset(&results, 0, sizeof(FDC1004_results_t));
	
	status = FCD1004_measure(&results, SAMPLE_RATE_100, AV_RATE_H);		// perform measurement with best accuracy
	if(status) return status;
	FDC1004_offset.ch1 = results.ch1;
	FDC1004_offset.ch2 = results.ch2;
	FDC1004_offset.ch3 = results.ch3;
	FDC1004_offset.ch4 = results.ch4;
		
	nrf_delay_ms(20);
	
	#ifdef PRINTOUT
		printStringLn("OFFSET:\t");
		printFloat(RESULT_INT_TO_FLOAT(FDC1004_offset.ch1));	printString("\t");
		printFloat(RESULT_INT_TO_FLOAT(FDC1004_offset.ch2));	printString("\t");
		printFloat(RESULT_INT_TO_FLOAT(FDC1004_offset.ch3));	printString("\t");
		printFloatLn(RESULT_INT_TO_FLOAT(FDC1004_offset.ch4));	
		printStringLn("ERROR from wanted mid point:\t"); 
		printFloat(((FDC1004_RANGE * (float)PULL_VS_PUSH_FORCE))/10.0 - RESULT_INT_TO_FLOAT(FDC1004_offset.ch1));	printString("\t");
		printFloat(((FDC1004_RANGE * (float)PULL_VS_PUSH_FORCE))/10.0 - RESULT_INT_TO_FLOAT(FDC1004_offset.ch2));	printString("\t");
		printFloat(((FDC1004_RANGE * (float)PULL_VS_PUSH_FORCE))/10.0 - RESULT_INT_TO_FLOAT(FDC1004_offset.ch3));	printString("\t");
		printFloatLn(((FDC1004_RANGE * (float)PULL_VS_PUSH_FORCE))/10.0 - RESULT_INT_TO_FLOAT(FDC1004_offset.ch4));	
	#else
	#endif
	
	return FDC1004_SUCCESS;
}

/*****************************************************************************
* R/W FUNKCIJE
*****************************************************************************/
uint8_t FDC1004_read_register(FDC1004_t * rw_struct){
	rw_struct->rw_status = twi_master_transfer(FDC1004_ADDRESS_W, &rw_struct->register_pointer, 1, TWI_DONT_ISSUE_STOP);	// write pointer register
	if (rw_struct->rw_status){
		rw_struct->rw_status = twi_master_transfer(FDC1004_ADDRESS_R, &rw_struct->data[0], 2, TWI_ISSUE_STOP);	// read from pointed register
	}
	if(rw_struct->rw_status){
		return FDC1004_SUCCESS;
	}
	else{
		return FDC1004_I2C_E;
	}
}

uint8_t FDC1004_write_register(FDC1004_t * rw_struct){
	uint8_t data_to_write[3] = {rw_struct->register_pointer, rw_struct->data[0], rw_struct->data[1]};	//merge struct to single data array
		
	rw_struct->rw_status = twi_master_transfer(FDC1004_ADDRESS_W, data_to_write, 3, TWI_ISSUE_STOP);	// write data to pointer register from struct
	if (rw_struct->rw_status){	// status: 1: OK, 0: ERROR
		return FDC1004_SUCCESS;
	}
	else{
		return FDC1004_I2C_E;
	}
}


uint8_t FDC1004_timer_setup(void){
	nrf_drv_timer_config_t timer0_struct;
	uint32_t time_ticks  = 0;
	
	timer0_struct.bit_width = NRF_TIMER_BIT_WIDTH_16;
	timer0_struct.frequency = NRF_TIMER_FREQ_500kHz;
	timer0_struct.interrupt_priority = 1;
	timer0_struct.mode = NRF_TIMER_MODE_TIMER;
	timer0_struct.p_context = timer0_handler;
	if(nrf_drv_timer_init(&timer0, &timer0_struct, timer0_handler) != NRF_SUCCESS)  return FDC1004_SETUP_E;
	
	time_ticks = nrf_drv_timer_ms_to_ticks(&timer0, MEAS_TIMEOUT);	// 1ms timer resolution * MEAS_TIMEOUT
	nrf_drv_timer_extended_compare(&timer0, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_STOP_MASK, true);
	
	return FDC1004_SUCCESS;
}

void timer0_handler(nrf_timer_event_t event_type, void* p_context){
	switch(event_type){
		case NRF_TIMER_EVENT_COMPARE0:	
			timeout_interrupt++; 
			break;
		default:
			//Do nothing.
			break;
	} 
}

void start_timer(void){
	timeout_interrupt = 0;
	nrf_drv_timer_clear(&timer0);
	nrf_drv_timer_enable(&timer0);
}

void stop_timer(void){
	nrf_drv_timer_disable(&timer0);
	nrf_drv_timer_clear(&timer0); 
}

/*
DVOJISKI KOMPLEMENT in ta zmeda. Pobrano s foruma, lahko bi se TI naucil pisat dokumentacijo.

"The measurement data is already encoded in Two's complement format. 

So if your data is data[23:0] = 0x04864F (two's complement), that would equate to 296527 in decimal. 
Then you would divide by 219.
two's complement (296527/(219)) -> 0.56558pF"

*/
