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

#include <asf.h>
#include "conf_board.h"
#include "uart.h"
#include "main.h"
#include "ui.h"


void uart_start_interrupt(USART_t * usartPtr)
{
	// Enable interrupt with priority higher than USB
	usartPtr->CTRLA = (register8_t) USART_RXCINTLVL_HI_gc | (register8_t) USART_DREINTLVL_OFF_gc;	
}

void uart_stop_interrupt(USART_t * usartPtr)
{
	// Enable interrupt with priority higher than USB
	usartPtr->CTRLA = 0x00;
}

void uart_open(USART_t * usartPtr){
	const usart_serial_options_t usart_options = {
		.baudrate = USART_SERIAL_BAUDRATE,
		.charlength = USART_SERIAL_CHAR_LENGTH,
		.paritytype = USART_SERIAL_PARITY,
		.stopbits = USART_SERIAL_STOP_BIT
	};
	
	usart_serial_init(usartPtr, &usart_options);
}

/*
void uart_close(usartSpecification usartSpec)
{
	sysclk_disable_module(usartSpec.sysClkPort, usartSpec.sysClk);
	// Disable interrupts
	usartSpec.usartPtr->CTRLA = 0;
	// Close RS232 communication
	usartSpec.usartPtr->CTRLB = 0;
}


void uart_rx_notify(usartSpecification usartSpec)
{
	// If UART is open
	if (usartSpec.usartPtr->CTRLA != 0) {
		// Enable UART TX interrupt to send values
		usartSpec.usartPtr->CTRLA = (register8_t) USART_RXCINTLVL_HI_gc | (register8_t)
		USART_DREINTLVL_HI_gc;
	}
}*/

// Interrupt vectors for COMM0
ISR(USART_COMM0_RX_Vect)
{
	uint8_t value;

	// Data received
	ui_com_tx_start();

	if (0 != (USART_COMM0.STATUS & (USART_FERR_bm | USART_BUFOVF_bm))) {
		udi_cdc_multi_signal_framing_error(0);
		ui_com_error();
	}

	// Transfer UART RX fifo to CDC TX
	value = USART_COMM0.DATA;
	if (!udi_cdc_multi_is_tx_ready(0)) {
		// Fifo full
		udi_cdc_multi_signal_overrun(0);
		ui_com_overflow();
	}else{
		udi_cdc_multi_putc(0, value);
	}
	ui_com_tx_stop();
}

ISR(USART_COMM0_DRE_Vect)
{
	// Data send
	if (udi_cdc_multi_is_rx_ready(0)) {
		// Transmit next data
		ui_com_rx_start();
		USART_COMM0.DATA = udi_cdc_multi_getc(0);
	} else {
		// Fifo empty then Stop UART transmission
		USART_COMM0.CTRLA = (register8_t) USART_RXCINTLVL_HI_gc |
				(register8_t) USART_DREINTLVL_OFF_gc;
		ui_com_rx_stop();
	}
}

// Interrupt vectors for COMM1
ISR(USART_COMM1_RX_Vect)
{
	uint8_t value;

	// Data received
	ui_com_tx_start();

	if (0 != (USART_COMM1.STATUS & (USART_FERR_bm | USART_BUFOVF_bm))) {
		udi_cdc_multi_signal_framing_error(1);
		ui_com_error();
	}

	// Transfer UART RX fifo to CDC TX
	value = USART_COMM1.DATA;
	if (!udi_cdc_multi_is_tx_ready(1)) {
		// Fifo full
		udi_cdc_multi_signal_overrun(1);
		ui_com_overflow();
		}else{
		udi_cdc_multi_putc(1, value);
	}
	ui_com_tx_stop();
}

ISR(USART_COMM1_DRE_Vect)
{
	// Data send
	if (udi_cdc_multi_is_rx_ready(1)) {
		// Transmit next data
		ui_com_rx_start();
		USART_COMM1.DATA = udi_cdc_multi_getc(1);
		} else {
		// Fifo empty then Stop UART transmission
		USART_COMM1.CTRLA = (register8_t) USART_RXCINTLVL_HI_gc |
		(register8_t) USART_DREINTLVL_OFF_gc;
		ui_com_rx_stop();
	}
}

// Interrupt vectors for COMM1
ISR(USART_XBEE_RX_Vect)
{
	uint8_t value;

	// Data received
	ui_com_tx_start();

	if (0 != (USART_XBEE.STATUS & (USART_FERR_bm | USART_BUFOVF_bm))) {
		udi_cdc_multi_signal_framing_error(2);
		ui_com_error();
	}

	// Transfer UART RX fifo to CDC TX
	value = USART_XBEE.DATA;
	if (!udi_cdc_multi_is_tx_ready(2)) {
		// Fifo full
		udi_cdc_multi_signal_overrun(2);
		ui_com_overflow();
		}else{
		udi_cdc_multi_putc(2, value);
	}
	ui_com_tx_stop();
}

ISR(USART_XBEE_DRE_Vect)
{
	// Data send
	if (udi_cdc_multi_is_rx_ready(2)) {
		// Transmit next data
		ui_com_rx_start();
		USART_XBEE.DATA = udi_cdc_multi_getc(2);
		} else {
		// Fifo empty then Stop UART transmission
		USART_XBEE.CTRLA = (register8_t) USART_RXCINTLVL_HI_gc |
		(register8_t) USART_DREINTLVL_OFF_gc;
		ui_com_rx_stop();
	}
}