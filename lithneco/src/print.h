/*
 * debug.h
 *
 * Created: 27-6-2013 22:17:10
 *  Author: Elco
 */ 


#ifndef DEBUG_H_
#define DEBUG_H_

#include <progmem.h>

#define DEBUG_PORT 0 // send debug messages to main serial port
#define PRINT_BUFFER_SIZE 256 // A debug message is maximally 256 bytes

#define debugMessage(string, ...) printfToPort_P(DEBUG_PORT, PSTR(string), ##__VA_ARGS__)

void printToPort(uint8_t port, const char * string, uint16_t numChars);

void printfToPort_P(uint8_t port, const char *fmt, ... );

void printfToPort(uint8_t port, char *fmt, ... );
	

#endif /* DEBUG_H_ */