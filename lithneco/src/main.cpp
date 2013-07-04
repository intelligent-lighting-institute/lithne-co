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

extern "C"{
	#include <asf.h>
}

#include "conf_usb.h"
#include "ui.h"
#include "print.h"
#include "uart.h"
#include "lithneProgrammer.h"

#include <Lithne/Lithne.h>

volatile bool main_b_cdc_enable = false;
volatile uint8_t main_port_open;

/*! \brief Main function. Execution starts here.
 */
int main(void)
{
	irq_initialize_vectors();
	cpu_irq_enable();
	delay_init();

	// Initialize the sleep manager
	sleepmgr_init();

	sysclk_init();
	board_init();
	ui_init();
	ui_powerdown();

	// Start USB stack to authorize VBus monitoring
	udc_start();
	
	uart_open(&USART_COMM0);
	uart_open(&USART_COMM1);
	uart_open(&USART_XBEE);
	
	uart_start_interrupt(&USART_COMM0);
	uart_start_interrupt(&USART_COMM1);
	uart_start_interrupt(&USART_XBEE);
	
	Lithne.setSerial(Serial1);
	// The main loop manages only the power mode
	// because the USB management is done by interrupt
	while (true) {
		/*
		if ( Lithne.available() ){
			// Only process messages inside the processing scope
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
			
		}*/
	}
}

void main_cdc_rx_notify(uint8_t port){
	// Byte received on USB on port in argument
	ui_com_rx_notify(port);
	while(udi_cdc_multi_is_rx_ready(port)){
		int c = udi_cdc_multi_getc(port);			
		usart_putchar(main_port_to_usart(port), c);
	}
}

void main_suspend_action(void)
{
	ui_powerdown();
}

void main_resume_action(void)
{
	ui_wakeup();
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
	// Serial settings received from USB
	USART_t * usart = main_port_to_usart(port);
	
	if(usart == NULL){
		return;
	}
	uart_config(usart, cfg)	;
}

void main_cdc_set_dtr(uint8_t port, bool b_enable)
{
	if (b_enable) {
		// Host terminal has open COM
		main_port_open |= 1 << port;
		if(port==0){
			lithneProgrammer.resetMain();
		}
		ui_com_open(port);
		main_cdc_open(port);
	}else{
		// Host terminal has close COM
		main_port_open &= ~(1 << port);
		ui_com_close(port);
		main_cdc_close(port);
	}
}


void main_cdc_open(uint8_t port)
{
	uart_open(main_port_to_usart(port));
}

void main_cdc_close(uint8_t port)
{
	uart_close(main_port_to_usart(port));
}

USART_t * main_port_to_usart(uint8_t port)
{
	USART_t * usart = NULL;
	switch(port){
		case 0:
		usart = &USART_COMM0;
		break;
		case 1:
		usart = &USART_COMM1;
		break;
		case 2:
		usart = &USART_XBEE;
		break;
	}
	return usart;
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
