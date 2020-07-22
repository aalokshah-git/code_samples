/* -------------------------------------------------------------------------
Filename: mc_i2c.h

Purpose: I2C related basic functionality including initialization, transmission and reception
Date Created: 11/05/2014

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

#ifndef MC_I2C_H_
#define MC_I2C_H_

	//_____  I N C L U D E S ______________________________________________________________

	#include "mc_system.h"			// Basic functionality for ATXMEGA MCU system

	//_____ M A C R O S ____________________________________________________________________

	//I2C interrupt related controlling
	#define ENABLE_I2C_INTERRUPTS		TWIF.MASTER.CTRLA |=TWI_MASTER_INTLVL_LO_gc				//Enable I2C with lower level interrupt
	#define DISABLE_I2C_INTERRUPTS		TWIF.MASTER.CTRLA |=TWI_MASTER_INTLVL_OFF_gc			//Disable I2C interrupt
	#define ENABLE_I2C_WRITE_INT		TWIF.MASTER.CTRLA |=TWI_MASTER_WIEN_bm					//write complete interrupt enable
	#define ENABLE_I2C_READ_INT			TWIF.MASTER.CTRLA |=TWI_MASTER_RIEN_bm					//read complete interrupt enable
	#define DISABLE_I2C_WRITE_INT		TWIF.MASTER.CTRLA &= (~TWI_MASTER_WIEN_bm)				//write complete interrupt disable
	#define DISABLE_I2C_READ_INT		TWIF.MASTER.CTRLA &= (~TWI_MASTER_RIEN_bm)				//read complete interrupt disable

	//Controlling I2C Module
	#define ENABLE_I2C_MODULE			TWIF.MASTER.CTRLA |= TWI_MASTER_ENABLE_bm				//Enable I2C
	#define DISABLE_I2C_MODULE			TWIF.MASTER.CTRLA &= (~TWI_MASTER_ENABLE_bm)			//Disable I2C

	//Supported I2C baud rate (Baud=(fsys/(2*ftwi))-5)
	#define SET_I2C_BAUD_100KHZ			TWIF.MASTER.BAUD = 0x4B									//100Khz at 16MHz System freq
	#define SET_I2C_BAUD_400KHZ			TWIF.MASTER.BAUD = 0x0F									//400KHz at 16MHz System freq

	//I2C status register
	#define I2C_MASTER_STATUS			TWIF.MASTER.STATUS
	#define I2C_BUSSTATE_BM				0x03					//I2C bus state bit mask

	//I2C Buffer size limit
	#define I2C_COMM_BUFFER_SIZE		50						//I2C communication buffer size

	//I2C Operating Modes
	#define I2C_READ_MODE				00
	#define I2C_WRITE_MODE				01
	#define I2C_READ_WRITE_MODE			02

	//Various I2C return types
	#define I2C_ERROR					1
	#define I2C_OP_COMPLETE				0
	#define I2C_RUNNING					(-1)

	//_____ D A T A   S T R U C T U R E S _________________________________________________

	//Data Structure to manage the I2C Communication
	typedef struct
	{
		uint8_t		chOperationMode;				//I2C Software defined Operation mode
		uint8_t		chCommAddress;					//I2C Device Address
		uint8_t		chCommWriteCount;
		uint8_t		chCommReadCount;
		uint8_t		chCommIndexCounter;
		uint8_t		chCommComplete;			
		uint8_t		chCommBuff[I2C_COMM_BUFFER_SIZE];
	}I2C_COMM_DATA_HANDLE;

	//_____ G L O B A L   D E F I N I T I O N S ______________________________________________________________

	//Declared in mc_i2c.c
	extern volatile uint8_t		gchI2cOccupiedFlag;
		
	//Object of data structure to manage the resources during I2C communication
	extern volatile I2C_COMM_DATA_HANDLE	ghI2cCommData;
	
	//_____ M A C R O S ____________________________________________________________________
	
	//MACROs for managing the shared resources on I2C bus
	#define ACQUIRE_I2C_INTERFACE		gchI2cOccupiedFlag=1
	#define RELEASE_I2C_INTERFACE		gchI2cOccupiedFlag=0
		
	//_____ F U N C T I O N   D E F I N I T I O N S ___________________________________________________________

	//_____ fnI2cInitialization ____________________________________________________________________
	//
	// @brief	Steps to initialize I2C in AVR-XMEGA MCU:
	//					1> Set baud rate on which to perform the I2C communication.
	//					2> Configure I2C for master mode operation with necessary settings.
	//					3> At start up I2C bus may be in UNKNOWN state so put it forcefully in IDLE state.
	//					4> Reset/configure all nessery global definitions.

	void fnI2cInitialization(void);

	//_____ fnI2cSendReceiveOperation ____________________________________________________________________
	//
	// @brief	This function will get called when upper layer wants to communicate over I2C interface. Following steps must be executed for reliable communication:
	//			1> fnCheckI2CStatus() must be executed to check the availability of the I2C bus
	//			2> If I2C bus is available than before performing any communication operation use ACQUIRE_I2C_INTERFACE to take control over I2C related software resources
	//			3> If wants to send data bytes to slave than chCommBuff of I2C_COMM_DATA_HANDLE must be filled with data bytes before executing this function
	//			4> If wants to receive data bytes from slave than execute this function by specifying valid parameters and chCommBuff of I2C_COMM_DATA_HANDLE will haev the received data
	//			5> It is necessary to release control from I2C relaetd software resources by using RELEASE_I2C_INTERFACE
	// @param	chI2cAddr		Slave Address to which Master/SENSOR MC wants to communicate
	//			chWriteCount	No of data bytes to send to Slave
	//			chReadCount		No of data bytes to receive from Slave
	// @return	FALSE if parameters are not within the boundary otherwise returns TRUE

	int8_t fnI2cSendReceiveOperation(uint8_t chI2cAddr,uint8_t chWriteCount, uint8_t chReadCount);

	//_____ fnI2cFreeForOperation ____________________________________________________________________
	//
	// @brief	This function can be used for identifying the status of the I2C bus when it is in operation.
	// @return	I2C_OP_COMPLETE if I2C communication is complete, I2C_RUNNING if I2C is in operations and I2C_ERROR if any type of arbitration or bus error occurs.

	int8_t fnI2cFreeForOperation(void);

	//_____ fnCheckI2CStatus ____________________________________________________________________
	//
	// @brief	This function can be used for identifying the availability of the I2C bus.
	// @return	TRUE if I2C bus is free for operation otherwise returns FALSE.

	int8_t fnCheckI2CStatus(void);
	
	//_____ fnResetI2cResources ____________________________________________________________________
	//
	// @brief	Release all the inter dependent I2C resources
	
	void fnResetI2cResources(void);

#endif /* MC_I2C_H_ */
