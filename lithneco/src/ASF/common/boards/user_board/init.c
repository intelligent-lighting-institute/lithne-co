/**
 * \file
 *
 * \brief User board initialization template
 *
 */

#include <asf.h>
#include <user_board.h>

void board_init(void)
{
	/* This function is meant to contain board-specific initialization code
	 * for, e.g., the I/O pins. The initialization can rely on application-
	 * specific board configuration, found in conf_board.h.
	 */
	sysclk_init(); // Initialize system clock as defined in config/conf_clock.h
	ioport_init(); // Initialize ioport service
	lithne_board_init();  // Initialize board pins for Lithne board			
}

void lithne_board_init(void){
	 ioport_set_pin_dir(C_DEBUGLED, IOPORT_DIR_OUTPUT);
}