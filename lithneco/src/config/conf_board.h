/**
 * \file
 *
 * \brief User board configuration template
 *
 */

#ifndef CONF_BOARD_H
#define CONF_BOARD_H

#define  USART_COMM0               USARTC0
#define  USART_COMM0_RX_Vect       USARTC0_RXC_vect
#define  USART_COMM0_DRE_Vect      USARTC0_DRE_vect
#define  USART_COMM0_SYSCLK        SYSCLK_USART0
#define  USART_COMM0_PORT          PORTC
#define  USART_COMM0_PORT_PIN_TX   (1<<3)  // PC3 (TXC0)
#define  USART_COMM0_PORT_PIN_RX   (1<<2)  // PC2 (RXC0)
#define  USART_COMM0_PORT_SYSCLK   SYSCLK_PORT_C

#define  USART_COMM1               USARTE0
#define  USART_COMM1_RX_Vect       USARTE0_RXC_vect
#define  USART_COMM1_DRE_Vect      USARTE0_DRE_vect
#define  USART_COMM1_SYSCLK        SYSCLK_USART0
#define  USART_COMM1_PORT          PORTE
#define  USART_COMM1_PORT_PIN_TX   (1<<3)  // PE3 (TXE0)
#define  USART_COMM1_PORT_PIN_RX   (1<<2)  // PE2 (RXE0)
#define  USART_COMM1_PORT_SYSCLK   SYSCLK_PORT_E

#define  USART_XBEE				   USARTC1
#define  USART_XBEE_RX_Vect        USARTC1_RXC_vect
#define  USART_XBEE_DRE_Vect       USARTC1_DRE_vect
#define  USART_XBEE_SYSCLK         SYSCLK_USART0
#define  USART_XBEE_PORT           PORTC
#define  USART_XBEE_PORT_PIN_TX    (1<<7)  // PC7 (TXC1)
#define  USART_XBEE_PORT_PIN_RX    (1<<6)  // PC2 (RXC1)
#define  USART_XBEE_PORT_SYSCLK    SYSCLK_PORT_C

#define BOARD USER_BOARD
#endif // CONF_BOARD_H
