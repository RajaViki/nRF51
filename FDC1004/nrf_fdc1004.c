/*
	Created by: Domen Jurkovic
	v1.3, 9-Feb-2016
*/

#include <stdbool.h>
#include <stdint.h>

#include "nrf_fdc1004.h"

// edit nrf_drv_config.h to enable TIMER0
// setup pins in twi_master.h
/*****************************************************************************
* I2C communication interface
*****************************************************************************/

FDC_offsets_t FDC1004_offset;

uint8_t timeout_interrupt = 0;
const nrf_drv_timer_t timer0 = NRF_DRV_TIMER_INSTANCE(0);	// TIMER 0 in use

// TWI/I2C bus init
uint8_t I2C_init(void){
	if(twi_master_init()){
		return FDC_OK;
	}
	else return _dbg(100);
}

// FDC setup
uint8_t FDC1004_init(void){
	FDC_data_t rw_struct;
	memset(&FDC1004_offset, 0, sizeof(FDC1004_offset));
		
	// software RESET
	rw_struct.register_pointer = FDC_CONF;	
	rw_struct.data[0] = 0x84; // software reset, 100SPS
	rw_struct.data[1] = 0x00;
	if(_fdc_write_reg(&rw_struct)) return _dbg(10);
	nrf_delay_ms(300);
	
	// SETUP cap pins and initial state
	rw_struct.register_pointer = CONF_MEAS1;
	rw_struct.data[0] = 0x1C;
	rw_struct.data[1] = 0x00;
	if(_fdc_write_reg(&rw_struct)) return _dbg(11);
		
	rw_struct.register_pointer = CONF_MEAS2;
	rw_struct.data[0] = 0x3C;
	rw_struct.data[1] = 0x00;
	if(_fdc_write_reg(&rw_struct)) return _dbg(12);
		
	rw_struct.register_pointer = CONF_MEAS3;
	rw_struct.data[0] = 0x5C;
	rw_struct.data[1] = 0x00;
	if(_fdc_write_reg(&rw_struct)) return _dbg(13);
	
	rw_struct.register_pointer = CONF_MEAS4;
	rw_struct.data[0] = 0x7C;
	rw_struct.data[1] = 0x00;
	if(_fdc_write_reg(&rw_struct)) return _dbg(14);
		
	//nrf_delay_ms(10);	

	/*
	rw_struct.register_pointer = MANUFACTURER_ID;
	if(_fdc_read_reg(&rw_struct))return (FDC1004_I2C_E | FDC1004_SETUP_E);	// read results for channel 1 - MSB
	printString("MANUFACTURER_ID: ");
	printNumber(rw_struct.data[0], HEX);
	printNumberLn(rw_struct.data[1], HEX);
	
	rw_struct.register_pointer = DEVICE_ID;
	if(_fdc_read_reg(&rw_struct))return (FDC1004_I2C_E | FDC1004_SETUP_E);	// read results for channel 1 - MSB
	printString("DEVICE_ID: ");
	printNumber(rw_struct.data[0], HEX);
	printNumberLn(rw_struct.data[1], HEX);
	*/
	
	return FDC_OK;
}

// Measurement functions
uint8_t FCD1004_start_repeated_measurement(uint8_t sample_rate){
	FDC_data_t rw_struct;
	uint32_t delay_time = 0;
	
	// START repeated measurement
	rw_struct.register_pointer = FDC_CONF;
	switch (sample_rate){
		case SAMPLE_RATE_100: 
			rw_struct.data[0] = 0x05;
			delay_time = MEAS_TIMEOUT_100SPS;
			break;
		case SAMPLE_RATE_200: 
			rw_struct.data[0] = 0x09; 
			delay_time = MEAS_TIMEOUT_200SPS;
			break;
		case SAMPLE_RATE_400: 
			rw_struct.data[0] = 0x0D;
			delay_time = MEAS_TIMEOUT_400SPS;
			break;
		default: 
			return _dbg(30);
	}
	rw_struct.data[1] = 0xF0;	// data[0] = MSB, data[1] = LSB
	if(_fdc_write_reg(&rw_struct)) return _dbg(31);	// start measuring all capacitors
	
	_set_timer_int_time(delay_time);	//	set timeout value
	
	return FDC_OK;
}

uint8_t FCD1004_stop_repeated_measurement(void){
	FDC_data_t rw_struct;
	
	// STOP repeated measurement
	rw_struct.register_pointer = FDC_CONF;
	rw_struct.data[0] = 0x04; // data[0] = MSB 
	rw_struct.data[1] = 0x00;	// data[1] = LSB
	if(_fdc_write_reg(&rw_struct)) return _dbg(35);	// stop measuring all capacitors
	
	return FDC_OK;
}

uint8_t FCD1004_get_results(FDC_results_t * result_struct, uint8_t av_rate){
	FDC_data_t rw_struct;
	uint8_t i = 0;
	uint32_t temp_res = 0;
	memset(result_struct, 0, sizeof(FDC_results_t));
		
	if( av_rate == 0){	
		return _dbg(39);	// wrong passed argumen; av_rate must be > 0
	}
	
	for(i=0; i<av_rate; i++){
		rw_struct.register_pointer = FDC_CONF;				
		if(_fdc_read_reg(&rw_struct))return _dbg(40);	// read register
		
		// WAIT for measurement completion
		_start_timer();
		while((rw_struct.data[1] & DONEX_MASK) != DONEX_MASK ){	// wait while DONE_1, 2, 3, 4 != 1
			if(_fdc_read_reg(&rw_struct))return _dbg(41);	// read register
			
			if(timeout_interrupt){ // measurement timeout
				_stop_timer(); 
				FCD1004_stop_repeated_measurement();
				return _dbg(42);
			}	
		}
		_stop_timer();
				
		// READ RESULTS
		// CH1
		rw_struct.register_pointer = MEAS1_MSB;
		if(_fdc_read_reg(&rw_struct)) return _dbg(43);	// read results for channel 1 - MSB
		temp_res = rw_struct.data[0] << 16;
		temp_res |= rw_struct.data[1] << 8;
		rw_struct.register_pointer = MEAS1_LSB;
		if(_fdc_read_reg(&rw_struct)) return _dbg(43);	// read results for channel 1 - LSB
		temp_res |= rw_struct.data[0];
		result_struct->ch1 += temp_res;
		temp_res = 0;
		// CH2
		rw_struct.register_pointer = MEAS2_MSB;
		if(_fdc_read_reg(&rw_struct)) return _dbg(44);	// read results for channel 2 - MSB
		temp_res = rw_struct.data[0] << 16;
		temp_res |= rw_struct.data[1] << 8;
		rw_struct.register_pointer = MEAS2_LSB;
		if(_fdc_read_reg(&rw_struct)) return _dbg(44);	// read results for channel 2 - LSB
		temp_res |= rw_struct.data[0];
		result_struct->ch2 += temp_res;
		temp_res = 0;
		// CH3
		rw_struct.register_pointer = MEAS3_MSB;
		if(_fdc_read_reg(&rw_struct)) return _dbg(45);	// read results for channel 3 - MSB
		temp_res = rw_struct.data[0] << 16;
		temp_res |= rw_struct.data[1] << 8;
		rw_struct.register_pointer = MEAS3_LSB;
		if(_fdc_read_reg(&rw_struct)) return _dbg(45);	// read results for channel 3 - LSB
		temp_res |= rw_struct.data[0];
		result_struct->ch3 += temp_res;
		temp_res = 0;
		// CH4
		rw_struct.register_pointer = MEAS4_MSB;
		if(_fdc_read_reg(&rw_struct)) return _dbg(46);	// read results for channel 4 - MSB
		temp_res = rw_struct.data[0] << 16;
		temp_res |= rw_struct.data[1] << 8;
		rw_struct.register_pointer = MEAS4_LSB;
		if(_fdc_read_reg(&rw_struct)) return _dbg(46);	// read results for channel 4 - LSB
		temp_res |= rw_struct.data[0];
		result_struct->ch4 += temp_res;
		temp_res = 0;

	}
	
	if(av_rate > 0){
		result_struct->ch1 /= av_rate;
		result_struct->ch2 /= av_rate;
		result_struct->ch3 /= av_rate;
		result_struct->ch4 /= av_rate;
	}
	
	if(FDC1004_offset.ch1 > 0){	// if CH1 has offset valuse different than 0, all channels have offset values
		result_struct->ch1 -= FDC1004_offset.ch1;
		result_struct->ch1 >>= OFFSET_BIT_SHIFT;
	
		result_struct->ch2 -= FDC1004_offset.ch2;
		result_struct->ch2 >>= OFFSET_BIT_SHIFT;
	
		result_struct->ch3 -= FDC1004_offset.ch3;
		result_struct->ch3 >>= OFFSET_BIT_SHIFT;

		result_struct->ch4 -= FDC1004_offset.ch4;
		result_struct->ch4 >>= OFFSET_BIT_SHIFT;
	}

	return FDC_OK;
}

// CAPDAC and offset elimination functions
uint8_t FDC1004_capdac_setup(void){
	FDC_results_t results;
	FDC_data_t rw_struct;
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

	#ifdef PRINT_INFO
		printStringLn("CAPDAC setup:");
		
		printString("\tPull vs push ratio: ");
		printNumber(PULL_VS_PUSH_FORCE, DEC);
		printString("0:");
		printNumber(10 - PULL_VS_PUSH_FORCE, DEC);
		printStringLn("0 (%)");
		
		printString("\tWanted midpoint of 15pF range: ");
		printFloatLn(mid_point_result_f);
	#endif
	
	if(FDC1004_init()) return _dbg(50);
	
	// CAPDAC OFFSET ELIMINATION
	do{
			// MEASURE all capacitors  - measure fast, only for rough range setup
				//if(FCD1004_measure(&results, SAMPLE_RATE_400, AV_RATE_N)) return _dbg(51);
				//nrf_delay_ms(8);	// ? ? ? ? ? ? 
		
		if(FCD1004_start_repeated_measurement(SAMPLE_RATE_400)) return _dbg(51);
		if(FCD1004_get_results(&results, 1)) return _dbg(52);	// av_rate = 1
		if(FCD1004_stop_repeated_measurement()) return _dbg(53);
		nrf_delay_ms(40);	
		
		for(i=0; i<4; i++){
			results_f = (float) *results_array[i];
			results_f = results_f / DIVIDE_RESULTS;		// in pF, integer+decimal number. ?/ CAPDAc resolution
			results_f_array[i] = results_f;
			results_i = (uint32_t) results_f; 
			
			if(results_i >= FDC1004_RANGE)	
			{
				capdac_array[i] += (uint16_t)((FDC1004_RANGE - mid_point_result_f) / CAPDAC_RESOLUTION);
			}
			
			// write CAPDAC value to register
			combined_register = ch_setup_register_pointer[i] | (capdac_array[i] << 5);
			rw_struct.register_pointer = conf_measx_register_pointer[i];
			rw_struct.data[0] = combined_register >> 8;
			rw_struct.data[1] = combined_register & 0x00FF;
			if(_fdc_write_reg(&rw_struct)) return _dbg(54);
		}
	}while(!((results_f_array[0] < FDC1004_RANGE) && (results_f_array[1] < FDC1004_RANGE) && (results_f_array[2] < FDC1004_RANGE) && (results_f_array[3] < FDC1004_RANGE)));
	
	// measure accurate, for fine capdac setup
		//if(FCD1004_measure(&results, SAMPLE_RATE_100, AV_RATE_H)) return _dbg(53);
	if(FCD1004_start_repeated_measurement(SAMPLE_RATE_100)) return _dbg(55);
	if(FCD1004_get_results(&results, AV_RATE_H)) return _dbg(56);	// av_rate = AV_RATE_H
	if(FCD1004_stop_repeated_measurement()) return _dbg(57);
	
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
		if(_fdc_write_reg(&rw_struct)) return _dbg(58);
	}
	
	#ifdef PRINT_INFO
		printStringLn("\tCAPDAC setup OK.");
	#endif
	
	return FDC_OK;
}

uint8_t FDC1004_elimintate_offset(void){
	FDC_results_t results;
	
	if(FDC1004_capdac_setup()) return _dbg(60);
	memset(&results, 0, sizeof(FDC_results_t));
	
	// perform measurement with best accuracy
		//if(FCD1004_measure(&results, SAMPLE_RATE_100, AV_RATE_H)) return _dbg(61);
	if(FCD1004_start_repeated_measurement(SAMPLE_RATE_100)) return _dbg(61);
	if(FCD1004_get_results(&results, AV_RATE_H)) return _dbg(62);	// av_rate = AV_RATE_H
	if(FCD1004_stop_repeated_measurement()) return _dbg(63);
	
	FDC1004_offset.ch1 = results.ch1;
	FDC1004_offset.ch2 = results.ch2;
	FDC1004_offset.ch3 = results.ch3;
	FDC1004_offset.ch4 = results.ch4;
	
	#ifdef PRINT_INFO
		printStringLn("Offset values:\t");
		printString("\t");
		printFloat(RESULT_INT_TO_FLOAT(FDC1004_offset.ch1));	printString("\t");
		printFloat(RESULT_INT_TO_FLOAT(FDC1004_offset.ch2));	printString("\t");
		printFloat(RESULT_INT_TO_FLOAT(FDC1004_offset.ch3));	printString("\t");
		printFloatLn(RESULT_INT_TO_FLOAT(FDC1004_offset.ch4));	
		printStringLn("Deviation from wanted mid point:\t"); 
		printString("\t");
		printFloat(((FDC1004_RANGE * (float)PULL_VS_PUSH_FORCE))/10.0 - RESULT_INT_TO_FLOAT(FDC1004_offset.ch1));	printString("\t");
		printFloat(((FDC1004_RANGE * (float)PULL_VS_PUSH_FORCE))/10.0 - RESULT_INT_TO_FLOAT(FDC1004_offset.ch2));	printString("\t");
		printFloat(((FDC1004_RANGE * (float)PULL_VS_PUSH_FORCE))/10.0 - RESULT_INT_TO_FLOAT(FDC1004_offset.ch3));	printString("\t");
		printFloatLn(((FDC1004_RANGE * (float)PULL_VS_PUSH_FORCE))/10.0 - RESULT_INT_TO_FLOAT(FDC1004_offset.ch4));	
	#endif
	
	return FDC_OK;
}

// Private timer functions
uint8_t FDC1004_timer_setup(void){
	nrf_drv_timer_config_t timer0_struct;
	
	timer0_struct.bit_width = NRF_TIMER_BIT_WIDTH_16;	// with NRF_TIMER_FREQ_31250Hz: max 2.097 seconds
	timer0_struct.frequency = NRF_TIMER_FREQ_31250Hz; // 32us each tick   before: NRF_TIMER_FREQ_500kHz;
	timer0_struct.interrupt_priority = 1;
	timer0_struct.mode = NRF_TIMER_MODE_TIMER;
	timer0_struct.p_context = _timer0_handler;
	if(nrf_drv_timer_init(&timer0, &timer0_struct, _timer0_handler) != NRF_SUCCESS) return _dbg(101);
		
	_set_timer_int_time(MEAS_TIMEOUT_100SPS);
		
	return FDC_OK;
}

void _start_timer(void){
	timeout_interrupt = 0;
	nrf_drv_timer_clear(&timer0);
	nrf_drv_timer_enable(&timer0);
}

void _stop_timer(void){
	nrf_drv_timer_disable(&timer0);
	nrf_drv_timer_clear(&timer0);
	timeout_interrupt = 0;
}

void _set_timer_int_time(uint32_t time_ms){
	uint32_t time_ticks = nrf_drv_timer_ms_to_ticks(&timer0, time_ms);	// ms to ticks
	nrf_drv_timer_extended_compare(&timer0, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_STOP_MASK, true);
}

/*****************************************************************************
* Private R/W functions
*****************************************************************************/
uint8_t _fdc_read_reg(FDC_data_t * rw_struct){
	
	// write pointer register to read; true = OK.
	if(twi_master_transfer(FDC1004_ADDRESS_W, &rw_struct->register_pointer, 1, TWI_DONT_ISSUE_STOP)){
		if(twi_master_transfer(FDC1004_ADDRESS_R, &rw_struct->data[0], 2, TWI_ISSUE_STOP)){
			return FDC_OK;
		}
		else return _dbg(1);
	}
	else return _dbg(1);
}

uint8_t _fdc_write_reg(FDC_data_t * rw_struct){
	//merge struct to single data array
	uint8_t data_to_write[3] = {rw_struct->register_pointer, rw_struct->data[0], rw_struct->data[1]};	
	
	// write data to pointer register from struct
	if(twi_master_transfer(FDC1004_ADDRESS_W, data_to_write, 3, TWI_ISSUE_STOP)){
		return FDC_OK;
	}
	else return _dbg(2);
}

void _timer0_handler(nrf_timer_event_t event_type, void* p_context){
	switch(event_type){
		case NRF_TIMER_EVENT_COMPARE0:	
			timeout_interrupt++;
			_dbg(3);
			break;
		default: break;	//Do nothing.
	}
}

uint8_t _dbg(uint8_t err_code){
	#ifdef DEBUG_PRINT
		printString("!");
		printNumber(err_code, DEC);
		printStringLn("!");
	#endif
	return err_code;
}


/* Note: about raw results
"The measurement data is already encoded in Two's complement format. 
So if your data is data[23:0] = 0x04864F (two's complement), that would equate to 296527 in decimal. 
Then you would divide by 219.
two's complement (296527/(219)) -> 0.56558pF"
*/


