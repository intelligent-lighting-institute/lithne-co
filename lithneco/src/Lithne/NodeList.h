/*
 * NodeList.h
 *
 * Created: 9-7-2013 23:15:08
 *  Author: Elco
 */ 

#include "Node.h"

#define MAX_NODES 10

class NodeList{
	uint8_t numNodes; //Contains the number of nodes in storage
	
	NodeList();
	~NodeList();
	
	Node nodes[MAX_NODES];
	bool addNode( uint8_t _id, XBeeAddress64 _addr64, uint16_t _addr16 );
};

