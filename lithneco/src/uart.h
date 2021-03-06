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

#ifndef _UART_H_
#define _UART_H_



/*! \brief Called by CDC interface
 * Callback running when CDC device have received data
 */
void uart_rx_notify(uint8_t port);

/*! \brief Configures communication line
 *
 * \param cfg      line configuration
 */
static usb_cdc_line_coding_t default_cfg = (usb_cdc_line_coding_t) {115200l, CDC_STOP_BITS_1, CDC_PAR_NONE, 8};
	
void uart_config(USART_t * usart, usb_cdc_line_coding_t * cfg = &default_cfg);

/*! \brief Opens communication line
 */
void uart_open(USART_t * usart);

/*! \brief Closes communication line
 */
void uart_close(USART_t * usart);


/*! \brief Custom baud rate function, because library function does not work well for 115200
 */
void uart_set_baudrate(USART_t * usart, le32_t baud);


#endif // _UART_H_
