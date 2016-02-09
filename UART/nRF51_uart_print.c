/*
 *	Created by: Domen Jurkovic, 8-Feb-2016
 *	Arduino like functions for serial printing of numbers and strings
 
 *	For other platforms (STM, Freescale, ...) edit only:
		- UART_Init()
		- uart_error_handle()
		- uart_send_byte()
		- uart_receive_byte()
			- include appropriate headers for UART

*/

#include "nRF51_uart_print.h"

/****************************************************************************************/
/* UART INIT FUNCTIONS - change accordingly to your HW */
/****************************************************************************************/
uint32_t UART_Init(uint8_t rx_pin, uint8_t tx_pin, uint32_t baud_rate){
	uint32_t err_code = 0;
	const app_uart_comm_params_t comm_params = {rx_pin, tx_pin, 0, 0, APP_UART_FLOW_CONTROL_DISABLED, false, baud_rate};
	
	//APP_UART_INIT(&comm_params, uart_error_handle, APP_IRQ_PRIORITY_LOW, err_code);
	
	APP_UART_FIFO_INIT(&comm_params,
                         1,		//UART_RX_BUF_SIZE
                         256,	//UART_TX_BUF_SIZE
                         uart_error_handle,
                         APP_IRQ_PRIORITY_LOW,
                         err_code);

	return err_code;
}

void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}

uint32_t uart_send_byte(uint8_t byte){
	while(app_uart_put(byte) != NRF_SUCCESS);
	
	return 0; 
}

// currently only single receive character is implemented
uint8_t uart_receive_byte(void){
	uint8_t byte;
	app_uart_get(&byte);
	
	return byte;
}


/****************************************************************************************/
/* PRINT/WRITE FUNCTIONS */
/****************************************************************************************/

/*
	Send/print character/string over UART. 
	Printable data for viewing on terminal.
	Call this function:		printString("test data");
	Data must be string.
*/
void printString(char *data){
	uint16_t i;
	uint16_t str_length = strlen(data);
	for(i = 0; i < str_length; i++)
	{
		uart_send_byte(data[i]);
	}
}

/*
	Send/print unsigned or signed number over UART. 
	Printable data for viewing on terminal.
	Call this function:		printNumber(number, DEC);		printNumber(2246, DEC);	
	Base: DEC, HEX, OCT, BIN
	Data must be number, int32_t.
*/
void printNumber(int32_t number, uint8_t base){
	if (number < 0) 
	{
		printString("-");
		number = -number;
		printUnsignedNumber(number, base);
	}
	else 
	{
		printUnsignedNumber(number, base);
	}
}
//print WITH new line and carriage return
void printStringLn(char *data){
	uint16_t i;
	uint16_t str_length = strlen(data);
	for(i = 0; i < str_length; i++)
	{
		uart_send_byte(data[i]);
	}
	printLn();
}

/*
	Send/print unsigned or signed number over UART. 
	Printable data for viewing on terminal.
	Call this function:		printNumber(number, DEC);		printNumber(2246, DEC);	
	Base: DEC, HEX, OCT, BIN
	Data must be number, int32_t.
*/
void printNumberLn(int32_t number, uint8_t base){
	if (number < 0) 
	{
		printString("-");
		number = -number;
		printUnsignedNumber(number, base);
	}
	else 
	{
		printUnsignedNumber(number, base);
	}
	printLn();
}

void printLn(){
	printString("\n\r");
}

/*
	Send raw data over UART. 
	Not "printable" data.
	Call this function:		writeData(&data, sizeof(data));
	Data can be any type.
*/
void writeData(void *data, uint8_t dataSize){
  uint8_t i, d;
  
  d = dataSize/2;
 
  for(i = 0; i < d; i++)
  {
    uart_send_byte(*( ((uint16_t *)data) + i ) );
  }
 
}

/*
	This is "private" function. It is used by other functions like: printNumber(int32_t number, uint8_t base). 
	However, it can be used by user.
	Send/print unsigned number over UART. 
	Printable data for viewing on terminal.
	Call this function:		printUnsignedNumber(number, DEC);		printUnsignedNumber(2246, DEC);	
	Base: DEC, HEX, OCT, BIN
	Data must be number, int32_t.
*/
void printUnsignedNumber(uint32_t n, uint8_t base){
	char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
	char *str = &buf[sizeof(buf) - 1];
	unsigned long m;
	char c;
  *str = '\0';

  //prevent crash if called with base == 1
  if (base < 2) base = 10;

  do 
	{
    m = n;
    n /= base;
    c = m - base * n;
    *--str = c < 10 ? c + '0' : c + 'A' - 10;
	} while(n);

	printString(str); 
}

// pay attention on "double" type range
// ? - max 6 digits after decimal point?
void printFloat(double number){
	char float_as_string[20];
	sprintf(float_as_string, "%f", number);
  printString(float_as_string);
}

void printFloatLn(double number){
	printFloat(number);
	printLn();
}

