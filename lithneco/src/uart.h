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
void uart_config(uint8_t port, usb_cdc_line_coding_t * cfg);

/*! \brief Opens communication line
 */
void uart_open(USART_t * usartPtr);

/*! \brief Closes communication line
 */
void uart_close(uint8_t port);

/*! \brief Enables receive complete interrupt and data register interrupt for serial port
 */
void uart_start_interrupt(USART_t * usartPtr);

/*! \brief Disables all interrupts for serial port
 */
void uart_stop_interrupt(USART_t * usartPtr);


#endif // _UART_H_
