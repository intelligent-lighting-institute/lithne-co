/*
 * nodeInfo.cpp
 *
 * Created: 4-7-2013 16:45:25
 *  Author: Elco
 */ 


#define EEPROM_STORAGE_SPACE 32

#define NODENAME_EEPROM 0
#define LASTUPLOAD_EEPROM 33
#define FILENAME_EEPROM 65

#include "nodeInfo.h"
#include <avr/eeprom.h>

#include "compiler.h"
#include <Arduino.h>
#include <Lithne/Lithne.h>
#include "lithneProgrammer.h"
#include "print.h"

NodeInfo::NodeInfo(){
	
}

NodeInfo::~NodeInfo(){
	
}

/* Functions to store stuff in teh EEPROM */
void NodeInfo::setNodeName( String val )
{
	writeToEEPROM( NODENAME_EEPROM, val );
}

void NodeInfo::setLastUpload( String val )
{
	writeToEEPROM( LASTUPLOAD_EEPROM, val );
}

void NodeInfo::setFileName( String val )
{
	writeToEEPROM( FILENAME_EEPROM, val );
}

/* Actual writing to EEPROM*/
void NodeInfo::writeToEEPROM( uint8_t startAddress, String val )
{
	// Determine the size of the string to store
	int valueSize = val.length();
	debugMessage("\t Writing to EEPROM, StartAdd:\t%u,\tval:\t%s,\tlength:%u", \
		startAddress, \
		&val[0], \
		valueSize);

	for (int i = 0; i < valueSize; i++)
	{
		eeprom_write_byte((unsigned char *) (startAddress + i), val.charAt( i ) );
	}
	for (int i = valueSize; i < EEPROM_STORAGE_SPACE; i++ )
	{
		eeprom_write_byte((unsigned char *) (startAddress+i),0x00); //   use this to add one 0x00 at the end of data if we have space left
	}
}

void NodeInfo::sendInfo()
{
	// Send the node info with random time intervals because many nodes in the network may do the same; reduces the chance of collision
	delay(random(0,100));
	sendNodeName();
	delay(random(20,100));
	sendFileName();
	delay(random(20,100));
	sendLastUpload();
}

/* Send Functions */

void NodeInfo::sendNodeName()
{
	// Loop through the storage space for the NODENAME
	// as long as we dont encounter an empty field, we add it to the Lithne message
	int i = 0;
	while ( i < EEPROM_STORAGE_SPACE && (eeprom_read_byte((unsigned char *)(NODENAME_EEPROM + i)) != 0x00))
	{
		Lithne.getOutgoingMessage()->addByte( eeprom_read_byte((unsigned char *) (NODENAME_EEPROM + i) ));
		i++;
	}
	Lithne.setRecipient( REMOTE );
	Lithne.setScope( lithneProgrammingScope );
	Lithne.setFunction( fNodeName );
	Lithne.send();
}

void NodeInfo::sendFileName()
{
	// Loop through the storage space for the FILENAME
	// as long as we dont encounter an empty field, we add it to the Lithne message
	int i = 0;
	while ( i < EEPROM_STORAGE_SPACE && (eeprom_read_byte((unsigned char *)(FILENAME_EEPROM + i)) != 0x00))
	{
		Lithne.getOutgoingMessage()->addByte( eeprom_read_byte((unsigned char *) (FILENAME_EEPROM + i) ));
		i++;
	}
	Lithne.setRecipient( REMOTE );
	Lithne.setScope( lithneProgrammingScope );
	Lithne.setFunction( fFileName );
	Lithne.send();
}

void NodeInfo::sendLastUpload()
{
	// Loop through the storage space for the LASTUPLOAD
	// as long as we dont encounter an empty field, we add it to the Lithne message
	int i = 0;
	while ( i < EEPROM_STORAGE_SPACE && (eeprom_read_byte((unsigned char *)(LASTUPLOAD_EEPROM + i)) != 0x00))
	{
		Lithne.getOutgoingMessage()->addByte(eeprom_read_byte((unsigned char *)(LASTUPLOAD_EEPROM + i) ));
		i++;
	}
	Lithne.setRecipient( REMOTE );
	Lithne.setScope( lithneProgrammingScope );
	Lithne.setFunction( fLastUpload );
	Lithne.send();
}