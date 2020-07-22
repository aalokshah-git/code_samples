/* -------------------------------------------------------------------------
Filename: mc_uart.c

Date Created: 10/08/2014

Purpose: UART related functionality including initialization, transmission and reception

Functions:
fnUartInitialization		UART initialization function
fnUartSetTx					Function to disable receive interrupt
fnUartSetRx					Function to enable receive interrupt
fnUartSendData				Function to manage transmission of data over UART
fnUartReceiveData			Function to manage reception of data over UART
fnUartSendComplete			Check for completion of on going UART communication

Interrupts:
USARTC0_RXC_vect			ISR for USARTC0 receive avail interrupt
USARTC0_TXC_vect			ISR for USARTC0 transmit complete interrupt


Author: Aalok Shah

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

Inputs - See specific function headers for inputs
Outputs - See specific function headers for outputs

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
#include "mc_timer.h"			// TIMER functionality for ATXMEGA MCU system

//_____ G L O B A L   D E F I N I T I O N S _________________________________________________

//Object of UART communication data structure
volatile UART_COMM_DATA_HANDLE	ghUartCommData;

//Increment counter variable used in UART related interrupt routines
volatile uint16_t				gnDataTxRxCounter;

// Will get set from UART frame delay management timer when appropriate reception is available
volatile uint8_t				gchRxAvail;

//Counter value storage variable which can be used to achieve specific value of delay using delay timer
volatile uint8_t				gchMaxDelayValue;

//Counter value incremental counter variable which can be used to achieve specific value of delay using delay timer
volatile uint8_t				gchCurrentDelayValue;

//Indicates completion of current data transmission
volatile uint8_t				gchTxCompleteFlag;

//____ fnUartInitialization  _________________________________________________________________
//
// @brief	Steps to initialize UART in AVR-XMEGA MCU:
//					1> Select appropriate mode of operation for UART.
//					2> Set UART data frame format with No parity, 1-Stop bit, 8-Character bits.
//					3> Set BSEL and BSCALE to choose Baud rate as per the equation BSEL = (clock freq /((2^BSCALE)*16*BaudRate))-1.
//					4> Enable receive interrupt initially to put UART in receive mode as default.
//					5> Reset/configure all nessery global definitions.

void fnUartInitialization(void)
{
	//UART initialization: 8-DataBits, 1-StopBits, No-ParityBits
	UART_FORMAT(UART_MODE_SELECT,UART_NO_PARITY,UART_ONE_STOP_BIT,UART_EIGHT_DATA_BITS);
	
	//UART initialization: 9600 BaudRate at 16MHZ frequency
	USARTC0_BAUDCTRLA = (uint8_t) UART_BSEL;
	USARTC0_BAUDCTRLB = (UART_BSCALE << BIT_4_bp) | (UART_BSEL >> BIT_8_bp);
	
	//UART RX interrupt enable (As default UART in receive mode)
	UART_RX_INT_ENABLE;
	
	//The Frame_Delay_Count is initialize to get 40ms delay with the 5ms timer
	gchMaxDelayValue=8;
	
	//Reset UART related TX-RX count
	ghUartCommData.nTxCount= RESET_COUNTER;
	ghUartCommData.nRxCount= RESET_COUNTER;
	
	UART_RX_ENABLE;
	UART_TX_ENABLE;
	return;
}

//____ fnUartSetTx  _________________________________________________________________
//
// @brief	In this design UART is configured to listen for reception and same buffer is used in both transmit and receive operation.
//			So it is necessary to terminate the receive interrupt before start accessing communication buffer used for UART sending operations.
//			Receive interrupt will automatically enable on successful transmission.
//			Upper layer must call this function before executing transmit related instructions.
//			If this function is called than upper layer must execute fnUartSendData() or fnUartSetRx() to avoid unnecessary condittion in UART mechanism.

void fnUartSetTx(void)
{
	UART_RX_INT_DISABLE;
	return;
}

//____ fnUartSetRx  _________________________________________________________________
//
// @brief	This function is used to enable the receive interrupt.
//			Main intense behind this function is to enable the interrupt which was previously disabled by fnUartSetTx() if needed in upper level system design.

void fnUartSetRx(void)
{
	UART_RX_INT_ENABLE;
	return;
}

//____ fnUartSendComplete  _________________________________________________________________
//
// @brief	This function can be used to check the status of UART during transmit operation.
// @return	TRUE if transmission is complete otherwise returns FALSE.

int8_t fnUartSendComplete(void)
{
	if(gchTxCompleteFlag)		//Check current sending operation is complete or not
	{
		return RETURN_TRUE;
	}
	
	return RETURN_FALSE;
}

//____ fnUartSendData  _________________________________________________________________
//
// @brief	This function will get called from upper level system when wants to send data over UART.
//			fnUartSetTx() must be called before executing this function.
//			nTxCount and chCommBuff fields of UART_COMM_DATA_HANDLE must be filled with the send data information before executing this function.

void fnUartSendData(void)
{
	//Reset receive avail decision flag
	gchRxAvail = RESET_FLAG;
	
	//Reset incremental definitions
	gnDataTxRxCounter = RESET_COUNTER;
	gchCurrentDelayValue = RESET_VALUE;
	
	gchTxCompleteFlag = RESET_FLAG;		//Indicates new sending operation just started
	
	//Send first byte and rest of the bytes will get transmitted from ISR
	USARTC0.DATA = ghUartCommData.chCommBuff[gnDataTxRxCounter++];
	
	//Enable UART Tx interrupt
	UART_TX_INT_ENABLE;
	
	return;
}

//____ fnUartReceiveData  _________________________________________________________________
//
// @brief	This function is executed by upper system to check the new reception on UART.
// @return  0 if no data were received otherwise returns the count value indicating total data bytes received.

uint16_t fnUartReceiveData(void)
{
	uint16_t chDummy = RESET_VALUE;		//Local definition as temporary storage of ghUartCommData.chRxCount.
	
	if(gchRxAvail == SET_FLAG)
	{
		chDummy=ghUartCommData.nRxCount;
		ghUartCommData.nRxCount = RESET_COUNTER;
		gchRxAvail = RESET_FLAG;		//This flag will get reseted on first call so upper system needs to take care of this***
		
		return chDummy;
	}
	
	return chDummy;
}

//_____ I S R - U A R T   R E C E I V E ____________________________________________________________________
//
//	@brief	ISR for UARTC0 Receive Avail:
//			UART is designed to keep listening for data byte.
//			When new character will get received, it will shifted to the buffer, rxCount value will get incremented and inter char delay timer will get restarted.
//			If value of total received characters exceeds the UART_COMM_BUF_SIZE than the counter will get flush to handle such unwanted situation securely.
//			When inter char frame delay exceeds than the time limit the reception will considered as complete.

ISR(USARTC0_RXC_vect)
{
	gchNewInterrupt = SET_NEW_ISR_FLAG;
	ghUartCommData.chCommBuff[ghUartCommData.nRxCount++] = USARTC0.DATA;
	
	if(ghUartCommData.nRxCount >= UART_COMM_BUF_SIZE)
	{
		ghUartCommData.nRxCount = RESET_COUNTER;			//Vanish the current reception
	}
	else
	{
		gchCurrentDelayValue = RESET_VALUE;
		fnUartDelayTimerEnable();			//Reinitialize/Initialize the frame delay timer
	}
}

//_____ I S R - U A R T   T R A N S M I T ____________________________________________________________________
//
// @brief	ISR for UARTC0 Transmit Complete:
//			On every interrupt one byte is shifted to UART send buffer.
//			When chTxCount limit will match with the gnDataTxRxCounter, transmission of all data bytes will complete
//
ISR(USARTC0_TXC_vect)
{
	gchNewInterrupt = SET_NEW_ISR_FLAG;
	if(gnDataTxRxCounter >= ghUartCommData.nTxCount)
	{
		gnDataTxRxCounter = RESET_COUNTER;
		gchTxCompleteFlag = SET_FLAG;		//Indicates current sending operation is complete
		UART_TX_INT_DISABLE;
		UART_RX_INT_ENABLE;
	}
	else
	{
		USARTC0.DATA = ghUartCommData.chCommBuff[gnDataTxRxCounter++];
	}
}
