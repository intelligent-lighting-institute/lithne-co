/**
 * \file
 *
 * \brief User board definition template
 *
 */

 /* This file is intended to contain definitions and configuration details for
 * features and devices that are available on the board, e.g., frequency and
 * startup time for an external crystal, external memory devices, LED and USART
 * pins.
 */

#ifndef USER_BOARD_H
#define USER_BOARD_H

#define BOARD USER_BOARD

#include <conf_board.h>

#define VIN_MEAS	IOPORT_CREATE_PIN(PORTA, 4)

#define C_DEBUGLED	IOPORT_CREATE_PIN(PORTB, 2)
#define XBEE_nCTS	IOPORT_CREATE_PIN(PORTB, 3)
#define XBEE_nRTS	IOPORT_CREATE_PIN(PORTC, 0)
#define XBEE_nDTR	IOPORT_CREATE_PIN(PORTC, 1)
#define COMM0_M2C	IOPORT_CREATE_PIN(PORTC, 2)
#define COMM0_C2M	IOPORT_CREATE_PIN(PORTC, 3)
#define XBEE_nRESET	IOPORT_CREATE_PIN(PORTC, 4)
#define MAIN_nRST	IOPORT_CREATE_PIN(PORTC, 5)
#define XBEE_DOUT	IOPORT_CREATE_PIN(PORTC, 6)
#define XBEE_DIN	IOPORT_CREATE_PIN(PORTC, 7)

#define CXP_GP0		IOPORT_CREATE_PIN(PORTD, 0)
#define CXP_GP1		IOPORT_CREATE_PIN(PORTD, 1)
#define CXP_RXD		IOPORT_CREATE_PIN(PORTD, 2)
#define CXP_TXD		IOPORT_CREATE_PIN(PORTD, 3)
#define CXP_GP2		IOPORT_CREATE_PIN(PORTD, 4)
#define CXP_GP3		IOPORT_CREATE_PIN(PORTD, 5)

//#define USB_DMIN	IOPORT_CREATE_PIN(PORTD, 6)
//#define USB_DPLUS	IOPORT_CREATE_PIN(PORTD, 7)

#define USBDET	IOPORT_CREATE_PIN(PORTE, 0)
#define nPBRD_PRESENT	IOPORT_CREATE_PIN(PORTE, 1)
#define COMM1_M2C	IOPORT_CREATE_PIN(PORTE, 2)
#define COMM1_C2M	IOPORT_CREATE_PIN(PORTE, 3)

#endif // USER_BOARD_H

