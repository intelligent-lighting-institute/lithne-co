/*
 * Lithne.h - The official Lithne library.
 *
 * Copyright (c) 2011
 * Serge Offermans, Remco Magielse
 * {s.a.m.offermans, r.magielse}@tue.nl
 * Intelligent Lighting Institute (ILI), TU/e.
 *
 * All rights reserved. LAST UPDATE: 23-11-2011
*/

/*! \mainpage The Lithne Library
 *	This Arduino Library is used to manage wireless communication using XBee modules in API mode. <br /> <br /> 
	<a href="functions_func.html">All available functions are documented here</a> 
 */

#ifndef Lithne_h
#define Lithne_h

//#include "Lithne_Config.h"

/* The following code makes the Library compatible with Arduino 1.0 */
#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
	#include "cppfix.h"
#endif

#include "LithneXBee.h"
#include "Message.h"
#include "Node.h"
#include "LithneDefinitions.h"

/*************************** MAX_NODES
Define the number of nodes that are created
- Note that no more than MAX_NODES can be added through the AddNode() function
Default is 20
**/
#ifndef MAX_NODES
	#define MAX_NODES 10
#endif
	
class LithneClass
{
  public:
  	/*	Constructor and destructor	*/
	LithneClass();
	~LithneClass();
	
	const static uint32_t 	COORDINATOR_MSB		=	0x0;
	const static uint32_t 	COORDINATOR_LSB		=	0x0;
	const static uint32_t 	BROADCAST_MSB		=	0x0;
	const static uint32_t 	BROADCAST_LSB		=	0xFFFF;

	const static uint16_t	F_PRINTLN			=	1004;	//Submit publicly accessible function numbers
	const static uint16_t	F_ERROR				=	774;	//Default error function
	const static uint8_t	F_FUNCTION_UNKNOWN	=	001;	//Too few arguments
	const static uint8_t	F_ARG_TOOFEW		=	002;	//Too few arguments
	const static uint8_t	F_ARG_TOOMANY		=	003;	//Too few arguments
	
	const static uint16_t	SEND_DELAY_BROADCAST=	1200;
	const static uint16_t	SEND_DELAY_UNICAST	=	30;

	const static uint16_t 	PROGRAMMING_SCOPE 	= 	1737;
	const static uint16_t 	NO_SCOPE 			= 	1;
	
	/*const static uint8_t	F_HELLOWORLD		=	201;	//Indicate your presence to the world
	const static uint8_t	F_METAINFO			=	202;	//Provide meta information, such as category and type 
	const static uint8_t	F_FUNCTIONLIST		=	203;	//Submit publicly accessible function numbers
	
	const static uint8_t	F_STATE				=	001;	//Control the state of the object
	const static uint8_t	F_INTENSITY			=	002;	//Control the intensity of the object
	const static uint8_t	F_CCT				=	003;	//Control the colour temperate of the object
	const static uint8_t	F_COLOR				=	004;	//Control the colour of the object
	const static uint8_t	F_LIGHTPARAMETERS	=	005;	//Control all light parameters of the object*/
	
	static Message incomingMessage;
	static Message outgoingMessage;
		
	/*	Functions that return nothing	*/
	void init( uint32_t _baud = 115200, HardwareSerial & port = Serial );
	void begin( uint32_t _baud = 115200, HardwareSerial & port = Serial );
	void setSerial(HardwareSerial &_port );
	void setRecipient( uint8_t _id );
	void toID( uint8_t _id );
	XBeeAddress64 toXBeeAddress( XBeeAddress64 _addr64 );
	void setRecipient( XBeeAddress64 _add64 );	//deprecated
	uint16_t toXBeeAddress( uint16_t _addr16 );
	void setRecipient16( uint16_t _add16 );		//deprecated

	void setFunction( uint16_t _function );
	void setFunction( String _function );
	void addArgument( uint16_t _arg );
	void setStringArgument( String _arg );
	void send( );
	void send( XBeeAddress64 recipient, uint8_t function );
	void send( uint8_t id, uint8_t function );
	bool sendMessage( Message * _mes );
	
	void println( String _stringArg );
	void println( uint8_t _recipId, String _stringArg );
	void println( XBeeAddress64 _add64, String _stringArg );
	void println( Node * _node, String _stringArg );
	
	void sendDBRequest( uint8_t _id );
	void sendDBRequest( XBeeAddress64 _add64 );
	void sendDBRequest16( uint16_t _addr16 );
	void sendDBRequest( XBeeAddress64 _add64, uint16_t _addr16 );

	void getMyInfo();
	void setScope( uint16_t _scope );
	void setScope( String _scope );
	void addScope( uint16_t _scope );
	void addScope( String _group );
	// void helloWorld( XBeeAddress64 _addr64 = XBeeAddress64(0x0, 0xFFFF), bool askReply = false );
	
	/*	Functions that return bits (booleans) 	*/
	bool available();
	bool dbAvailable();
	bool myInfoAvailable();
	bool xbeePacketAvailable();

	bool addNode( uint8_t _id, XBeeAddress64 _addr64 = XBeeAddress64(0x0, 0xFFFE), uint16_t _addr16 = UNKNOWN_16B );
	bool nodeKnown( uint8_t _id );
	bool nodeKnown64( XBeeAddress64 _add64 );
	bool nodeKnown16( uint16_t _addr16 );
	uint8_t newDBForNode( );
	bool newDBForNode( uint8_t _id );
	bool functionIs( String _func );
	bool hasScope( uint16_t _scope );
	bool removeScope( uint16_t _scope );
	bool removeScope( String _group );
	bool equals( XBeeAddress64 _addr1, XBeeAddress64 _addr2 );
	bool isFromNodeID( uint8_t _id );
	
	/*	Functions that return bytes (8-bit integers)	*/
	
	// The following functions allow you to do low level XBee Packet things, dont use them unless you know what you are doing - so far only used for CoProcessor firmware
	uint8_t getXBeePacketSize();
	uint8_t * getXBeePacket();							//
	void sendBytePublic( uint8_t b, bool escape );	// Actual sending of a byte, hardcore stuff =]
	
	uint8_t	digitalPin( uint8_t _position );
	uint8_t	pwmPin( uint8_t _position );
	uint8_t getNumberOfArguments();
	uint8_t getNumberOfNodes();
//	uint8_t getNodeId( XBeeAddress64 _add64 );							DEPRECATED getNodeBy64(_addr64)->getID();
//	uint8_t getNodeId( uint16_t _add16 );								DEPRECATED getNodeBy16(_addr16)->getID();
//	uint8_t setNodeAddress( XBeeAddress64 _add64, uint16_t _add16 );	DEPRECATED getNodeBy64(_addr64)->setAddress16(_addr16);
	
	/*	Functions that return standard integers (16-bit integers)	*/
	uint16_t getFunction();
	uint16_t getArgument( uint8_t _arg );
	uint16_t getDB( uint8_t _id );										// DEPRECATED getNodeByID(_id)->getDB();
//	uint16_t getNodeAddress16( XBeeAddress64 _add64 );					DEPRECATED getNodeBy64(_addr64)->getAddress16();
//	uint16_t getNodeAddress16( uint8_t _nodeId );						DEPRECATED getNodeByID(_id)->getAddress16();

	uint16_t getMyAddress16( bool forceCheck = false );
	uint16_t getMyPAN( bool forceCheck = false );
	uint16_t getMyAssociationStatus( bool forceCheck = false );
	uint16_t getScope();
	uint16_t hash( String _group );
	uint16_t getSender16();

	String getStringArgument();
	
	/*	Functions that return longs (32-bit integers)	*/
	uint32_t sendATCommand( uint8_t * cmd, uint16_t waitForResponse = 0 );
	
	/*	Functions that return Objects	*/
//	XBeeAddress64 getNodeAddress64( uint8_t _id );						DEPRECATED getNodeByID(_id)->getAddress64();
	XBeeAddress64 getSender64();
	XBeeAddress64 getMyAddress64( bool forceCheck = false );
	
	Message * getIncomingMessage( );
	Message * getOutgoingMessage( );

	bool messageDelivered( ); // Returns true if the sent message was delivered; false as long as it is not.
	
	/*	Node pointer retrieval	*/
	Node * getNode( uint8_t _position );
	Node * getNodeBy64( XBeeAddress64 _addr64 );
	Node * getNodeBy16( uint16_t _addr16 );
	Node * getNodeByID( uint8_t _nodeId );
  private:  	
  	//void setRecipient( Node * receivingNode );

  	void setMessageDelivered( bool s );	
	
  	/* These functions are only local to retrieve the position in the array */
  	uint16_t getNodeArrayPosition( uint8_t _nodeId );
	uint16_t getNodeArrayPosition( uint16_t _add16 );
	uint16_t getNodeArrayPosition( XBeeAddress64 _add64 );
	
	uint32_t _lastSend;
	
	void readXbee ();
	
	public:

	/* Define objects */
	static XBee xbee;
	static ZBRxResponse rx;
	static ZBTxRequest zbTx;
	static ZBTxStatusResponse txStatus;
	static ModemStatusResponse msr;
	static RemoteAtCommandResponse rATcmd;

//	Message * incomingMessage;
//	Message * outgoingMessage;
};

extern LithneClass Lithne;

#endif
