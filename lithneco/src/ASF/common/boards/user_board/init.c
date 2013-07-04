/**
 * \file
 *
 * \brief User board initialization template
 *
 */

#include <asf.h>
#include <user_board.h>

void board_init(void)
{
	/* This function is meant to contain board-specific initialization code
	 * for, e.g., the I/O pins. The initialization can rely on application-
	 * specific board configuration, found in conf_board.h.
	 */
	ioport_init(); // Initialize ioport service
	
	ioport_set_pin_dir(C_DEBUGLED, IOPORT_DIR_OUTPUT);
	
	ioport_set_pin_dir(COMM0_M2C, IOPORT_DIR_INPUT); // RX
	ioport_set_pin_high(COMM0_C2M); // TX
	ioport_set_pin_dir(COMM0_C2M, IOPORT_DIR_OUTPUT); // TX
	
	ioport_set_pin_dir(COMM1_M2C, IOPORT_DIR_INPUT); // RX
	ioport_set_pin_high(COMM1_C2M); // TX
	ioport_set_pin_dir(COMM1_C2M, IOPORT_DIR_OUTPUT); // TX
	
	ioport_set_pin_dir(XBEE_DOUT, IOPORT_DIR_INPUT); // Xbee -> me
	ioport_set_pin_high(XBEE_DIN); // me -> xbee
	ioport_set_pin_dir(XBEE_DIN, IOPORT_DIR_OUTPUT); // me -> xbee	
	
	ioport_set_pin_dir(nPBRD_PRESENT, IOPORT_DIR_INPUT);
	
	ioport_set_pin_dir(MAIN_nRST, IOPORT_DIR_INPUT); // main processor has a pull-up resistor, set as input to prevent pulling it down.
	ioport_set_pin_mode(MAIN_nRST, IOPORT_MODE_PULLUP);
	
	ioport_set_pin_dir(USBDET, IOPORT_DIR_INPUT);
		
	// CTS - "Clear to Send" this is a flow control pin that can be used to determine if there is data in the XBee input buffer ready to be read
	ioport_set_pin_dir(XBEE_nCTS, IOPORT_DIR_INPUT); 
	ioport_set_pin_high(XBEE_nCTS);
	// RTS - "Ready to Send" this is a flow control pin that can be used to tell the XBee to signal that the computer or microcontroller needs a break from reading serial data.
	ioport_set_pin_dir(XBEE_nRTS, IOPORT_DIR_OUTPUT); 
	ioport_set_pin_high(XBEE_nRTS);
	// DTR - "Data terminal ready" this is a flow control pin used to tell the XBee that the microcontroller or computer host is ready to communicate.
	ioport_set_pin_dir(XBEE_nDTR, IOPORT_DIR_OUTPUT); 
	ioport_set_pin_high(XBEE_nDTR);
	// RST - this pin can be used to reset the XBee. By default it is pulled high by the 10K resistor under the module. To reset, pull this pin low.'
	// Set as input here to prevent pulling it low.
	ioport_set_pin_dir(XBEE_nRESET, IOPORT_DIR_INPUT);
}