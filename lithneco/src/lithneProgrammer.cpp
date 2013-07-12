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
	doneUploading  = false;
	programSucces  = false;
	programming    = false;	
	lastPacketRequestTime = 0;
	pageNumber = 0;
	oldInterruptSetting = 0;
}
	
LithneProgrammer::~LithneProgrammer(){
		
}

void LithneProgrammer::init(USART_t * usart){
	progUsart = usart;
	Lithne.addNode( REMOTE, XBeeAddress64(0x00,0x0000FFFF) );         // Broadcast by default
}

void LithneProgrammer::updateRemoteAddress(){
	// Check if the remote address is different from the one we have stored
	if ( Lithne.getNodeByID( REMOTE )->getAddress64().getMsb() != Lithne.getSender64().getMsb() ||
	Lithne.getNodeByID( REMOTE )->getAddress64().getLsb() != Lithne.getSender64().getLsb() )
	{
		// If so, we set the new 64 bit address.
		Lithne.getNodeByID( REMOTE )->setAddress64( Lithne.getSender64() );
		debugMessage("Setting Remote Add (%lX, %lX): ", \
			Lithne.getNodeByID( REMOTE )->getAddress64().getMsb(), \
			Lithne.getNodeByID( REMOTE )->getAddress64().getLsb());
	}
}

void LithneProgrammer::processPacket(){
	uint16_t dataLength = Lithne.getIncomingMessage()->getNumberOfBytes();   //   Returns number of bytes without Lithne header and footer
	lastPacketTimer = millis();                                         //   set lastPacketTimer to current millis() so we don't time out
	boolean firstPacket = false;
	// The first programming packet is 2 bytes bigger than the normal 'packetSize'; this is how we determine the start of the programming sequence
	// The two bytes contain the number of incoming packets that can be expected
	if (dataLength > packetSize)
	{
		firstPacket      = true;
		// We reset the pageBuffer as we are going to start filling it with new data
		pbuff.reset();
	}
	// Add the incoming data to the page buffer
	for(uint16_t i=0; i < dataLength; i++)                                   //   read data in xbeepacket and copy to buffer[] for processing
	{
		pbuff.add( Lithne.getIncomingMessage()->getByte(i) );
	}
	// Do the programming, let it know if this is the first packet or not
	program( firstPacket );
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
		Lithne.setScope( lithneProgrammingScope );
		Lithne.setFunction( fCheckingIn );
		Lithne.send();
	}
}

void LithneProgrammer::processNodeName(){
    if ( Lithne.getIncomingMessage()->getNumberOfBytes() > 0 )
	{
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
void LithneProgrammer::program( bool firstPacket )
{
	programming = true;      // We are now in programming mode				
	if ( firstPacket )
	{
		doneUploading    = false;
		programSucces    = false;
		packetsReceived  = 0;
		pageNumber       = 0;                                                            // Reset the number of the page we are working on
		packetsIncoming  = (pbuff.getByte(0)*256) + pbuff.getByte(1);    // Get number of packets from the first two bytes of the first packet: Combine MSB and LSB
		
		// make sure main serial usart is open and configured at default settings.
		uart_open(progUsart);
		uart_config(progUsart);
			
		oldInterruptSetting = progUsart->CTRLA;
		progUsart->CTRLA = 0; // disable all interrupts while programming.
		
		debugMessage("Strat programming; Incoming Packets: %u", packetsIncoming);
				
		// We remove the first two bytes from the buffer as they dont contain program code, but the amount of incoming packets
		pbuff.removeHeaderFromFirstPage();
	}
	
	// Keep track of the number of received packets
	packetsReceived++;
	
	// If we receive the last packet, we fill up the buffer with 0x00, stop the programming (open up for other things) and send a message to the uploader
	if ( packetsIncoming == packetsReceived )
	{
		// Fill in rest of page with 0xff
		while( pbuff.getPos() < pageSize )
		{
			pbuff.add(0xff);
		}

		doneUploading = true;
		
		// And let the programmer know that the upload is complete
		Lithne.setRecipient( REMOTE );
		Lithne.setScope( lithneProgrammingScope );
		Lithne.setFunction( fUploadCompleted );
		Lithne.send();
	}
	
	// This happens every time the pageBuffer is full (every 8 packets)
	if ( pbuff.getPos() >= pageSize )
	{
		debugMessage("Reached PageSize!");
				
		//   start copier - actually program the main proc
		copyPage(pageNumber);
		pageNumber++;    // Keep track of the number of pages written so far. This is only used to determine the first page for reset
		
		// Reset the program packet buffer so we are able to receive the new stuff
		pbuff.reset();
		
		//  If the upload is completed and programming succesful let the uploader know
		//  Note that uploading success is different from programming success
		if ( programSucces == true )
		{
			Lithne.setRecipient( REMOTE );
			Lithne.setScope( lithneProgrammingScope );
			Lithne.setFunction( fCodeProgrammed );
			Lithne.send();
			programSucces = false;
		}
	}
	
	// If we expect more packets to be incoming than we have received so far - request next packet
	if (packetsIncoming > packetsReceived)
	{
		debugMessage("RequestNxtPckt: \t %u", packetsReceived);
		debugMessage("From Remote (%lX, %lX): ", \
			Lithne.getNodeByID( REMOTE )->getAddress64().getMsb(), \
			Lithne.getNodeByID( REMOTE )->getAddress64().getLsb());
		
		Lithne.setRecipient( REMOTE );
		Lithne.setScope( lithneProgrammingScope );
		Lithne.setFunction( fRequestNextPacket );
		Lithne.addArgument( packetsReceived );
		Lithne.send();
		lastPacketRequestTime = millis();
	}
}

// Check if we have received the packet we requested within half of the timeout time, otherwise request it again
void LithneProgrammer::checkUploadProgress()
{
	if( programming )
	{
		unsigned long timeSinceLastRequest = millis() - lastPacketRequestTime;
		if (timeSinceLastRequest > (PROGRAM_TIMEOUT/2) )
		{
			lastPacketRequestTime = millis();
			debugMessage("TIMED PCK REQUEST: \t %u", packetsReceived);
			debugMessage("Time since last request %lu, millis: %lu", timeSinceLastRequest, millis());
			Lithne.setRecipient( REMOTE );
			Lithne.setScope( lithneProgrammingScope );
			Lithne.setFunction( fRequestNextPacket );
			Lithne.addArgument( packetsReceived );
			Lithne.send();
		}
	}
}


//   Reads byte(s) on serial port with a delay. We always expect this to be 0x0d during programming
/*uint8_t LithneProgrammer::readByteWithDelay()
{                                  
	delay(20);
	return usart_getchar(progUsart);
}*/

/* Bits below based on Arduino Copier - An arduino sketch that can upload sketches to other boards, by George Caley. */
int LithneProgrammer::copyPage( int pageNum )
{
	uint8_t responseByte;
	if ( pageNum == 0 )                                //   only if first page, reset Main board and set programming parameters.
	{
		resetMain();
		
		
	/*	while (progSerial->available())
		{               //   Clean up unwanted data that may be stored in Serial buffer, to prevent these bytes being read during readByteWithDelay()
			responseByte = progSerial->read();
		}*/
		usart_get(progUsart); // discard existing bytes in serial buffer
		
		usart_putchar(progUsart, 0x54);                       //   Select device type = 0x7b
		usart_putchar(progUsart, 0x7b);

		responseByte = usart_getchar(progUsart);
		if (responseByte != 0x0d) return 0;
		
		usart_putchar(progUsart, 0x50);                       //   Enter programming mode
		responseByte = usart_getchar(progUsart);
		if (responseByte != 0x0d) return 0;
		
		usart_putchar(progUsart, 0x41);                       //   Set Address (0x00, 0x00)
		usart_putchar(progUsart, byte(0x00));
		usart_putchar(progUsart, byte(0x00));

		responseByte = usart_getchar(progUsart);
		if (responseByte != 0x0d) return 0;
	}

	// Now comes the interesting part
	// We pour blocks of data stored in buffer[], ready to write to the AVR's flash
	// We only write in blocks of 512 bytes for the atXmega256.

	usart_putchar(progUsart, 0x42);                       //   Write data into flash, address increment is done automatically
	usart_putchar(progUsart, 0x02);
	usart_putchar(progUsart, byte(0x00));
	usart_putchar(progUsart, 0x46);
	
	for (uint16_t i = 0; i < pageSize; i++)
	{
		usart_putchar(progUsart,  pbuff.getByte(i) );
	}

	responseByte = usart_getchar(progUsart);
	if (responseByte != 0x0d) return 0;
	
	// If we are done uploading
	if ( doneUploading == true )
	{
		usart_putchar(progUsart, 0x4c);                       //   Leave programming mode
		responseByte = usart_getchar(progUsart);
		if (responseByte != 0x0d) return 0;
		
		usart_putchar(progUsart, 0x45);                       //   Exit bootloader
		responseByte = usart_getchar(progUsart);
		if (responseByte != 0x0d) return 0;
		
		resetMain();					//   Reset main processor
		
		programSucces = true;
		programming = false;
		progUsart->CTRLA = oldInterruptSetting;
	}
	// If we've reached here, then current packet has been programmed properly onto the xmega256
	// But please note: no sketch verification is done by this sketch!
	return 1;
}

bool LithneProgrammer::busyProgramming(){
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

void LithneProgrammer::preventHangup(){
	//  Timeout check to prevent code from staying in program mode for ever
	if ( busyProgramming() )
	{
		if ( millis()-lastPacketTimer > PROGRAM_TIMEOUT )
		{
			programming = false;
			// restore original interrupt settings for serial port
			progUsart->CTRLA = oldInterruptSetting;
			
			// reset the main proc in case we were in bootloader or something
			resetMain();
			Lithne.getNodeByID( REMOTE )->setAddress64( XBeeAddress64(0x00,0x00) );
		}
		// If we are programming, we also check whether we receive new packets on our requests - if not, we request again
		checkUploadProgress();
	}
}

LithneProgrammer lithneProgrammer;