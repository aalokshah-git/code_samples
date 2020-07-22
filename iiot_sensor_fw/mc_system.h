/* -------------------------------------------------------------------------
Filename: mc_system.h

Purpose: MCU related basic functionality
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

#ifndef MC_SYSTEM_H_
#define MC_SYSTEM_H_

	//_____  I N C L U D E S ______________________________________________________________

	#include "system_debug.h"			// Functionality for System Debug Support
	
	//_____ I N L I N E  A S S E M B L Y ________________________________________________________________

	#define _NOP			asm("NOP")
	#define _SLEEP			asm("SLEEP")
	#define RESET_WDT		asm("wdr")		//Reset Watchdog timer
	
	//_____ M A C R O S ____________________________________________________________________

	#define CONVERT_TO_ASCII					48
	#define ASCII_OF_ZERO						48
	#define NEWLINE_CHAR						'\n'
	#define NULL_CHAR							'\0'
	#define PERIOD_CHAR							'.'
	#define ERR_CODE_CHAR						'E'
	
	//Interrupt Management
	#define ENABLE_GLOBAL_INTERRUPTS			sei()								//Enable all interrupts
	#define DISABLE_GLOBAL_INTERRUPTS			cli()								//Disable all interrupts

	#define ENABLE_LOWER_LAYER_INTERRUPT		PMIC.CTRL |= PMIC_LOLVLEN_bm		//Enable all interrupts initialized for low priority
	#define ENABLE_MIDDLE_LAYER_INTERRUPT		PMIC.CTRL |= PMIC_MEDLVLEN_bm		//Enable all interrupts initialized for medium priority
	#define ENABLE_UPPER_LAYER_INTERRUPT		PMIC.CTRL |= PMIC_HILVLEN_bm		//Enable all interrupts initialized for high priority

	#define DISABLE_LOWER_LAYER_INTERRUPT		PMIC.CTRL &= (~PMIC_LOLVLEN_bm)		//Disable all interrupts initialized for low priority
	#define DISABLE_MIDDLE_LAYER_INTERRUPT		PMIC.CTRL &= (~PMIC_MEDLVLEN_bm)	//Disable all interrupts initialized for medium priority
	#define DISABLE_UPPER_LAYER_INTERRUPT		PMIC.CTRL &= (~PMIC_HILVLEN_bm)		//Disable all interrupts initialized for high priority

	//Power Saving & Sleep Management
	#define DISABLE_ALL_SLEEP_MODES				SLEEP.CTRL = 0x00									//Disable all power savings

	#define ENABLE_POWER_DOWN_SLEEP_MODE		SLEEP.CTRL = (SLEEP_SEN_bm | SLEEP_SMODE_PDOWN_gc)	//MCU- Power Down mode enable
	#define DISABLE_POWER_DOWN_SLEEP_MODE		DISABLE_ALL_SLEEP_MODES

	#define ENABLE_POWER_SAVING_SLEEP_MODE		SLEEP.CTRL = (SLEEP_SEN_bm | SLEEP_SMODE_PSAVE_gc)	//MCU- Power Save mode enable
	#define DISABLE_POWER_SAVING_SLEEP_MODE		DISABLE_ALL_SLEEP_MODES

	#define ENABLE_IDLE_POWER_SLEEP_MODE		SLEEP.CTRL = (SLEEP_SEN_bm | SLEEP_SMODE_IDLE_gc)	//MCU- Idle mode enable
	#define DISABLE_IDLE_POWER_SLEEP_MODE		DISABLE_ALL_SLEEP_MODES

	//Reset Source Identification
	#define RESET_BY_PWRON			BIT_0_bm		//Bit mask to check reset by power on
	#define RESET_BY_EXT			BIT_1_bm		//Bit mask to check reset by external reset
	#define RESET_BY_WDT			BIT_4_bm		//Bit mask to check reset by watch dog timer
	#define RESET_BY_SOFTWARE		BIT_6_bm		//Bit mask to check reset by software

	//MACRO to perform the Software Reset (Currently not used)
	#define SOFTWARE_RESET			do{\
									CCP = CCP_IOREG_gc;\
									RST.CTRL = 0x01;\
									}while(0)

	//MACROs used in fnFloatToString
	#define FLOAT_RESOLUTION		1000
	#define FRACTIONAL_DIGIT		3				//No of digits to consider after decimal point
	
	//_____ G L O B A L   D E F I N I T I O N S ______________________________________________________________
	
	extern Bit_Mask_t			ghBitMask;
	extern Bit_Position_t		ghBitPosittion;
	extern Step_Index_values	ghStepIndex;
	
	//_____ F U N C T I O N   D E F I N I T I O N S ___________________________________________________________

	//_____ fnInitializeClock ____________________________________________________________________
	//
	// @brief	Configures internal clock source of 32MHZ by following bellowed steps:
	//				1> Select the appropriate oscilator as a clock source
	//				2> Wait for selected clock source to get stabilized
	//				3> Initialize desired CPU clock by using selected clock source
	//				4> Enable appropriate clock source for RTC (if in use)

	void fnInitializeClock(void);

	//_____  fnMemSetToValue ____________________________________________________________________
	//
	// @brief	Initialize memory locations with specific value
	// @param	pchMemAddr		pointer to the beginning of the memory location
	//			chValue			constant value to initialize in given address space
	//			nMemAddrCount	total no of memory addresses to initialize starting from base address

	void fnMemSetToValue(volatile uint8_t *pchMemAddr,uint8_t chValue,uint16_t nMemAddrCount);

	//_____ fnWait_uSecond ____________________________________________________________________
	//
	// @brief	This function can be used to generate the desired value of delay during firmware execution
	//			The minimum delay value that can be generate by using this function is 0.5uS
	// @param	lDelayCount		Value for which timer needs to generate the delay

	void fnWait_uSecond(volatile float lDelayCount);

	//_____ fnFloatToString ____________________________________________________________________
	//
	// @brief	Converts float to string value
	// @param	fVal	Float value which needs to convert in string
	//			pchStr	Converted float value in string

	void fnFloatToString(float fVal,uint8_t *pchStr);

	//_____ fnMemCopy ____________________________________________________________________
	//
	// @brief	It performs block copy operation between two memory blocks
	// @param	pchPtr1		Pointer which points to the source memory base
	//			pchPtr2		Pointer which points to the destination memory base
	//			chLength	No of bytes for which to perform the block copy operation
	
	void fnMemCopy(uint8_t *pchPtr1,uint8_t *pchPtr2,uint8_t chLength);

	//_____ fnEnableWDT ____________________________________________________________________
	//
	// @brief	This function enables the watch dog timer for 4-seconds to avoid unnecessary hanging of the system.
	//			Note: It remains enable even in sleep mode so disable the WDT before putting the SENSOR in deep sleep mode.

	void fnEnableWDT(void);
	
	//_____ fnDisableWDT ____________________________________________________________________
	//
	//@brief	This function disables the Watchdog timer

	void fnDisableWDT(void);
	
#endif /* MC_SYSTEM_H_ */