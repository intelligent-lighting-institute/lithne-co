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

	void setID( uint8_t _id );
	void setAddress64( XBeeAddress64 _add64 );
	void setAddress16( uint16_t _add16 );
	void addDBMeasurement( uint16_t _id );
	void openDBRequest();
	void closeDBRequest();

	bool isNewMeasurement();
	bool hasAddress16();

	uint8_t			getID();
	uint16_t		getDB();
	uint16_t		getAddress16();
	uint32_t		getMSB();
	uint32_t		getLSB();
	XBeeAddress64 	getAddress64();

  private:
	bool 		openRequest;
	uint8_t 	nodeId;
	uint16_t 	nodeAddress16;
	uint16_t	db[DB_MEMORY];
	
	uint32_t 	lastDBReceptionTime, lastDBRequestTime, lastMessageTime;
	
	XBeeAddress64	nodeAddress64;
};

#endif

