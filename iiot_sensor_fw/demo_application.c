/* -------------------------------------------------------------------------
Filename: demo_application.c

Job#: 20473
Date Created: 11/28/2014

Purpose:
This file contains various functions written for specific demo applications during the SENSOR MC firmware implementation.
 
Functions:
fnUartRadioEchoHandler

Interrupts:
-NA-


Author: 

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

#include "system_globals.h"				//Contains global definitions required to handle the various system level task
#include "radio_communication.h"		//Contains scenarios to manage the RF communication 

//_____ G L O B A L  V A R I A B L E S _________________________________________________

//Data buffer for radio communication
volatile uint8_t gchCommBuff[SPI_RADIO_COMM_BUF_SIZE];
	
//Object of UART communication data structure
extern volatile UART_COMM_DATA_HANDLE	ghUartCommData;

//____ fnUartRadioEchoHandler  _________________________________________________________________
//
// Basic chit-chat task works as a gateway between SPI (CC112x-Radio Chip) and UART interface
// It manages data handling in following ways:
//			1> Receives data on UART and send it to the device connected through Radio interface (SPI)
//			2> Receives data on Radio (SPI) and send it to the device connected through UART interface

void fnUartRadioEchoHandler(void)
{
	static uint16_t nRxBytes=0;
	uint16_t nCounter;
	int32_t lDummyCounter=0;
	uint8_t  chDummyByte;
	
	nRxBytes=fnUartReceiveData();						//Check for possible reception on UART
	
	if(nRxBytes>0)
	{
		if(nRxBytes>=MAX_RADIO_PACKET_LENGTH)			//If received count exceeds the maximum value than vanish the reception
		{
			nRxBytes=0;									//Ignore current reception
			SEND_HIGH_LEVEL_DEBUG_STRING("E2\n");		//Error Code: "E2" (Buffer Overflow)
		}
		else
		{
			fnUartSetTx();								//Put UART in transmit mode and avoid reception during data processing
			
			//To set the TX mode in Radio chip first put Radio chip in IDLE mode
			fnCC112xSendReceiveHandler(RADIO_COMMAND_STROBE,CC112X_SIDLE,1,NULL);
			
			//Copy data from UART received buffer to SPI transmit buffer (Variable length mode is selected so first byte of SPI transmit buffer includes length)
			for(nCounter=0;nCounter<nRxBytes;nCounter++)
			{
				gchCommBuff[nCounter+1]=ghUartCommData.chCommBuff[nCounter];	//Variable mode is selected
			}
			
			fnUartSetRx();								//Put UART in receive mode after successful data processing
			
			*gchCommBuff=nRxBytes;
			fnCC112xSendReceiveHandler(RADIO_TRANSMIT_BYTES,CC112X_TXFIFO,nRxBytes+1,gchCommBuff);		//Transmit bytes to TX FIFO of Radio chip
			fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,CC112X_MARCSTATE,1,&chDummyByte);			//Check the Radio chip status
			
			//If error related to TX FIFO of Radio chip than flush the FIFO
			if((chDummyByte & 0x1f) == CC112X_STATE_TXFIFO_ERROR)
			{
				SEND_HIGH_LEVEL_DEBUG_STRING("CC1125-TX FIFO ERROR\n");
				fnCC112xSendReceiveHandler(RADIO_COMMAND_STROBE,CC112X_SFTX,1,NULL);
			}
			else
			{
				//Start sending data on Radio by sending command strobe "STX"
				fnCC112xSendReceiveHandler(RADIO_COMMAND_STROBE,CC112X_STX,1,NULL);
				nRxBytes=0;
				while(fnCC112xSendDataComplete())		//Wait for the transmit complete interrupt from Radio chip
				{
					if(++lDummyCounter>1000000)			//Avoid stuck if GPIO2 interrupt is not received
					{
						SEND_HIGH_LEVEL_DEBUG_STRING("CC1125-GPIO0 INTERRUPT TIMEOUT\n");
						fnCC112xSendReceiveHandler(RADIO_COMMAND_STROBE,CC112X_SFTX,1,NULL);
						break;
					}
				}
			}
		}
	}
	
	else
	{
		if(!fnCC112xReceiveDataAvail())				//Wait for the receive avail interrupt from Radio chip
		{
			//Get count of total bytes received
			fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,CC112X_NUM_RXBYTES,1,&chDummyByte);
			
			fnUartSetTx();							//Put UART in transmit mode and avoid reception during data processing
			fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,CC112X_RXFIFO,chDummyByte,gchCommBuff);
			
			if(!(gchCommBuff[chDummyByte-1] & 0x80))
			{
				SEND_HIGH_LEVEL_DEBUG_STRING("CC1125-CRC ERROR\n");
			}
			else
			{
				nRxBytes=chDummyByte-3;

				//Copy data from SPI received buffer to UART transmit buffer (Variable length mode is selected so first byte of SPI receive buffer includes length)
				for(nCounter=0;nCounter<nRxBytes;nCounter++)
				{
					ghUartCommData.chCommBuff[nCounter]=gchCommBuff[nCounter+1];
				}
				
				ghUartCommData.nTxCount=nRxBytes;
				fnUartSendData();
			}
			
			//Put Radio chip in receiving mode by sending command strobe "SRX"
			fnCC112xSendReceiveHandler(RADIO_COMMAND_STROBE,CC112X_SRX,1,NULL);
		}
	}
	
	return;
}

