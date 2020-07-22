/* -------------------------------------------------------------------------
Filename: mc_timer.h

Purpose: Includes functionality for various timers used in SENSOR MC firmware
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

Note: See document 000xxxx for complete project requirements
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

#ifndef MC_TIMER_H_
#define MC_TIMER_H_

	//_____  I N C L U D E S ______________________________________________________________

	#include "mc_system.h"			// Basic functionality for ATXMEGA MCU system

	//_____ M A C R O S ____________________________________________________________________

	//UART Char Delay Management Timer related definitions
	#define ENABLE_UART_DELAY_TIMER_INTERUPT		TCE0.INTCTRLA = TC_OVFINTLVL_LO_gc
	#define DISABLE_UART_DELAY_TIMER_INTERUPT		TCE0.INTCTRLA = TC_OVFINTLVL_OFF_gc
	#define ENABLE_UART_DELAY_TIMER					TCE0.CTRLA = TC_CLKSEL_DIV256_gc			//Enable Timer with the Prescalar of 256
	#define DISABLE_UART_DELAY_TIMER				TCE0.CTRLA = TC_CLKSEL_OFF_gc

	//Communication Wait Timer related definitions
	#define ENABLE_COMMUNICATION_TIMER_INTERUPT		TCC1.INTCTRLA = TC_OVFINTLVL_LO_gc
	#define DISABLE_COMMUNICATION_TIMER_INTERUPT	TCC1.INTCTRLA = TC_OVFINTLVL_OFF_gc
	#define ENABLE_COMMUNICATION_TIMER				TCC1.CTRLA = TC_CLKSEL_DIV1024_gc			//Enable Timer with the Prescalar of 1024
	#define DISABLE_COMMUNICATION_TIMER				TCC1.CTRLA = TC_CLKSEL_OFF_gc

	//5V- Voltage Stabilize Timer related definitions
	#define ENABLE_VOL_STABLE_TIMER_INTERUPT		TCE1.INTCTRLA = TC_OVFINTLVL_LO_gc
	#define DISABLE_VOL_STABLE_TIMER_INTERUPT		TCE1.INTCTRLA = TC_OVFINTLVL_OFF_gc
	#define ENABLE_VOL_STABLE_TIMER					TCE1.CTRLA = TC_CLKSEL_DIV1024_gc			//Enable Timer with the Prescalar of 1024
	#define DISABLE_VOL_STABLE_TIMER				TCE1.CTRLA = TC_CLKSEL_OFF_gc

	//System Delay Timer related definitions
	#define ENABLE_WAIT_TIMER_INTERUPT				TCF1.INTCTRLA = TC_OVFINTLVL_LO_gc
	#define DISABLE_WAIT_TIMER_INTERUPT				TCF1.INTCTRLA = TC_OVFINTLVL_OFF_gc
	#define ENABLE_WAIT_TIMER						TCF1.CTRLA = TC_CLKSEL_DIV8_gc				//Enable Timer with the Prescalar of 8
	#define DISABLE_WAIT_TIMER						TCF1.CTRLA = TC_CLKSEL_OFF_gc

	//Scenarios supported by Voltage Stabilize Timer
	#define WAIT_5_VOL_TIMER						0		//Voltage Stability Timer will execute for 80ms + 60us
	#define WAIT_3_VOL_TIMER						1		//Voltage Stability Timer will execute for 60us

	//_____ G L O B A L   D E F I N I T I O N S ______________________________________________________________
	
	//Global flags to indicate successful timer value generations
	extern volatile uint8_t		gchRxAvail;
	extern volatile uint8_t		gchCommunicationTimeOut;
	extern volatile uint8_t		gchCounterDelayTimeOut;
	extern volatile uint8_t		gchVoltageStableTimerFlag;

	//_____ F U N C T I O N   D E F I N I T I O N S ___________________________________________________________

	//_________ fnTimersInit ___________________________________________________________
	//
	//	@brief	This function is called from fnHardwareInit function
	//			It manages to call all the timer functions for their initialization
	
	void fnTimersInit(void);

	//_____ fnUartDelayTimerEnable ____________________________________________________________________
	//
	// @brief	Enable the UART Frame Delay Timer
	
	void fnUartDelayTimerEnable(void);

	//_____ fnUartDelayTimerDisable ____________________________________________________________________
	//
	// @brief	Disable the UART Frame Delay Timer
	
	void fnUartDelayTimerDisable(void);

	//_____ fnRadioCommunicationTimerEnable ____________________________________________________________________
	//
	// @brief	Enable the Radio Comm. Delay Timer
	
	void fnRadioCommunicationTimerEnable(void);

	//_____ fnRadioCommunicationTimerDisable ____________________________________________________________________
	//
	// @brief	Disable the Radio Comm. Delay Timer
	
	void fnRadioCommunicationTimerDisable(void);

	//_____ fnDisableSystemDelayTimer ____________________________________________________________________
	//
	// @brief	Disable system delay timer
	
	void fnDisableSystemDelayTimer(void);

	//_____ fnEnableSystemDelayTimer ____________________________________________________________________
	//
	// @brief	Enable system delay timer
	
	void fnEnableSystemDelayTimer(void);

	//_____fnStartSystemDelayTimer_______________________________________________________________
	//
	// @brief	Will start the system delay timer for specified time delay value generation
	//			It supports minimum of 0.5uSec delay generation
	// @param	fDelayCount		Value for which to initialize the Timer
	
	void fnStartSystemDelayTimer(float lDelayCount);

	//_________fnStartCommunicationTimer________________________________________________
	//
	// @brief	It will start the Radio Communication Timer with the desired value. Timer is designed to support wait time between 150 to 65535 ms
	//			It indicates the time for which SENSOR MC should get reception after successful transmission
	// @param	fDelayCount		Value for which to initialize the Timer
	
	void fnStartCommunicationTimer(float nTimerVal);

	//_____fnStartVolStableTimer_______________________________________________________________
	//
	// @brief	This function start voltage regulator stability timer which is used to generate the time delay for 5.5V and 3.3V regulator to get stabilized
	// @param	chVal	Flag indicating stabilization time required to get generated for 5V and 3.3V regulators.
	
	void fnStartVolStableTimer(uint8_t chVal);

#endif /* MC_TIMER_H_ */
