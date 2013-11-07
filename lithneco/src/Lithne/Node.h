/**
 * Node.h - The official Lithne library.
 *
 * Copyright (c) 2011-2012
 * Serge Offermans, Remco Magielse
 * {s.a.m.offermans, r.magielse}@tue.nl
 * Intelligent Lighting Institute (ILI), TU/e.
 *
 * All rights reserved. LAST UPDATE: 01-04-2012
**/

#ifndef Node_h
#define Node_h

#ifndef	DB_MEMORY
	#define DB_MEMORY 10	//Define the DB_MEMORY to 5, meaning that we store up to 5 DB measures
#endif

#include "LithneXBee.h"
#include "LithneDefinitions.h"

class Node
{
  public:
	Node(	uint8_t _id = UNKNOWN_NODE_ID, 
			XBeeAddress64 _add64 = XBeeAddress64( 0x00000000, 0x00000000 ), 
			uint16_t = UNKNOWN_16B );
	~Node();

	uint8_t setID( uint8_t _id );

	XBeeAddress64 setXBeeAddress64( XBeeAddress64 _addr64 );
	XBeeAddress64 	getXBeeAddress64();
	XBeeAddress64 	getAddress64();				//deprecated
	void setAddress64( XBeeAddress64 _add64 );	//deprecated

	uint16_t setXBeeAddress16( uint16_t _addr16 );
	uint16_t getXBeeAddress16();
	uint16_t getAddress16();					//deprecated
	void setAddress16( uint16_t _add16 );		//deprecated
	

	void addDBMeasurement( uint16_t _id );
	void openDBRequest();
	void closeDBRequest();

	bool isNewMeasurement();
	bool hasXBeeAddress16();

	uint8_t			getID();
	uint16_t		getDB();
	
	uint32_t		getMSB();
	uint32_t		getLSB();


  private:
	bool 		openRequest;
	uint8_t 	nodeId;
	uint16_t 	nodeAddress16;
	uint16_t	db[DB_MEMORY];
	
	uint32_t 	lastDBReceptionTime, lastDBRequestTime, lastMessageTime;
	
	XBeeAddress64	nodeAddress64;
};

#endif

