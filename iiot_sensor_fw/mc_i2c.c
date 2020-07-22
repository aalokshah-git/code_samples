/* -------------------------------------------------------------------------
Filename: mc_i2c.c

Date Created: 11/05/2014

Purpose: I2C related functionality including initialization, transmission and reception+

Functions:
fnI2cInitialization					Initialize the I2C for master mode of operation
fnI2cSendReceiveOperation			Start I2C communication
fnI2cFreeForOperation				Check for I2C status in operating condition
fnCheckI2CStatus					Check for Availability of I2C bus
fnResetI2cResources					Reset all I2C related shared resources

Interrupt :
TWIF_TWIM_vect						Manage I2C Master related communications in various software defined operation modes


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

#include "mc_i2c.h"				// I2C functionality for ATXMEGA MCU system
#include "mc_gpio.h"			// GPIO functionality for ATXMEGA MCU system

//_____ G L O B A L   D E F I N I T I O N S _________________________________________________

//Object of data structure used to manage the resources during I2C communication
volatile I2C_COMM_DATA_HANDLE	ghI2cCommData;

//Holds the status value of I2C bus when it is operational
static volatile uint8_t		gchI2cStatus;

//Will get set when any type of arbitration or bus error arise during communication
static volatile uint8_t		gchAckError;

//Variable used to indicate the availability of the I2C bus
//Will get set by the upper layer system when it wants to communicate over I2C bus
//Will get reset by the upper level system when  it doesn't want to communicate further over I2C bus
volatile uint8_t		gchI2cOccupiedFlag;

//_____ fnI2cInitialization ____________________________________________________________________
//
// @brief	Steps to initialize I2C in AVR-XMEGA MCU:
//					1> Set baud rate on which to perform the I2C communication.
//					2> Configure I2C for master mode operation with necessary settings.
//					3> At start up I2C bus may be in UNKNOWN state so put it forcefully in IDLE state.
//					4> Reset/configure all nessery global definitions.

void fnI2cInitialization(void)
{
	//Configure I2C for 400KHZ
	SET_I2C_BAUD_400KHZ;
	
	//Enable various interrupts for I2C interface
	ENABLE_I2C_INTERRUPTS;
	ENABLE_I2C_WRITE_INT;
	ENABLE_I2C_READ_INT;
	
	//Acknowledgment Action-ACK set for Master read mode
	TWIF.MASTER.CTRLC &= (~TWI_MASTER_ACKACT_bm);			
	
	//Put I2C bus forcefully in idle mode
	I2C_MASTER_STATUS |= TWI_MASTER_BUSSTATE_IDLE_gc;	
	
	//Enable I2C module	
	ENABLE_I2C_MODULE;
	return;
}

//_____ fnResetI2cResources ____________________________________________________________________
//
// @brief	Release all the inter dependent I2C resources

void fnResetI2cResources(void)
{
	gchAckError = RESET_FLAG;
	RELEASE_I2C_INTERFACE;
	return;
}

//_____ fnCheckI2CStatus ____________________________________________________________________
//
// @brief	This function can be used for identifying the availability of the I2C bus.
// @return	TRUE if I2C bus is free for operation otherwise returns FALSE.

int8_t fnCheckI2CStatus(void)
{
	if(gchI2cOccupiedFlag)		//Check for I2C occupancy
	{
		return RETURN_FALSE;
	}
	
	return RETURN_TRUE;
}

//_____ fnI2cFreeForOperation ____________________________________________________________________
//
// @brief	This function can be used for identifying the status of the I2C bus when it is in operation.
// @return	I2C_OP_COMPLETE if I2C communication is complete, I2C_RUNNING if I2C is in operations and I2C_ERROR if any type of arbitration or bus error occurs.

int8_t fnI2cFreeForOperation(void)
{
	if(gchAckError == SET_FLAG)
	{
		return I2C_ERROR;							//I2C Bus Error
	}
	
	else if(ghI2cCommData.chCommComplete)
	{
		ghI2cCommData.chCommComplete = RESET_FLAG;
		return I2C_OP_COMPLETE;						//I2C Operation Complete
	}
	
	return I2C_RUNNING;								//I2C is in operation
}

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

int8_t fnI2cSendReceiveOperation(uint8_t chI2cAddr,uint8_t chWriteCount, uint8_t chReadCount)
{
	ghI2cCommData.chCommAddress=chI2cAddr;
	ghI2cCommData.chCommReadCount=chReadCount;
	ghI2cCommData.chCommWriteCount=chWriteCount;

	//Reset control flags and counter values
	gchAckError = RESET_FLAG;
	ghI2cCommData.chCommComplete = RESET_FLAG;
	ghI2cCommData.chCommIndexCounter = RESET_COUNTER;
	
	//Check for boundary
	if(chWriteCount>I2C_COMM_BUFFER_SIZE || chReadCount>I2C_COMM_BUFFER_SIZE)
	{
		return RETURN_FALSE;	
	}
	
	//Check for the I2C operation mode on the basis of the parameters
	if(chWriteCount> RESET_COUNTER && chReadCount> RESET_COUNTER)
	{
		ghI2cCommData.chOperationMode=I2C_READ_WRITE_MODE;
	}
	else if(chWriteCount > RESET_COUNTER)
	{
		ghI2cCommData.chOperationMode=I2C_WRITE_MODE;
	}
	else if(chReadCount > RESET_COUNTER)
	{
		ghI2cCommData.chOperationMode=I2C_READ_MODE;
	}
	else
	{
		return RETURN_FALSE;
	}
	
	// Check for I2C bus state
	// In normal conditions bus state must be Idle or Owner while performing the communication
	gchI2cStatus=I2C_MASTER_STATUS;
	
	if((gchI2cStatus & I2C_BUSSTATE_BM) == TWI_MASTER_BUSSTATE_UNKNOWN_gc)
	{
		SEND_DEBUG_STRING("I2C: STATE UNKNOWN\n");
		I2C_MASTER_STATUS |= TWI_MASTER_BUSSTATE_IDLE_gc;				//if bs satte is Unknown than set it to Idle forcefully
		return RETURN_FALSE;
	}
	else if ((gchI2cStatus & I2C_BUSSTATE_BM) == TWI_MASTER_BUSSTATE_BUSY_gc)
	{
		SEND_DEBUG_STRING("I2C: STATE BUSY\n");
		return RETURN_FALSE;
	}
	else if ((gchI2cStatus & I2C_BUSSTATE_BM) == TWI_MASTER_BUSSTATE_OWNER_gc)
	{
		SEND_DEBUG_STRING("I2C: STATE OWNER\n");
	}
	
	// If Write Command: Send the START + ADDR + 'R/_W = 0'
	if (ghI2cCommData.chOperationMode==I2C_WRITE_MODE || ghI2cCommData.chOperationMode==I2C_READ_WRITE_MODE)
	{
		TWIF.MASTER.ADDR = (ghI2cCommData.chCommAddress<< BIT_0_bm) & (~BIT_0_bm);
	}
	
	// If Write Command: Send the START + ADDR + 'R/_W = 1'
	else if(ghI2cCommData.chOperationMode==I2C_READ_MODE)
	{
		TWIF.MASTER.ADDR = (ghI2cCommData.chCommAddress<<BIT_0_bm) | BIT_0_bm;
	}
	
	else
	{
		return RETURN_FALSE;
	}
	
	return RETURN_TRUE;			//I2C communication started rest will be taken care in interrupt routines
}

//_____ I S R - I 2 C   M A S T E R   C O M M U N I C A T I O N ____________________________________________________________________
//
// @brief	ISR for I2C Master Operations:
//			This interrupt occurs for every single event occur on I2C bus during communication between master and dedicate slave
//			Following categories may arise during I2C communication and interrupts the MCU:
//			1> Arbitration or Bus Error:
//				In this case ISR will set the gchAckError flag and complete the operation.
//			2> NACK received during communication:
//				In this case ISR will send Stop condition to I2C bus and complete the operation.
//			3> Successful write opeartion:
//				If I2C mode is WRITE_MODE than it will send next data byte till all the data bytes will get sent and generate the stop condition.
//				If I2C mode is WRITE_READ_MODE than it will send next data byte till all the data bytes will get sent and generate repeated start condition to put I2C in receive mode.
//			4> Successful read operation:
//				If I2C mode is READ_MODE than it will wait for receiving data bytes till the required count get received and generate the stop condition.

ISR(TWIF_TWIM_vect)
{
	volatile int8_t chDiffValue;
	
	gchI2cStatus=I2C_MASTER_STATUS;
	gchNewInterrupt = SET_NEW_ISR_FLAG;
	
	//If Arbitration Lost or Bus Error
	if (gchI2cStatus & TWI_MASTER_ARBLOST_bm)
	{
		I2C_MASTER_STATUS |= TWI_MASTER_ARBLOST_bm;
		gchAckError = SET_FLAG;
	}
	else if(gchI2cStatus & TWI_MASTER_BUSERR_bm)
	{
		I2C_MASTER_STATUS |= TWI_MASTER_BUSERR_bm;
		gchAckError = SET_FLAG;
	}
	
	//Successful Write
	else if (gchI2cStatus & TWI_MASTER_WIF_bm)
	{
		//NACK received in last data transfer
		if (I2C_MASTER_STATUS & TWI_MASTER_RXACK_bm)
		{
			TWIF.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
			I2C_MASTER_STATUS |= TWI_MASTER_RXACK_bm;
			gchAckError = SET_FLAG;
		}
		
		else if(ghI2cCommData.chOperationMode==I2C_WRITE_MODE || ghI2cCommData.chOperationMode==I2C_READ_WRITE_MODE)
		{
			chDiffValue=ghI2cCommData.chCommWriteCount-ghI2cCommData.chCommIndexCounter;
			
			if (chDiffValue <= RESET_VALUE)
			{
				TWIF.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;							//Required bytes transmitted - Terminate the operation
				ghI2cCommData.chCommComplete = SET_FLAG;
			}
			else
			{
				TWIF.MASTER.DATA=ghI2cCommData.chCommBuff[ghI2cCommData.chCommIndexCounter++];
				
				if((chDiffValue == SET_VALUE) && ghI2cCommData.chOperationMode==I2C_READ_WRITE_MODE)
				{
					ghI2cCommData.chCommIndexCounter = RESET_COUNTER;
					TWIF.MASTER.ADDR = (ghI2cCommData.chCommAddress<< BIT_0_bm) | BIT_0_bm;			//Repeated start to put I2C in receiving mode
				}
			}
		}

		I2C_MASTER_STATUS |= TWI_MASTER_WIF_bm;
	}
	
	//Successful Read
	else if (gchI2cStatus & TWI_MASTER_RIF_bm)
	{
		ghI2cCommData.chCommBuff[ghI2cCommData.chCommIndexCounter++] = TWIF.MASTER.DATA;
		
		if (ghI2cCommData.chCommIndexCounter < ghI2cCommData.chCommReadCount)
		{
			TWIF.MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;		//Wait for next byte to get received
		}
		else
		{
			ghI2cCommData.chCommComplete = SET_FLAG;
			TWIF.MASTER.CTRLC = (TWI_MASTER_CMD_STOP_gc | TWI_MASTER_ACKACT_bm);	//Required bytes received - Terminate the operation
		}
		
		I2C_MASTER_STATUS |= TWI_MASTER_RIF_bm;
	}
}


