/*
 * Lithne.cpp - The official Lithne library.
 *
 * Copyright (c) 2011
 * Serge Offermans, Remco Magielse
 * {s.a.m.offermans, r.magielse}@tue.nl
 * Intelligent Lighting Institute (ILI), TU/e.
 *
 * All rights reserved. LAST UPDATE: 30-03-2012
 *
 *	TO DO:
 *	- Make integrated 'send' function, where all data is passed in the arguments
 *	- 16B DBrequest function
 *	-	Clean up setting the recipient
*/

#include "Lithne.h"

/* Include other libraries *
#include "XBee.h"
#include "Message.h"
#include "Node.h"
#include <stdlib.h>
#include "LithneDefinitions.h"
*/

#if defined(ARDUINO) && ARDUINO >= 100
	#define ARDUINO1DOT0 true
#else
	#include "cppfix.h"
	#define ARDUINO1DOT0 false
#endif

//#define UNKNOWN_NODE_ID 254	 //The standard ID for an unknown node

/*	Boolean	*/
bool newMessage		=	false;

/* uint8_t */
uint8_t DIGITAL[6]	=	{2, 4, 7, 8, 12, 13};	//defines the digital pins
uint8_t PWM[6]		=	{3, 5, 6, 9, 10, 11};	//defines the pwm pins
uint8_t atSH[]      =	{'S','H'};
uint8_t atSL[]      =	{'S','L'};
uint8_t atMY[]      =	{'M','Y'};
uint8_t atID[]      = 	{'I','D'};
uint8_t atAI[]      = 	{'A','I'};
uint8_t atDB[]		=	{'D','B'};

/*	uint16_t	*/
uint16_t scopes[MAX_SCOPES]	=	{0};	//Stores the HASH conversions of the groups the node belongs to
uint16_t numNodes			=	0;		//Contains the number of nodes in storage
uint16_t last16B			=	0xFFFE;	//Stores the last 16-bit address we send a message to.
uint16_t myAddress16		=	0xFFFE;	//Stores own 16-bit address.
uint16_t myPANid			=	0;		//Stores the PAN ID of the network
uint16_t myAssStat			=	0;		//Stores the association status of the node

/*	XBeeAddresses	*/
XBeeAddress64	last64B			=	XBeeAddress64( 0x0, 0xFFFF);	//Stores the last 64-bit address we send a message to.
XBeeAddress64	myAddress64		=	XBeeAddress64( 0x0, 0xFFFF);


/* Define objects */
XBee xbee 						=	XBee();
ZBRxResponse rx 				=	ZBRxResponse();
ZBTxRequest zbTx				= 	ZBTxRequest();
ZBTxStatusResponse txStatus 	=	ZBTxStatusResponse();
ModemStatusResponse msr 		=	ModemStatusResponse();
RemoteAtCommandResponse rATcmd	=	RemoteAtCommandResponse();

/* Create two pointers to standard messages we use */
Message * incomingMessage		=	new Message();
Message * outgoingMessage		=	new Message();

/* Create a pointer to an array of nodes with length MAX_NODES */
Node * nodes[MAX_NODES]			=	{0};

/* 
  ___ ___  _  _ ___ _____ ___ _   _  ___ _____ ___  ___  ___ 
 / __/ _ \| \| / __|_   _| _ \ | | |/ __|_   _/ _ \| _ \/ __|
| (_| (_) | .` \__ \ | | |   / |_| | (__  | || (_) |   /\__ \
 \___\___/|_|\_|___/ |_| |_|_\\___/ \___| |_| \___/|_|_\|___/

*/
/** 
	Default constructor called when importing the library. Used to initialize the nodes and scopes arrays and create incoming and outgoing message objects
**/
LithneClass::LithneClass()
{
	for( uint8_t i=0; i<MAX_NODES; i++ )
	{
		nodes[i]	=	0;		//Node();
	}
//	for( uint8_t i; i<MAX_SCOPES; i++ )
//	{
//		scopes[i]	=	0;
//	}

	incomingMessage = 	new Message();
	outgoingMessage = 	new Message();
//	numNodes 		= 	0;
//	newMessage 		= 	false;
	
//	myAddress64		=	XBeeAddress64( 0, UNKNOWN_64B );
//	myAddress16		=	UNKNOWN_16B;
//	myPANid			=	UNKNOWN_PAN_ID;
//	myAssStat		=	UNKNOWN_STATUS;
}

LithneClass::~LithneClass()
{
//	delete incomingMessage;
//	delete outgoingMessage;
	/*	There is little to no need to clean up here, 
		as we don't expect to ever dispose of this class.	*/
}

/* 
 ___ _   _ ___ _    ___ ___   ___ _   _ _  _  ___ _____ ___ ___  _  _ ___ 
| _ \ | | | _ ) |  |_ _/ __| | __| | | | \| |/ __|_   _|_ _/ _ \| \| / __|
|  _/ |_| | _ \ |__ | | (__  | _|| |_| | .` | (__  | |  | | (_) | .` \__ \
|_|  \___/|___/____|___\___| |_|  \___/|_|\_|\___| |_| |___\___/|_|\_|___/
                                                                          
*/

/** Specify the baud rate at which to communicate to the XBee **/
void LithneClass::init( uint32_t _baud, HardwareSerial &_port )
{
	xbee.setSerial( _port );
	xbee.begin( _baud );
}

/** Specify the baud rate at which to communicate to the XBee **/
void LithneClass::begin( uint32_t _baud, HardwareSerial &_port )
{
	xbee.setSerial( _port );
	xbee.begin( _baud );
}

/** Set the function to call in the outgoing message using a String. 
	This String is internally hashed by the Lithne.hash( String ) function,
	and stored in a two-byte integer.
	**/
void LithneClass::setFunction( String _function )
{
	outgoingMessage->setFunction( _function );
}

/** Set the function to call in the outgoing message **/
void LithneClass::setFunction( uint16_t _function )
{
	outgoingMessage->setFunction( _function );
}

/** Set the address of the receiving XBee by using the ID (not the same as array position in the Nodes array!) 
	The ID is supplied in the addNode function
**/
void LithneClass::setRecipient( uint8_t _id )
{
	/*	Retrieve the node the user refers to	*/
	Node * receivingNode	=	getNodeByID( _id );
	/*	From this node, get the 16-bit and 64-bit address and
		write it to the outgoing message	*/
	if (receivingNode != NULL)
	{
		outgoingMessage->setRecipient64( receivingNode->getAddress64() );
		outgoingMessage->setRecipient16( receivingNode->getAddress16() );
		// delete receivingNode;
	}
}
/** Set the address of the receiving XBee by using the full 64 bit address **/
void LithneClass::setRecipient( XBeeAddress64 _add64 )
{
	/*	Retrieve the node the user refers to	*/
	Node * receivingNode	=	getNodeBy64( _add64 );
	
	/*	From this node, get the 16-bit and 64-bit address and
		write it to the outgoing message	*/
	if (receivingNode != NULL)
	{
		outgoingMessage->setRecipient64( _add64 );
		outgoingMessage->setRecipient16( receivingNode->getAddress16() );
	}
}
/* Deprecated private function to set the recipient using a pointer to a node
void LithneClass::setRecipient( Node * _receivingNode )
{
	outgoingMessage->setRecipient64 ( _receivingNode->getAddress64() );
	outgoingMessage->setRecipient16 ( _receivingNode->getAddress16() );
}*/

/**	Sets the receipient of the outgoing message using the 16 bit address **/
void LithneClass::setRecipient16( uint16_t _add16 )
{	
/*	Retrieve the node the user refers to	*/
	Node * receivingNode	=	getNodeBy16( _add16 );
	
	/*	From this node, get the 16-bit and 64-bit address and
		write it to the outgoing message	*/
	if (receivingNode != NULL)
	{
		outgoingMessage->setRecipient64( receivingNode->getAddress64() );
		outgoingMessage->setRecipient16( _add16 );
	}
}

/** Add an argument to the outgoing message **/
void LithneClass::addArgument( uint16_t _arg )
{
	outgoingMessage->addArgument( _arg );
}

/** Set the String argument for the outgoingMessage. The string can be retrieved on the other end using getStringArgument()  **/ 
void LithneClass::setStringArgument( String _arg )
{
	outgoingMessage->setStringArgument( _arg );
}

/** Sends a string to the specified node **/
void LithneClass::println( String _stringArg )
{
	/* By default we send to the coordinator */
	println( XBeeAddress64(0x00000000, 0x00000000), _stringArg);
}
/** Sends a string to the specified node **/
void LithneClass::println( uint8_t _id, String _stringArg )
{
	/*	Retrieve the node the user refers to	*/
	Node * receivingNode	=	getNodeByID( _id );
	if (receivingNode != NULL)
	{
		println(receivingNode, _stringArg);
	}
}
/** Set the address of the receiving XBee by using the full 64 bit address **/
void LithneClass::println( XBeeAddress64 _add64, String _stringArg )
{
	/*	Retrieve the node the user refers to	*/
	Node * receivingNode	=	getNodeBy64( _add64 );
	if (receivingNode != NULL)
	{
		println(receivingNode, _stringArg);
	}
	else
	{
		Node * tempNode = new Node(UNKNOWN_NODE_ID, _add64);
		println(tempNode, _stringArg);
	}
}

/** Sends a string to the specified node **/
void LithneClass::println( Node * _node, String _stringArg )
{	
	/*	From this node, get the 16-bit and 64-bit address and
		write it to the outgoing message	*/
	
	if (_node != NULL)
	{
		outgoingMessage->setRecipient64( _node->getAddress64() );
		outgoingMessage->setRecipient16( _node->getAddress16() );
		outgoingMessage->setFunction( F_PRINTLN );
		outgoingMessage->setScope( NO_SCOPE );
		outgoingMessage->setStringArgument( _stringArg );
		send();
	}
}


/** Transmit the outgoing message, use setRecipient and setFunction before calling this function  **/
void LithneClass::send( )
{
	sendMessage( outgoingMessage );
	/* After transmitting the message, we immediately clear the arguments */
	outgoingMessage->clearArguments();
	/*	After transmitting the message we set the scope to NO_SCOPE, so 
		our next message is always received by any other node (unless we specify
		a scope again)	*/
	outgoingMessage->setScope( NO_SCOPE );
}
/**	Transmit outgoing message with the specified recipient by 64 bit address and function **/
void LithneClass::send( XBeeAddress64 recipient, uint8_t function )
{
	setRecipient( recipient );
	setFunction( function );
	send();
}
/**	Transmit outgoing message with the specified recipient ID and function **/
void LithneClass::send( uint8_t _id, uint8_t _function )
{
	setRecipient( _id );
	setFunction( _function );
	send();
}

/** EXPERT FUNCTION - DON'T CALL UNLESS YOU KNOW WHAT YOU ARE DOING!
	Transmits the message in the argument and consequently read the	response from the XBee.	
	Message * message: Message to be transmitted.
**/
void LithneClass::sendMessage( Message * message )
{
	/* Place the total payload in the local payload array
	uint8_t payload[message->getPayloadSize()];
	for (int i = 0; i < sizeof(payload); i++)
	{
		payload[i] = message->getPayloadByte(i);
	}*/

  	/* Retrieve the 64-bit address from the message*/
	XBeeAddress64 addr64 	=	message->getRecipient64();
	uint16_t	  addr16	=	message->getRecipient16();

	/*	We always supply the 64-bit and 16-bit address. In case the
		16-bit address is NOT known, the ZBTxRequest will automatically
		use the 64-bit address.
	*/
	zbTx = ZBTxRequest( addr64, addr16, 0, 0, message->getPayload(), message->getPayloadSize(), 0x01 );
	// zbTx = ZBTxRequest( addr64, addr16, 0, 0, payload, sizeof(payload), 0x01 );
	
	bool sentTooFast = false;
	if ( addr64.getMsb() == 0x0 && 
		 addr64.getLsb() == 0xFFFF )
	{
		zbTx.setFrameId(NO_RESPONSE_FRAME_ID); //disable TX_STATUS_RESPONSE for Broadcast which speeds up the transmission rate
		if (abs( millis() - _lastSend) < SEND_DELAY_BROADCAST)
		{
			sentTooFast = true;
		}
	}
	else
	{
		if (abs( millis() - _lastSend) < SEND_DELAY_UNICAST)
		{
			sentTooFast = true;
		}
	}
	/* We check if the user is not transmitting too fast and flooding the network */
	if ( !sentTooFast )
	{
		/* The complete ZBTxRequest is transmitted */
		_lastSend = millis();
		xbee.send(zbTx);
	}
	
	/* Store the addresses we transmit to */	
	last16B	=	addr16;
	last64B	=	addr64;
	
	/* After transmitting, read any possible status response */
	readXbee();
}

/** Transmit a DB request (RSSI, signal strength) by using the node identifier **/
void LithneClass::sendDBRequest( uint8_t _id )
{
	Node * receiverNode = getNodeByID( _id );
	
	if (receiverNode != NULL)
	{
		sendDBRequest( receiverNode->getAddress64(), receiverNode->getAddress16() );
	}
	// delete receiverNode;
	/*	This code has been changed, but not tested!
 	uint16_t _addr16 		=	receivingNode->getAddress16();
 	
 	if ( _addr16 != UNKNOWN_NODE ) 
	{
		sendDBRequest16( _addr16 );
	}
	// the 16-bit address is not known, so we send a 64B request 
	else 
 	{
 		sendDBRequest( receivingNode->getAddress64() );
 	}
 	*/
}
/** Transmit a DB request (RSSI, signal strength) by using the 64-bit address **/
void LithneClass::sendDBRequest( XBeeAddress64 _addr64 )
{

	 sendDBRequest( _addr64, UNKNOWN_16B );
	
	/* !!!This code has been changed, but not tested	*
	Node * receivingNode	=	getNodeBy64( _addr64 );
	uint16_t _addr16 		=	receivingNode->getAddress16();
	
 	if ( _addr16 != UNKNOWN_NODE ) 
	{
		sendDBRequest16( _addr16 );
	}
	// The 16B address is not known, so we send a 64B request 
	else 
	{
		RemoteAtCommandRequest db = RemoteAtCommandRequest( addr64, atID );
    	// Send your request
		xbee.send(db);
		
		uint8_t remoteNode = getNodeId( addr64 );

		/*	If we broadcast the DB request, open the node request on
			all nodes	*
    	if ( _addr64.getMsb() == 0 && _addr64.getLsb() == 0xFFFF )
    	{
			for (int i =0; i< numNodes; i++)
			{
				nodes[i]->openDBRequest();
    		}
    	}
    	/*	Otherwise, just open the request on the node we
    		transmit the DB request to	*
    	else
    	{
			for (int i =0; i< numNodes; i++)
	 		{
				if (remoteNode == nodes[i]->getID())
				{
			  		nodes[i]->openDBRequest();
				}
    		}
    	}
    }
    */
}
/** Transmit a DB request (RSSI, signal strength) by using the 16-bit address **/
void LithneClass::sendDBRequest16( uint16_t _addr16 )
{
	sendDBRequest( XBeeAddress64(0x0,0x0), _addr16 );
	/*	!!! Code has been changed, but not tested!
	RemoteAtCommandRequest db = RemoteAtCommandRequest( _addr16, atDB );
    // Send your request
    xbee.send(db);

	if( nodeKnown( _addr16 ) )
	{
    	uint8_t remoteNode = getNodeId( _addr16 );

		for (int i =0; i< numNodes; i++)
	 	{
			if (remoteNode == nodes[i]->getID())
			{
		  		nodes[i]->openDBRequest();
			}
   		}
	}
	*/
}
/*	Send a DB request to a specific node. This is the new function all the
	other request forward their code to. **/
void LithneClass::sendDBRequest( XBeeAddress64 _addr64, uint16_t _addr16 )
{	
	
	/* Serial.print("Opening DB req. on node ");
	Serial.print(getNodeBy64( _addr64 )->getID());
	Serial.print(" from 16 bit add ");
	Serial.print( _addr16 );
	Serial.print(" and 64 bit add ");
	Serial.print( _addr64.getMsb() , HEX );
	Serial.print( ", ");
	Serial.println( _addr64.getLsb() , HEX );*/
	
	RemoteAtCommandRequest db;
	/* If we know the 16 bit address, we transmit to that */
	if( _addr16 != UNKNOWN_16B )
	{
		db = RemoteAtCommandRequest( _addr16, atDB );
		// If the 64 bit add is not a broadcast, we set that add as well.
		if ( _addr64.getMsb() != 0x0 || 
			 _addr64.getLsb() != 0xFFFF )
		{
				db.setRemoteAddress64( _addr64 );
		}
		
		/*	Open the request on the node we	transmit the DB request to	*/
		if (nodeKnown16( _addr16 ) )
		{
			getNodeBy16( _addr16 )->openDBRequest();
		}
	}
	
	/* If we don't know the 16 bit address, we use the 64 bit, at least if we know it */
	else // if ( _addr64 != NULL ) // unfortunately, we can't put NULL as a 64 bit address
	{
		db = RemoteAtCommandRequest( _addr64, atDB );
		
		/*	If we broadcast the DB request, open the node request on all nodes	*/
		if ( _addr64.getMsb() == 0x0 && 
			 _addr64.getLsb() == 0xFFFF )
		{
			for(uint16_t i=0; i< numNodes; i++)
			{
				nodes[i]->openDBRequest();
			}
		}
		/*	Otherwise, we open the request on the node we	transmit the DB request to	*/
		else 
		{
			Node * receiverNode = getNodeBy64( _addr64 );
			if (receiverNode != NULL)
			{
				receiverNode->openDBRequest();
				/* Serial.print("Found node by 64bit add to send DB request to; node ID: ");
				Serial.println( receiverNode->getID() ); */
			}
		}
		/*Serial.print("Opened DB req. on node ");
		Serial.print(getNodeBy64( _addr64 )->getID());
		Serial.print(" from 64 bit add ");
		Serial.println( _addr64.getLsb() ); */
	}
    /*	Transmit the request	*/
	/*Serial.print("Transmitting DB Request:");
	for (int i = 0; i < db.getFrameDataLength (); i++)
	{
		Serial.print(db.getFrameData(i),HEX);
		Serial.print(" ");
	}
	Serial.println();*/
    xbee.send( db );
}

/**	Collect information of the XBee connected to the local node.
	However, this information is only written when the XBee is read.**/
void LithneClass::getMyInfo()
{
	sendATCommand( atSH );
	sendATCommand( atSL );
	sendATCommand( atMY );
	sendATCommand( atID);
	sendATCommand( atAI );
}

/**	Sets the hash code of the group (scope) to the outgoing message	**/
void LithneClass::setScope( uint16_t _scope )
{
	outgoingMessage->setScope( _scope );
}
/** Sets the hash code of the group (scope) to the outgoing message using a string **/
void LithneClass::setScope( String _scope )
{
	setScope( hash( _scope ) );
}

/**	Add the	new scope to the existing array	**/
void LithneClass::addScope( uint16_t _scope )
{
	/*	Go through all the scopes, until we find an empty location, 
		write the scope there	*/
	for( int i=0; i<MAX_SCOPES; i++ )
	{
		if( scopes[i]	==	0)
		{
			scopes[i]	=	_scope;
			break;
		}
	}
}

/**	Creates a hash code of the specified string and adds this to the internal hash register.
	arg: String with maximum 20 characters	**/
void LithneClass::addScope( String _group )
{
	addScope( hash(_group) );
}

/**	Broadcasts a hello world command and either requests a reply.
	If a reply is requested, a node will reply with hello world,
	but should NOT ask for a reply (otherwise you end up in an infinite
	loop).
**/
void LithneClass::helloWorld( XBeeAddress64 _addr64, bool askReply )
{
	setRecipient( _addr64 );
	setFunction ( HELLO_WORLD );
	addArgument(  getMyAddress16() );
	addArgument(  askReply );
	send();
}

/*
 ___  ___   ___  _    ___   _   _  _ 
| _ )/ _ \ / _ \| |  | __| /_\ | \| |
| _ \ (_) | (_) | |__| _| / _ \| .` |
|___/\___/ \___/|____|___/_/ \_\_|\_|

*/

/** Check whether new data is available. If so, raise newMessage flag **/
bool LithneClass::available()
{
	readXbee();

	if( newMessage )
	{
		newMessage = false; // reset flag
		return true;
	}
	else
	{
		return false;
	}

}

/** Add a new node to the memory, giving it a specific ID (0-255)
	If succesful, return true, otherwise return false.	**/
bool LithneClass::addNode( uint8_t _id, XBeeAddress64 _addr64, uint16_t _addr16 )
{
	/*	If we have not passed the maximum number of nodes	*/
	if( numNodes < MAX_NODES && 
		!nodeKnown( _id ) && 
		!nodeKnown64( _addr64)	)
	{
		nodes[numNodes] = new Node( _id, _addr64, _addr16 );
		numNodes++;
		return true;	//Return true, because we have succesfully added a node
	}
	else
	{
		return false;	//If we can't add a new node, return false
	}
}

/**	Checks whether there is a node with the specified ID in the memory	**/
bool LithneClass::nodeKnown( uint8_t _id )
{
	/*	getNodeByID returns the node with the specific identifier, or a new
		node with unknown data (id and address). If this is the case, the
		node is thus not known. Otherwise, it is known	*/
	if( getNodeByID( _id ) == NULL )
	{
		return false;
	}
	else
	{
		return true;
	}
}
/** Checks whether the node with the specified 64-bit address exists in the nodes[]. **/
bool LithneClass::nodeKnown64( XBeeAddress64 _addr64 )
{
	if( getNodeBy64( _addr64 ) == NULL )
	{
		return false;
	}
	else
	{
		return true;
	}
	
	/*	!!! CODE REPLACED, UNTESTED
	for( int i=0; i<numNodes; i++)
	{
	/** For each of the know nodes, check if the MSB and LSB
		are similar to the specified address in add64. **=
		if( _addr64.getMsb() == nodes[i]->getMSB() 
		&&  _addr64.getLsb() == nodes[i]->getLSB() )
		{
			return true;
			break;
		}
	}
	
	return false;
	*/
}
/** Checks whether the node with the specified 16-bit address exists in the nodes[]. **/
bool LithneClass::nodeKnown16( uint16_t _addr16 )
{
	if( getNodeBy16( _addr16 ) == NULL )
	{
		return false;
	}
	else
	{
		return true;
	}
	
	/*	!!! CODE REPLACED, UNTESTED
	bool known	=	false;
		
	for( int i=0; i<numNodes; i++)
	{
		Node * curNode	=	getNode( i );
		
		if( curNode->getAddress16() == _addr16 )
		{
		/** For each of the know nodes, check if the 16-bit address is 
			similar to the specified address in _addr16. **
			known	=	true;
			break;
		}
	}
	
	return known;
	*/
}

/** Check if a new DB measurement is available for a particular node (identified by Node ID) **/
bool LithneClass::newDBMeasurement( uint8_t _id )
{
	bool newMeasure = false;	//reset the flag
	Node * currentNode	= getNodeByID( _id );
	
	if (currentNode != NULL)
	{
		if( currentNode->isNewMeasurement() )
		{
			/*
			Serial.print("NEW BD INCOMING on node : ");
			Serial.println(_id);*/
			currentNode->closeDBRequest();
			newMeasure = true;
		}
	}
	// delete currentNode;
	return newMeasure;
}
/**	Checks whether the provided hash scope is included
	in the known scopes.	**/
bool LithneClass::hasScope( uint16_t _scope )
{
	if( _scope == NO_SCOPE )
	{
		return true;
	}
	for(int i = 0; i < MAX_SCOPES; i++)
    {
		if( scopes[i]	== _scope)
		{
			return true;
		}
/*		this is not neccesarily valid, because we might also
		remove scopes in the center of the array and then this
		function will never reach the later scopes
		else if (scopes[i] == 0)
		{
			break;
		}
		*/
	}
	return false;
}

/**	Removes the group from a specific scope	**/
bool LithneClass::removeScope( uint16_t _scope )
{
	for(int i = 0; i < MAX_SCOPES; i++)
    {
    	if( scopes[i]	==	_scope )
    	{
    		scopes[i]	=	0;
    		return true;
    	}
    }
    return false;
}
/**	Removes the group from a specific scope	**/
bool LithneClass::removeScope( String _group )
{
	return removeScope( hash(_group) );
}




/*
 ___ _  _ _____ ___ ___ ___ ___ 
|_ _| \| |_   _| __/ __| __| _ \
 | || .` | | | | _| (_ | _||   /
|___|_|\_| |_| |___\___|___|_|_\
                                
*/

/** Returns the pin number of the digital out pins. 
	This makes it easy to refer	to the digital output 
	pins as 0-6, as	they are mapped on the Lithne node **/
uint8_t	LithneClass::digitalPin( uint8_t position )
{
	position = constrain( position, 0, 5 );
	return DIGITAL[position];
}

/** Returns the pin number of the pwm (DAC) out pins.
	This makes it easy to refer	to the digital output 
	pins as 0-6, as	they are mapped on the Lithne node **/
uint8_t	LithneClass::pwmPin( uint8_t position )
{
	position = constrain( position, 0, 5 );
	return PWM[position];
}

/** Returns a pointer to the incoming message **/
Message * LithneClass::getIncomingMessage()
{
	return incomingMessage;
}
/** Returns a pointer to the incoming message **/
Message * LithneClass::getOutgoingMessage()
{
	return outgoingMessage;
}

/** Returns the number of arguments in the incoming message **/
uint8_t LithneClass::getNumberOfArguments()
{
	return incomingMessage->getNumberOfArguments();
}
/**  functionIs returns true if the function of the incoming message is the same as the function that is provided as an argument to this function.
	Internally, this function compares a hash() of the provided argument with the internally stored function ID inetger.
*/
bool LithneClass::functionIs( String _func )
{
	return incomingMessage->functionIs( _func );
}

/** Returns the number of nodes currently known **/
uint8_t LithneClass::getNumberOfNodes()
{
	return numNodes;
}

/* DEPRECATED. Use getNodeBy16( _addr16 )->getID() instead.
Return the ID number of the node, based on the 16-bit address **
uint8_t LithneClass::getNodeId( uint16_t _addr16 )
{
	return getNodeBy16( _addr16 )->getID();
	/* !!! CODE REVISED, UNTESTED
	uint16_t id = UNKNOWN_NODE_ID;
	uint8_t pos	=	0;
	
	for (int i = 0; i < numNodes; i++)
	{
		if( _addr16 == nodes[i]->getAddress16() )
      	{
			id = nodes[i]->getID();
			pos	=	i;
		}
	}
	return id;
}
*/

/* DEPRECATED. Use getNodeBy64( _addr64 )->getID() instead
Return the ID number of the node, based on the 64-bit address **
uint8_t LithneClass::getNodeId( XBeeAddress64 _addr64 )
{
	return getNodeBy64( _addr64 )->getID();
	/*	!!! CODE REVISED, UNTESTED
	uint16_t id = UNKNOWN_NODE_ID;
	uint8_t	pos	=	0;
	
	for (int i = 0; i < numNodes; i++)
	{
		if( addr64.getMsb() == nodes[i]->getAddress64().getMsb() 
		 && addr64.getLsb() == nodes[i]->getAddress64().getLsb() )
      	{
			id = nodes[i]->getID();
			pos	=	i;
		}
	}
	
	return id;
}
*/

/*	FUNCTION HAS BECOME OBSOLETE...NO LONGER IN USE (DEPRECATED)
	This function looks up the specified 64-bit address and if neccesary 
	adds the 16-bit address to this node.
	RETURNS: the position of the node in the array

uint8_t LithneClass::setNodeAddress( XBeeAddress64 _add64, uint16_t _add16 )
{
	uint8_t nodePositionInArray	=	UNKNOWN_NODE;
	
	/*	If the address least significant byte is 0, this is a broadcast message.
		Then we shouldn't overwrite the 16-bit address *
		
		/* TO DO: Compare the _add64 with a broadcast address instead of number *
	if( _add64.getMsb() != 0 )
	{
		for( uint8_t i=0; i<numNodes; i++)
		{
			if( _add64.getMsb() == nodes[i]->getAddress64().getMsb() 
			 && _add64.getLsb() == nodes[i]->getAddress64().getLsb() )
			 {
			 	nodePositionInArray	=	i;
			 	nodes[i]->setAddress16( _add16 );
			 	break;
			 }
		}
	}
	
	return nodePositionInArray;
}
*/

/** Returns the function ID of the incoming message **/
uint16_t LithneClass::getFunction()
{
	return incomingMessage->getFunction();
}

/**	Return the scope of the incoming message. The scope is an integer used to define for whom the message is intended **/
uint16_t LithneClass::getScope()
{
	return incomingMessage->getScope();
}

/** Return the int argument at the specified position **/
uint16_t LithneClass::getArgument( uint8_t arg )
{
	return incomingMessage->getArgument(arg);
}

/** Returns the content of the incoming message as a String. 
	This is useful if you used setStringArgument("content") when sending the message
**/
String LithneClass::getStringArgument( )
{
	return incomingMessage->getStringArgument();
}

/* 	DEPRECATED. Retrieve the address by using getNode()
Returns the 16-bit address based on the 64-bit address 
*
uint16_t LithneClass::getNodeAddress16( XBeeAddress64 _add64 )
{
	uint16_t _add16 = UNKNOWN_NODE;
	
	for(uint8_t i = 0; i < numNodes; i++)
	{
		if ( _add64.getMsb() == nodes[i]->getAddress64().getMsb() 
		&& 	 _add64.getLsb() == nodes[i]->getAddress64().getLsb() )
      	{
			_add16 = nodes[i]->getAddress16();
			break;
		}
	}
	
	return _add16;
}
*/

/* 	DEPRECATED. Retrieve the address by using getNodeByID()
Returns the 16-bit address based on the nodeId  *
uint16_t LithneClass::getNodeAddress16( uint8_t _nodeId )
{
	uint16_t add16 = UNKNOWN_NODE;
	
	for (int i = 0; i < numNodes; i++)
	{
		if ( _nodeId	== nodes[i]->getID() )
      	{
			add16 = nodes[i]->getAddress16();
		}
	}
	
	return add16;
}
*/

/** Return the DB measure for the specified node **/
uint16_t LithneClass::getDB( uint8_t id )
{
	Node * refNode = getNodeByID(id);
	uint16_t dbStrength = 0;
	
	if (refNode != NULL) 
	{
		dbStrength = refNode->getDB();
		
		/*Serial.print("Getting DB for nodeId ");
		Serial.print(id);
		Serial.print("(getId:  ");
		Serial.print(refNode->getID());
		Serial.print("). found value ");
		Serial.println(dbStrength); */
	}
	return dbStrength;
}

/**	Returns the 16-bit address of the own node	**/
uint16_t LithneClass::getMyAddress16( bool forceCheck )
{
	if( myAddress16	==	UNKNOWN_16B || forceCheck )
	{	//We request the 16-bit address and wait 1 second for a response
		myAddress16	=	sendATCommand( atMY, 1000 ) & 0xFFFF;
	}
	
	return myAddress16;
}

/**	Returns the PAN ID of the xbee network	**/
uint16_t LithneClass::getMyPAN( bool forceCheck )
{
	if( myPANid	==	UNKNOWN_PAN_ID || forceCheck )
	{	//We request the PAN ID and wait 1 second for a respons
		myPANid	=	sendATCommand( atID, 1000 ) & 0xFFFF;
	}
	
	return myPANid;
}

/**	Returns the association status of the XBee	**/
uint16_t LithneClass::getMyAssociationStatus( bool forceCheck )
{
	if( myAssStat == UNKNOWN_STATUS || forceCheck )
	{	//We request the association status and wait 1 second for a response
		myAssStat	=	sendATCommand( atAI, 1000 ) & 0xFFFF;
	}
	return myAssStat;
}

/**	Returns a hash code based on the name of a group **/
/* -- This function is copied into the Message class; in case of alternations; do it in both classes */
uint16_t LithneClass::hash( String _group )
{
	uint16_t wordValue  =  0;
	for(uint16_t i=0; i < _group.length(); i++ )
	{
		wordValue  +=  _group.charAt(i);
	}
	wordValue += _group.length();
	wordValue += _group.charAt(0);
	wordValue += _group.charAt( _group.length() - 1 );
		
	return wordValue;
}

/**	Returns the 16-bit address of the sender as stored in the message	**/
uint16_t LithneClass::getSender16()
{
	return incomingMessage->getSender16();
}

/**	Sends an ATCommand and returns the response as a 32-bit integer.
	If you specify a value of waitForResponse, the program will wait
	for the amount of ms specified by waitForResponse and immediately
	process the AT response. If you don't specify a value or specify 0
	the program will call the general readXbee() and process the response
	there and write it to the corresponding variables.
	This function will then return 0.	**/
uint32_t LithneClass::sendATCommand( uint8_t * cmd, uint16_t waitForResponse )
{
	uint32_t	atAnswer	=	0;
	
	/*	Create a new AtComand Request and Response	*/
	AtCommandRequest atRequest   	= AtCommandRequest();

	/*	Set the specific command you want to send to the XBee in the atRequest	*/
	atRequest.setCommand( cmd );
	
	/*	Send this request to the xbee	*/
	xbee.send( atRequest );
  
	/*	The processing of the AT response is handled in the readXBee().
		We thus call this function. If we manually tell this function to
		wait for a response, it will skip this and attempt to read the
		information at this point. */
	if( waitForResponse == 0 )
	{
		readXbee();
	}
	else
	{
		if ( xbee.readPacket(waitForResponse) )
		{
			AtCommandResponse atResponse 	= AtCommandResponse();
			
			/*	If the packet is indeed an AT_COMMAND_RESPONSE,	we write it to the atResponse.	*/
			if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) 
			{
				xbee.getResponse().getAtCommandResponse(atResponse);
				/*	If the atResponse is correct (does not return an error code), we can process it.	*/
				if (atResponse.isOk()) 
				{
					if (atResponse.getValueLength() > 0) 
					{  
						for (int i = 0; i < atResponse.getValueLength(); i++) 
						{
							atAnswer	=	(atAnswer << 8) + atResponse.getValue()[i];
						}
        			}
				}
			}
  		} 
		else 
  		{
	    // at command failed
    	if (xbee.getResponse().isError()) 
	    {
//      	Serial.print("Error reading packet.  Error code: ");  
//  	    Serial.println(xbee.getResponse().getErrorCode());
	    } 
    	else 
	    {
//      	Serial.print("No response from radio");  
    	} 
  	}
  }

  return atAnswer;
  
}

/* 
__  _____            _      _    _                 __ _ _  
\ \/ / _ ) ___ ___  /_\  __| |__| |_ _ ___ ______ / /| | | 
 >  <| _ \/ -_) -_)/ _ \/ _` / _` | '_/ -_|_-<_-</ _ \_  _|
/_/\_\___/\___\___/_/ \_\__,_\__,_|_| \___/__/__/\___/ |_| 

*/

/** Return the address of the sender of the message **/
XBeeAddress64 LithneClass::getSender64()
{
	return incomingMessage->getSender64();
}

/* DEPRECATED. Use getNodeByID(id)->getAddress64() instead.
Return the node address of the specified ID **
XBeeAddress64 LithneClass::getNodeAddress64( uint8_t id )
{	//Create a new XBeeAddress64
	XBeeAddress64 recip = XBeeAddress64( 0, 0 );

	for (int i = 0; i < numNodes; i++)
	{
		if ( id == nodes[i]->getID() )
		{
			recip = nodes[i]->getAddress64();
		}
		else
		{

		}
	}
	return recip;
}
*/

/**	Returns the own 64-bit Serial Address of the XBee connected	**/
XBeeAddress64 LithneClass::getMyAddress64( bool forceCheck )
{
	/*	If the address is unknown, or if we force a recheck.	*/
	if( myAddress64.getLsb()	==	UNKNOWN_64B || forceCheck )
	{
		uint32_t msb	=	sendATCommand( atSH, 1000 );	//We want to wait for an answer
		uint32_t lsb	=	sendATCommand( atSL, 1000 );	//We want to wait for an answer
		myAddress64		=	XBeeAddress64( msb, lsb );
	}
	
	return myAddress64;
}
/*
  ___  ___    _ ___ ___ _____ ___ 
 / _ \| _ )_ | | __/ __|_   _/ __|
| (_) | _ \ || | _| (__  | | \__ \
 \___/|___/\__/|___\___| |_| |___/

 */
 
/**	Returns the node at the specific location in
	the node array.
	Argument: the location in the node array
**/
Node * LithneClass::getNode( uint8_t position )
{
	if( position < MAX_NODES && position < numNodes )
	{
		return nodes[position];
	}
	
	return NULL;
}


/**	Searches the node array for the node with specified 64-bit address. 
	If this node ID exists we return a pointer to that node, 
	otherwise we return a new node with an unkown node id	**/
Node * LithneClass::getNodeBy64( XBeeAddress64 _addr64 )
{
	for(uint16_t i=0; i<numNodes; i++ )
	{
		Node * curNode	=	getNode( i );
		if( curNode->getAddress64().getMsb() == _addr64.getMsb() &&
			curNode->getAddress64().getLsb() == _addr64.getLsb() )
		{
			// delete curNode;
			return nodes[i];
		}
		// delete curNode;
	}
	return NULL;
}

/**	Searches the node array for the node with specified 16-bit address. 
	If this node ID exists we return a pointer to that node, 
	otherwise we return a new node with an unkown node id	**/
Node * LithneClass::getNodeBy16( uint16_t _addr16 )
{
	for( uint16_t i=0; i<numNodes; i++ )
	{
		Node * curNode	=	getNode( i );
		
		if( curNode->getAddress16() == _addr16 )
		{
			return nodes[i];
		}
	}
	return NULL;
}

/**	Searches the node array for the node specified node ID. 
	If this node ID exists we return a pointer to that node, 
	otherwise we return a new node with an unkown node id	**/
Node * LithneClass::getNodeByID( uint8_t _id )
{
	for( uint16_t i=0; i<numNodes; i++ )
	{
		Node * curNode	=	getNode( i );
		if( curNode->getID() == _id )
		{
			// delete curNode;
			return curNode;
		}
		// delete curNode;
	}
	return NULL;
}

/*
 ___ ___ _____   ___ _____ ___   ___ _   _ _  _  ___ _____ ___ ___  _  _ ___ 
| _ \ _ \_ _\ \ / /_\_   _| __| | __| | | | \| |/ __|_   _|_ _/ _ \| \| / __|
|  _/   /| | \ V / _ \| | | _|  | _|| |_| | .` | (__  | |  | | (_) | .` \__ \
|_| |_|_\___| \_/_/ \_\_| |___| |_|  \___/|_|\_|\___| |_| |___\___/|_|\_|___/

*/

/** Look up the position of the node in the array, based on the ID **/
uint16_t LithneClass::getNodeArrayPosition( uint8_t _nodeId )
{
	for( uint8_t i=0; i<numNodes; i++)
	{
		if( getNode(i)->getID() == _nodeId )
		{
			return i;
		}
	}
	return UNKNOWN_NODE_ID;
}

/** Look up the position of the node in the array, based on the 64-bit address **/
uint16_t LithneClass::getNodeArrayPosition( uint16_t _addr16 )
{
	for( uint8_t i=0; i<numNodes; i++)
	{
		if( getNodeBy16( i )->getAddress16() == _addr16 )
		{
			return i;
		}
	}
	return UNKNOWN_NODE_ID;
}

/** Look up the position of the node in the array, based on the 64-bit address **/
uint16_t LithneClass::getNodeArrayPosition( XBeeAddress64 _addr64 )
{
	for( uint8_t i=0; i<numNodes; i++)
	{
		if( getNode( i )->getAddress64().getMsb() == _addr64.getMsb() &&
			getNode( i )->getAddress64().getLsb() == _addr64.getLsb() )
		{
			return i;
		}
	}
	return UNKNOWN_NODE_ID;
}

/** This function takes care of all processing when a message is received.
	It is called after Lithne.available();
	This section is based on examples in the XBee library by Andrew Rapp**/
void LithneClass::readXbee()
{
/*DONT EDIT IF YOU DON'T KNOW WHAT YOU'RE DOING!!!	*/

/* Reads all available serial bytes until a packet is parsed, an error occurs, or the buffer is empty. */
xbee.readPacket();  

if (xbee.getResponse().isAvailable()) //Returns a reference to the current response Note: once readPacket is called again this response will be overwritten!
{
	// Serial.println(" XBee Pack available " );
	int responseType	=	xbee.getResponse().getApiId();
	
  	if (responseType == ZB_RX_RESPONSE)
    { //Call with instance of ZBRxResponse class only if getApiId() == ZB_RX_RESPONSE to populate response.

     	/* Indicate we have received a new message */
     	newMessage = true; // set flag
     	
     	/* Clear the old received message */
     	incomingMessage->clearArguments();
     	
     	xbee.getResponse().getZBRxResponse(rx);
     	
     	/* Retrieve the sender from the packet and store it in the message */
     	XBeeAddress64	addr64	=	rx.getRemoteAddress64();
     	uint16_t		addr16	=	rx.getRemoteAddress16();

		/*  if the 16 bit add shows this is the coordinator, we store the default
			coordinator address 0x0 0x0 instead of the hardware address       */
		if (addr16 == 0) 
		{
			addr64 = XBeeAddress64(0x0, 0x0);
		}
     	
     	incomingMessage->setSender( addr16, addr64 );
     	 
     	/*	The scope of the message is stored in the first two bytes
     		of the payload.	Here we retrieve this and write it to the
     		incoming message	*/
     	incomingMessage->setScope( (rx.getData(Message::SCOPE_MSB) << 8) + rx.getData(Message::SCOPE_LSB) );
     	
		/* 	The function identifier (1 byte) is stored in the third
			byte of the payload. Here we retrieve this and write it to
			the incoming message	*/
      	incomingMessage->setFunction((rx.getData(Message::FUNCTION_MSB) << 8) + rx.getData(Message::FUNCTION_LSB) );

      	/*	The remainder of the payload contains our arguments. Here
      		we retrieve the number of arguments, by subtracting the 
      		first three bytes (which contain the scope and the function
      		ID) and dividing the number of bytes by two, since we send
      		16-bit values and not 8-bit values (2x 8-bits = 1x 16bits)	*/
      	uint16_t numOfBytes	=	(rx.getDataLength()-Message::MSG_HEADER_SIZE);
		
		/* Store the arguments in the incomingMessage */
    	for( uint16_t i = 0; i < numOfBytes; i++ )
      	{
    		uint16_t pos    =	i + Message::MSG_HEADER_SIZE;
        	incomingMessage->addByte( rx.getData(pos) );
    	}
    	
    	/*	Here we always overwrite the 16-bit address. The received address 
    		is directly taken from the header information and thus correct.	*/
    	
		Node * senderNode = getNodeBy64( addr64 );
		if (senderNode != NULL) 
		{	
			senderNode->setAddress16( addr16 );
		}
		// delete senderNode;
    	
    	/* If the sender(!) of the message got an acknowledgement, this code is executed */
    	if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED)
		{
			
		}
		/*	If not, something strange has happened, because we got the message, but
			the sender did not receive an acknowledgement */
      	else
      	{
        	//we got it (obviously) but sender didn't get an ACK
      	}
    }
    
    /*	If the packet is indeed an AT_COMMAND_RESPONSE,	we write it to the atResponse.	*/
	else if (responseType == AT_COMMAND_RESPONSE) 
	{
//		Serial.println("Received AT answer");
		AtCommandResponse atResponse 	=	AtCommandResponse();
		uint32_t atAnswer				=	0;
		
		xbee.getResponse().getAtCommandResponse( atResponse );

		/*	If the atResponse is correct (does not return an error code), we can process it.	*/
		if (atResponse.isOk()) 
		{
			if (atResponse.getValueLength() > 0) 
			{
				for (int i = 0; i < atResponse.getValueLength(); i++) 
				{
					atAnswer	=	(atAnswer << 8) + atResponse.getValue()[i];
				}
       		}
       		
       		/*	If we got a Serial High	('S','H')	*/
       		if( atResponse.getCommand()[0] == atSH[0] &&
       			atResponse.getCommand()[1] == atSH[1] )
       		{
       			myAddress64.setMsb( atAnswer );
       		}
       		/*	If we got a Serial LOW ('S','L')	*/
       		if( atResponse.getCommand()[0] == atSL[0] &&
       			atResponse.getCommand()[1] == atSL[1] )
       		{
       			myAddress64.setLsb( atAnswer );
       		}
       		/*	If we got my 16-bit address ('M','Y')	*/
       		if( atResponse.getCommand()[0] == atMY[0] &&
       			atResponse.getCommand()[1] == atMY[1] )
       		{
       			myAddress16	=	atAnswer & 0xFFFF;
       		}
       		/*	If we got my 16-bit address ('M','Y')	*/
       		if( atResponse.getCommand()[0] == atID[0] &&
       			atResponse.getCommand()[1] == atID[1] )
       		{
       			myPANid		=	atAnswer & 0xFFFF;
       		}
       		/*	If we got my 16-bit address ('M','Y')	*/
       		if( atResponse.getCommand()[0] == atAI[0] &&
       			atResponse.getCommand()[1] == atAI[1] )
       		{
       			myAssStat	=	atAnswer & 0xFFFF;
       		}
		}
		else {	} 
	}
    
    /* NODE JOINING/LEAVING THE NETWORK */
	else if( responseType == MODEM_STATUS_RESPONSE )
	{
    	xbee.getResponse().getModemStatusResponse(msr);

       	if (msr.getStatus() == ASSOCIATED) 
       	{
        	/* A new node has joined the network.
        		Here we add the node if it is not yet known and store the 
        		64-bit and 16-bit address
        	*/
//        	xbee.getResponse().;
       	} 
	  	else if (msr.getStatus() == DISASSOCIATED) 
	  	{
         	/* Node leaves the network
         	*/
       	} 
	}
	/* DB MEASUREMENT RESPONSE */
	else if( responseType == REMOTE_AT_COMMAND_RESPONSE ) 	//REMOTE_AT_COMMAND_RESPONSE
	{	
		/* Serial.println("Received Remote AT Command Response");*/
		xbee.getResponse().getRemoteAtCommandResponse(rATcmd);
		
      	if ( rATcmd.getCommand()[0]	==	atDB[0] &&
      		 rATcmd.getCommand()[1]	==	atDB[1] )
      	{
	      	XBeeAddress64 rAddress64	=	rATcmd.getRemoteAddress64();
	      	uint16_t rAddress16			=	rATcmd.getRemoteAddress16();
			
			// This is now done at the end of this section
			//getNodeBy64( rAddress64 )->addDBMeasurement( rATcmd.getValue()[0] );
			
			/*	The line above replaces this section
			for (int i = 0; i < numNodes; i++)
			{
				if( nodes[i]->getID() == remoteId )
				{
	    			nodes[i]->addDBMeasurement( rATcmd.getValue()[0] );
		    	}
	      	}
	      	*/
	      	/* we use this function also to relate 16 bit addresses to 64 bit addresses */
	      	if( !nodeKnown16( rAddress16 ) && nodeKnown64( rAddress64 ) )
	      	{
	      		getNodeBy64( rAddress64 )->setAddress16( rAddress16 );
	      		// Serial.println("We know the 64-bit address, but not the 16-bit - now setting");
	      	}
			/* If we receive a message from the coordinator; it has 16-bit add 0. We related this to the node with 64 bit address 0x0 0x0; the coordinator. */
			else if ( rAddress16 == 0 && !nodeKnown16( rAddress16 ) )
			{ 			
				Node * defaultCoordinator = getNodeBy64( XBeeAddress64(0x0,0x0) );
				if (defaultCoordinator != NULL) 
				{
					defaultCoordinator->setAddress16( rAddress16 );
				}
				/*Serial.print("Set 16 bit add of coordinator to ");
				Serial.print( getNodeBy64( XBeeAddress64(0x0,0x0) )->getAddress16(  ) ); 
				Serial.print(" which is Node ID ");
				Serial.print( getNodeBy64( XBeeAddress64(0x0,0x0) )->getID(  ) );  */
			}
			
			/*Serial.print("Received Remote DB from nodeId ");
			Serial.print( getNodeBy16( rAddress16 )->getID() );
			Serial.print(" (64b: ");
			Serial.print( rAddress64.getLsb(), HEX );
			Serial.print(", 16b: ");
			Serial.print( rAddress16, DEC ); 
			
			Serial.print( ") DB val: ");
			Serial.println( rATcmd.getValue()[0] ); */
			
			Node * remoteNode = getNodeBy16( rAddress16 );
			if ( remoteNode != NULL) 
			{
				remoteNode->addDBMeasurement( rATcmd.getValue()[0] );
			}
	    }
	    /*	Here we can add other remote AT command responses
	    else if( rATcmd.getCommand()[0]	==	atSH[0] &&
      		 	 rATcmd.getCommand()[1]	==	atSH[1] )
		{
		
		}
		*/
    }
    /*	Confirmation on transmitted package. This is received
    	everytime a message is transmitted and has details
    	whether the package is received or not.
	*/
    else if (responseType == ZB_TX_STATUS_RESPONSE)
    {
	    xbee.getResponse().getZBTxStatusResponse(txStatus);
     	
     	if (txStatus.getDeliveryStatus() == SUCCESS) 
	 	{
			/* Als we hier zijn aangekomen, hebben we een berichtje gestuurd,
	 		dat is goed aangekomen. Deze response geeft echter alleen een
	 		16-bit address terug en geen 64-bit, dus die kunnen we niet aan
	 		een node koppelen.
	 		Nu willen we kijken of de 16-bit bekend is in onze node lijst.
	 		Als dat zo is, dan hoeven we niets te doen, we kennen deze node.
	 		Als dat NIET zo is, dan willen we van deze node ook het 64-bit
	 		address opvragen, zodat we 16-bit en 64-bit op kunnen slaan.
	 		*/
			
	 		uint16_t rAddress16	=	txStatus.getRemoteAddress();	//Stores the 16-bit address
	 		
	 		if( !nodeKnown16( rAddress16 ) )
	 		{
	 			/*Here we send something that requests data from the remote node
				We can link the two addresses in the returned data (remote at command response)*/
	 			sendDBRequest16( rAddress16 );
	 		}
		}      	
      	else 
	  	{
        	/* the remote XBee did not receive our packet. 
        		If this is because the 16 bit address is outdated; we wish to reset that */
       		uint16_t rAddress16	=	txStatus.getRemoteAddress();	//Get the 16-bit address
			if( nodeKnown16( rAddress16 ) ) // If we know that address, but the message was not received
			{
				Node * remoteNode = getNodeBy16( rAddress16 );
				if ( remoteNode != NULL) 
				{
					remoteNode->setAddress16( UNKNOWN_16B ); // We reset the 16 bit address so it will use the 64 bit again
				}
			}
     	}
    }
    //Something occured that is unknown, or we do not care about
    else
    {
    }
  }
}

/* LOW LEVEL STUFF BEYOND THIS POINT */

/** This function allows low level sending of bytes - including XBee API headers and escaped chars; suggest you don't use this **/
void LithneClass::sendBytePublic( uint8_t b, bool escape )
{
	xbee.sendBytePublic( b, escape );
}

/** returns the size of the complete low level xbee packet including XBee API headers and escaped chars; suggest you don't use this **/
uint8_t LithneClass::getXBeePacketSize()
{
	return xbee.getXBeePacketSize();
}

/** returns the complete low level xbee packet including XBee API headers and escaped chars; suggest you don't use this **/
uint8_t * LithneClass::getXBeePacket()
{
	return xbee.getXBeePacket();
}

/* Define an instance of the Lithneclass here, named Lithne */
LithneClass Lithne;