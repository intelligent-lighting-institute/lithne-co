/*
 * pageBuffer.cpp
 *
 * Created: 4-7-2013 15:02:38
 *  Author: Elco
 */ 

#include "pageBuffer.h"
#include "print.h"

PageBuffer::PageBuffer(){
	reset();
}

PageBuffer::~PageBuffer(){
}

// This is called when we start a new upload or the buffer is full
void PageBuffer::reset()
{
	bytesInPageBuffer = 0;
	//  memset(_surfaceReadings, 0, sizeof(_surfaceReadings));
	for( uint16_t i=0; i<pageSize; i++ )
	{
		buffer[i]  =  0;
	}
}

// Returns the buffer position that will be written to next
uint16_t PageBuffer::getPos()
{
	return bytesInPageBuffer;
}

// Add a byte to the page buffer
void PageBuffer::add( uint8_t b )
{
	// First check if the buffer is not full (prevent array index out of bound)
	// This should never happen in principle
	if (bytesInPageBuffer > pageSize)
	{
		// If there are too many bytes in the page buffer we limit it
		debugMessage("Too many bytes in pageBuff! \r\n");
		bytesInPageBuffer = pageSize;
	}
	// Write the byte to the buffer
	buffer[bytesInPageBuffer] = b;
	// Update the byte counter
	bytesInPageBuffer++;
}

// In the first packet the buffer contains 2 bytes of info about the number of pages to be expected
// These have to be removed as they are not part of the program, we do that here
void PageBuffer::removeHeaderFromFirstPage()
{
	// Shift all values two to the left.
	for(int i = 0; i < 64; i++)  //   move code data to beginning of buffer for correct processing
	{
		buffer[i] = buffer[i+2];
	}
	bytesInPageBuffer -= 2;
}

// Read a byte from the buffer
uint8_t PageBuffer::getByte( uint16_t pos )
{
	return buffer[pos];
}

// Get the pointer to the entire buffer
uint8_t * PageBuffer::getBuffer()
{
	return buffer;
}