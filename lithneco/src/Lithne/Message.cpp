/**
 * Message.cpp - The official Lithne library.
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

#include "Message.h"
#include "LithneXBee.h"
#include "Node.h"

#include "LithneDefinitions.h"

/** Default Message constructor */
Message::Message()
{
	payloadSize 	=	0;		//Contains the number of bytes in the payload including scope and function
	sender16		=	0xFFFE;	//Contains the 16 bit address of the Sender
	recipient16		=	0xFFFE;	//Contains the 16 bit address of the Recipient
	sender64		=	XBeeAddress64( 0x0, 0xFFFF);	// Contains the 64 bit address of the Sender
	recipient64 	=	XBeeAddress64( 0x0, 0xFFFF);	//Contains the 64 bit address of the Recipient

	for(int i=MSG_HEADER_SIZE;i<MAXIMUM_PAYLOAD_BYTES;i++)
	{
		payload[i]	=	0;
	}
}

Message::~Message()
{

}

/* 	
__   _____ ___ ___  
\ \ / / _ \_ _|   \ 
 \ V / (_) | || |) |
  \_/ \___/___|___/ 
                    
*/

/** Set the recipient for the message using 64 bit Address **/
void Message::setRecipient64( XBeeAddress64 _addr64 )
{
	recipient64	=	_addr64;
}
/** Set the recipient for the message using 16 bit Address, as this is varying, you wont use this in your code **/
void Message::setRecipient16( uint16_t _addr16 )
{
	recipient16	=	_addr16;
}
/** Set the recipient for the message using a Node pointer **/
void Message::setRecipient( Node * node )
{
	if (node != NULL)
	{
		recipient16	=	node->getAddress16();
		recipient64	=	node->getAddress64();
	}
}
/*
void Message::setRecipient( uint8_t _id )
{
	Node * node = getNodeByID(_id);
	if (node != NULL)
	{
		recipient16	=	node->getAddress16();
		recipient64	=	node->getAddress64();
	}
} */


/** Clear all message arguments (set them to 0) **/
void Message::clearArguments()
{
	for(int i=MSG_HEADER_SIZE; i<MAXIMUM_PAYLOAD_BYTES; i++)
	{
		payload[i] = 0;
	}
	payloadSize	=	MSG_HEADER_SIZE;
}

/** Set the function the message should call **/
void Message::setFunction( String _func )
{
	setFunction( hash(_func) );
}
/** Set the function the message should call **/
void Message::setFunction( uint16_t _func )
{
	payload[FUNCTION_MSB]	=	( _func >> 8 ) 		& 0xFF;
	payload[FUNCTION_LSB]	=	_func 				& 0xFF;
}
/** Store the 64 bit address of the sender of the message **/
void Message::setSender64( XBeeAddress64 _addr64 )
{
	sender64	=	_addr64;
}
/**	Store the 16-bit address of the sender	**/
void Message::setSender16( uint16_t _addr16 )
{
	sender16	=	_addr16;
}
/**	Set both the 16-bit and 64-bit address of the sender in one go	**/
void Message::setSender( uint16_t _addr16, XBeeAddress64 _addr64 )
{
	sender16	=	_addr16;
	sender64	=	_addr64;
}

/** Sets the scope for this message as a string (this is hashed into an int) **/
void Message::setScope( String _scope )
{
	setScope( hash( _scope ) );
}

/**	Sets the scope for this message as an int **/
void Message::setScope( uint16_t _scope )
{
	payload[SCOPE_MSB]	=	( _scope >> 8 ) 	& 0xFF;
	payload[SCOPE_LSB]	=	_scope 				& 0xFF;
	/*	Here we can add a line that sets the recipient
		to the BROADCAST address if no address has been
		set yet. This will make sure the message can
		be delivered.	*/
}

/*
 ___  ___   ___  _    ___   _   _  _ 
| _ )/ _ \ / _ \| |  | __| /_\ | \| |
| _ \ (_) | (_) | |__| _| / _ \| .` |
|___/\___/ \___/|____|___/_/ \_\_|\_|

*/

/** Add a byte argument to the list of arguments **/
bool Message::addByte( uint8_t arg )
{
	if( payloadSize+1 < MAXIMUM_PAYLOAD_BYTES )
	{
		payload[payloadSize]	=	arg;
		payloadSize++;
		return true;
	}
	else
	{
		return false;
	}
}

/** Add an argument to the list of arguments **/
bool Message::addArgument( uint16_t arg )
{
	if( payloadSize+2 < MAXIMUM_PAYLOAD_BYTES )
	{
		payload[payloadSize]	=	( arg >> 8 ) 	& 0xFF;
		payload[payloadSize+1]	=	arg 			& 0xFF;
		payloadSize+=2;
		return true;
	}
	else
	{
		return false;
	}
}

/** Set a String argument. All existing arguments will be overwritten. The string can be retrieved on the other end using getStringArgument()  **/
bool Message::setStringArgument( String arg )
{
	//First we clear the arguments
	clearArguments();
	
	for( uint8_t i=0; i < arg.length(); i++  )	//Loop through the entire argument
	{
		addByte(arg[i]);
		if ( payloadSize >= MAXIMUM_PAYLOAD_BYTES)
		{
			break;
		}
	}
	
	if( MSG_HEADER_SIZE+sizeof(arg) <= MAXIMUM_PAYLOAD_BYTES )
	{
		return true;
	}
	else
	{
		return false;
	}
}

/* How to add multiple arguments to the array? Discuss this with Elco
boolean addArg( uint16_t* args ) //Add an array of arguments to the array
{

}
*/

/**  FunctionIs returns true if the function of the message is the same as the function that is provided as an argument to this function.
	Internally, this function compares a hash() of the provided argument with the internally stored function ID
*/
boolean Message::functionIs( String _func )
{
	if ( hash( _func ) == getFunction() )
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
 ___ _  _ _____ ___ ___ ___ ___ 
|_ _| \| |_   _| __/ __| __| _ \
 | || .` | | | | _| (_ | _||   /
|___|_|\_| |_| |___\___|___|_|_\
                                
*/

/** Return the number of bytes in the payload **/
uint8_t Message::getNumberOfBytes()
{
	return payloadSize-MSG_HEADER_SIZE;
}

/** Return the number of arguments **/
uint8_t Message::getNumberOfArguments()
{
	return (payloadSize-MSG_HEADER_SIZE)/2;
}

/** Return the payload size **/
uint8_t Message::getPayloadSize()
{
	return payloadSize;
}

/** Returns the byte at a position in the data, excluding scope and function **/
uint8_t Message::getByte( uint8_t _pos )
{
	if (_pos >= 0 && _pos < payloadSize - MSG_HEADER_SIZE )
	{
		return payload[_pos+MSG_HEADER_SIZE];
	}
	else 
	{
		return 0;
	}
}

uint8_t * Message::getPayload()
{
	return payload;
}

/** Returns the byte at a position in the total payload - including scope and function **/
uint8_t Message::getPayloadByte( uint8_t _pos )
{
	if (_pos >= 0 && _pos < payloadSize )
	{
		return payload[_pos];
	}
	else 
	{
		return 0;
	}
}

/**	Returns a hash code based on the name of a group **/
/* -- This function is copied into the Message class; in case of alternations; do it in both classes */
uint16_t Message::hash( String _group )
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

/** return the argument at the specified position **/
uint16_t Message::getArgument( uint8_t position )
{
	position = constrain( position, 0, MAXIMUM_PAYLOAD_BYTES );
	uint16_t arg  =  NO_ARGUMENT;
    if ( position < getNumberOfArguments() )
    {
    	uint16_t argMSB	=	payload[ MSG_HEADER_SIZE+(position*2)  ] << 8;
    	uint8_t  argLSB	=	payload[ MSG_HEADER_SIZE+(position*2)+1];
    	arg 		=	argMSB + argLSB;
    }
	return arg;
}
/** return the argument at the specified position **/
String Message::getStringArgument( )
{
	char characterArr[ getNumberOfBytes()+1 ];		// +1 is necessary for the '/0' terminator in Arduino strings, this is automatically added
	for (uint8_t i = 0; i < sizeof(characterArr); i++ )
	{
		characterArr[i]   =	 (char)(payload[MSG_HEADER_SIZE+i]);
	}
	return	characterArr;
}
/** Return the functionID of the message **/
uint16_t Message::getFunction()
{
	uint16_t funcMSB	=	payload[ FUNCTION_MSB ] << 8;
	uint16_t funcLSB		=	payload[ FUNCTION_LSB ];
	return funcMSB + funcLSB;
}
/** return the 16 bit address of the recipient **/
uint16_t Message::getRecipient16()
{
	return recipient16;
}
/** Returns the 16 bit address of the sender **/
uint16_t Message::getSender16()
{
	return sender16;
}
/** Returns the scope of the message **/
uint16_t Message::getScope()
{
	uint16_t scopeMSB	=	payload[ SCOPE_MSB ] << 8;
	uint16_t scopeLSB	=	payload[ SCOPE_LSB ];
	return scopeMSB + scopeLSB;
}

/*
__  _____            _      _    _                 __ _ _  
\ \/ / _ ) ___ ___  /_\  __| |__| |_ _ ___ ______ / /| | | 
 >  <| _ \/ -_) -_)/ _ \/ _` / _` | '_/ -_|_-<_-</ _ \_  _|
/_/\_\___/\___\___/_/ \_\__,_\__,_|_| \___/__/__/\___/ |_| 

*/

/** Return the receiving XBee address **/
XBeeAddress64 Message::getRecipient64()
{
	return recipient64;
}
/** Return the transmitting XBee address **/
XBeeAddress64 Message::getSender64( )
{
	return sender64;
}