/* -------------------------------------------------------------------------
Filename: mc_system.c

Date Created: 10/29/2014

Purpose: Holds basic system level functionality including initialization of clock, interrupts and power saving modes.
It also includes declarations of commonly used functions in firmware designing.

Functions:
fnInitializeClock					Initialization function for configuring the CPU clock
fnMemSetToValue						Initialize the memory locations with specific values
fnWait_uSecond						Delay generation of specified ms
fnSendDebugMessageStringUART		Send debug strings on UART
fnSendDebugMessageStringUART		Send debug byte on UART
fnFloatToString						Convert float value to string
fnEnableWDT							Enable the watchdog timer
fnDisableWDT						Disable the watchdog timer
fnMemCopy							Perform block COPY operation

Interrupts:
-NA-


Author: , Aalok Shah

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

--------------------------------------------------------------
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

//_____  I N C L U D E S ______________________________________________________________

#include "mc_uart.h"			// UART functionality for ATXMEGA MCU system
#include "mc_system.h"			// Basic functionality for ATXMEGA MCU system
#include "mc_timer.h"			// TIMER functionality for ATXMEGA MCU system

//_____ G L O B A L   D E F I N I T I O N S _________________________________________________

// Generic global definitions
Bit_Mask_t			ghBitMask;
Bit_Position_t		ghBitPosittion;
Step_Index_values	ghStepIndex;

//_____ fnInitializeClock ____________________________________________________________________
//
// @brief	Configures internal clock source of 32MHZ by following bellowed steps:
//				1> Select the appropriate oscilator as a clock source
//				2> Wait for selected clock source to get stabilized
//				3> Initialize desired CPU clock by using selected clock source
//				4> Enable appropriate clock source for RTC (if in use)

void fnInitializeClock(void)
{
	// Instruction has to be done "within 4 clock cycles"
	// In AVR this must needs to be done when it is required to deal with system level configurations
	CCP = CCP_IOREG_gc;
	
	// 32 MHz Internal RC Oscillator Enable
	OSC.CTRL = OSC_RC32MEN_bm;

	// Wait for clock to get stabilize
	while( ! ( OSC.STATUS & OSC_RC32MRDY_bm ) );

	// Instruction has to be done "within 4 clock cycles"
	CCP = CCP_IOREG_gc;
	
	// System Clock Selection: 32 MHz Internal RC Oscillator
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc;
	
	// Instruction has to be done "within 4 clock cycles"
	CCP = CCP_IOREG_gc;
	
	// Divide 32MHz Oscillator by 2 to make clock of 16MHz
	CLK.PSCTRL = CLK_PSADIV0_bm;
	
	// RTC Clock Selection: 32 KHZ Internal Oscillator/32 (1KHZ)
	CLK.RTCCTRL = CLK_RTCSRC_ULP_gc | CLK_RTCEN_bm;
	
	return;
}

//_____  fnMemSetToValue ____________________________________________________________________
//
// @brief	Initialize memory locations with specific value
// @param	pchMemAddr		pointer to the beginning of the memory location
//			chValue			constant value to initialize in given address space
//			nMemAddrCount	total no of memory addresses to initialize starting from base address

void fnMemSetToValue(volatile uint8_t *pchMemAddr,uint8_t chValue,uint16_t nMemAddrCount)
{
	do
	{
		*(pchMemAddr++)=chValue;
	}while(--nMemAddrCount);
	
	return;
}

//_____ fnWait_uSecond ____________________________________________________________________
//
// @brief	This function can be used to generate the desired value of delay during firmware execution
//			The minimum delay value that can be generate by using this function is 0.5uS
// @param	lDelayCount		Value for which timer needs to generate the delay

void fnWait_uSecond(float lDelayCount)
{
	fnStartSystemDelayTimer(lDelayCount);
	while(!gchCounterDelayTimeOut);
	
	return;
}

//_____ fnFloatToString ____________________________________________________________________
//
// @brief	Converts float to string value
// @param	fVal	Float value which needs to convert in string
//			pchStr	Converted float value in string

void fnFloatToString(float fVal,uint8_t *pchStr)
{
	uint32_t lReal = RESET_VALUE;
	uint8_t  chTmp = RESET_VALUE;
	uint8_t  *pchPtr1 = NULL;
	uint8_t  *pchPtr2 = NULL;
	uint8_t  chTmp2= RESET_VALUE;

	lReal=fVal * FLOAT_RESOLUTION;

	if(lReal)											//Check for the non zero value
	{
		while(chTmp2++ != FRACTIONAL_DIGIT)				//3 digit-Fractional Part
		{
			pchStr[chTmp++] = ((lReal % 10) + CONVERT_TO_ASCII);
			lReal /= 10;
		}
		
		pchStr[chTmp++] =PERIOD_CHAR;							//Decimal point insertion
		
		if(lReal)										//Check for the non zero INTEGER PART
		{
			while(lReal)								//Integer Part
			{
				pchStr[chTmp++] = ((lReal % 10) + CONVERT_TO_ASCII);
				lReal /= 10;
			}
		}
		else
		{
			pchStr[chTmp++] = ASCII_OF_ZERO;
		}
		
	}
	else
	{
		pchStr[chTmp++] = ASCII_OF_ZERO;
		pchStr[chTmp++] = ASCII_OF_ZERO;
		pchStr[chTmp++] = ASCII_OF_ZERO;
		pchStr[chTmp++] = PERIOD_CHAR;
		pchStr[chTmp++] = ASCII_OF_ZERO;
	}
	
	pchStr[chTmp] =NULL_CHAR;

	//Reverse the entire string with the 2-Variable swapping using EX-OR operation
	for(pchPtr1 = pchStr, pchPtr2 = pchStr + strlen((char*)pchStr) - 1; pchPtr2 > pchPtr1; ++pchPtr1, --pchPtr2)
	{
		*pchPtr1 ^= *pchPtr2;
		*pchPtr2 ^= *pchPtr1;
		*pchPtr1 ^= *pchPtr2;
	}
	
	return;
}

//_____ fnEnableWDT ____________________________________________________________________
//
// @brief	This function enables the watch dog timer for 4-seconds to avoid unnecessary hanging of the system.
//			Note: It remains enable even in sleep mode so disable the WDT before putting the SENSOR in deep sleep mode.

void fnEnableWDT(void)
{
	uint8_t temp = WDT_CEN_bm | WDT_ENABLE_bm | WDT_PER_4KCLK_gc;	//Enable the watchdog timer for provided timer value
	
	CCP = CCP_IOREG_gc;												//Config change protection
	WDT.CTRL = temp;
	while(WDT.STATUS & WDT_SYNCBUSY_bm);							//Wait for the data synchronized from system clock to WDT clock
	
	return;
}

//_____ fnDisableWDT ____________________________________________________________________
//
//@brief	This function disables the Watchdog timer
	
void fnDisableWDT(void)
{
	uint8_t temp = WDT_CEN_bm | (WDT.CTRL & (~WDT_ENABLE_bm));		//Disable the watchdog timer
	
	CCP = CCP_IOREG_gc;												//config change protection
	WDT.CTRL = temp;
	
	return;
}

//_____ fnMemCopy _____________________________________________________
//
// @brief	It performs block copy operation between two memory blocks
// @param	pchPtr1		Pointer which points to the source memory base
//			pchPtr2		Pointer which points to the destination memory base
//			pchPtr2		No of bytes for which to perform the block copy operation

void fnMemCopy(uint8_t *pchPtr1,uint8_t *pchPtr2,uint8_t chLength)
{
	uint8_t chIndex = RESET_COUNTER;
	
	for(chIndex = RESET_COUNTER ; chIndex<chLength ; chIndex++)
	{
		pchPtr1[chIndex]= pchPtr2[chIndex];
	}
	
	return;
}

//_____ fnSendDebugMessageStringUART ____________________________________________________________________
//
// @brief	Send string on UART as a part of SENSOR MC debug functionality
//			This function is blocking function. The execution will come out from this function only when all the chars of the strings will get sent.
// @param	pchStr	Pointer to the string which needs to be sent

void fnSendDebugMessageString(char *pchStr)
{
	uint32_t nCounter = RESET_COUNTER;
	
	if(strlen((char*)pchStr)>UART_COMM_BUF_SIZE)	//If the length of debug string exceeds the UART communication buffer limit than ignore the message
	{
		SEND_DEBUG_ERROR_CODES(SENSOR_UART_COMM_BUFF_OVERFLOW);
		return;
	}
	
	fnUartSetTx();									//Put UART in transmit mode and avoid reception during data processing
	
	// Copy string in the UART communication buffer and send it
	ghUartCommData.nTxCount=strlen((char*)pchStr);
	for(nCounter= RESET_COUNTER ;nCounter<ghUartCommData.nTxCount; nCounter++)
	{
		ghUartCommData.chCommBuff[nCounter]=pchStr[nCounter];
	}
	
	fnUartSendData();
	while(fnUartSendComplete())
	{
		if(++nCounter>1000000)
		{
			break;									//Avoid unwanted exception and stuck in loop
		}
	}
	
	return;
}

//_____ fnSendDebugMessageDataBytesUART ____________________________________________________________________
//
// @brief	Send hex bytes on UART as a part of SENSOR MC debug functionality
//			This function is blocking function. The execution will come out from this function only when all the data bytes will get sent.
// @param	pchStr		Pointer to the data bytes which needs to be sent
//			chLength	Length indicating no of data bytes to send on UART

void fnSendDebugMessageDataBytes(uint8_t *pchBytes,uint8_t chLength)
{
	uint32_t nCounter = RESET_COUNTER;
	
	if(chLength>UART_COMM_BUF_SIZE)
	{
		SEND_DEBUG_ERROR_CODES(SENSOR_UART_COMM_BUFF_OVERFLOW);
		return;
	}
	
	fnUartSetTx();									//Put UART in transmit mode and avoid reception during data processing
	
	// Copy bytes in the UART communication buffer and send it
	ghUartCommData.nTxCount=chLength;
	for(nCounter = RESET_COUNTER ; nCounter<chLength ; nCounter++)
	{
		ghUartCommData.chCommBuff[nCounter]=pchBytes[nCounter];
	}
	
	nCounter= RESET_COUNTER;
	fnUartSendData();
	while(fnUartSendComplete())
	{
		if(++nCounter>1000000)
		{
			break;									//Avoid unwanted exception and stuck in loop
		}
	}
	
	return;
}

//_____ fnSendErrorCode ____________________________________________________________________
//
// @brief	Send error code on UART as a part of SENSOR MC debug functionality
//			This function is blocking function. The execution will come out from this function only when all the data bytes will get sent.
// @param	hErrorCode	Holds the value which points to specific system related error

void fnSendErrorCode(SENSOR_MC_ERROR_CODES hErrorCode)
{
	uint32_t nCounter = RESET_COUNTER;
	
	fnUartSetTx();									//Put UART in transmit mode and avoid reception during data processing
	ghUartCommData.nTxCount=4;
	ghUartCommData.chCommBuff[0]=ERR_CODE_CHAR;
	ghUartCommData.chCommBuff[1]=((uint8_t)hErrorCode / 10) + CONVERT_TO_ASCII;
	ghUartCommData.chCommBuff[2]=((uint8_t)hErrorCode % 10) + CONVERT_TO_ASCII;
	ghUartCommData.chCommBuff[3]=NEWLINE_CHAR;
	
	fnUartSendData();
	while(fnUartSendComplete())
	{
		if(++nCounter>1000000)
		{
			break;									//Avoid unwanted exception and stuck in loop
		}
	}
	
	return;
}
