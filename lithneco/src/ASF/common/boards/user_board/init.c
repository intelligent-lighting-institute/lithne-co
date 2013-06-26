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
	ioport_init(); // Initialize ioport service
	
	ioport_set_pin_dir(C_DEBUGLED, IOPORT_DIR_OUTPUT);
}