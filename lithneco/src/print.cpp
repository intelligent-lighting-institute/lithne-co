/*
 * debug.c
 *
 * Created: 27-6-2013 22:11:53
 *  Author: Elco
 */ 

extern "C"{
#include "asf.h"	
}

#include <stdarg.h>
#include <stdio.h>
#include "print.h"
	
static char printBuffer[PRINT_BUFFER_SIZE]; 
extern volatile uint8_t main_port_open;


void printToPort(uint8_t port, bool printLine, const char * string, uint16_t numChars){	
	if (!(main_port_open & (1 << port))) {
		// Port not open
		return;
	}		
	udi_cdc_multi_write_buf(port, string, numChars);
	if(printLine){
		udi_cdc_multi_putc(port, '\n');
		udi_cdc_multi_putc(port, '\r');
	}
}

void debugMessageImpl_P(const char *fmt, ... ){
	va_list args;
	printToPort(DEBUG_PORT, false, "\t\t\t\tDEBUG\t", sizeof("\t\t\t\tDEBUG\t")); // print indent and DEBUG without line end
	va_start (args, fmt );
	printfToPort_P(DEBUG_PORT, true, fmt, args); // print debug message and line end
	va_end (args);
}

// create a printf like interface to each port. Format string stored in PROGMEM
void printfToPort_P(uint8_t port, bool printLine, const char *fmt, ... ){	
	uint16_t numChars = 0;
	va_list args;
	va_start (args, fmt );
	numChars = vsnprintf_P(printBuffer, PRINT_BUFFER_SIZE, fmt, args);
	va_end (args);
	if (numChars > PRINT_BUFFER_SIZE){
		numChars = PRINT_BUFFER_SIZE;
	}
	printToPort(port, printLine, printBuffer, numChars);	
}

// create a printf like interface to each port. Format string stored in RAM
void printfToPort(uint8_t port, bool printLine, char *fmt, ... ){
	uint16_t numChars = 0;
	va_list args;
	va_start (args, fmt );
	numChars = vsnprintf(printBuffer, PRINT_BUFFER_SIZE, fmt, args);
	va_end (args);
	if (numChars > PRINT_BUFFER_SIZE){
		numChars = PRINT_BUFFER_SIZE;
	}
	printToPort(port, printLine, printBuffer, numChars);
}