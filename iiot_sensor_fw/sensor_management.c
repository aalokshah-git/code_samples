/* -------------------------------------------------------------------------
Filename: sensor_management.c

Job#: 20473
Date Created: 11/04/2014

Purpose: All the sensors access functionality is given in this file

Functions:
fnFetchSensorDataLength						Fetch data length for sensor passed in argument
fnCheckI2cAvailability						Check for availability of I2C resources
fnFetchChamberPressure						Fetch chamber pressure sensor measurements
fnWriteGyrometerRegister					Perform write operation with Gyroscope
fnReadGyrometerRegister						Perform read operation with Gyroscope
fnInitializeGyrometer						Initialize Gyroscope for operations
fnFetchGyrometerMeasurements				Fetch Gyroscope measurements
fnStartChamberTemperature					Start sampling ADC input of chamber temperature sensor
fnFetchChamberTemperature					Fetch chamber temperature sensor measurements
fnE2PROMWriteOperation						Perform sequential write with E2PROM
fnE2PROMReadOpeartion						Perform sequential read with E2PROM
fnSetResetSmartSensorSelectlines			Set environments for specific smart sensor operation
fnStartSmartSensorSampling					Notify smart sensor to start sampling 
fnSmartSensorsDataCollection				Fetch smart sensor measurements
fnSidewall_Serial_Task						---- Not Implemented ---
fnWatch_Dog_Manage_Task						---- Not Implemented ---

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

#include "sensor_management.h"		//Contains scenarios to manage the communication with various sensors available on SENSOR MC Hardware

//_____ G L O B A L   D E F I N I T I O N S _________________________________________________

//Will holds the fresh and valid value of RSSI fetched from CC1125
volatile uint8_t gchUplinkRSSI;

//Shared step indexes used by sensors to perform I2C communication (Algorithms designed to support data fetching in step modes)
volatile uint8_t gchLvl1StepIndexI2C;
volatile uint8_t gchLvl2StepIndexI2C;
volatile uint8_t gchStepIndexE2PROM;

//List of sensors supported in SENSOR MC design 
SENSOR_MC_SENSOR_LIST ghSensorList;

//_____  G L O B A L   C O N S T A N T S ______________________________________________________________

//This structure holds sensor id along with the data length of individual sensor's data
//All sensors must needs to be included in this structure to provide application an abstract idea about the no of data it has to manage for it
//??????  
static const SENSOR_DETAILS ghSensorInfo[]=
{
	{CHAMBER_TEMPERATURE		,	1},
	{CHAMBER_PRESSURE			,	1},
	{UPLINK_RADIO_RSSI			,	1},
	{GYRO_METER					,	3}
};

//_____ fnFetchSensorDataLength ____________________________________________________________________
//
// @brief	It will perform linear search in all the entries of SENSOR_DETAILS to find out the data length of the sensor passed in argument
// @param	chSensorID		Sensor ID for which searching for the data length
// @return	FALSE if no entry found for given Sensor ID otherwise returns data length specific to provided Sensor ID

int8_t fnFetchSensorDataLength(uint8_t chSensorID)
{
	uint8_t chCounter = RESET_COUNTER;
	
	for(chCounter = RESET_COUNTER;chCounter<(sizeof(ghSensorInfo)/sizeof(SENSOR_DETAILS));chCounter++)
	{
		if(chSensorID==ghSensorInfo[chCounter].chSensorID)
		{
			return ghSensorInfo[chCounter].chSensorDataLen;
		}
	}
	
	return RETURN_FALSE;			//If no entry found for provided ID
}

//_____ fnCheckI2cAvailability ____________________________________________________________________
//
// @brief	It will just call	lower level function fnCheckI2CStatus() to find out I2C availability for operation
// @return	TRUE if I2C is free otherwise returns FALSE

int8_t fnCheckI2cAvailability(void)
{
	return fnCheckI2CStatus();
}

//_____ fnFetchChamberPressure ____________________________________________________________________
//
// @brief	This function will fetch the pressure value by communicating to sensor over I2C interface
// @return	FALSE if pressure data are not fetched successfully otherwise returns Pressure Data fetched from sensor

int16_t fnFetchChamberPressure(void)
{
	int8_t chCheckStatus = RESET_VALUE;
	
	if(gchLvl1StepIndexI2C == STEP_0_Val)
	{
		if(RETURN_TRUE==fnI2cSendReceiveOperation(PRESSURE_SENSOR_I2C_ADDR,0,4))
		{
			gchLvl1StepIndexI2C=STEP_1_Val;
		}
	}
	else
	{
		chCheckStatus=fnI2cFreeForOperation();
		if(chCheckStatus != I2C_RUNNING)
		{
			gchLvl1StepIndexI2C=STEP_0_Val;						//Release the flag for next I2C communication 
			
			if(chCheckStatus == I2C_OP_COMPLETE)
			{
				if((ghI2cCommData.chCommBuff[0] & 0xC0) != 0)	//First two bits in first byte are status bits of chamber pressure sensor
				{
					return RETURN_FALSE;						
				}
				
				return (((ghI2cCommData.chCommBuff[0] & 0x3f)<<8) | ghI2cCommData.chCommBuff[1]);		//Pressure value
			}
			
			return RETURN_FALSE;
		}
	}
	
	return RETURN_FALSE;
}

//_____ fnWriteGyrometerRegister ____________________________________________________________________
//
// @brief	Function can be called to perform the write operation with Gyrometer
// @param	chRegAddr	Register address of Gyrometer wants to modify
//			chRegValue  Value by which to modify the provided register address in Gyroscope
// @return	FASLE if write operation is not completed successfully otherwise returns TRUE

inline int8_t fnWriteGyrometerRegister(uint8_t chRegAddr,uint8_t chRegValue)
{
	if(gchLvl2StepIndexI2C == STEP_0_Val)
	{
		ghI2cCommData.chCommBuff[0]=chRegAddr;
		ghI2cCommData.chCommBuff[1]=chRegValue;
		
		if(fnI2cSendReceiveOperation(GYRO_METER_I2C_ADDR,2,0) == RETURN_TRUE)
		{
			gchLvl2StepIndexI2C=STEP_1_Val;
		}
	}
	else 
	{
		if(fnI2cFreeForOperation()==I2C_OP_COMPLETE)
		{
			gchLvl2StepIndexI2C=STEP_0_Val;									//Release the flag for next I2C communication 		
			return RETURN_TRUE;
		}
	}
	
	return RETURN_FALSE;
}

//_____ fnReadGyrometerRegister ____________________________________________________________________
//
// @brief	Function can be called to perform the read operation with Gyrometer
// @param	chRegAddr	Register address of Gyrometer wants to read
//			chRegValue  Memory to hold the value read from specified register address of Gyrometer
// @return	FASLE if read operation is not completed successfully otherwise returns TRUE

inline int8_t fnReadGyrometerRegister(uint8_t chRegAddr, uint8_t *chRegValue)
{
	if(gchLvl2StepIndexI2C==STEP_0_Val)
	{
		ghI2cCommData.chCommBuff[0]=chRegAddr;
		
		if(fnI2cSendReceiveOperation(GYRO_METER_I2C_ADDR,1,1) == RETURN_TRUE)
		{
			gchLvl2StepIndexI2C=STEP_1_Val;
		}
	}
	else 
	{
		if(fnI2cFreeForOperation()==I2C_OP_COMPLETE)
		{
			*chRegValue=ghI2cCommData.chCommBuff[0];
			gchLvl2StepIndexI2C=STEP_0_Val;												//Release the flag for next I2C communication 	
			return RETURN_TRUE;
		}
	}
	
	return RETURN_FALSE;
}

//_____ fnInitializeGyrometer ____________________________________________________________________
//
// @brief	It will initialize the Gyrometer for operations
//			At power on Gyroscope is not operational so this function must be execution to instruct the Gyroscope for start sampling
// @return	FASLE if initialization procedure is not completed successfully otherwise returns TRUE

int8_t fnInitializeGyrometer(void)
{
	if(gchLvl1StepIndexI2C==STEP_0_Val)
	{
		if(fnWriteGyrometerRegister(GYRO_MAX21000_BANK_SEL,0x00) == RETURN_TRUE)			//Select the normal register address bank	
		{
			gchLvl1StepIndexI2C=STEP_1_Val;
		}
	}
	else if(gchLvl1StepIndexI2C==STEP_1_Val)
	{
		if(fnWriteGyrometerRegister(GYRO_MAX21000_SENSE_CNFG1,0x10) == RETURN_TRUE)			//BandWidth-10HZ
		{
			gchLvl1StepIndexI2C=STEP_2_Val;
		}
	}
	else if(gchLvl1StepIndexI2C==STEP_2_Val)
	{
		if(fnWriteGyrometerRegister(GYRO_MAX21000_SENSE_CNFG2,0x01) == RETURN_TRUE)			//Output Data Rate-0.2ms
		{
			gchLvl1StepIndexI2C=STEP_3_Val;
		}
	}
	else if(gchLvl1StepIndexI2C==STEP_3_Val)
	{
		if(fnWriteGyrometerRegister(GYRO_MAX21000_SENSE_CNFG0,0x0F) == RETURN_TRUE)			//Normal Mode of Operation
		{
			gchLvl1StepIndexI2C=STEP_0_Val;													//Release the flag for next I2C communication 
			return RETURN_TRUE;
		}
	}
	
	return RETURN_FALSE;
}

//_____ fnFetchGyrometerMeasurements ____________________________________________________________________
//
// @brief	It will fetch current measurements of Gyroscope
//			fnInitializeGyrometer() function must be executed once before calling this function so that Gyroscope can start sampling the real time data
//			This function will check for the Gyroscope status to avoid mismatch in data fetching
// @return	FASLE if Gyroscope data fetching is not completed successfully otherwise returns TRUE

int8_t fnFetchGyrometerMeasurements(uint16_t *pnBuff)
{
	uint8_t	chDataAvailStatus = RESET_VALUE;
	uint8_t chLoopVar = RESET_COUNTER;
	uint8_t chCounter = RESET_COUNTER;
	
	if(gchLvl1StepIndexI2C==STEP_0_Val)
	{
		if(RETURN_TRUE==fnReadGyrometerRegister(GYRO_MAX21000_SYS_STATUS,&chDataAvailStatus))
		{
			if(chDataAvailStatus & 0x01)		//If Gyroscope Data Ready
			{
				gchLvl1StepIndexI2C=STEP_1_Val;
			}
		}
		else
		{
			return RETURN_FALSE;
		}
	}
	
	if(gchLvl1StepIndexI2C==STEP_1_Val)
	{
		ghI2cCommData.chCommBuff[0]=GYRO_MAX21000_DATA_START_ADDR;
		
		if(!fnI2cSendReceiveOperation(GYRO_METER_I2C_ADDR,1,6))
		{
			gchLvl1StepIndexI2C=STEP_2_Val;
		}
	}
	else if(gchLvl1StepIndexI2C==STEP_2_Val)
	{
		if(fnI2cFreeForOperation()==I2C_OP_COMPLETE)
		{
			gchLvl1StepIndexI2C=STEP_0_Val;													//Release the flag for next I2C communication 
			chCounter=RESET_COUNTER;
			
			for(chLoopVar=RESET_COUNTER;chLoopVar<6;chLoopVar+=2)							//Read 6-Bytes from Gyrometer (16 bit values of X,Y and Z)
			{
				pnBuff[chCounter++]=(ghI2cCommData.chCommBuff[chLoopVar] << 8) | ghI2cCommData.chCommBuff[chLoopVar+1];
			}
			
			return RETURN_TRUE;
		}
	}

	return RETURN_FALSE;
}

//__________fnStartChamberTemperature______________________
//
// @brief	It will call the lower level ADC function to start sampling the ADC input of chamber temperature sensor
// @return	FALSE if no ADC resources are free otherwise returns TRUE

int8_t fnStartChamberTemperature(void)
{
	{
		SEND_DEBUG_ERROR_CODES(SENSOR_ADC_RESOURCES_ARE_NOT_FREE);
		return RETURN_FALSE;
	}
	
	return RETURN_TRUE;
}

//_____ fnFetchChamberTemperature ____________________________________________________________________
//
// @brief	This function will fetch the sampled chamber temperature data from ADC
//			fnStartChamberTemperature() must be execued first to start sampling for chamber temperature
// @return	FALSE if data sampling is running otherwise sampled data value
//			In case of mismatch it will return 0 as sampled data

int16_t fnFetchChamberTemperature(void)
{
	int16_t nResolutionValue = RESET_VALUE;
	
	nResolutionValue=fnADCFetchSampledData(CHAMBER_TEMPERATURE_ADC_INDEX);
	
	if(nResolutionValue == RETURN_ADC_DATA_COLLECTION_RUNNING)
	{
		return RETURN_FALSE;
	}
	else
	{
		if(nResolutionValue == RETURN_ADC_DATA_COLLECTION_MISMATCH)
		{
			SEND_DEBUG_ERROR_CODES(SENSOR_ADC_CONVERSION_MISMATCH);
			nResolutionValue=0;
		}
		
		if(nResolutionValue<0)					//ADC is in signed mode but we are avoiding -ve value
		{
			nResolutionValue=0;
		}	
	}
	
	return nResolutionValue;
}

//_____ fnE2PROMWriteOperation ____________________________________________________________________
//
// @brief	Use this function to write block of data sequentially in to E2PROM
// @param	pchBuff		Pointer to the memory resources to hold the data bytes needed to write in E2PROM
//			chLength	Specifies value of count for which to perform write operation
//			nAddress	Base address in E2PROM from where to perform sequential write of data
// @return	FALSE if operation is not completed otherwise sampled data value
//			In the case of overflow function will return SENSOR_I2C_COMM_BUFF_OVERFLOW

SENSOR_MC_ERROR_CODES fnE2PROMWriteOperation(uint8_t *pchBuff, uint8_t chLength, uint16_t nAddress)
{
	uint8_t chLoopVar = RESET_COUNTER;
	
	if(gchStepIndexE2PROM==STEP_0_Val)
	{
		if(fnCheckI2cAvailability()==RETURN_TRUE)
		{
			ACQUIRE_I2C_INTERFACE;
			
			if(chLength>I2C_COMM_BUFFER_SIZE-2)				//2-Bytes are Occupied for Address
			{
				return SENSOR_I2C_COMM_BUFF_OVERFLOW;
			}
			
			ghI2cCommData.chCommBuff[0]=nAddress >> 8;
			ghI2cCommData.chCommBuff[1]=nAddress & 0xff;
			
			for(chLoopVar=RESET_COUNTER;chLoopVar<chLength;chLoopVar++)
			{
				ghI2cCommData.chCommBuff[chLoopVar+2]=pchBuff[chLoopVar];
			}
			
			E2PROM_WR_CONTROL_ACTIVE;
			fnI2cSendReceiveOperation(E2PROM_MEMORY_I2C_ADDR,chLength+2,0);		//2- Bytes of address are also included in communication buffer
			gchStepIndexE2PROM=STEP_1_Val;
		}
		else if(gchStepIndexE2PROM==STEP_1_Val)
		{
			if(fnI2cFreeForOperation()!=I2C_RUNNING)
			{
				gchStepIndexE2PROM=STEP_0_Val;
				fnWait_uSecond(50);			//Wait required after successful write operation to E2PROM
				E2PROM_WR_CONTROL_DEACTIVE;
				RELEASE_I2C_INTERFACE;
				return RETURN_TRUE;
			}
		}
	}
	
	return RETURN_FALSE;
}

//_____ fnE2PROMReadOpeartion ____________________________________________________________________
//
// @brief	Use this function to read block of data sequentially from E2PROM
// @param	pchBuff		Pointer to the memory resources to hold the data bytes after reading from the E2PROM
//			chLength	Specifies value of count for which to perform read operation
//			nAddress	Base address in E2PROM from where to perform sequential read of data
// @return	FALSE if operation is not completed otherwise sampled data value
//			In the case of overflow function will return SENSOR_I2C_COMM_BUFF_OVERFLOW

SENSOR_MC_ERROR_CODES fnE2PROMReadOpeartion(uint8_t *pchBuff, uint8_t chLength, uint16_t nAddress)
{
	uint8_t chLoopVar = RESET_COUNTER;
	uint8_t chCheckStatus = RESET_VALUE;
	
	if(gchStepIndexE2PROM==STEP_0_Val)
	{
		if(fnCheckI2cAvailability()==RETURN_TRUE)
		{
			ACQUIRE_I2C_INTERFACE;
			
			if(chLength>I2C_COMM_BUFFER_SIZE-2)		//2-Bytes are Occupied for Address
			{
				return SENSOR_I2C_COMM_BUFF_OVERFLOW;
			}
			
			ghI2cCommData.chCommBuff[0]=nAddress >> 8;
			ghI2cCommData.chCommBuff[1]=nAddress & 0xff;

			fnI2cSendReceiveOperation(E2PROM_MEMORY_I2C_ADDR,2,chLength);
			gchStepIndexE2PROM=STEP_1_Val;
		}
	}
	else if(gchStepIndexE2PROM==STEP_1_Val)
	{
		chCheckStatus=fnI2cFreeForOperation();
		
		if(chCheckStatus!=I2C_RUNNING)
		{
			if(chCheckStatus==I2C_OP_COMPLETE)
			{
				for(chLoopVar=0;chLoopVar<chLength;chLoopVar++)
				{
					pchBuff[chLoopVar]=ghI2cCommData.chCommBuff[chLoopVar];
				}
			}
			
			gchStepIndexE2PROM=STEP_0_Val;
			RELEASE_I2C_INTERFACE;
			return RETURN_TRUE;
		}
	}

	return RETURN_FALSE;
}

//_____ fnSetResetSmartSensorSelectlines ____________________________________________________________________
//
// @brief	In SENSOR MC hardware around 16 sensors are sharing the 3-SPI interfaces
//			So this function will set the selection lines (port pins) for the sensor with which we wants to communicate
//			This function must be executed before performing communication with the smart sensor
// @param	chSensorGroup	It indicates the group/interface to which this Sensor is connected
//			chSensorValue	It indicates the Sensor to which communication will be established
// @return FALSE if Sensor Group or Sensor Value is invalid otherwise returns TRUE

inline int8_t fnSetResetSmartSensorSelectlines(uint8_t chSensorGroup,uint8_t chSensorValue)
{
	switch(chSensorGroup)
	{
		case SMART_SENSOR_SPID_GROUP:
		
			switch(chSensorValue)
			{
				case SMART_SENSOR_SPID0:
					SET_SPID0_LOW;
					SET_SPID1_LOW;
					SET_SPID2_LOW;
				break;
			
				case SMART_SENSOR_SPID1:
					SET_SPID0_HIGH;
					SET_SPID1_LOW;
					SET_SPID2_LOW;
				break;
			
				case SMART_SENSOR_SPID2:
					SET_SPID0_LOW;
					SET_SPID1_HIGH;
					SET_SPID2_LOW;
				break;
			
				case SMART_SENSOR_SPID3:
					SET_SPID0_HIGH;
					SET_SPID1_HIGH;
					SET_SPID2_LOW;
				break;
			
				case SMART_SENSOR_SPID4:
					SET_SPID0_LOW;
					SET_SPID1_LOW;
					SET_SPID2_HIGH;
				break;
			
				default:
					return RETURN_FALSE;
				break;
			}
		break;
		
		case SMART_SENSOR_SPIE_GROUP:
		
			switch(chSensorValue)
			{
				case SMART_SENSOR_SPIE0:
					SET_SPIE0_LOW;
					SET_SPIE1_LOW;
					SET_SPIE2_LOW;
				break;
			
				case SMART_SENSOR_SPIE1:
					SET_SPIE0_HIGH;
					SET_SPIE1_LOW;
					SET_SPIE2_LOW;
				break;
			
				case SMART_SENSOR_SPIE2:
					SET_SPIE0_LOW;
					SET_SPIE1_HIGH;
					SET_SPIE2_LOW;
				break;
			
				case SMART_SENSOR_SPIE3:
					SET_SPIE0_HIGH;
					SET_SPIE1_HIGH;
					SET_SPIE2_LOW;
				break;
			
				case SMART_SENSOR_SPIE4:
					SET_SPIE0_LOW;
					SET_SPIE1_LOW;
					SET_SPIE2_HIGH;
				break;
			
				default:
					return RETURN_FALSE;
				break;
			}
		break;
		
		case SMART_SENSOR_SPIF_GROUP:

			switch(chSensorValue)
			{
				case SMART_SENSOR_SPIF0:
					SET_SPIF0_LOW;
					SET_SPIF1_LOW;
					SET_SPIF2_LOW;
				break;
			
				case SMART_SENSOR_SPIF1:
					SET_SPIF0_HIGH;
					SET_SPIF1_LOW;
					SET_SPIF2_LOW;
				break;
			
				case SMART_SENSOR_SPIF2:
					SET_SPIF0_LOW;
					SET_SPIF1_HIGH;
					SET_SPIF2_LOW;
				break;
			
				case SMART_SENSOR_SPIF3:
					SET_SPIF0_HIGH;
					SET_SPIF1_HIGH;
					SET_SPIF2_LOW;
				break;
			
				case SMART_SENSOR_SPIF4:
					SET_SPIF0_LOW;
					SET_SPIF1_LOW;
					SET_SPIF2_HIGH;
				break;
			
				case SMART_SENSOR_SPIF5:
					SET_SPIF0_HIGH;
					SET_SPIF1_LOW;
					SET_SPIF2_HIGH;
				break;
			
				default:
					return RETURN_FALSE;
				break;
			}
		break;
		
		default:
			return RETURN_FALSE;
		break;
	}
	
	return RETURN_TRUE;
}

//_____ fnStartSmartSensorSampling ____________________________________________________________________
//
// @brief	This function will notify the Smart Sensor to start sampling with the provided sensor tasking table
//			fnSetResetSmartSensorSelectlines() must needs to be executed to make communication path (SPI interface) between MCU and Smart Sensor through various select lines 
//			?????
// @param	chSensorGroup	It indicates the group/interface to which this Sensor is connected
//			chSensorValue	It indicates the Sensor to which communication will be established
// @return FALSE if Sensor Group or Sensor Value is invalid otherwise returns TRUE

int8_t fnStartSmartSensorSampling(uint8_t chSensorGroup,uint8_t chSensorValue)
{
	//Definitions required to upload the sensor control byte in the sensor
	
	//????????????????????? This is just for the overview for future design - All the scenarios will get change on the basis of requirement in Smart Sensor Communication
	
	if(RETURN_TRUE==fnSetResetSmartSensorSelectlines(chSensorGroup,chSensorValue))
	{
		if(chSensorGroup==SMART_SENSOR_SPID_GROUP)
		{
			//Upload sampling related Sensor Tasking Table
		}
		else if(chSensorGroup==SMART_SENSOR_SPIE_GROUP)
		{
			//Upload sampling related Sensor Tasking Table
		}
		else if(chSensorGroup==SMART_SENSOR_SPIF_GROUP)
		{
			//Upload sampling related Sensor Tasking Table
		}
		
		return RETURN_TRUE;
	}
	
	return RETURN_FALSE;
}

//_____ fnSmartSensorsDataCollection ____________________________________________________________________
//
// @brief	This function will fetch the sampled data from the Smart Sensor
//			fnSetResetSmartSensorSelectlines() must needs to be executed to make communication path (SPI interface) between MCU and Smart Sensor through various select lines
//			?????
// @param	chSensorGroup	It indicates the group/interface to which this Sensor is connected
//			chSensorValue	It indicates the Sensor to which communication will be established
//			pchSampledData	Pointer to memory resources where wants to put the sample data after receiving
//			chSampledlength	No of sampled data to fetch from the Smart Sensor
// @return FALSE if Sensor Group or Sensor Value is invalid otherwise returns TRUE

int8_t fnSmartSensorsDataCollection(uint8_t chSensorGroup,uint8_t chSensorValue,uint8_t *pchSampledData,uint8_t chSanpledlength)
{
	uint8_t chSensorDataLength = RESET_VALUE;
	uint8_t chLoopCounter = RESET_COUNTER;
	uint8_t chSensorDataBuff[SPI_SMART_SENSOR_COMM_BUF_SIZE];
	
	//????????????????????? This is just for the overview for future design - All the scenarios will get change on the basis of requirement in Smart Sensor Communication
	
	if(RETURN_TRUE==fnSetResetSmartSensorSelectlines(chSensorGroup,chSensorValue))
	{
		chSensorDataLength=chSanpledlength;
		fnMemSetToValue(chSensorDataBuff,0,chSensorDataLength);
		
		if(chSensorGroup==SMART_SENSOR_SPID_GROUP)
		{
			fnSPID_SendReceiveOperation(chSensorDataBuff,chSensorDataLength);
		}
		else if(chSensorGroup==SMART_SENSOR_SPIE_GROUP)
		{
			fnSPIE_SendReceiveOperation(chSensorDataBuff,chSensorDataLength);
		}
		else if(chSensorGroup==SMART_SENSOR_SPIF_GROUP)
		{
			fnSPIF_SendReceiveOperation(chSensorDataBuff,chSensorDataLength);
		}
		
		for(chLoopCounter=0;chLoopCounter<chSanpledlength;chLoopCounter++)
		{
			pchSampledData[chLoopCounter]=chSensorDataBuff[chLoopCounter];
		}
		
		return RETURN_TRUE;
	}
	
	return RETURN_FALSE;
}

//_____ fnSidewall_Serial_Task ____________________________________________________________________
//
//	?????Implementation pending

int8_t fnSidewall_Serial_Task(void)
{
	SEND_DEBUG_STRING("Serial Task");
	return RETURN_TRUE;
}

//_____ fnWatch_Dog_Manage_Task ____________________________________________________________________
//
//	???? Implementation Pending

int8_t fnWatch_Dog_Manage_Task(void)
{
	SEND_DEBUG_STRING("Watchdog Task");
	return RETURN_TRUE;
}