/* -------------------------------------------------------------------------
Filename: mc_uart.h

Purpose: Basic UART implementation
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

#ifndef MC_UART_H_
#define MC_UART_H_

	//_____  I N C L U D E S ______________________________________________________________

	#include "mc_system.h"			// Basic functionality for ATXMEGA MCU system

	//_____ H A R D W A R E   R E G I S T E R S / B I T   D E F I N A T I O N S ____________________________

	//Communication Parameters
	#define UART_MODE_SELECT			0x00

	#define UART_NO_PARITY				0x00
	#define UART_EVEN_PARITY			0x20
	#define UART_ODD_PARITY				0x30

	#define UART_ONE_STOP_BIT			0x00
	#define UART_TWO_STOP_BITS			0x08
	#define	UART_EIGHT_DATA_BITS		0x03

	#define USART_TXEN					0x08
	#define USART_RXEN					0x10

	//_____ M A C R O S ____________________________________________________________________

	//Configured for 9600 BaudRate with 16MHZ Clock
	#define UART_BSEL					12		//BSEL = (I/O clock freq /((2^BSCALE) * 16 * Baud Rate)) - 1
	#define UART_BSCALE					3		//For fractional baud rate setting: range -7 to +7

	//UART Communication Control MACRO
	#define UART_TX_ENABLE		(USARTC0.CTRLB |= USART_TXEN)	//Enable UART transmitter
	#define UART_RX_ENABLE		(USARTC0.CTRLB |= USART_RXEN)	//Enable UART receiver

	#define UART_TX_DISABLE		(USARTC0_CTRLB &= ~USART_TXEN)	//Disable UART transmitter
	#define UART_RX_DISABLE		(USARTC0_CTRLB &= ~USART_RXEN)	//Disable UART receiver

	//Set UART Frame Format
	#define UART_FORMAT(MODE, PARITY, STOP_BIT, CHAR_SIZE)		(USARTC0.CTRLC = MODE | PARITY | STOP_BIT | CHAR_SIZE)

	//Set UART TXD interrupt level.
	#define UART_TXDINTLVL_SET(UART, TX_INTLVL)					UART.CTRLA = (UART.CTRLA & ~USART_TXCINTLVL_gm) | TX_INTLVL

	//Set UART RXD interrupt level.
	#define UART_RXDINTLVL_SET(UART, RX_INTLVL)					UART.CTRLA = (UART.CTRLA & ~USART_RXCINTLVL_gm) | RX_INTLVL

	//UART Interrupt Controlling
	#define UART_TX_INT_ENABLE			UART_TXDINTLVL_SET(USARTC0,USART_TXCINTLVL_LO_gc);		//UARTC0 transmit interrupt enable with low priority
	#define UART_RX_INT_ENABLE			UART_RXDINTLVL_SET(USARTC0,USART_RXCINTLVL_LO_gc);		//UARTC0 receive interrupt enable with low priority

	#define UART_TX_INT_DISABLE			UART_TXDINTLVL_SET(USARTC0,USART_TXCINTLVL_OFF_gc);		//UARTC0 transmit interrupt disable
	#define UART_RX_INT_DISABLE			UART_RXDINTLVL_SET(USARTC0,USART_RXCINTLVL_OFF_gc);		//UARTC0 receive interrupt disable

	//UART Max Communication Buffer Size
	#define UART_COMM_BUF_SIZE			135

	//_____ D A T A   S T R U C T U R E S _________________________________________________

	//Structure to manage data during UART communication
	typedef struct
	{
		uint16_t nTxCount;
		uint16_t nRxCount;
		uint8_t	 chCommBuff[UART_COMM_BUF_SIZE];
	}UART_COMM_DATA_HANDLE;

	//_____ G L O B A L   D E F I N I T I O N S ______________________________________________________________
	
	//Object of data structure used to manage resources during communication over UART
	extern volatile UART_COMM_DATA_HANDLE	ghUartCommData;

	//_____ F U N C T I O N   D E F I N I T I O N S ___________________________________________________________

	//____ fnUartInitialization  _________________________________________________________________
	//
	// @brief	Steps to initialize UART in AVR-XMEGA MCU:
	//					1> Select appropriate mode of operation for UART.
	//					2> Set UART data frame format with No parity, 1-Stop bit, 8-Character bits.
	//					3> Set BSEL and BSCALE to choose Baud rate as per the equation BSEL = (clock freq /((2^BSCALE)*16*BaudRate))-1.
	//					4> Enable receive interrupt initially to put UART in receive mode as default.
	//					5> Reset/configure all nessery global definitions.

	void fnUartInitialization(void);

	//____ fnUartSendData  _________________________________________________________________
	//
	//	@brief	This function will get called from upper level system when wants to send data over UART.
	//			fnUartSetTx() must be called before executing this function.
	//			nTxCount and chCommBuff fields of UART_COMM_DATA_HANDLE must be filled with the send data information before executing this function.

	void fnUartSendData(void);

	//____ fnUartReceiveData  _________________________________________________________________
	//
	// @brief	This function is executed by upper system to check the new reception on UART.
	// @return  0 if no data were received otherwise returns the count value indicating total data bytes received.

	uint16_t fnUartReceiveData(void);

	//____ fnUartSetTx  _________________________________________________________________
	//
	// @brief	In this design UART is configured to listen for reception and same buffer is used in both transmit and receive operation.
	//			So it is necessary to terminate the receive interrupt before start accessing communication buffer used for UART sending operations.
	//			Receive interrupt will automatically enable on successful transmission.
	//			Upper layer must call this function before executing transmit related instructions.
	//			If this function is called than upper layer must execute fnUartSendData() or fnUartSetRx() to avoid unnecessary condittion in UART mechanism.

	void fnUartSetTx(void);

	//____ fnUartSetRx  _________________________________________________________________
	//
	// @brief	This function is used to enable the receive interrupt.
	//			Main intense behind this function is to enable the interrupt which was previously disabled by fnUartSetTx() if needed in upper level system design.

	void fnUartSetRx(void);

	//____ fnUartSendComplete  _________________________________________________________________
	//
	// @brief	This function can be used to check the status of UART during transmit operation.
	// @return	TRUE if transmission is complete otherwise returns FALSE.

	int8_t fnUartSendComplete(void);

#endif /* MC_UART_H_ */