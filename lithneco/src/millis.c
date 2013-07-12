/*
 * millis.cpp
 *
 * Created: 11-7-2013 15:48:27
 *  Author: Elco
 */ 

#include "asf.h"
#include "millis.h"

// We don't use the Arduino init(), because it initializes system clocks, pwm clock and pins.
// We define our own timer interrupt here.

void millis_init(void){
	PR.PRPD &= ~(PR_TC0_bm);	// enable TCD0 module
	PR.PRPD &= ~(PR_TC1_bm);	// enable TCD1 module
	PR.PRGEN &= ~(PR_EVSYS_bm);
		
	TCD0.PER		= 0xFFFF;
	TCD0.CTRLB		= TC_WGMODE_NORMAL_gc;
	TCD0.CTRLD		= 0; // no event action
	EVSYS.CH0MUX = EVSYS_CHMUX_TCD0_OVF_gc; // set event event channel 0 to TCD0 overflow
	
	TCD1.PER		= 0xFFFF;
	TCD1.CTRLB		= TC_WGMODE_NORMAL_gc;
	TCD1.CTRLD		= TC0_EVDLY_bm | TC_EVACT_CAPT_gc; // set event action to input capture, delay event source by 1
	
	// start the timers by setting the clock sources
	TCD0.CTRLA		= TC_CLKSEL_DIV1024_gc; // timer 0 clock source = 32250 Hz
	TCD1.CTRLA		= TC_EVSEL_CH0_gc; // timer 1 clock source = event channel 0
}

void delay(unsigned long ms){
	delay_ms(ms);
}

unsigned long millis(void){
	// get 32 bit value from combination of both timers. Save as 64 bit to be able to multiply by 2
	volatile uint64_t timers_combined = TCD1.CNT;
	timers_combined = (timers_combined<<16) + TCD0.CNT; 
	return ((timers_combined*2) / 65); // scale to milliseconds
}