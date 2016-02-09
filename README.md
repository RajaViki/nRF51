# Nordic-nRF51-custom-libraries
Some custom libraries for nRF. Tested with nRF51822.

1. UART: 
This is basic UART driver similar to Arduino Serial library. Includes fuctions like:
printString(), printStringLn(), printNumber(), printNumberLn(), printFloat(), printFloatLn(), printLn() 
-> for readable data. For sending raw data, there is writeData();
Works for unsigned and signed 32-bit integers. Numbers can be viewed as HEX, DEC, BIN or OCT format.  

2. HX711: 
This are basic functions for HX711 bridge ADC. Settable GAIN, RATE and offset (tara). Both channels. 

3. FDC1004: 
This are basic functions for FDC1004 CAPDAC - capacitive adc.
