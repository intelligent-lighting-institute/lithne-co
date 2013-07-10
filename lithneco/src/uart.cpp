/**
 * \file
 *
 * \brief UART functions
 *
 * Copyright (c) 2009-2012 Atmel Corporation. All rights reserved.
 *
 *
 * \asf_license_stop
 */

extern "C"{
#include "asf.h"	
}

#include "conf_board.h"
#include "uart.h"
#include "main.h"
#include "ui.h"
#include "print.h"
#include <HardwareSerial.h>

// when xbeeDirect is true, the xbee serial port is disconnected from the main processor and connected to USB
extern volatile bool xbeeDirect;

void uart_open(USART_t * usart)
{
	uart_config(usart); // set to default settings
	sysclk_enable_peripheral_clock(usart);
	usart_tx_enable(usart);
	usart_rx_enable(usart);
}

void uart_close(USART_t * usart)
{
	sysclk_disable_peripheral_clock(usart);
	// Disable interrupts
	usart->CTRLA = 0;
	// Close RS232 communication
	usart->CTRLB = 0;
}

void uart_config(USART_t * usart, usb_cdc_line_coding_t * cfg)
{
	uint8_t reg_ctrlc;

	reg_ctrlc = USART_CMODE_ASYNCHRONOUS_gc;

	switch (cfg->bCharFormat) {
		case CDC_STOP_BITS_2:
		reg_ctrlc |= USART_SBMODE_bm;
		break;
		case CDC_STOP_BITS_1:
		default:
		// Default stop bit = 1 stop bit
		break;
	}

	switch (cfg->bParityType) {
		case CDC_PAR_EVEN:
		reg_ctrlc |= USART_PMODE_EVEN_gc;
		break;
		case CDC_PAR_ODD:
		reg_ctrlc |= USART_PMODE_ODD_gc;
		break;
		default:
		case CDC_PAR_NONE:
		reg_ctrlc |= USART_PMODE_DISABLED_gc;
		break;
	}

	switch (cfg->bDataBits) {
		case 5:
		reg_ctrlc |= USART_CHSIZE_5BIT_gc;
		break;
		case 6:
		reg_ctrlc |= USART_CHSIZE_6BIT_gc;
		break;
		case 7:
		reg_ctrlc |= USART_CHSIZE_7BIT_gc;
		break;
		default:
		case 8:
		reg_ctrlc |= USART_CHSIZE_8BIT_gc;
		break;
		case 9:
		reg_ctrlc |= USART_CHSIZE_9BIT_gc;
		break;
	}
		
	sysclk_enable_peripheral_clock(usart);
	// Set configuration
	usart->CTRLC = reg_ctrlc;
	// Update baudrate
	uart_set_baudrate(usart, cfg->dwDTERate);
	// debugMessage("baud %lu", cfg->dwDTERate);
}

void uart_set_baudrate(USART_t * usart, le32_t baud){
	le32_t UART_BSEL_VALUE;
	le32_t UART_BSCALE_VALUE;
	if(0){//baud == 115200){
		// library function does not work well for 115200
		UART_BSEL_VALUE = 1047;
		UART_BSCALE_VALUE = -6;
		usart->BAUDCTRLA = (UART_BSEL_VALUE & USART_BSEL_gm);
		usart->BAUDCTRLB = ((UART_BSCALE_VALUE << USART_BSCALE_gp) & USART_BSCALE_gm) | ((UART_BSEL_VALUE >> 8) & ~USART_BSCALE_gm);
	}
	else{
		usart_set_baudrate(usart, baud, sysclk_get_cpu_hz());
	}
}


// Defined in HardwareSerial.cpp:
/*
extern ring_buffer Serialrx_buffer; // COMM0 RX
extern ring_buffer Serial1rx_buffer; // XBEE_RX
extern ring_buffer Serial4rx_buffer; // COMM1 RX
void store_char(unsigned char c, ring_buffer *rx_buffer);
*/

// Interrupt vectors for COMM0
ISR(USART_COMM0_RX_Vect)
{
	int val;

	// Data received
	ui_com_tx_start();

	if (0 != (USART_COMM0.STATUS & (USART_FERR_bm | USART_BUFOVF_bm))) {
		udi_cdc_multi_signal_framing_error(0);
		ui_com_error();
	}

	// Transfer UART RX fifo to CDC TX
	val = USART_COMM0.DATA;
	
	if (!udi_cdc_multi_is_tx_ready(0)) {
		// Fifo full
		udi_cdc_multi_signal_overrun(0);
		ui_com_overflow();
		}
	else{
		udi_cdc_multi_putc(0, val);
	}
	ui_com_tx_stop();
}

ISR(USART_COMM0_DRE_Vect)
{
	// TX register empty, check USB for new data
	if (udi_cdc_multi_is_rx_ready(0)) {
		// Transmit next data
		ui_com_rx_start();
		USART_COMM0.DATA = udi_cdc_multi_getc(0);
	}
	else {
		// Fifo empty then Stop UART transmission
		USART_COMM0.CTRLA = (register8_t) USART_RXCINTLVL_HI_gc |
		(register8_t) USART_DREINTLVL_OFF_gc;
		ui_com_rx_stop();
	}
}

// prototype for orignal Arduino ISR body
void arduino_ISR_body_for_USARTC1_RXC_vect();

// Interrupt vectors for COMM1
ISR(USART_XBEE_RX_Vect)
{
	int val;
	if(main_cdc_is_open(1)){ // xbee linked to USB
		// send data received from XBEE straight to USB port 1
		ui_com_tx_start();

		if (0 != (USART_XBEE.STATUS & (USART_FERR_bm | USART_BUFOVF_bm))) {
			udi_cdc_multi_signal_framing_error(1);
			ui_com_error();
		}

		// Transfer UART RX fifo to CDC TX
		val = USART_XBEE.DATA;
		if (!udi_cdc_multi_is_tx_ready(1)) {
			// Fifo full
			udi_cdc_multi_signal_overrun(1);
			ui_com_overflow();
		}
		else{
			udi_cdc_multi_putc(1, val);
		}
		ui_com_tx_stop();
	}
	else{
		// place data in Arduino's serial rx buffer by using original arduino ISR
		arduino_ISR_body_for_USARTC1_RXC_vect();	
	}
}

ISR(USART_XBEE_DRE_Vect)
{
	// Data send
	if(main_cdc_is_open(1)){
		if (udi_cdc_multi_is_rx_ready(1)) {
			// Transmit next data
			ui_com_rx_start();
			USART_XBEE.DATA = udi_cdc_multi_getc(1);
			return; // return, keep DRE interrupt enabled
		}
	}
	// Fifo empty, stop UART transmission
	usart_set_dre_interrupt_level(&USART_XBEE, USART_INT_LVL_OFF);
	ui_com_rx_stop();
}

// prototype for original Arduino ISR body
void arduino_ISR_body_for_USARTE0_RXC_vect();

// Interrupt vectors for COMM1
ISR(USART_COMM1_RX_Vect)
{
	// place data in Arduino's serial rx buffer by using original arduino ISR
	arduino_ISR_body_for_USARTE0_RXC_vect();
}

ISR(USART_COMM1_DRE_Vect)
{
	// This interrupt should not be enabled. Turn it off again. http://www.youtube.com/watch?v=Z86V_ICUCD4
	// this will keep going back into interrupt when no new data is send in this interrupt.
	usart_set_dre_interrupt_level(&USART_COMM1, USART_INT_LVL_OFF);
}