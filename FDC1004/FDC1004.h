/*
	Created by: Domen Jurkovic
	v1.3, 9-Feb-2016
*/

#ifndef FDC1004_H
#define FDC1004_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "nrf.h"
#include "twi_master.h"
#include "nrf_delay.h"
#include "nrf_drv_timer.h"

#include "nRF51_uart_print.h"

/*****************************************************************************
* change according to sensor and data
*****************************************************************************/
#define PRINTOUT

#define PULL_VS_PUSH_FORCE	3
// X*10; (if x=3, pull force represent 30% of FDC range, pull force 70%
																// pull force increases capacitance
																// push force decreases capacitance (should be larger %)
																// must be integer, 1, 2, 3, 4, ..., 9.
									// NOTE: it is always possible of max 1.5pF (10% of FDC range) error. This library supports only CAPDAC settings, 
									// while OFFSET registers remain unchanged. 

#define AV_RATE_N	4		// normal averaging rate of measurement. increases complete measurement time. 
#define AV_RATE_H	20		// high averaging rate of measurement (for offset). increases complete measurement time. 


/*****************************************************************************
* device setup
*****************************************************************************/

#define RESULT_INT_TO_FLOAT(number)	(((float)number) / DIVIDE_RESULTS)

#define CAP1	1
#define CAP2	2
#define CAP3	3
#define CAP4	4

#define SAMPLE_RATE_100	1
#define SAMPLE_RATE_200	10
#define SAMPLE_RATE_400	100
#define DONEX_MASK			0x0F		// measurement done flags mask
#define FDC1004_RANGE		15.0			// 15 pF 
#define DIVIDE_RESULTS	524288.0	// 2^19
#define CAPDAC_RESOLUTION	3.125	// pF
#define CAPDAC_RANGE		32			// 32 * 3.125 = 100pF
#define OFFSET_BIT_SHIFT	11		// after offset, make bitshift right to eliminate randomines

#define CONF_MEAS1_CH_SETUP	0x1000
#define CONF_MEAS2_CH_SETUP	0x3000
#define CONF_MEAS3_CH_SETUP	0x5000
#define CONF_MEAS4_CH_SETUP	0x7000

#define FDC1004_ADDRESS_R	0xA1	// read command = 1
#define FDC1004_ADDRESS_W	0xA0	// write command = 0

// FDC registers
#define MEAS1_MSB		0x00			// MSB portion of Measurement 1
#define MEAS1_LSB		0x01			// LSB portion of Measurement 1
#define MEAS2_MSB		0x02			// MSB portion of Measurement 2
#define MEAS2_LSB		0x03			// LSB portion of Measurement 2
#define MEAS3_MSB		0x04			// MSB portion of Measurement 3
#define MEAS3_LSB		0x05			// LSB portion of Measurement 3
#define MEAS4_MSB		0x06			// MSB portion of Measurement 4
#define MEAS4_LSB		0x07			// LSB portion of Measurement 4
#define CONF_MEAS1	0x08			// Measurement 1 Configuration
#define CONF_MEAS2	0x09			// Measurement 2 Configuration
#define CONF_MEAS3	0x0A			// Measurement 3 Configuration
#define CONF_MEAS4	0x0B			// Measurement 4 Configuration
#define FDC_CONF		0x0C			// Capacitance to Digital Configuration
#define OFFSET_CAL_CIN1	0x0D	// CIN1 Offset Calibration
#define OFFSET_CAL_CIN2	0x0E	// CIN2 Offset Calibration
#define OFFSET_CAL_CIN3	0x0F	// CIN3 Offset Calibration
#define OFFSET_CAL_CIN4	0x10	// CIN4 Offset Calibration
#define GAIN_CAL_CIN1		0x11 	// CIN1 Gain Calibration
#define GAIN_CAL_CIN2		0x12 	// CIN2 Gain Calibration
#define GAIN_CAL_CIN3		0x13 	// CIN3 Gain Calibration
#define GAIN_CAL_CIN4		0x14 	// CIN4 Gain Calibration
#define MANUFACTURER_ID	0xFE	// ID of Texas Instruments
#define DEVICE_ID				0xFF	// ID of FDC1004 device

// errors
#define FDC1004_SUCCESS 0			// everything is OK
#define FDC1004_I2C_E		1			// FDC I2C communication error
#define FDC1004_DATA_E	2			// FDC I2C timeout error - measurement didn't complete in MEAS_TIMEOUT
#define FDC1004_SETUP_E	4			// FDC setup error

#define MEAS_TIMEOUT		50		// [ms] Measurement must complete (DONEX set) in this time

typedef struct{
	uint8_t rw_status;
	uint8_t register_pointer;
	uint8_t data[2];	// data[0] = MSB, data[1] = LSB
} FDC1004_t;

typedef struct{
	int32_t ch1;
	int32_t ch2;
	int32_t ch3;
	int32_t ch4;
	uint8_t measure_status;
} FDC1004_results_t;

typedef struct{
	int32_t ch1;
	int32_t ch2;
	int32_t ch3;
	int32_t ch4;
} FDC1004_offset_t;

/*****************************************************************************
*	Initialization and read/write functions
*****************************************************************************/
uint8_t FDC1004_init(void);
uint8_t FCD1004_measure(FDC1004_results_t * result_struct, uint8_t sample_rate, uint8_t av_rate);

uint8_t FCD1004_measure_single_cap(FDC1004_results_t * result_struct, uint8_t sample_rate, uint8_t select_cap, uint8_t av_rate);
uint8_t FDC1004_capdac_setup(void);
uint8_t FDC1004_elimintate_offset(void);

//FDC1004_t FDC1004_read_register(uint8_t reg);
uint8_t FDC1004_read_register(FDC1004_t * rw_struct);
uint8_t FDC1004_write_register(FDC1004_t * rw_struct);

uint8_t FDC1004_timer_setup(void);		// change to your custom timer; timer resoulution = 1ms
void timer0_handler(nrf_timer_event_t event_type, void* p_context);
void start_timer(void);
void stop_timer(void);


#endif

