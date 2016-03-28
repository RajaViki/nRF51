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
#include <math.h>

#include "nrf.h"
#include "twi_master.h"
#include "nrf_delay.h"
#include "nrf_drv_timer.h"

#include "nRF51_uart_print.h"

/*****************************************************************************
* change according to sensor and data
*****************************************************************************/
//#define PRINT_INFO	// print data directly from functions like init, capdac and offset elimination
//#define DEBUG_PRINT	//print error number (example): !2! (= i2c write to fdc error)

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
#define CAPDAC_RANGE		31			// 32 * 3.125 = 100pF
#define OFFSET_BIT_SHIFT	11		// after offset, make bitshift right to eliminate randomines

#define CAPDAC_REG_BITS_MASK	 0xFC1F	// mask with zeroes at CAPDAC position
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

// status
#define FDC_OK					0			// everything is OK

// timeout values (note: x4 because of measuring 4 capacitors)
#define MEAS_TIMEOUT_100SPS		41	//	[ms] Measurement must complete (DONEX set) in this time
#define MEAS_TIMEOUT_200SPS		21	//	[ms] Measurement must complete (DONEX set) in this time
#define MEAS_TIMEOUT_400SPS		11	//	[ms] Measurement must complete (DONEX set) in this time

/* ERROR CODE
0 - OK
1 - read
2 - write
3 - timer interrupt (measurement timeout)

* FDC1004_init()
10 - software reset
11 - cap pins and initial state: CONF_MEAS1
12 - cap pins and initial state: CONF_MEAS2
13 - cap pins and initial state: CONF_MEAS3
14 - cap pins and initial state: CONF_MEAS4

* FCD1004_measure()
19 - wrong passed sample rate
20 - write op: start measurement.
21 - read op: wait for measurement completion
22 - measurement timeout
23 - read op: results for channel 1
24 - read op: results for channel 2
25 - read op: results for channel 3
26 - read op: results for channel 4
27 - write op: stop repeated measurement

* FCD1004_start_repeated_measurement()
29 - _get_measurement_state() internal error
30 - wrong passed sample rate
31 - write op: start repeated measurement of all capacitors

* FCD1004_stop_repeated_measurement()
35 - stop measuring all capacitors
36 - unknown sample rate

* FCD1004_get_results()
39 - wrong passed av_rate = 0
40 - write op: start measurement.
41 - read op: wait for measurement completion
42 - measurement timeout
43 - read op: results for channel 1
44 - read op: results for channel 2
45 - read op: results for channel 3
46 - read op: results for channel 4

* FDC1004_capdac_setup()
50 - start measuring all capacitors (fast, only for rough range setup)
	51 - get_results
	52 - stop measuring all capacitors
53 - wrong capdac value returned
	54 - setup new capdac value


* FDC1004_elimintate_offset()
60 - init error
61 - capdac setup error
62 - start measuring all capacitors (precise, high av_rate)
	63 - get_results
	64 - stop measuring all capacitor

* I2C_init()
100 - twi_master_init() error

* FDC1004_timer_setup()
101 - nrf_drv_timer_init() error

* _get_capdac()
110 - read op: get chosen capdac register

* _set_capdac()
120 - wrong passed argument: capdac value too large
121 - read op: current register setings
122 - write op: back to register

* _get_measurement_state()
131 - read op: get conf register
*/

typedef struct{
	uint8_t register_pointer;
	uint8_t data[2];	// data[0] = MSB, data[1] = LSB
} FDC_data_t;

typedef struct{
	int32_t ch1;
	int32_t ch2;
	int32_t ch3;
	int32_t ch4;
	uint8_t measure_status;
} FDC_results_t;

typedef struct{
	int32_t ch1;
	int32_t ch2;
	int32_t ch3;
	int32_t ch4;
} FDC_offsets_t;

/*****************************************************************************
*	Initialization and read/write functions
*****************************************************************************/
uint8_t I2C_init(void);
uint8_t FDC1004_init(void);

uint8_t FCD1004_start_repeated_measurement(uint8_t sample_rate);
uint8_t FCD1004_stop_repeated_measurement(void);
uint8_t FCD1004_get_results(FDC_results_t * result_struct, uint8_t av_rate);

uint8_t FDC1004_elimintate_offset(void);
uint8_t FDC1004_capdac_setup(void);


/*****************************************************************************
*	Timer init and handling functions
*****************************************************************************/
uint8_t FDC1004_timer_setup(void);		// change to your custom timer; timer resoulution = 1ms
void _start_timer(void);
void _stop_timer(void);
void _set_timer_int_time(uint32_t time_ms);

/*****************************************************************************
*	Low level private functions
*****************************************************************************/
uint8_t _fdc_read_reg(FDC_data_t * rw_struct);
uint8_t _fdc_write_reg(FDC_data_t * rw_struct);
void _timer0_handler(nrf_timer_event_t event_type, void* p_context);

uint8_t _get_measurement_state(void);
uint8_t _set_capdac(uint8_t reg, uint8_t capdac_value);
uint8_t _get_capdac(uint8_t reg);
uint8_t _get_sample_rate(void);

uint8_t _dbg(uint8_t err_code);

#endif
