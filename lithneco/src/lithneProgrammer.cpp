/*
 * lithneProgrammer.cpp
 *
 * Created: 4-7-2013 13:53:52
 *  Author: Elco
 */ 

#include "lithneProgrammer.h"
#include <Lithne/Lithne.h>
#include "main.h"
#include "delay.h"
#include "ioport.h"
#include "user_board.h"
#include "print.h"
#include "uart.h"
#include <asf.h>

LithneProgrammer::LithneProgrammer(){
	programming = false;	
	lastPacketRequestTime = 0;
	pageNumber = 0;
	oldInterruptSetting = 0;
}
	
LithneProgrammer::~LithneProgrammer(){
		
}

void LithneProgrammer::init(USART_t * usart, HardwareSerial * serial){
	progSerial = serial;
	progUsart = usart;
	Lithne.addNode( REMOTE, XBeeAddress64(0x00,0x0000FFFF) );         // Broadcast by default
	debugMessage("Programmer Init Complete");
	
}

void LithneProgrammer::updateRemoteAddress(){
	
	
	/*
	debugMessage("UpdateRemote() Remote Address to (%lX, %lX), original was (%lX, %lX)", \
	Lithne.getSender64().getMsb(), \
	Lithne.getSender64().getLsb(), \
	Lithne.getNodeByID( REMOTE )->getAddress64().getMsb(), \
	Lithne.getNodeByID( REMOTE )->getAddress64().getLsb() ); */
	
	// Check if the remote address is different from the one we have stored
	if (	Lithne.getNodeByID( REMOTE )->getAddress64().getMsb() != Lithne.getSender64().getMsb() ||
			Lithne.getNodeByID( REMOTE )->getAddress64().getLsb() != Lithne.getSender64().getLsb() )
	{
		// If so, we set the new 64 bit address.
		Lithne.getNodeByID( REMOTE )->setAddress64( Lithne.getSender64() );
		debugMessage("Setting Remote Address to (%lX, %lX): ", \
			Lithne.getNodeByID( REMOTE )->getAddress64().getMsb(), \
			Lithne.getNodeByID( REMOTE )->getAddress64().getLsb());
	}
}

void LithneProgrammer::processPacket(){
	uint16_t dataLength = Lithne.getIncomingMessage()->getNumberOfBytes();   //   Returns number of bytes without Lithne header and footer
	lastPacketTimer = millis();                                         //   set lastPacketTimer to current millis() so we don't time out
	// The first programming packet is 2 bytes bigger than the normal 'packetSize'; this is how we determine the start of the programming sequence
	// The two bytes contain the number of incoming packets that can be expected
	bool firstPacket = false;
	
	if (dataLength > packetSize)
	{
		firstPacket = true;
				
		// We reset the pageBuffer as we are going to start filling it with new data
		pbuff.reset();		
		packetsReceived  = 0;
		pageNumber       = 0;                                                            // Reset the number of the page we are working on
		packetsIncoming  = (Lithne.getIncomingMessage()->getByte(0)*256 + Lithne.getIncomingMessage()->getByte(1));    // Get number of packets from the first two bytes of the first packet: Combine MSB and LSB
						
		debugMessage("Start programming; Incoming Packets: %u", packetsIncoming);
						
		if(!startProgrammer()){
			debugMessage("Could not succesfully configure programmer, aborting");
			stopProgrammer();
			return;
		}
	}
	
	// Add the incoming data to the page buffer. Do this first, because stopProgrammer could reset the pagebuffer on error.
	for(uint16_t i=0; i < dataLength; i++)                                   //   read data in xbeepacket and copy to buffer[] for processing
	{
		pbuff.add( Lithne.getIncomingMessage()->getByte(i) );
	}	
	
	if(firstPacket){
		// We remove the first two bytes from the buffer as they don't contain program code, but the amount of incoming packets
		pbuff.removeHeaderFromFirstPage();
	}

	// Do the programming, stop the programmer when an error has occurred.
	if(!program()){
		stopProgrammer();
	}
}

void LithneProgrammer::processCheckin(){
	// If we receive a check-in message with an argument, the sender wants to get node info
	if ( Lithne.getIncomingMessage()->getNumberOfArguments() == 1 )
	{
		nodeInfo.sendInfo();
	}
	else // Otherwise we just reply with a check in message
	{
		// Send check-in message
		Lithne.setRecipient( REMOTE );
		Lithne.setScope( lithneProgrammingReturnScope );
		Lithne.setFunction( fCheckingIn );
		Lithne.send();
	}
}

void LithneProgrammer::processNodeName(){
    if ( Lithne.getIncomingMessage()->getNumberOfBytes() > 0 )
	{
		debugMessage("Setting NodeName");
		nodeInfo.setNodeName(Lithne.getStringArgument());
	}
	nodeInfo.sendNodeName();
}

void LithneProgrammer::processLastUpload(){
	if ( Lithne.getIncomingMessage()->getNumberOfBytes() > 0 )
	{
		nodeInfo.setLastUpload(Lithne.getStringArgument());
	}
	nodeInfo.sendLastUpload();
}

void LithneProgrammer::processFileName(){
	if ( Lithne.getIncomingMessage()->getNumberOfBytes() > 0 )
	{
		nodeInfo.setFileName(Lithne.getStringArgument());
	}
	nodeInfo.sendFileName();
}

void LithneProgrammer::processKill(){
	if (Lithne.getNumberOfArguments() == 1)
	{
		setMainReset( Lithne.getArgument(0) );
	}
}

/* Processes the incoming packets */
bool LithneProgrammer::program(void)
{	
	// Keep track of the number of received packets
	packetsReceived++;
	bool lastPacket = false;
	// If we receive the last packet, we fill up the buffer with 0xff, stop the programming (open up for other things) and send a message to the uploader
	if ( packetsReceived >= packetsIncoming)
	{
		// Fill in rest of page with 0xff
		debugMessage("Received last packet");
		while( pbuff.getPos() < pageSize )
		{
			pbuff.add(0xff);
		}
		lastPacket = true;
	}
	
	// Check if the page buffer is full to write it to the main processor
	if ( pbuff.getPos() >= pageSize )
	{
		debugMessage("Reached PageSize, copying to main processor");
				
		//   start copier - actually program the main processor
		bool success = copyPage(pageNumber);
	
		pageNumber++;    // Keep track of the number of pages written so far. This is only used to determine the first page for reset
		
		// Reset the program packet buffer so we are able to receive the new stuff
		pbuff.reset();
		
		//  Abort if page is not copied successfully
		if (!success){
			debugMessage("Error while copying page");
			return 0;
		}			
	}
		
	// If we expect more packets to be incoming than we have received so far - request next packet
	if (!lastPacket && programming)
	{
		requestNextPacket();
	}
	else{
		// let the remote know that the upload is complete
		Lithne.setRecipient( REMOTE );
		Lithne.setScope( lithneProgrammingReturnScope );
		Lithne.setFunction( fUploadCompleted );
		Lithne.send();
		if(stopProgrammer()){
			debugMessage("Exit bootloader after successful programming.");
			Lithne.setRecipient( REMOTE );
			Lithne.setScope( lithneProgrammingReturnScope );
			Lithne.setFunction( fCodeProgrammed );
			Lithne.send();
		}
		else{
			debugMessage("Did not receive ack in exit program mode/bootloader");
		}
	}	
	
	return 1;
}

// Check if we have received the packet we requested within half of the timeout time, otherwise request it again
void LithneProgrammer::checkUploadProgress()
{
	if( programming )
	{
		unsigned long timeSinceLastRequest = millis() - lastPacketRequestTime;
		if (timeSinceLastRequest > (PROGRAM_TIMEOUT/2) )
		{
			debugMessage("Request packet %u again", packetsReceived);
			requestNextPacket();
		}
	}
}


//  Reads byte(s) on serial port with retries.
//	Keep reading until something is received or until timeout
volatile uint8_t LithneProgrammer::readByte()
{                                  
	uint32_t start = millis();
	uint32_t elapsed; // declaring it here prevents the compiler from optimizing the calculation away for some reason.
	while (! progSerial->available()) { // wait while receive complete interrupt flag is false
		elapsed = millis() - start;
		if(elapsed > 1000u){ // 1 second timeout
			debugMessage("receive timed out");
			return 0;
		}
	}
	uint8_t received = progSerial->read();
	// debugMessage("rec %c [%x]", received, received);
	return received;
}

// See AVR109 self-programming app note for programming protocol: http://www.atmel.com/images/doc1644.pdf
bool LithneProgrammer::startProgrammer(void){
	uint8_t responseByte;
	programming = true;      // We are now in programming mode
	resetMain();
	
	delay_ms(100); //give the processor some time to boot
	
	// Clean up unwanted data that may be stored in Serial buffer, to prevent these bytes being read during readByteWithDelay()
	progSerial->flush();
	
	// make sure main serial usart is open and configured at default settings.
	oldInterruptSetting = progUsart->CTRLA;
	progUsart->CTRLA = 0; // only enable receive interrupt, used in combination with arduino serial functions
	usart_set_rx_interrupt_level(progUsart, USART_INT_LVL_HI); // this will put the received bytes in Serial's buffer
	
	uart_open(progUsart); // open usart with default settings
	
	progSerial->write(0x1b); // send the start bootloader byte
	delay_ms(10);
	
	progSerial->write('S');				// Return Software Identifier

	printfToPort_P(DEBUG_PORT, false, PSTR("Detected bootloader: "));
	for(int i = 0; i < 7; i++){
		responseByte = readByte();
		printfToPort_P(DEBUG_PORT, false, PSTR("%c"), responseByte);
	}

	progSerial->write('p');				// Return programmer type

	printfToPort_P(DEBUG_PORT, false, PSTR(" Type: %c"), readByte());

	progSerial->write('V');				// Return programmer version
	uint8_t v1 = readByte();
	uint8_t v2 = readByte();
	printfToPort_P(DEBUG_PORT, true, PSTR(" Software Version: %c.%c"), v1, v2);

	progSerial->write('t');				// Get support device type
	printfToPort_P(DEBUG_PORT, false, PSTR(" Supported device types: ["));
	bool rightType = false;
	for(int i = 0; i<8; i++){ // read max 8 device types, or this could block.
		responseByte = readByte();
		if(responseByte == 0x7b){
			rightType = true;
		}
		if(responseByte == 0x00){
			break; // end of list
		}
		else{
			printfToPort_P(DEBUG_PORT, false, PSTR(" %X, "), responseByte);
		}
	}
	printfToPort_P(DEBUG_PORT, true, PSTR("]"));
	
	if(!rightType){
		printfToPort_P(DEBUG_PORT, true, PSTR("device code 0x7b not supported by bootloader"));
		return 0;
	}

	progSerial->write('T');				// Set device type
	progSerial->write(0x7b);

	if(readByte() == 0x0d){
		printfToPort_P(DEBUG_PORT, true, PSTR("devcode selected: 0x7b"));
	}
	else{
		printfToPort_P(DEBUG_PORT, true, PSTR("Error selecting devcode: 0x7b"));
		return 0;
	}

	progSerial->write('P');				// Set device type

	if(readByte() == 0x0d){
		printfToPort_P(DEBUG_PORT, true, PSTR("AVR device initialized and ready to accept instructions"));
	}
	else{
		printfToPort_P(DEBUG_PORT, true, PSTR("Error entering program mode"));
		return 0;
	}

	progSerial->write('s');				// Set device type
	printfToPort_P(DEBUG_PORT, true, PSTR("Device code: %X %X %X"), readByte(), readByte(), readByte() );

	progSerial->write('A');				//   Set Address (0x00, 0x00)
	progSerial->write((uint8_t) 0x00);
	progSerial->write((uint8_t) 0x00);

	if (readByte() != 0x0d){
		debugMessage("Could not set address to 0x00 0x00");
		return 0;
	}
	
	pbuff.reset();
	
	// programmer successfully initialized 
	return 1;
}

bool LithneProgrammer::stopProgrammer(void){
	bool success = true;
	uint8_t returnByte;
	
	debugMessage("Stopping programmer");
	pbuff.reset();
	
	progSerial->write('L');                      //   Leave programming mode
	returnByte = readByte();
	success = success && (returnByte == 0x0d);
			
		
	progSerial->write('E');                      //   Exit bootloader
	returnByte = readByte();
	success = success && (returnByte == 0x0d);
		
	resetMain();										//   Reset main processor
	
	progUsart->CTRLA = oldInterruptSetting;
	programming = false;
	return success;
}

bool LithneProgrammer::copyPage( uint16_t pageNum )
{
	// We pour blocks of data stored in buffer[], ready to write to the AVR's flash
	// We only write in blocks of 512 bytes for the atXmega256.
	debugMessage("Writing flash page %u", pageNum);
	
	progSerial->write('B');					//   Start flash block load
	progSerial->write((uint8_t) 0x02);					//   ?? Part of 'B' command, values copied from avrdude
	progSerial->write((uint8_t) 0x00);					//	 ??
	progSerial->write('F');					//	 Flash block
	
	for (uint16_t i = 0; i < pageSize; i++)
	{
		progSerial->write( pbuff.getByte(i) );
	}

	if (readByte() != 0x0d) return 0;
		
	// If we reached here, then current packet has been programmed successfully.
	// But please note: no verification is done like avrdude, only bootloader replies are checked!
	return 1;
}

bool LithneProgrammer::busyProgramming(void){
	return programming;	
}

void LithneProgrammer::resetMain(void)
{
	setMainReset(true);
	delay_ms(100);
	setMainReset(false);
}

void LithneProgrammer::resetXbee(void)
{
	setXbeeReset(true);
	delay_ms(100);
	setXbeeReset(false);
}

void LithneProgrammer::setMainReset(bool holdInReset)
{
	if(holdInReset){
		ioport_set_pin_dir(MAIN_nRST, IOPORT_DIR_OUTPUT);
		ioport_set_pin_level(MAIN_nRST, false);
	}
	else{
		ioport_set_pin_level(MAIN_nRST, true);
		ioport_set_pin_dir(MAIN_nRST, IOPORT_DIR_INPUT);
	}
}

void LithneProgrammer::setXbeeReset(bool holdInReset)
{
	if(holdInReset){
		ioport_set_pin_dir(XBEE_nRESET, IOPORT_DIR_OUTPUT);
		ioport_set_pin_level(XBEE_nRESET, false);
	}
	else{
		ioport_set_pin_level(XBEE_nRESET, true);
		ioport_set_pin_dir(XBEE_nRESET, IOPORT_DIR_INPUT);
	}
}

void LithneProgrammer::preventHangup(void){
	//  Timeout check to prevent code from staying in program mode for ever
	if ( busyProgramming() )
	{
		if ( millis()-lastPacketTimer > PROGRAM_TIMEOUT )
		{
			debugMessage("Programming timed out");
			stopProgrammer();
			Lithne.getNodeByID( REMOTE )->setAddress64( XBeeAddress64(0x00,0x00) );
		}
		// If we are programming, we also check whether we receive new packets on our requests - if not, we request again
		checkUploadProgress();
	}
}

void LithneProgrammer::requestNextPacket(void){
		debugMessage("Request Packet %u from Remote (%lX, %lX)", packetsReceived, \
		Lithne.getNodeByID( REMOTE )->getAddress64().getMsb(), \
		Lithne.getNodeByID( REMOTE )->getAddress64().getLsb());
		
		//  update progress
		Lithne.setRecipient( REMOTE );
		Lithne.setScope( lithneProgrammingReturnScope );
		Lithne.setFunction( fRequestNextPacket );
		Lithne.addArgument( packetsReceived );
		Lithne.send();
		lastPacketRequestTime = millis();
}

LithneProgrammer lithneProgrammer;