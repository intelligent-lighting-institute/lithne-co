/**
 * Node.cpp - The official Lithne library.
 *
 * Copyright (c) 2011-2012
 * Serge Offermans, Remco Magielse
 * {s.a.m.offermans, r.magielse}@tue.nl
 * Intelligent Lighting Institute (ILI), TU/e.
 *
 * All rights reserved. LAST UPDATE: 01-04-2012
**/

/* The following code makes the Library compatible with Arduino 1.0 */
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "Node.h"
#include "LithneXBee.h"
#include "LithneDefinitions.h"

//uint16_t 	db[DB_MEMORY] = {0};
//uint16_t 	db[DB_MEMORY];

/** Default Constructor **/
Node::Node( uint8_t _id, XBeeAddress64 _addr64,  uint16_t _addr16 )
{
	nodeAddress64		=	_addr64;
	nodeAddress16		=	_addr16;
	nodeId 				= 	_id;
	openRequest			=	false;

	lastDBReceptionTime	=	0;
	lastDBRequestTime	=	0;
	lastMessageTime		=	0;
	
	/*DEBUG PRINTS
	Serial.print("Adding Node ");
	Serial.print( nodeId );
	Serial.print(", with XBee Add ");
	Serial.println( getLSB(), HEX );
	*/
	
	for( int i=0; i<DB_MEMORY; i++)
	{
		db[i]	=	0;
	}
}

Node::~Node()
{
	/*	Perhaps we want to destruct some things here	*/
}

/*
__   _____ ___ ___  
\ \ / / _ \_ _|   \ 
 \ V / (_) | || |) |
  \_/ \___/___|___/ 
                    
*/

/** Set the identifier for the node **/
void Node::setID( uint8_t _id )
{	
	nodeId	=	_id;
}

/** Set the 64-bit address for this node **/
void Node::setAddress64( XBeeAddress64 _addr64 )
{
	nodeAddress64		=	_addr64;
}

/** Set the 16-bit address for this node **/
void Node::setAddress16( uint16_t _add16 )
{
	nodeAddress16	=	_add16;
}

/** Add a new DB Measurement **/
void Node::addDBMeasurement( uint16_t _db )
{
	for( int i=DB_MEMORY; i>1; i-- )
	{
		db[i-1]	=	db[i-2];
	}
	
	db[0]				=	_db;
	/*
	Serial.print("New DB reception for node ID ");
	Serial.print(nodeId,DEC); 
	Serial.print(". Values are now:  "); 
	for( int i=0; i < DB_MEMORY; i++ )
	{
		Serial.print( "," );
		Serial.print( db[i] );
	}
	Serial.println(); */
	
	
	lastDBReceptionTime = 	millis();
}

/** Open a new distance measurement request **/
void Node::openDBRequest()
{
	lastDBRequestTime 	= 	millis();
	openRequest 		= 	true;
	/* Serial.print("opening DB request IN node ");
	Serial.println(nodeId,DEC);*/
}

/** Close the measurement request **/
void Node::closeDBRequest()
{
	openRequest 		= 	false;
	/* Serial.print("closing DB request on node ");
	Serial.println(nodeId,DEC); */
}

/*
 ___  ___   ___  _    ___   _   _  _ 
| _ )/ _ \ / _ \| |  | __| /_\ | \| |
| _ \ (_) | (_) | |__| _| / _ \| .` |
|___/\___/ \___/|____|___/_/ \_\_|\_|

*/

/** Check if the DB measurement is a new measurement **/
bool Node::isNewMeasurement()
{
	if ( openRequest && lastDBRequestTime < lastDBReceptionTime)
	{
		return true;
	}
	else
	{
		return false;
	}
}
/**	Check if the current 16-bit address is known.
	if the add 16 of the node is equal to 
	the UNKNOWN_ADD16. If so, the address has not been set yet */
bool Node::hasAddress16()
{
	if( nodeAddress16 == UNKNOWN_ADD16 )
	{
		return false;
	}
	else
	{
		return true;
	}
}

/* 
 ___ _  _ _____ ___ ___ ___ ___ 
|_ _| \| |_   _| __/ __| __| _ \
 | || .` | | | | _| (_ | _||   /
|___|_|\_| |_| |___\___|___|_|_\
                                
*/

/** Returns the identifier of the node **/
uint8_t Node::getID()
{
	return nodeId;
}
/** Returns the latste DB measurement of the node (approximation of ditance) **/
uint16_t Node::getDB()
{
	return db[0];
}

/**	Returns the 16-bit address of the node	**/
uint16_t Node::getAddress16()
{
	/*	If the least significant byte is 0, this is the broadcast node.
		Then we don't return the stored address, but the UNKNOWN_ADD16 */
	if( getLSB() != 0xffff )
	{
		return	nodeAddress16;
	}
	else
	{
		return UNKNOWN_ADD16;
	}
}

/** Returns the most significant byte of the node address **/
uint32_t Node::getMSB()
{
	return nodeAddress64.getMsb();
}
/** Returns the least significant byte of the node address **/
uint32_t Node::getLSB()
{
	return nodeAddress64.getLsb();
}

/*
__  _____            _      _    _                 __ _ _  
\ \/ / _ ) ___ ___  /_\  __| |__| |_ _ ___ ______ / /| | | 
 >  <| _ \/ -_) -_)/ _ \/ _` / _` | '_/ -_|_-<_-</ _ \_  _|
/_/\_\___/\___\___/_/ \_\__,_\__,_|_| \___/__/__/\___/ |_| 

*/

XBeeAddress64 Node::getAddress64()
{
	return nodeAddress64;
}

