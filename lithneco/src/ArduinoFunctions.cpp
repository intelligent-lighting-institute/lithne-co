/*
 * Copyright 2012-2013 BrewPi/Elco Jacobs.
 *
 * This file is part of BrewPi.
 * 
 * BrewPi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * BrewPi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with BrewPi.  If not, see <http://www.gnu.org/licenses/>.
 */

// Include all cpp files that are used from Arduino in this single cpp file.
// This way we don't have to add them to the project manually.
// A bit dirty, but it works
#define ARDUINO_MAIN

// Disable some warnings for the Arduino files
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wattributes"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wuninitialized"
#pragma GCC diagnostic ignored "-Wcpp"
#pragma GCC diagnostic ignored "-Wnarrowing"

#ifndef F_CPU
	#define F_CPU 32000000L
#endif

#include "compiler.h"
#include <Arduino.h>
extern "C"{
	#include <pins_arduino.h>
}


#include <Print.cpp>
#include <new.cpp>
#include <wiring.c>
#include <wiring_digital.c>
//#include <Stream.cpp>
#include <WString.cpp>
#include <WMath.cpp> // for Random function

// Include HardwareSerial.cpp, but do not let it define interrupt service routines.
// These are already defined in uart.cpp
#pragma push_macro("ISR")
#undef ISR
#define ISR(_vector) void removed_ISR_for##_vector(void)
#include <HardwareSerial.cpp>
#undef ISR
#pragma pop_macro("ISR")
    
// Unused source files:

//#include <WInterrupts.c>
//#include <wiring_analog.c>
//#include <wiring_pulse.c>
//#include <wiring_shift.c>
//#include <IPAddress.cpp>
//#include <Tone.cpp>

// Restore original warnings configuration
#pragma GCC diagnostic pop