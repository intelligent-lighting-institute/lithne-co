/**
 * \file *********************************************************************
 *
 * \brief USART Serial configuration
 *
 * Copyright (c) 2011 Atmel Corporation. All rights reserved.
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

#ifndef CONF_USART_SERIAL_H_INCLUDED
#define CONF_USART_SERIAL_H_INCLUDED

#define  USART_COMM0               USARTC0
#define  USART_COMM0_RX_Vect       USARTC0_RXC_vect
#define  USART_COMM0_DRE_Vect      USARTC0_DRE_vect
//#define  USART_COMM0_SYSCLK        SYSCLK_USART0
//#define  USART_COMM0_PORT          PORTC
//#define  USART_COMM0_PORT_SYSCLK   SYSCLK_PORT_C

#define  USART_COMM1               USARTE0
#define  USART_COMM1_RX_Vect       USARTE0_RXC_vect
#define  USART_COMM1_DRE_Vect      USARTE0_DRE_vect
//#define  USART_COMM1_SYSCLK        SYSCLK_USART0
//#define  USART_COMM1_PORT          PORTE
//#define  USART_COMM1_PORT_SYSCLK   SYSCLK_PORT_E

#define  USART_XBEE				   USARTC1
#define  USART_XBEE_RX_Vect        USARTC1_RXC_vect
#define  USART_XBEE_DRE_Vect       USARTC1_DRE_vect
//#define  USART_XBEE_SYSCLK         SYSCLK_USART0
//#define  USART_XBEE_PORT           PORTC
//#define  USART_XBEE_PORT_SYSCLK    SYSCLK_PORT_C

#define USART_SERIAL_BAUDRATE            115200
#define USART_SERIAL_CHAR_LENGTH         USART_CHSIZE_8BIT_gc
#define USART_SERIAL_PARITY              USART_PMODE_DISABLED_gc
#define USART_SERIAL_STOP_BIT            false


#endif /* CONF_USART_SERIAL_H_INCLUDED */
