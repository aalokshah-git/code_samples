/* -------------------------------------------------------------------------
Filename: radio_communication.c

Date Created: 10/20/2014

Purpose: CC112X- Radio chip communication driver with functionality and easy interface to upper layer system

Functions:
fnCC112xSendReceiveHandler			Manages communication with CC112x over SPI
fnRegisterConfigurationCC112X		Configures CC112x register settings
fnManualConfigurationCC112X			Perform calibration on CC112x radio chip
fnCC112xSendDataComplete			To check end of transmission in CC112x
fnCC112xReceiveDataAvail			To check possible reception in CC112x
fnRead8BitRssi						To fetch the valid RSSI value
fnRadioCC1125Initialization			Calls fnRegisterConfigurationCC112X and fnManualConfigurationCC112X

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

#include "radio_communication.h"				//Contains scenarios to manage the RF communication

//______ M A C R O S  ___________________________________________________________

#define	 VERIFY_CC1125_REGISTERS		1		//To perform CC112X-Register configuration checking (Extended Debugging) 

//_____  G L O B A L   C O N S T A N T S ______________________________________________________________

//Contains value with which to initialize the CC112x configuration registers
const CC112X_REGSET hRegSet[]=
{
	{CC112X_IOCFG3,            0xB0},
	{CC112X_IOCFG2,            0xB0},	
	{CC112X_IOCFG1,            0xB0},
	{CC112X_IOCFG0,            0x06},		//Configure to take interrupt from CC112x- At Tx and Rx event
	{CC112X_DEVIATION_M,       0x26},
	{CC112X_MODCFG_DEV_E,      0x1D},
	{CC112X_DCFILT_CFG,        0x1C},
	{CC112X_PREAMBLE_CFG0,     0x2A},
	{CC112X_IQIC,              0xCE},
	{CC112X_CHAN_BW,           0x0E},
	{CC112X_MDMCFG0,           0x05},
	{CC112X_SYMBOL_RATE2,      0x5A},
	{CC112X_SYMBOL_RATE1,      0x36},
	{CC112X_SYMBOL_RATE0,      0xE3},
	{CC112X_AGC_REF,           0x20},
	{CC112X_AGC_CS_THR,        0x19},
	{CC112X_AGC_CFG3,          0x91},
	{CC112X_AGC_CFG2,          0x20},
	{CC112X_AGC_CFG1,          0x2D},
	{CC112X_AGC_CFG0,          0x5F},
	{CC112X_FIFO_CFG,          0x00},
	{CC112X_SETTLING_CFG,      0x03},
	{CC112X_FS_CFG,            0x1B},
	{CC112X_PKT_CFG1,          0x05},		//Append CRC,RSSI in received data and variable length mode is selected
	{CC112X_PKT_CFG0,          0x20},
	{CC112X_PA_CFG2,           0x7C},
	{CC112X_PA_CFG0,           0x7E},
	{CC112X_PKT_LEN,           0xFF},
	{CC112X_IF_MIX_CFG,        0x00},
	{CC112X_FREQOFF_CFG,       0x22},
	{CC112X_TOC_CFG,           0x0A},
	{CC112X_FREQ2,             0x5C},
	{CC112X_FREQ1,             0x66},
	{CC112X_FREQ0,             0x66},
	{CC112X_IF_ADC0,           0x05},
	{CC112X_FS_DIG1,           0x00},
	{CC112X_FS_DIG0,           0x5F},
	{CC112X_FS_CAL1,           0x40},
	{CC112X_FS_CAL0,           0x0E},
	{CC112X_FS_DIVTWO,         0x03},
	{CC112X_FS_DSM0,           0x33},
	{CC112X_FS_DVC0,           0x17},
	{CC112X_FS_PFD,            0x50},
	{CC112X_FS_PRE,            0x6E},
	{CC112X_FS_REG_DIV_CML,    0x14},
	{CC112X_FS_SPARE,          0xAC},
	{CC112X_FS_VCO0,           0xB4},
	{CC112X_XOSC5,             0x0E},
	{CC112X_XOSC1,             0x07},
};

//____ fnCC112xSendReceiveHandler  _________________________________________________________________
//
// @brief	Main CC112X middle ware driver function which manages all the communication with chip over SPI interface
//			It fill ups the data structure necessary for communication and pass execution to lower level to complete the communication operations
// @param	chOperationType		Indicates type of operation going to be performed with CC112x (Command,Tx,Rx)
//			nRadioAddress		Address of CC112x register wants to access for performing block send-receive operations
//			chCommLength		No of bytes to send or receive over SPI
//			pchDataBytes		Memory resources to hold the receive or transmit data
// @return	FALSE if communication with chip fails (Such error will get generated from lower level in the case of chip is not ready)

int8_t fnCC112xSendReceiveHandler(uint8_t chOperationType,uint16_t nRadioAddress,uint8_t chCommLength,uint8_t *pchDataBytes)
{
	//Initialize values to RADIO_COMM_DATA_HANDLE structure
	ghRadioSpiCommData.nCommAddress		= nRadioAddress;
	ghRadioSpiCommData.chCommType		= chOperationType;
	ghRadioSpiCommData.pchCommBuff		= pchDataBytes;
	ghRadioSpiCommData.nCommCount		= chCommLength;
	ghRadioSpiCommData.chCommOperation	= chOperationType;
	
	//To perform this operation need not to specify any specific detail while calling this function
	if(chOperationType==RADIO_COMMAND_STROBE)																	//Radio Command
	{
		ghRadioSpiCommData.nCommCount= RESET_COUNTER;
		return fnSpiRadioSendReceiveData();
	}
	
	//RADIO_TRANSMIT_BYTES: To perform this operation must fill up the nCommCount and nCommAddress field of RADIO_COMM_DATA_HANDLE while calling this function
	//RADIO_RECEIVE_BYTES: To perform this operation must fill up the nCommCount field of RADIO_COMM_DATA_HANDLE while calling this function
	else if(chOperationType==RADIO_TRANSMIT_BYTES || chOperationType==RADIO_RECEIVE_BYTES)						//Radio Send/Receive Command
	{
		return fnSpiRadioSendReceiveData();
	}

	SEND_DEBUG_ERROR_CODES(SENSOR_INVALID_EXECUTION_DEVELOPER_ERROR);
	return RETURN_FALSE;	//Invalid operation mode (Programmer's Error)
}

//____ fnRegisterConfigurationCC112X  _________________________________________________________________
//
// @brief	Configure CC112x configuration registers with the value specified in hRegSet to establish a successful communication over RF at 154MHZ 
//			In the case of SENSOR_CC1125_REGISTER_INIT_FAIL, this function might stay in continuous loop
// @return	FALSE if communication with chip fails

inline int8_t  fnRegisterConfigurationCC112X(void)
{
	uint8_t chCounter			= RESET_COUNTER;
	uint8_t chDummyByte			= RESET_VALUE;
	uint8_t chRadioCheckFlag	= RESET_FLAG;

	//Put chip in reset mode before configuring the registers 
	fnCC112xSendReceiveHandler(RADIO_COMMAND_STROBE,CC112X_SRES,1,NULL);
	
	while(1)
	{
		chRadioCheckFlag= RESET_FLAG;
		chDummyByte=hRegSet[chCounter].chRegValue;
		
		if(fnCC112xSendReceiveHandler(RADIO_TRANSMIT_BYTES,hRegSet[chCounter].nRegAddr,1,&chDummyByte) != RETURN_TRUE)		//Write register value
		{
			return RETURN_FALSE;
		}
		
		#if VERIFY_CC1125_REGISTERS
		
			chDummyByte=RESET_VALUE;
			if(fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,hRegSet[chCounter].nRegAddr,1,&chDummyByte) != RETURN_TRUE)		//Read register value
			{
				return RETURN_FALSE;
			}
		
			if(chDummyByte == hRegSet[chCounter].chRegValue)				//Cross checking to avoid conflict and proper setup of radio chip for further communication
			{
				chRadioCheckFlag=SET_FLAG;
			}
			else
			{
				SEND_DEBUG_ERROR_CODES(SENSOR_CC1125_REGISTER_INIT_FAIL);		//Register checking fail
			}
		
		#else

			chRadioCheckFlag=SET_FLAG;

		#endif
		
		if(chRadioCheckFlag==SET_FLAG)
		{
			if(++chCounter >=  (sizeof(hRegSet)/sizeof(CC112X_REGSET)))
			{
				break;
			}
		}
	}
	
	return RETURN_TRUE;
}

//____ fnManualConfigurationCC112X  _________________________________________________________________
//
// @brief	Perform Calibration on CC112x chip for better communications 
// @return	FALSE if communication with chip fails

inline int8_t  fnManualConfigurationCC112X(void)
{
	uint8_t chOriginalCal = RESET_VALUE;
	uint8_t chDummyByte = RESET_VALUE;
	
	//To hold multiple FS_VCO2, FS_VCO4 and FS_CHP register values during calibration
	uint8_t chCalResultsStartHigh[3];	
	uint8_t chCalResultsStartMid[3];	

	// Set VCO cap-array to 0 (FS_VCO2 = 0x00)
	chDummyByte=RESET_VALUE;
	if(fnCC112xSendReceiveHandler(RADIO_TRANSMIT_BYTES,CC112X_FS_VCO2,1,&chDummyByte) == RETURN_FALSE)
	{
		return RETURN_FALSE;
	} 
	
	// Start with high VCDAC (original VCDAC_START + 2):
	if(fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,CC112X_FS_CAL2,1,&chOriginalCal) == RETURN_FALSE)
	{
		return RETURN_FALSE;
	}
	
	chDummyByte= chOriginalCal + VCDAC_START_OFFSET;
	if(fnCC112xSendReceiveHandler(RADIO_TRANSMIT_BYTES,CC112X_FS_CAL2,1,&chDummyByte) == RETURN_FALSE)
	{
		return RETURN_FALSE;
	}

	// Calibrate and wait for calibration to be done
	if(fnCC112xSendReceiveHandler(RADIO_COMMAND_STROBE,CC112X_SCAL,1,NULL) == RETURN_FALSE)
	{
		return RETURN_FALSE;
	}
	
	do
	{
		if(fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,CC112X_MARCSTATE,1,&chDummyByte) == RETURN_FALSE)
		{
			return RETURN_FALSE;
		}
	} while (chDummyByte != 0x41);

	// Read FS_VCO2, FS_VCO4 and FS_CHP register obtained with high VCDAC_START value
	if(fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,CC112X_FS_VCO2,1,&chCalResultsStartHigh[FS_VCO2_INDEX]) == RETURN_FALSE)
	{
		return RETURN_FALSE;
	}
	
	if(fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,CC112X_FS_VCO4,1,&chCalResultsStartHigh[FS_VCO4_INDEX]) == RETURN_FALSE)
	{
		return RETURN_FALSE;
	}
	
	if(fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,CC112X_FS_CHP,1,&chCalResultsStartHigh[FS_CHP_INDEX]) == RETURN_FALSE)
	{
		return RETURN_FALSE;
	}
	
	// Set VCO cap-array to 0 (FS_VCO2 = 0x00)
	chDummyByte=RESET_VALUE;
	if(fnCC112xSendReceiveHandler(RADIO_TRANSMIT_BYTES,CC112X_FS_VCO2,1,&chDummyByte) == RETURN_FALSE)
	{
		return RETURN_FALSE;
	}

	// Continue with mid VCDAC (original VCDAC_START):
	if(fnCC112xSendReceiveHandler(RADIO_TRANSMIT_BYTES,CC112X_FS_CAL2,1,&chOriginalCal) == RETURN_FALSE)
	{
		return RETURN_FALSE;
	}

	// Calibrate and wait for calibration to be done
	// (Put Radio back in IDLE state)
	if(fnCC112xSendReceiveHandler(RADIO_COMMAND_STROBE,CC112X_SCAL,1,NULL) == RETURN_FALSE)
	{
		return RETURN_FALSE;
	}
	
	do
	{
		if(fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,CC112X_MARCSTATE,1,&chDummyByte)==RETURN_FALSE)
		{
			return RETURN_FALSE;
		}
	} while (chDummyByte != 0x41);

	// Read FS_VCO2, FS_VCO4 and FS_CHP register obtained with mid VCDAC_START value
	if(fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,CC112X_FS_VCO2,1,&chCalResultsStartMid[FS_VCO2_INDEX]) == RETURN_FALSE)
	{
		return RETURN_FALSE;
	}
	
	if(fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,CC112X_FS_VCO4,1,&chCalResultsStartMid[FS_VCO4_INDEX]) == RETURN_FALSE)
	{
		return RETURN_FALSE;
	}
	
	if(fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,CC112X_FS_CHP,1,&chCalResultsStartMid[FS_CHP_INDEX]) == RETURN_FALSE)
	{
		return RETURN_FALSE;
	}
	
	// Write back highest FS_VCO2 and corresponding FS_VCO, FS_CHP result
	if (chCalResultsStartHigh[FS_VCO2_INDEX] > chCalResultsStartMid[FS_VCO2_INDEX])
	{
		if(fnCC112xSendReceiveHandler(RADIO_TRANSMIT_BYTES,CC112X_FS_VCO2,1,&chCalResultsStartHigh[FS_VCO2_INDEX]) == RETURN_FALSE)
		{
			return RETURN_FALSE;
		}
		
		if(fnCC112xSendReceiveHandler(RADIO_TRANSMIT_BYTES,CC112X_FS_VCO4,1,&chCalResultsStartHigh[FS_VCO4_INDEX]) == RETURN_FALSE)
		{
			return RETURN_FALSE;
		}
		
		if(fnCC112xSendReceiveHandler(RADIO_TRANSMIT_BYTES,CC112X_FS_CHP,1,&chCalResultsStartHigh[FS_CHP_INDEX]) == RETURN_FALSE)
		{
			return RETURN_FALSE;
		}
	}
	else
	{
		if(fnCC112xSendReceiveHandler(RADIO_TRANSMIT_BYTES,CC112X_FS_VCO2,1,&chCalResultsStartMid[FS_VCO2_INDEX]) == RETURN_FALSE)
		{
			return RETURN_FALSE;
		}
		
		if(fnCC112xSendReceiveHandler(RADIO_TRANSMIT_BYTES,CC112X_FS_VCO4,1,&chCalResultsStartMid[FS_VCO4_INDEX]) == RETURN_FALSE)
		{
			return RETURN_FALSE;
		}
		
		if(fnCC112xSendReceiveHandler(RADIO_TRANSMIT_BYTES,CC112X_FS_CHP,1,&chCalResultsStartMid[FS_CHP_INDEX]) == RETURN_FALSE)
		{
			return RETURN_FALSE;
		}
	}
	
	return RETURN_TRUE;
}

//____ fnCC112xReceiveDataAvail  _________________________________________________________________
//
// @brief	This function will get called from upper level when it is expecting the data from the Radio chip
//			As CC112x chip is configured to get the interruption on receive avail, this function will check for gchDataCommFlagCC1125
// @return	FALSE if communication with chip fails or new reception is not available

int8_t fnCC112xReceiveDataAvail(void)
{
	uint8_t chDummyByte = RESET_VALUE;
	
	if(gchDataCommFlagCC1125== SET_FLAG)		//Check for new reception
	{
		gchDataCommFlagCC1125 = RESET_FLAG;
		
		//Check for no. of bytes received in chip
		if(fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,CC112X_NUM_RXBYTES,1,&chDummyByte) == RETURN_FALSE)
		{
			return RETURN_FALSE;
		}
		
		if(chDummyByte > RESET_VALUE)
		{
			if(fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,CC112X_MARCSTATE,1,&chDummyByte) == RETURN_FALSE)
			{
				return RETURN_FALSE;
			}
			
			//Check for chip status to avoid unwanted exception
			if((chDummyByte & CC1125_FIFO_ERR_CHECK_BM) == CC112X_STATE_RXFIFO_ERROR)		
			{
				SEND_DEBUG_ERROR_CODES(SENSOR_CC1125_RX_FIIO_ERR);
				
				//Flush the RX_FIFO on Error
				fnCC112xSendReceiveHandler(RADIO_COMMAND_STROBE,CC112X_SFRX,1,NULL);
				return RETURN_FALSE;
			}
			return RETURN_TRUE;
		}
	}
	return RETURN_FALSE;
}

//____ fnCC112xSendDataComplete  _________________________________________________________________
//
// @brief	Normally this function will get called from upper level when data were filled in TX FIFO of Radio chip
//			As CC112x chip is configured to get the interruption on transmission complete, this function will check for gchDataCommFlagCC1125
// @return	FALSE if transmission complete interrupt is not triggered by chip

int8_t fnCC112xSendDataComplete(void)
{
	if(gchDataCommFlagCC1125 == SET_FLAG)		//Check for transmission complete
	{
		gchDataCommFlagCC1125 = RESET_FLAG;
		fnCC112xSendReceiveHandler(RADIO_COMMAND_STROBE,CC112X_SRX,1,NULL);		//Put chip back in its default state (Receive)
		return RETURN_TRUE;
	}
	
	return RETURN_FALSE;
}

//____ fnRead8BitRssi  _________________________________________________________________
//
// @brief	This function can be called to fetch the fresh and valid RSSI value from CC112x chip
// @return	0 if RSSI value is not valid otherwise returns RSSI value in return

uint8_t fnRead8BitRssi(void)
{
	uint8_t chRSSI = RESET_VALUE;
	uint8_t chDummy = RESET_VALUE;
	
	// Read RSSI_VALID from RSSI0
	if(fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,CC112X_RSSI0,1,&chDummy) == RETURN_FALSE)
	{
		return 0;
	}
	
	// Check if the RSSI_VALID flag is set
	if(chDummy & CC1125_RSSI_VAL_CHECK_BM)
	{
		// Read RSSI from MSB register
		if(fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,CC112X_RSSI1,1,&chDummy) == RETURN_FALSE)
		{
			return 0;
		}
		
		chRSSI = (int8_t)chDummy - CC112X_RSSI_OFFSET;
		return chRSSI;
	}
	
	// return 0 since new RSSI value is not valid
	return 0;
}

//____fnRadioCC1125Initialization _________________________________________________________________
//
// @brief	Function manages to configure and calibrate the CC112x chip by calling fnManualConfigurationCC112X and fnRegisterConfigurationCC112X
// @return	FALSE if calibration or configuration fails otherwise returns TRUE

SENSOR_MC_ERROR_CODES fnRadioCC1125Initialization(void)
{
	//Configure Radio chip with all the required register settings
	if(fnRegisterConfigurationCC112X() == RETURN_FALSE)
	{
		return SENSOR_CC1125_CHIP_NOT_READY;
	}

	//Performs manual calibration operation with the Radio chip
	if(fnManualConfigurationCC112X() == RETURN_FALSE)
	{
		return SENSOR_CC1125_CALIBRATION_FAIL;
	}

	return SENSOR_NO_ERROR;
}

