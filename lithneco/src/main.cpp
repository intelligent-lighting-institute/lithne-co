/**
 * \file
 *
 * \brief CDC Application Main functions
 *
 * Copyright (c) 2012 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#include "lithneProgrammer.h"
#undef F_CPU

extern "C"{
	#include <asf.h>
}

#define MAX_NODES 1

#include "conf_usb.h"
#include "ui.h"
#include "print.h"
#include "uart.h"

extern "C"{
	#include "millis.h"
}


#include <Lithne/Lithne.h>

volatile bool main_b_cdc_enable = false;
volatile uint8_t main_port_open;

/*! \brief Main function. Execution starts here.
 */
int main(void)
{
	
	board_init();
	ui_init();
		
	lithneProgrammer.setMainReset(true);
	lithneProgrammer.setXbeeReset(true);
	
	irq_initialize_vectors();
	cpu_irq_enable();
		
	sysclk_init();
	
	delay_init(sysclk_get_cpu_hz());
	millis_init();	
	
	// Start USB stack to authorize VBus monitoring
	udc_start();
	lithneProgrammer.setMainReset(false);
	lithneProgrammer.setXbeeReset(false);
	
	// USART_COMM0 is a directly forwarded serial to the main processor, handled by interrupts.
	// The port is opened and interrupts are enabled on USB connect on port 0
	
	// USART_COMM1 connects to the main processor for xbee rx/tx forwarding.
	// USART_XBEE connects to the xbee module
	// Messages outside the programming scope are forwarded between these ports in the main loop.
	
	// When USB port 1 in opened, the main processor is held in reset and USB port 1 is transparently coupled to the XBEE.
	// Only in that case the RXE interrupt is enabled.
	
	// open both COM ports for xbee forward with default settings
	uart_open(&USART_COMM1);
	uart_open(&USART_XBEE);
		
	usart_set_rx_interrupt_level(&USART_COMM1, USART_INT_LVL_HI);
	usart_set_rx_interrupt_level(&USART_XBEE, USART_INT_LVL_HI);
	
	
	debugMessage("free memory before Lithne init\t%d", freeRam());	
	
	lithneProgrammer.init(&serialCo2MainSerial);
	Lithne.setSerial(serialCo2Xbee);
		
	while (true) {
		if(main_cdc_is_open(1)){
			// XBEE is directly linked to USB. Skip Lithne forwarding/programming
			continue;
		}
		
		if (Lithne.available() ){
			// Only process messages inside the programming scope
			if ( Lithne.getScope() == lithneProgrammingScope ){
				lithneProgrammer.updateRemoteAddress();
			}
				
			// The programming function receives all data packets containing the program to be written
			if ( Lithne.getFunction() == fProgramming )
			{
				lithneProgrammer.processPacket();
			}
			// Check-in Function
			else if (Lithne.getFunction() == fCheckingIn && !lithneProgrammer.busyProgramming())
			{
				lithneProgrammer.processCheckin();
			}
			// Node Name Functions - Empty messages are a request, Messages with content set the node name
			else if (Lithne.getFunction() == fNodeName && !lithneProgrammer.busyProgramming())
			{
				lithneProgrammer.processNodeName();	
			}
			// LastUpload Functions - Empty messages are a request, Messages with content set the time of last upload
			else if (Lithne.getFunction() == fLastUpload && !lithneProgrammer.busyProgramming())
			{
				lithneProgrammer.processLastUpload();
			}
			// File Name Functions - Empty messages are a request, Messages with content set the file name
			else if (Lithne.getFunction() == fFileName && !lithneProgrammer.busyProgramming())
			{
				lithneProgrammer.processFileName();
			}
			// reset the main processor
			else if (Lithne.getFunction() == fResetMain && !lithneProgrammer.busyProgramming())
			{
				lithneProgrammer.resetMain();
			}
			// Kill the main processor for a longer period of time, or turn it back on again
			else if (Lithne.functionIs("killMain") && !lithneProgrammer.busyProgramming())
			{
				lithneProgrammer.processKill();
			}
			// If the message is not in the 'programming scope' this is a regular incoming Lithne message for the user (main proc) - Forward all bytes to main processor
			else
			{
				for(int i=0; i < Lithne.getXBeePacketSize(); i++)                               //   send data in XBee packet straight through to the main processor
				{
					serialCo2MainXbee.write( Lithne.getXBeePacket()[i] );
				}
				serialCo2MainXbee.flush();
			}
		}
		
		// forward communication from main processor to xbee
		if ( !lithneProgrammer.busyProgramming() && serialCo2MainXbee.available() )
		{
			uint8_t byte_to_pass_on = serialCo2MainXbee.read();
			Lithne.sendBytePublic(byte_to_pass_on, false);
			while (serialCo2MainXbee.available())
			{
				byte_to_pass_on = serialCo2MainXbee.read();
				Lithne.sendBytePublic(byte_to_pass_on, true);
			}
		}		
		
		lithneProgrammer.preventHangup();
	}
}

void main_cdc_rx_notify(uint8_t port){
	// Byte received on USB on port in argument
	ui_com_rx_notify(port);
	switch(port){
		case 0:
			// send directly to serial port
			while(udi_cdc_multi_is_rx_ready(port)){
				int c = udi_cdc_multi_getc(port);
				if(!lithneProgrammer.busyProgramming()){ // don't send anything to the serial port when the programmer is busy
					usart_putchar(&USART_COMM0, c);
				}
			}
			break;
		case 1:
			while(udi_cdc_multi_is_rx_ready(port)){
				int c = udi_cdc_multi_getc(port);
				usart_putchar(&USART_XBEE, c);
			}
			break;
		default:
			return; // unknown port, do nothing
	}
}


void main_sof_action(void)
{
	if (!main_b_cdc_enable)
		return;
	ui_process(udd_get_frame_number());
}

bool main_cdc_enable(uint8_t port)
{
	main_b_cdc_enable = true;
	main_port_open = 0;	
	return true;
}

void main_cdc_disable(uint8_t port)
{
	main_b_cdc_enable = false;
}

void main_cdc_config(uint8_t port, usb_cdc_line_coding_t * cfg)
{
	switch(port){
		case 0: // COMM0
			uart_config(&USART_COMM0, cfg);
		break;
		case 1: // XBEE direct TODO
			uart_config(&USART_XBEE, cfg);
		break;
	}
}

void main_cdc_set_dtr(uint8_t port, bool b_enable)
{
	if (b_enable) {
		// Host terminal has open COM
		main_port_open |= 1 << port;	
		ui_com_open(port);
		main_cdc_open(port);
	}else{
		// Host terminal has close COM
		main_port_open &= ~(1 << port);
		ui_com_close(port);
		main_cdc_close(port);
	}
}

bool main_cdc_is_open(uint8_t port){
	return main_port_open & (1 << port);
}

void main_cdc_open(uint8_t port)
{
	switch(port){
		case 0: // COMM0
			lithneProgrammer.resetMain();
			uart_open(&USART_COMM0);
			usart_set_rx_interrupt_level(&USART_COMM0, USART_INT_LVL_HI); // receive interrupt
		break;
		case 1: // Switched to XBEE direct: close COMM1
			uart_close(&USART_COMM1); // close COMM1, this also disables interrupts						
			usart_set_rx_interrupt_level(&USART_XBEE, USART_INT_LVL_HI);					
		break;						
	}
}

void main_cdc_close(uint8_t port)
{
	switch(port){
		case 0: // COMM0		
			uart_close(&USART_COMM0); // close COMM0, this also disables interrupts
		break;
		case 1: // Switch back to XBEE forwarding/programming, open COMM1
			uart_open(&USART_COMM1); // restore communication with main processor on COMM1
			usart_set_rx_interrupt_level(&USART_COMM1, USART_INT_LVL_HI);
		break;
	}
}

int freeRam () {
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}


/**
 * \mainpage ASF USB Device CDC
 *
 * \section intro Introduction
 * This example shows how to implement a USB Device CDC
 * on Atmel MCU with USB module.
 * The application note AVR4907 provides more information
 * about this implementation.
 *
 * \section desc Description of the Communication Device Class (CDC)
 * The Communication Device Class (CDC) is a general-purpose way to enable all
 * types of communications on the Universal Serial Bus (USB).
 * This class makes it possible to connect communication devices such as
 * digital telephones or analog modems, as well as networking devices
 * like ADSL or Cable modems.
 * While a CDC device enables the implementation of quite complex devices,
 * it can also be used as a very simple method for communication on the USB.
 * For example, a CDC device can appear as a virtual COM port, which greatly
 * simplifies application development on the host side.
 *
 * \section startup Startup
 * The example is a implementation of 7 USB CDC interface.
 *
 * In this example, we will use a PC as a USB host:
 * - Connect the application to a USB host (e.g. a PC)
 *   with a mini-B (embedded side) to A (PC host side) cable.
 * The application will behave as 7 virtual COM (see Windows Device Manager).
 * - Open a HyperTerminal on a virtual COM port
 * - Ignore virtual COM port configuration because this does not manage a
 *   true UART
 * - Type the character 'p' in the HyperTerminal and it will display the port
 *   number"PORTX" corresponding at virtual COM port opened.
 *
 * \note
 * On the first connection of the board on the PC,
 * the operating system will detect a new peripheral:
 * - This will open a new hardware installation window.
 * - Choose "No, not this time" to connect to Windows Update for this
 *   installation
 * - click "Next"
 * - When requested by Windows for a driver INF file, select the
 *   atmel_devices_cdc.inf file in the directory indicated in the Atmel Studio
 *   "Solution Explorer" window.
 * - click "Next"
 *
 * \copydoc UI
 *
 * \section example About example
 *
 * The example uses the following module groups:
 * - Basic modules:
 *   Startup, board, clock, interrupt, power management
 * - USB Device stack and CDC modules:
 *   <br>services/usb/
 *   <br>services/usb/udc/
 *   <br>services/usb/class/cdc/
 * - Specific implementation:
 *    - main.c,
 *      <br>initializes clock
 *      <br>initializes interrupt
 *      <br>manages UI
 *      <br>
 *    - specific implementation for each target "./examples/product_board/":
 *       - conf_foo.h   configuration of each module
 *       - ui.c        implement of user's interface (leds,buttons...)
 */

ISR(BADISR_vect){
	ioport_set_pin_level(C_DEBUGLED, true);
	delay_ms(50);
	ioport_set_pin_level(C_DEBUGLED, false);
	delay_ms(50);
};