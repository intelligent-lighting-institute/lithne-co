/*
 * pageBuffer.h
 *
 * Created: 4-7-2013 15:02:59
 *  Author: Elco
 */ 


#ifndef PAGEBUFFER_H_
#define PAGEBUFFER_H_

#include <inttypes.h>

static const uint16_t pageSize = 512;                   //   pageSize = 512 for xmega256 and 256 for xmega32

class PageBuffer{
	public:
	PageBuffer();
	~PageBuffer();
	void reset();
	uint16_t getPos();
	void add(uint8_t b);
	void removeHeaderFromFirstPage();
	uint8_t getByte(uint16_t pos);
	uint8_t * getBuffer();
	
	// Buffer to store all bytes in a page so we can write it entirely (8 packs in a page)
	uint8_t buffer[pageSize];

	// Keep track of the current position in the page buffer -- TODO Rename to pageBufferSize
	private:
	uint16_t bytesInPageBuffer;
};



#endif /* PAGEBUFFER_H_ */