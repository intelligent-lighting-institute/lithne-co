/*
 * nodeInfo.h
 *
 * Created: 4-7-2013 16:43:13
 *  Author: Elco
 */ 


#ifndef NODEINFO_H_
#define NODEINFO_H_

#include "compiler.h"
#include <Arduino.h>

#define EEPROM_STORAGE_SPACE 32

#define NODENAME_EEPROM 0
#define LASTUPLOAD_EEPROM 33
#define FILENAME_EEPROM 65

class NodeInfo{
	public:
	NodeInfo();
	~NodeInfo();
	
	/* Functions to store stuff in the EEPROM */
	void setNodeName( String value );
	void setLastUpload( String value );
	void setFileName( String value );

	/* Actual writing to EEPROM*/
	void writeToEEPROM( uint8_t startAddress, String value );

	void sendInfo();

	/* Send Functions */
	void sendNodeName();
	void sendFileName();
	void sendLastUpload();
};

#endif /* NODEINFO_H_ */