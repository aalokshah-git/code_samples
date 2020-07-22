/* -------------------------------------------------------------------------
Filename: system_debug.h

Job#: 20473
Purpose: File contains all the definitions required to handle the debugging with the help of UART and LEDs
Date Created: 10/27/2014

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

#ifndef SYS_DEBUG_H_
#define SYS_DEBUG_H_

	//_____  I N C L U D E S ______________________________________________________________

	#include <avr/io.h>					// Include file with definitions related to AVR-AXMEGAA1U Peripherals
	#include <avr/interrupt.h>			// Interrupt definitions related to AVR-AXMEGAA1U Peripherals
	#include <string.h>					// String related generic APIs
	#include "generic_macro.h"			// Generally used MACROS in entire SENSOR MC design

	//_____ M A C R O S ____________________________________________________________________

	#define		DEBUG_MESSAGE_PRINTS				1		//Controls sending of a lower level char strings on UART
	#define		DEBUG_ERROR_PRINTS					1		//Controls sending of a middle level char strings on UART
	#define		DEBUG_DATA_PRINTS					1		//Controls sending of hex bytes on UART

	//_____ E N U M E R A T I O N S ____________________________________________________________________
	
	//Various Error Codes Supported in System
	typedef enum
	{
		SENSOR_NO_ERROR=0,							// No error
		SENSOR_CC1125_TX_FIIO_ERR,					// CC1125 TX FIFO error
		SENSOR_CC1125_RX_FIIO_ERR,					// CC1125 RX FIFO error
		SENSOR_CC1125_CRC_MISMATCH_ERR,				// CRC Mismatch error
		SENSOR_CC1125_TX_GPIO_INTTERUPT_FAIL,			// CC1125 GPIO interrupt fail
		SENSOR_CC1125_COMMUNICATION_WAIT_TIMEOUT,		//CC1125 response wait time out
		SENSOR_CC1125_CHIP_NOT_READY,					// CC1125 chip not ready
		SENSOR_CC1125_REGISTER_INIT_FAIL,				// CC1125 initialization fail
		SENSOR_UART_COMM_BUFF_OVERFLOW,				// UART communication buffer overflow
		SENSOR_SPI_COMM_BUFF_OVERFLOW,				// SPI communication buffer overflow
		SENSOR_I2C_COMM_BUFF_OVERFLOW,				// I2C communication buffer overflow
		SENSOR_I2C_BUS_ARBITRATION,					// I2C bus arbitration
		SENSOR_BUS_STATE_BUSY,
		SENSOR_PACKET_HEADER_UNDEFINED,
		SENSOR_SENSOR_ID_UNDEFINED,
		SENSOR_ADC_RESOURCES_ARE_NOT_FREE,
		SENSOR_ADC_CONVERSION_MISMATCH,
		SENSOR_SENSOR_SAMPLING_OVERRUN,
		SENSOR_RADIO_DIVISOR_OVERRUN,
		SENSOR_COMM_WAIT_TIME_MISMATCH,
		SENSOR_CC1125_NACK_RECEIVED,
		SENSOR_CC1125_TXRX_UNDEFINED_STATE,
		SENSOR_SAMPLE_AVERAGE_COUNT_EXCEEDS,
		SENSOR_INVALID_EXECUTION_DEVELOPER_ERROR,
		SENSOR_CC1125_CALIBRATION_FAIL
	}SENSOR_MC_ERROR_CODES;

	//_____ G L O B A L   D E F I N I T I O N S ______________________________________________________________
	
	//List of error codes available in SENSOR MC firmware
	extern SENSOR_MC_ERROR_CODES	ghSensorErrCodes;
	
	//Flag will get set in all the ISRs
	extern volatile uint8_t		gchNewInterrupt;

	//_____ F U N C T I O N   D E F I N I T I O N S ___________________________________________________________

	//_____ fnSendDebugMessageStringUART ____________________________________________________________________
	//
	// @brief	Send string on UART as a part of SENSOR MC debug functionality
	//			This function is blocking function. The execution will come out from this function only when all the chars of the strings will get sent.
	// @param	pchStr	Pointer to the string which needs to be sent

	void fnSendDebugMessageString(char *pchStr);

	//_____ fnSendDebugMessageDataBytesUART ____________________________________________________________________
	//
	// @brief	Send hex bytes on UART as a part of SENSOR MC debug functionality
	//			This function is blocking function. The execution will come out from this function only when all the data bytes will get sent.
	// @param	pchStr		Pointer to the data bytes which needs to be sent
	//			chLength	Length indicating no of data bytes to send on UART

	void fnSendDebugMessageDataBytes(uint8_t *pchBytes,uint8_t chLength);

	//_____ fnSendErrorCode ____________________________________________________________________
	//
	// @brief	Send error code on UART as a part of SENSOR MC debug functionality
	//			This function is blocking function. The execution will come out from this function only when all the data bytes will get sent.
	// @param	hErrorCode	Holds the value which points to specific system related error
	
	void fnSendErrorCode(SENSOR_MC_ERROR_CODES hErrorCode);

	//_____ I N L I N E   M A C R O S ____________________________________________________________

	//The values of various MACROs decide the debug functionality supported in SENSOR MC firmware.
	//This feature is designed in such a way to avoid debug functionality when require to save the power consumption and increase the execution time.

	#if	DEBUG_MESSAGE_PRINTS
		#define SEND_DEBUG_STRING(string)				fnSendDebugMessageString(string)
	#else
		#define SEND_DEBUG_STRING(string)				_NOP
	#endif

	#if	DEBUG_ERROR_PRINTS
		#define SEND_DEBUG_ERROR_CODES(error)			 fnSendErrorCode(error)
	#else
		#define SEND_DEBUG_ERROR_CODES(error)			_NOP
	#endif

	#if	DEBUG_DATA_PRINTS
		#define SEND_DEBUG_DATA_BYTES(bytes,length)		fnSendDebugMessageDataBytes((uint8_t*) bytes,length)
	#else
		#define SEND_DEBUG_DATA_BYTES(bytes,length)		_NOP
	#endif

#endif /* SYSTEM_DEBUG_H_ */