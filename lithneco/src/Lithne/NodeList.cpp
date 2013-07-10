/*
 * CPPFile1.cpp
 *
 * Created: 9-7-2013 23:12:30
 *  Author: Elco
 */ 

#include "Node.h"
#include "NodeList.h"

NodeList::NodeList(void){
	numNodes = 0;
	for(int i = 0; i < MAX_NODES; i++)	{
		nodes[i].setID(UNKNOWN_NODE_ID);
		nodes[i].setAddress64( XBeeAddress64(0x00000000 , 0x00000000 ));
		nodes[i].setAddress16( UNKNOWN_16B);
	}
	
}

bool NodeList::addNode( uint8_t _id, XBeeAddress64 _addr64, uint16_t _addr16 ){
		/*	If we have not passed the maximum number of nodes	*/
		if( numNodes < MAX_NODES &&
		!nodeKnown( _id ) &&
		!nodeKnown64( _addr64)	)
		{
			nodes[numNodes].setID( _id);
			nodes[numNodes].setAddress64(_addr64);
			nodes[numNodes].setAddress16(_addr16);
			
			numNodes++;
			return true;	//Return true, because we have succesfully added a node
		}
		else
		{
			return false;	//If we can't add a new node, return false
		}
}

Node * NodeList::getByID(uint8_t _id){
	
}

/**	Checks whether there is a node with the specified ID in the memory	**/
bool NodeList::nodeKnown( uint8_t _id )
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
bool NodeList::nodeKnown64( XBeeAddress64 _addr64 )
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
bool NodeList::nodeKnown16( uint16_t _addr16 )
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