/*
	Defines for communication, status responses, incoming and outgoing packets, errors 
*/

//status & data responses
#define S_CONNECTED 'c'
#define S_OFFSET 'o'
#define S_CALIBRATION 'l'
#define S_MEASUREMENT_STARTED 'x'
#define S_MEASUREMENT_STOPPED 's'
#define S_INIT_COMPLETE 'i'

// incoming commands 
#define I_CONNECT 'c'
#define I_ELIMINATE_OFFSET 'o'
#define I_START_MEAS 'm'
#define I_STOP_MEAS 's'
#define I_RESET 'r'

// measurement control
#define START_MEASUREMENT	1
#define STOP_MEASUREMENT	0

