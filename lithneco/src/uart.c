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


void uart_start_interrupt(USART_t * usart)
{
	// Enable interrupt with priority higher than USB
	usart_set_dre_interrupt_level(usart, USART_DREINTLVL_HI_gc);
	usart_set_rx_interrupt_level(usart, USART_RXCINTLVL_HI_gc);
}

void uart_stop_interrupt(USART_t * usart)
{
	// Disable interrupts set by uart_start_interrupt()
	usart_set_dre_interrupt_level(usart, USART_DREINTLVL_OFF_gc);
	usart_set_rx_interrupt_level(usart, USART_RXCINTLVL_OFF_gc);
}

void uart_open(USART_t * usart){
	sysclk_enable_peripheral_clock(usart);
	usart_set_mode(usart, USART_CMODE_ASYNCHRONOUS_gc);
	usart_format_set(usart, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false); // Set usart to 8 bit, no parity, one stop bit
	
	usart_set_baudrate(usart, 115200, sysclk_get_cpu_hz());
	
	usart_tx_enable(usart);
	usart_rx_enable(usart);
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
	uint8_t val;

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
	}else{
		udi_cdc_multi_putc(0, val);
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
	uint8_t val;

	// Data received
	ui_com_tx_start();

	if (0 != (USART_COMM1.STATUS & (USART_FERR_bm | USART_BUFOVF_bm))) {
		udi_cdc_multi_signal_framing_error(1);
		ui_com_error();
	}

	// Transfer UART RX fifo to CDC TX
	val = USART_COMM1.DATA;
	if (!udi_cdc_multi_is_tx_ready(1)) {
		// Fifo full
		udi_cdc_multi_signal_overrun(1);
		ui_com_overflow();
		}else{
		udi_cdc_multi_putc(1, val);
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
	uint8_t val;

	// Data received
	ui_com_tx_start();

	if (0 != (USART_XBEE.STATUS & (USART_FERR_bm | USART_BUFOVF_bm))) {
		udi_cdc_multi_signal_framing_error(2);
		ui_com_error();
	}

	// Transfer UART RX fifo to CDC TX
	val = USART_XBEE.DATA;
	if (!udi_cdc_multi_is_tx_ready(2)) {
		// Fifo full
		udi_cdc_multi_signal_overrun(2);
		ui_com_overflow();
		}else{
		udi_cdc_multi_putc(2, val);
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