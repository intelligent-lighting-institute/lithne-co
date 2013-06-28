/**
 * \file
 *
 * \brief UART functions
 *
 * Copyright (c) 2009-2012 Atmel Corporation. All rights reserved.
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

#ifndef _UART_H_
#define _UART_H_

typedef struct{
	USART_t *const usartPtr;
	uint8_t sysClk;
	PORT_t *const portPtr;
	uint8_t pinRx;
	uint8_t pinTx;
	enum sysclk_port_id sysClkPort;
} usartSpecification;

static const usartSpecification COMM0_SPEC = {
	&USART_COMM0,
	USART_COMM0_SYSCLK,
	&USART_COMM0_PORT,
	USART_COMM0_PORT_PIN_TX,
	USART_COMM0_PORT_PIN_RX,
	USART_COMM0_PORT_SYSCLK
};

static const usartSpecification COMM1_SPEC = {
	&USART_COMM1,
	USART_COMM1_SYSCLK,
	&USART_COMM1_PORT,
	USART_COMM1_PORT_PIN_TX,
	USART_COMM1_PORT_PIN_RX,
	USART_COMM1_PORT_SYSCLK
};

static const usartSpecification XBEE_SPEC = {
	&USART_XBEE,
	USART_XBEE_SYSCLK,
	&USART_XBEE_PORT,
	USART_XBEE_PORT_PIN_TX,
	USART_XBEE_PORT_PIN_RX,
	USART_XBEE_PORT_SYSCLK
};

/*! \brief Called by CDC interface
 * Callback running when CDC device have received data
 */
void uart_rx_notify(usartSpecification usartSpec);

/*! \brief Configures communication line
 *
 * \param cfg      line configuration
 */
void uart_config(usartSpecification usartSpec, usb_cdc_line_coding_t * cfg);

/*! \brief Opens communication line
 */
void uart_open(usartSpecification usartSpec);

/*! \brief Closes communication line
 */
void uart_close(usartSpecification usartSpec);

#endif // _UART_H_
