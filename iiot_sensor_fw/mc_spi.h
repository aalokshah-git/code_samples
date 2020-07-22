/* -------------------------------------------------------------------------
Filename: mc_spi.h

Purpose: SPI related basic functionality
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

#ifndef MC_SPI_H_
#define MC_SPI_H_

	//_____  I N C L U D E S ______________________________________________________________

	#include "mc_system.h"			// Basic functionality for ATXMEGA MCU system
	#include "mc_gpio.h"			// GPIO functionality for ATXMEGA MCU system

	//_____ M A C R O S ____________________________________________________________________

	//MACROs to support communication with CC1125 chip over SPI
	#define RADIO_READY_WAIT_DELAY_COUNT				0xffffff
	#define CC1125_ADDR_BYTE_BM							0x00ff		//Bit mask for CC1125 address
	#define CC1125_EXT_ADDR_BM							0x2f		//Bit mask for CC1125 extended address

	//Various communication operations for CC112x chip
	#define	RADIO_COMMAND_STROBE						1
	#define RADIO_TRANSMIT_BYTES						2
	#define RADIO_RECEIVE_BYTES							3

	//Radio mechanism controlling
	#define ENABLE_RADIO_SPI_COMM						SPIC.CTRL |= SPI_ENABLE_bm			//Enable radio communication
	#define DISABLE_RADIO_SPI_COMM						SPIC.CTRL &= (~SPI_ENABLE_bm)		//Disable radio communication

	#define ENABLE_RADIO_SPI_COMM_INTERRUPT				SPIC.INTCTRL = SPI_INTLVL_LO_gc		//Enable radio interrupt in low level priority
	#define DISABLE_RADIO_SPI_COMM_INTERRUPT			SPIC.INTCTRL = SPI_INTLVL_OFF_gc	//Disable radio interrupt

	//SPI Operation Related
	#define RADIO_SPI_SS_SET							SET_PINS_LOW(PORTC,PC_SPIC_SS)		//Assert chip select
	#define RADIO_SPI_SS_RESET							SET_PINS_HIGH(PORTC,PC_SPIC_SS)		//De-assert  chip select
	#define RADIO_NOT_READY_FOR_COMM					(GET_PIN_VALUE(PORTC,PC_SPIC_MISO))	//Wait for MISO to get Low after asserting the chip select

	#define START_SPI_COMMUNICATION						RADIO_SPI_SS_SET;\
														_NOP

	#define STOP_SPI_COMMUNICATION						_NOP;\
														RADIO_SPI_SS_RESET

	#define RADIO_CLEAR_SPI_STATUS						SPIC.STATUS &= SPI_IF_bm;			//Clear SPI status flag
	#define RADIO_SPI_STATUS_NOT_SET					(!(SPIC.STATUS & SPI_IF_bm))		//Wait for SPI status flag to get set after byte transfer

	//SPI- Radio communication modes
	#define RADIO_BURST_ACCESS							0x40
	#define RADIO_SINGLE_ACCESS							0x00
	#define RADIO_READ_ACCESS							0x80
	#define RADIO_WRITE_ACCESS							0x00

	//Controlling MACRO for various SPI interfaces connected to Smart Sensors
	#define ENABLE_SMART_SENSOR_SPID_COMM				SPID.CTRL |= SPI_ENABLE_bm
	#define DISABLE_SMART_SENSOR_SPID_COMM				SPID.CTRL &= (~SPI_ENABLE_bm)

	#define ENABLE_SMART_SENSOR_SPID_COMM_INTERRUPT		SPID.INTCTRL = SPI_INTLVL_LO_gc
	#define DISABLE_SMART_SENSOR_SPID_COMM_INTERRUPT	SPID.INTCTRL = SPI_INTLVL_OFF_gc

	#define ENABLE_SMART_SENSOR_SPIE_COMM				SPIE.CTRL |= SPI_ENABLE_bm
	#define DISABLE_SMART_SENSOR_SPIE_COMM				SPIE.CTRL &= (~SPI_ENABLE_bm)

	#define ENABLE_SMART_SENSOR_SPIE_COMM_INTERRUPT		SPIE.INTCTRL = SPI_INTLVL_LO_gc
	#define DISABLE_SMART_SENSOR_SPIE_COMM_INTERRUPT	SPIE.INTCTRL = SPI_INTLVL_OFF_gc

	#define ENABLE_SMART_SENSOR_SPIF_COMM				SPIF.CTRL |= SPI_ENABLE_bm
	#define DISABLE_SMART_SENSOR_SPIF_COMM				SPIF.CTRL &= (~SPI_ENABLE_bm)

	#define ENABLE_SMART_SENSOR_SPIF_COMM_INTERRUPT		SPIF.INTCTRL = SPI_INTLVL_LO_gc
	#define DISABLE_SMART_SENSOR_SPIF_COMM_INTERRUPT	SPIF.INTCTRL = SPI_INTLVL_OFF_gc

	//SPID Operation Related
	#define SMART_SENSOR_SPID_SS_SET					SET_PINS_LOW(PORTD,PD_SPID_SS)		//Assert chip select
	#define SMART_SENSOR_SPID_SS_RESET					SET_PINS_HIGH(PORTD,PD_SPID_SS)		//De-assert  chip select

	#define START_SPID_COMMUNICATION					SMART_SENSOR_SPID_SS_SET;\
														_NOP

	#define STOP_SPID_COMMUNICATION						_NOP;\
														SMART_SENSOR_SPID_SS_RESET

	#define SMART_SENSOR_CLEAR_SPID_STATUS				SPID.STATUS &= SPI_IF_bm;			//Clear SPI status flag
	#define SMART_SENSOR_SPID_STATUS_NOT_SET			(!(SPID.STATUS & SPI_IF_bm))		//Wait for SPI status flag to get set after byte transfer

	//SPIE Operation Related
	#define SMART_SENSOR_SPIE_SS_SET					SET_PINS_LOW(PORTE,PE_SPIE_SS)		//Assert chip select
	#define SMART_SENSOR_SPIE_SS_RESET					SET_PINS_HIGH(PORTE,PE_SPIE_SS)		//De-assert  chip select

	#define START_SPIE_COMMUNICATION					SMART_SENSOR_SPIE_SS_SET;\
														_NOP

	#define STOP_SPIE_COMMUNICATION						_NOP;\
														SMART_SENSOR_SPIE_SS_RESET

	#define SMART_SENSOR_CLEAR_SPIE_STATUS				SPIE.STATUS &= SPI_IF_bm;			//Clear SPI status flag
	#define SMART_SENSOR_SPIE_STATUS_NOT_SET			(!(SPIE.STATUS & SPI_IF_bm))		//Wait for SPI status flag to get set after byte transfer

	//SPIF Operation Related
	#define SMART_SENSOR_SPIF_SS_SET					SET_PINS_LOW(PORTF,PF_SPIF_SS)		//Assert chip select
	#define SMART_SENSOR_SPIF_SS_RESET					SET_PINS_HIGH(PORTF,PF_SPIF_SS)		//De-assert  chip select

	#define START_SPIF_COMMUNICATION					SMART_SENSOR_SPIF_SS_SET;\
														_NOP

	#define STOP_SPIF_COMMUNICATION						_NOP;\
														SMART_SENSOR_SPIF_SS_RESET

	#define SMART_SENSOR_CLEAR_SPIF_STATUS				SPIF.STATUS &= SPI_IF_bm;			//Clear SPI status flag
	#define SMART_SENSOR_SPIF_STATUS_NOT_SET			(!(SPIF.STATUS & SPI_IF_bm))		//Wait for SPI status flag to get set after byte transfer

	//_____ D A T A   S T R U C T U R E S _________________________________________________

	//Structure to manage data during communication with CC1125/CC2520 over SPI
	typedef struct
	{
		uint16_t nCommCount;
		uint16_t nCommAddress;
		uint8_t	 chCommType;
		uint8_t  chCommOperation;
		uint8_t	 chStatusByte;
		uint8_t	 *pchCommBuff;
	}RADIO_COMM_DATA_HANDLE;

	//_____ G L O B A L   D E F I N I T I O N S ______________________________________________________________
	
	//Object of data structure designed to manage resources during communication with CC1125
	extern volatile RADIO_COMM_DATA_HANDLE	ghRadioSpiCommData;

	//_____ F U N C T I O N   D E F I N I T I O N S ___________________________________________________________

	//____ fnResetSpiResource  _________________________________________________________________
	//
	// @brief	Release all the inter dependent SPI resources

	void fnResetSpiResources(void);
	
	//____ fnSpiInitialization  _________________________________________________________________
	//
	// @brief	Responsible for initializing all the SPI interfaces shared between CC1125, CC2520 and various Smart Sensors
	
	void fnSpiInitialization(void);

	//____ fnSpiRadioSendReceiveData  _________________________________________________________________
	//
	// @brief	This function is design to communicate with CC1125 Radio chip over SPI.
	//			As register address space in CC1125 categorized in different sections, This function performs all the required steps to manage communication accordingly
	//			It also manages various modes of communication with CC1125 chip.
	//			For more detail please refer CC1125- Reference manual.
	// @return	FALSE	If unable to get MISO low after asserting CS within finite duration

	int8_t fnSpiRadioSendReceiveData(void);

	//____ fnSPID_SendReceiveOperation  _________________________________________________________________
	//
	// @brief	This function performs send-receive operations with the connected Smart Sensors on this SPI interface
	// @param	pchBuff		Memory resources which holds data to be sent or received over SPI interface
	//			chLength	No of bytes for which to perform send-receive operation
	
	void fnSPID_SendReceiveOperation(uint8_t *pchBuff, uint8_t chLength);
	
	//____ fnSPIE_SendReceiveOperation  _________________________________________________________________
	//
	// @brief	This function performs send-receive operations with the connected Smart Sensors on this SPI interface
	// @param	pchBuff		Memory resources which holds data to be sent or received over SPI interface
	//			chLength	No of bytes for which to perform send-receive operation
	
	void fnSPIE_SendReceiveOperation(uint8_t *pchBuff, uint8_t chLength);
	
	//____ fnSPIF_SendReceiveOperation  _________________________________________________________________
	//
	// @brief	This function performs send-receive operations with the connected Smart Sensors on this SPI interface
	// @param	pchBuff		Memory resources which holds data to be sent or received over SPI interface
	//			chLength	No of bytes for which to perform send-receive operation
	
	void fnSPIF_SendReceiveOperation(uint8_t *pchBuff, uint8_t chLength);

#endif /* MC_SPI_H_ */