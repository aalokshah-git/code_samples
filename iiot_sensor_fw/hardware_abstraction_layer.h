/* -------------------------------------------------------------------------
Filename: hardware_abstraction_layer.h

Job#: 20473
Purpose: Holds all MCU peripheral dependent HAL layer header files
Date Created: 10/08/2014

(NOTE: latest version is the top version)

Author: Aalok Shah
Changes: Initial version

Naming Conventions:
ALL CAPS =          MACRO, DEFINE
ALL CAPS =          Structure Notation
First Word Cap =    start of function or variable

Table: Simplified Hungarian Notation. (Ref: http://vis.eng.uci.edu/standards/node19.html)
Except were noted in the code

Prefix         Type					Description							Example
--------	---------------			--------------------------------	------------
n            int					any integer type                    nCount
ch           char					any character type                  chLetter
f            float, double			floating point                      fPercent
g            global					global scope variable               gnCount
b            bool					any boolean type                    bDone
l            long					any long type                       lDistance
p            *   any				pointer                             pObject, pnCount
sz           *   nul				terminated string of characters     szText
pfn              *					function pointer                    pfnProgress
h            handle					handle to something                 hMenu
fn<*>        function				function call<return type>			fnnXmit(pchBuffer)

Input: None
Output: None

-----------------------------------------------------------------------------
NOTE: this is filled in by the tester – not the author
-----------------------------------------------------------------------------
Test:
Tester:
Test Date:
Test Procedure:
Test Results:

-----------------------------------------------------------------------------
NOTE: the test section repeats for each time the code is tested ...see the example below:
-----------------------------------------------------------------------------
Test:
Tester:
Test Date:
Test Procedure:
Test Results:

-------------------------------------------------------------------------------------*/

#ifndef HARDWARE_ABSTRACTION_LAYER_H_
#define HARDWARE_ABSTRACTION_LAYER_H_

	//_____  I N C L U D E S ______________________________________________________________

	#include "mc_gpio.h"			// GPIO functionality for ATXMEGA MCU system
	#include "mc_timer.h"			// TIMER functionality for ATXMEGA MCU system
	#include "mc_uart.h"			// UART functionality for ATXMEGA MCU system
	#include "mc_spi.h"				// SPI functionality for ATXMEGA MCU system
	#include "mc_adc.h"				// ADC functionality for ATXMEGA MCU system
	#include "mc_i2c.h"				// I2C functionality for ATXMEGA MCU system

#endif /* HARDWARE_ABSTRACTION_LAYER_H_ */