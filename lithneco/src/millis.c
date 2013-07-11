/*
 * millis.cpp
 *
 * Created: 11-7-2013 15:48:27
 *  Author: Elco
 */ 

#include "asf.h"
#include "millis.h"

volatile unsigned long millis_count;

// We don't use the Arduino init(), because it initializes system clocks, pwm clock and pins.
// We define our own timer interrupt here.

void millis_init(void){
	PR.PRPD &= ~(PR_TC0_bm);	
	millis_count = 0;
	TCD0.PER   = 4000;
	TCD0.CTRLB    = ( TCD0.CTRLB & ~TC0_WGMODE_gm ) | TC_WGMODE_NORMAL_gc;
	TCD0.INTCTRLA = TC_OVFINTLVL_LO_gc;
	TCD0.CTRLA = TC_CLKSEL_DIV8_gc;
}

void delay(unsigned long ms){
	delay_ms(ms);
}

unsigned long millis(void){
	return millis_count;
}

ISR(TCD0_OVF_vect)
{
	++millis_count;
}

