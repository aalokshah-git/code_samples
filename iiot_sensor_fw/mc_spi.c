/* -------------------------------------------------------------------------
Filename: mc_spi.c

Date Created: 10/08/2014

Purpose: SPI related functionality including initialization, transmission and reception of interfaces

Functions:
fnSpiInitialization				Responsible for initialization of all the SPI interfaces (Calls fnSpiRadioInitialization and fnSpiSmartSensorInitialization)
fnSpiRadioInitialization		Initialize SPI interface dedicated to CC1125/CC2520
fnSpiSmartSensorInitialization	Initialize SPI interfaces dedicated to Smart Sensors
fnSpiRadioSendReceiveData		Function which handles communication operations with CC1125/CC2520 over SPI
fnSPID_SendReceiveOperation		Function handles communication with the Smart Sensors connected on SPID interface
fnSPIE_SendReceiveOperation		Function handles communication with the Smart Sensors connected on SPIE interface
fnSPIF_SendReceiveOperation		Function handles communication with the Smart Sensors connected on SPIF interface

Interrupts:
-NA-


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

#include "mc_spi.h"				// SPI functionality for ATXMEGA MCU system

//_____ G L O B A L   D E F I N I T I O N S _________________________________________________

//Object of SPI-CC112X communication data structure
volatile RADIO_COMM_DATA_HANDLE		ghRadioSpiCommData;

//____ fnSpiRadioInitialization  _________________________________________________________________
//
// @brief	Initialize the SPI interface to effectively communicate with CC1125/CC2520 radio chip by following steps:
//				1> With appropriate prescalar value decide the baudrate for SPI interface.
//				2> Configure SPI for Master Mode with Mode-0 and MSB send first.
//				3> Reset/configure all nessery global definitions including interrupts.

inline void fnSpiRadioInitialization(void)
{
	//At 16MHZ Clock Setting: SPI is configured to communicate at 1MHZ with the scalar of value 4
	//Configured as Master with MODE-0 and MSB transmit first
	SPIC.CTRL = (SPI_MASTER_bm | SPI_MODE_0_gc | SPI_PRESCALER_DIV16_gc);
	
	//Reset necessary definitions
	ghRadioSpiCommData.nCommCount=RESET_COUNTER;

	//Disable SPI interrupts (designed polling based)
	DISABLE_RADIO_SPI_COMM_INTERRUPT;				
	
	//Enable SPI interfaces for communication
	ENABLE_RADIO_SPI_COMM;
	
	return;
}

//____ fnSpiSmartSensorInitialization  _________________________________________________________________
//
// @brief	Initialize the SPI interface to effectively communicate with Smart Sensors by following steps:
//				1> With appropriate prescalar value decide the baudrate for various SPI interfaces.
//				2> Configure SPI for Master Mode with Mode-0 and MSB send first.
//				3> Reset/configure all nessery global definitions including interrupts.

inline void fnSpiSmartSensorInitialization(void)
{
	//At 16MHZ Clock Setting: SPI is configured to communicate at 8MHZ with the scalar of value 4 and setting the clock doubling
	//Configured as Master with MODE-0 and MSB transmit first
	SPID.CTRL = (SPI_MASTER_bm | SPI_MODE_0_gc | SPI_PRESCALER_DIV4_gc | SPI_CLK2X_bm);
	SPIE.CTRL = (SPI_MASTER_bm | SPI_MODE_0_gc | SPI_PRESCALER_DIV4_gc | SPI_CLK2X_bm);
	SPIF.CTRL = (SPI_MASTER_bm | SPI_MODE_0_gc | SPI_PRESCALER_DIV4_gc | SPI_CLK2X_bm);
	
	//Disable SPI interrupts (designed polling based)
	DISABLE_SMART_SENSOR_SPID_COMM_INTERRUPT;
	DISABLE_SMART_SENSOR_SPIE_COMM_INTERRUPT;
	DISABLE_SMART_SENSOR_SPIF_COMM_INTERRUPT;
	
	//Enable SPI interfaces for communication
	ENABLE_SMART_SENSOR_SPID_COMM;
	ENABLE_SMART_SENSOR_SPIE_COMM;
	ENABLE_SMART_SENSOR_SPIF_COMM;
	
	return;
}

//____ fnSpiInitialization  _________________________________________________________________
//
// @brief	Responsible for initializing all the SPI interfaces shared between CC1125, CC2520 and various Smart Sensors

void fnSpiInitialization(void)
{
	fnSpiRadioInitialization();
	fnSpiSmartSensorInitialization();
	
	return;
}

//____ fnResetSpiResource  _________________________________________________________________
//
// @brief	Release all the inter dependent SPI resources

void fnResetSpiResources(void)
{
	//TBD (Currently Not Required)
	return;
}

//____ fnSpiRadioSendReceiveData  _________________________________________________________________
//
// @brief	This function is design to communicate with CC1125 Radio chip over SPI.
//			As register address space in CC1125 categorized in different sections, This function performs all the required steps to manage communication accordingly
//			It also manages various modes of communication with CC1125 chip.
//			For more detail please refer CC1125- Reference manual.
// @return	FALSE	If unable to get MISO low after asserting CS within finite duration

int8_t fnSpiRadioSendReceiveData(void)
{
	uint32_t nRadioReadyDelayCounter = RADIO_READY_WAIT_DELAY_COUNT;			//Initialize dummy wait counter
	uint8_t	 chDataCounter = RESET_COUNTER;
	uint8_t  chAddrByte = RESET_VALUE;
	uint8_t  chExtAddrByte = RESET_VALUE;
	uint8_t  chCheckAddrByte = RESET_FLAG;
	uint8_t  chDummyByte = RESET_VALUE;
	
	//Split address in to multiple bytes
	chCheckAddrByte=(ghRadioSpiCommData.nCommAddress) >> BIT_8_bp;
	chAddrByte=(ghRadioSpiCommData.nCommAddress) & CC1125_ADDR_BYTE_BM;
	
	//Check if current address comes under extended address space??
	if(chCheckAddrByte == CC1125_EXT_ADDR_BM)
	{
		chExtAddrByte=chAddrByte;
		chAddrByte=chCheckAddrByte;
	}
	
	//Include SPI communication mode and read/write operation flag with address byte (As per CC112x communication protocol)
	//Upper level must manage the value of chCommType***
	if(ghRadioSpiCommData.chCommType == RADIO_TRANSMIT_BYTES)
	{
		chAddrByte |= (RADIO_BURST_ACCESS | RADIO_WRITE_ACCESS);
	}
	else if(ghRadioSpiCommData.chCommType == RADIO_RECEIVE_BYTES)
	{
		chAddrByte |= (RADIO_BURST_ACCESS | RADIO_READ_ACCESS);
	}
	else
	{
		chAddrByte |= RADIO_SINGLE_ACCESS;
	}
	
	//Start SPI communication by asserting the chip select
	START_SPI_COMMUNICATION;
	
	//Wait for Radio chip to become ready for communication (MISO will get low as an ACK when Radio chip is ready)
	while(RADIO_NOT_READY_FOR_COMM)
	{
		//Dummy counter to avoid unwanted stuck in loop
		if(!(--nRadioReadyDelayCounter))
		{
			//Stop SPI communication by de-asserting the chip select (high)
			STOP_SPI_COMMUNICATION;
			return RETURN_FALSE;			//Error
		}
	}

	//Send address byte
	SPIC.DATA=chAddrByte;
	while(RADIO_SPI_STATUS_NOT_SET);				//Wait for byte to shifted in/out
	ghRadioSpiCommData.chStatusByte=SPIC.DATA;		//Status Byte in response to the Address Byte
	
	//If address comes under extended register space than send extended address byte
	if(chCheckAddrByte == CC1125_EXT_ADDR_BM)
	{
		SPIC.DATA=chExtAddrByte;
		while(RADIO_SPI_STATUS_NOT_SET);			//Wait for byte to shifted in/out
		chExtAddrByte=SPIC.DATA;					//Just Dummy Read
	}

	//Send/Receive bytes specified in nCommCount by upper level system
	if(ghRadioSpiCommData.chCommOperation==RADIO_TRANSMIT_BYTES)
	{
		for(chDataCounter= RESET_COUNTER; chDataCounter<ghRadioSpiCommData.nCommCount; chDataCounter++)
		{
			SPIC.DATA=ghRadioSpiCommData.pchCommBuff[chDataCounter];
			while(RADIO_SPI_STATUS_NOT_SET);			//Wait for byte to shifted in/out
			chDummyByte=SPIC.DATA;
		}
	}
	else if(ghRadioSpiCommData.chCommOperation==RADIO_RECEIVE_BYTES)
	{
		chDummyByte = RESET_VALUE;
		for(chDataCounter = RESET_COUNTER; chDataCounter<ghRadioSpiCommData.nCommCount; chDataCounter++)
		{
			SPIC.DATA=chDummyByte;
			while(RADIO_SPI_STATUS_NOT_SET);			//Wait for byte to shifted in/out
			ghRadioSpiCommData.pchCommBuff[chDataCounter]=SPIC.DATA;
		}
	}
	
	//Stop SPI communication by de-asserting the chip select
	_NOP;
	STOP_SPI_COMMUNICATION;
	return RETURN_TRUE;							//Success
}

//____ fnSPID_SendReceiveOperation  _________________________________________________________________
//
// @brief	This function performs send-receive operations with the connected Smart Sensors on this SPI interface
// @param	pchBuff		Memory resources which holds data to be sent or received over SPI interface
//			chLength	No of bytes for which to perform send-receive operation

void fnSPID_SendReceiveOperation(uint8_t *pchBuff, uint8_t chLength)
{
	uint8_t chDataCounter = RESET_COUNTER;
	
	//Start SPI communication by asserting the chip select
	START_SPID_COMMUNICATION;
	
	for(chDataCounter = RESET_COUNTER; chDataCounter<chLength; chDataCounter++)
	{
		SPID.DATA=pchBuff[chDataCounter];
		while(SMART_SENSOR_SPID_STATUS_NOT_SET);				//Wait for byte to shifted in/out
		pchBuff[chDataCounter]=SPID.DATA;
	}
	
	//Stop SPI communication by de-asserting the chip select
	_NOP;
	STOP_SPID_COMMUNICATION;
	
	return;
}

//____ fnSPIE_SendReceiveOperation  _________________________________________________________________
//
// @brief	This function performs send-receive operations with the connected Smart Sensors on this SPI interface
// @param	pchBuff		Memory resources which holds data to be sent or received over SPI interface
//			chLength	No of bytes for which to perform send-receive operation

void fnSPIE_SendReceiveOperation(uint8_t *pchBuff, uint8_t chLength)
{
	uint8_t chDataCounter = RESET_COUNTER;
	
	//Start SPI communication by asserting the chip select
	START_SPIE_COMMUNICATION;
	
	for(chDataCounter = RESET_COUNTER; chDataCounter<chLength; chDataCounter++)
	{
		SPIE.DATA=pchBuff[chDataCounter];
		while(SMART_SENSOR_SPIE_STATUS_NOT_SET);				//Wait for byte to shifted in/out
		pchBuff[chDataCounter]=SPIE.DATA;
	}
	
	//Stop SPI communication by de-asserting the chip select
	_NOP;
	STOP_SPIE_COMMUNICATION;
	
	return;
}

//____ fnSPIF_SendReceiveOperation  _________________________________________________________________
//
// @brief	This function performs send-receive operations with the connected Smart Sensors on this SPI interface
// @param	pchBuff		Memory resources which holds data to be sent or received over SPI interface
//			chLength	No of bytes for which to perform send-receive operation

void fnSPIF_SendReceiveOperation(uint8_t *pchBuff, uint8_t chLength)
{
	uint8_t chDataCounter = RESET_COUNTER;
	
	//Start SPI communication by asserting the chip select
	START_SPIF_COMMUNICATION;
	
	for(chDataCounter = RESET_COUNTER; chDataCounter<chLength; chDataCounter++)
	{
		SPIF.DATA=pchBuff[chDataCounter];
		while(SMART_SENSOR_SPIF_STATUS_NOT_SET);				//Wait for byte to shifted in/out
		pchBuff[chDataCounter]=SPIF.DATA;
	}
	
	//Stop SPI communication by de-asserting the chip select
	_NOP;
	STOP_SPIF_COMMUNICATION;
	
	return;
}