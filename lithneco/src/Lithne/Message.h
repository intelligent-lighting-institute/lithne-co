/**
 * Message.h - The official Lithne library.
 *
 * Copyright (c) 2011-2012
 * Serge Offermans, Remco Magielse
 * {s.a.m.offermans, r.magielse}@tue.nl
 * Intelligent Lighting Institute (ILI), TU/e.
 *
 * All rights reserved. LAST UPDATE: 01-04-2012
**/

#ifndef Message_h
#define Message_h

#include "Node.h"
#include "LithneXBee.h"
#include "LithneDefinitions.h"


class Message
{
  public:

	Message();
	
	~Message();
	
	// const static uint8_t ERROR_MESSAGE			=	255;
	const static uint16_t NO_ARGUMENT			=	65535;
	// const static uint8_t NO_SCOPE             	=  	1;
	
	//Payload constants
	const static uint8_t SCOPE_MSB				=	0;
	const static uint8_t SCOPE_LSB				=	1;
	const static uint8_t FUNCTION_MSB			=	2;
	const static uint8_t FUNCTION_LSB			=	3;
	const static uint8_t MAXIMUM_PAYLOAD_BYTES 	=  	72;
	const static uint8_t MSG_HEADER_SIZE		= 	4;
	
	void clearArguments();

	void setRecipient64( XBeeAddress64 _addr64 );
	void setRecipient16( uint16_t _addr16 );
	// void setRecipient(   uint8_t _id );
	void setRecipient( Node * );
	void setFunction( 	 uint16_t _func );
	void setFunction( 	 String _func );
	void setSender64( 	 XBeeAddress64 _addr64 );
	void setSender16( 	 uint16_t _addr16 );

	void setSender( 	 uint16_t _addr16, XBeeAddress64 _addr64 );
	
	void setScope( 		 String _scope );
	void setScope( 		 uint16_t _scope );
	
	bool addArgument( 	 uint16_t _arg );
	bool addByte( 	 	 uint8_t _byte );
	bool setStringArgument( String _arg );
	

//	boolean addArg( uint32_t* arg); Discuss with Elco how to implement this
	boolean functionIs( String _func );
	
	uint8_t getNumberOfArguments();
	uint8_t getNumberOfBytes();
	uint8_t getPayloadSize();
	
	uint8_t * getPayload();
	uint8_t getPayloadByte( uint8_t _position );// Returns the byte at teh specified position in the total payload (including scope and function)
	uint8_t getByte( uint8_t _position );		// Returns the byte at the specified position in the data bytes
	uint16_t getArgument( uint8_t _position );	// Returns the integer from the arguments
	
	uint16_t getFunction();
	uint16_t getSender16();
	uint16_t getRecipient16();
	uint16_t getScope();
	uint16_t hash( String _group );
	
	String 	 getStringArgument();
	
	XBeeAddress64 getSender64( );
	XBeeAddress64 getRecipient64( );

  private:
  	uint8_t 		payloadSize		;	//Contains the number of bytes in the payload including the Scope (2 bytes) and function(1 byte)
	uint8_t 		payload[MAXIMUM_PAYLOAD_BYTES];
	uint16_t		sender16		;	//Contains the 16 bit address of the Sender
	uint16_t		recipient16		;	//Contains the 16 bit address of the Recipient
	XBeeAddress64	sender64		;	//Contains the 64 bit address of the Sender
	XBeeAddress64	recipient64 	;	//Contains the 64 bit address of the Recipient
		
};

#endif

