/* -------------------------------------------------------------------------
Filename: sample_collection.c

Job#: 20473
Date Created: 11/27/2014

Purpose: All the mechanisms for data sampling and data collection task

Functions:
fnData_Sampling_Task					This task performs operations to fetch the sampled data from sensors by following current tasking table 
fnData_Collection_Task					This task performs operations to prepare the data packet ready for transmission
fnResetFirmwareResourceAllocations		Reset all the resources shared in middle and lower level design


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

//_____ I N C L U D E S ______________________________________________________________

#include "system_globals.h"			//Contains definitions related to various system level task
#include "sensor_management.h"		//Contains scenarios to manage the communication with various sensors available on SENSOR MC Hardware

//_____ G L O B A L   D E F I N I T I O N S ______________________________________________________________

//Memory resources allocated to support all the sensor related operations
volatile SMART_SENSORS_STRUCT		ghSensorControl[MAX_SENSOR_COUNT];	
	
//Flag to find out current Sample Clock Occurrence in Data Sampling Task
// Will get set at every Master Sample Clock
volatile uint8_t gchSampleClockIndicator;	

//Flag to find out current Radio Clock Occurrence in Data Collection Task
// Will get set at every Master Radio Clock
volatile uint8_t gchRadioClockIndicator;	

//Indexes to maintain the communication packet generation in Data Collection Task
static uint8_t gchCollectionPacketIndex;
static uint8_t gchCollectionDataIndex;

//_____ fnData_Sampling_Task ____________________________________________________________________
//
// @brief	This function performs all the data sampling related operations on the basis of available tasking table
//			The entire mechanism can be explained in listed steps:
//			STEP-1:
//				gchSampleClockIndicator flag is set by the fnSystem_Timing_Task at the clock phase 0.
//				Flag is used to prevent re-comparison of sample clock divisor with the sensor sample clock counter. 
//				It means for every clock cycle sample clock divisor of each sensor is compared with current sample clock counter just once.
//				If match is found and sensor is in off mode than it will be put in a operation mode.
//				If match is found and sensor is already in operation mode than sensor sampling overrun error will get raised.
//
//			STEP-2:
//				In FOR loop each sensor's current status will get checked and depend on it all the sensors will get access of middle and lower level resources
//				The same will be performed till the sensor data will be fetched successfully than sensor will again be put in a off mode.
//			
//			STEP-3:
//				At the ending of task status of all the sensors will get checked.
//				If all sensors in off mode than this task will finally terminated till next availability of sample clock
//
// @return	TRUE if data sampling task is completed or all sensors are in off mode otherwise it returns FALSE

int8_t fnData_Sampling_Task(void)
{
	uint8_t	chSensorCounter	= RESET_COUNTER;
	uint8_t chCheckFlag		= RESET_FLAG;
	int16_t nDummyValue	= RESET_VALUE;
	
	if(gchSampleClockIndicator == SET_FLAG)
	{
		gchSampleClockIndicator= RESET_FLAG;
		chCheckFlag = RESET_FLAG;
		
		for(chSensorCounter = RESET_COUNTER;chSensorCounter<gchTotalSensorEntry;chSensorCounter++)
		{
			//This will check if the sensor current sample counter is less then sensor's sample clock divisor then don't process further
			if(++ghSensorControl[chSensorCounter].chSampleClockCounter >= ghSensorControl[chSensorCounter].chSampleClockDivisor)
			{
				ghSensorControl[chSensorCounter].chSampleClockCounter = RESET_COUNTER;
				chCheckFlag = SET_FLAG;
				
				//If sampling in already running than ignore its next sampling (Mismatch in TaskingTable design)
				if(ghSensorControl[chSensorCounter].chSensorStatusFlag==SAMPLE_STATE_OFF)
				{
					if(ghSensorControl[chSensorCounter].chSensorSampleAvgCounter < ghSensorControl[chSensorCounter].chSensorSampleAvgTotalCount)
					{
						ghSensorControl[chSensorCounter].chSensorStatusFlag=SAMPLE_STATE_START;
					}
					else
					{
						//Require Samples already taken
					}
				}
				else
				{
					SEND_ERROR_CODE_OVER_RF(SENSOR_SENSOR_SAMPLING_OVERRUN);
					SEND_DEBUG_ERROR_CODES(SENSOR_SENSOR_SAMPLING_OVERRUN);
				}
			}
		}
		
		if(chCheckFlag == RESET_FLAG)
		{
			return RETURN_TRUE;
		}
	}
	
	for(chSensorCounter = RESET_COUNTER;chSensorCounter<gchTotalSensorEntry;chSensorCounter++)
	{
		if(ghSensorControl[chSensorCounter].chSensorStatusFlag  != SAMPLE_STATE_OFF)
		{
			switch (ghSensorControl[chSensorCounter].chSensorID)
			{
				case CHAMBER_TEMPERATURE:
				
					if(ghSensorControl[chSensorCounter].chSensorStatusFlag==SAMPLE_STATE_START)
					{
						if(RETURN_TRUE==fnPowerSourceManager(POWER_SOURCE_ENABLE,TEMPERATURE_POWER_IDENTITY))
						{
							ghSensorControl[chSensorCounter].chSensorStatusFlag=SAMPLE_STATE_RUN_LVL0;
						}
					}
					else if(ghSensorControl[chSensorCounter].chSensorStatusFlag==SAMPLE_STATE_RUN_LVL0)
					{
						if(RETURN_TRUE==fnStartChamberTemperature())
						{
							ghSensorControl[chSensorCounter].chSensorStatusFlag=SAMPLE_STATE_RUN;
						}
						else
						{
							SEND_ERROR_CODE_OVER_RF(SENSOR_ADC_RESOURCES_ARE_NOT_FREE);
							SEND_DEBUG_ERROR_CODES(SENSOR_ADC_RESOURCES_ARE_NOT_FREE);
						}
					}
					else if(ghSensorControl[chSensorCounter].chSensorStatusFlag==SAMPLE_STATE_RUN)
					{
						nDummyValue=fnFetchChamberTemperature();
					
						if(RETURN_FALSE!=nDummyValue)
						{
							ghSensorControl[chSensorCounter].nSensorAvgData += nDummyValue;
						
							if(++ghSensorControl[chSensorCounter].chSensorSampleAvgCounter>=ghSensorControl[chSensorCounter].chSensorSampleAvgTotalCount)
							{
								ghSensorControl[chSensorCounter].nSensorData[0]= (ghSensorControl[chSensorCounter].nSensorAvgData/ghSensorControl[chSensorCounter].chSensorSampleAvgTotalCount);
							}
						
							ghSensorControl[chSensorCounter].chSensorStatusFlag = SAMPLE_STATE_OFF;		//This indicates sampling is done
							SEND_DEBUG_STRING("Chamber Temperature Data Available\n");
							fnPowerSourceManager(POWER_SOURCE_DISABLE,TEMPERATURE_POWER_IDENTITY);
						}
					}
				break;
				
				case GYRO_METER:
				
					if(ghSensorControl[chSensorCounter].chSensorStatusFlag==SAMPLE_STATE_START)
					{
						if(fnCheckI2cAvailability()==RETURN_TRUE)
						{
							ACQUIRE_I2C_INTERFACE;
							ghSensorControl[chSensorCounter].chSensorStatusFlag=SAMPLE_STATE_RUN_LVL0;
						}
					}
					else if(ghSensorControl[chSensorCounter].chSensorStatusFlag==SAMPLE_STATE_RUN_LVL0)
					{
						if(fnInitializeGyrometer()==RETURN_TRUE)
						{
							SEND_DEBUG_STRING("Gyrometer Initialized\n");
							ghSensorControl[chSensorCounter].chSensorStatusFlag=SAMPLE_STATE_RUN;
						}
					}
					else if(ghSensorControl[chSensorCounter].chSensorStatusFlag==SAMPLE_STATE_RUN)
					{
						if(RETURN_TRUE==fnFetchGyrometerMeasurements((uint16_t*)ghSensorControl[chSensorCounter].nSensorData))
						{
							ghSensorControl[chSensorCounter].chSensorSampleAvgCounter=ghSensorControl[chSensorCounter].chSensorSampleAvgTotalCount;
							SEND_DEBUG_STRING("Gyrometer Data Available\n");
							ghSensorControl[chSensorCounter].chSensorStatusFlag = SAMPLE_STATE_OFF;			//This will indicate sampling is done
							RELEASE_I2C_INTERFACE;
						}
					}
				break;
				
				case CHAMBER_PRESSURE:
				
					if(ghSensorControl[chSensorCounter].chSensorStatusFlag==SAMPLE_STATE_START)
					{
						if(RETURN_TRUE==fnPowerSourceManager(POWER_SOURCE_ENABLE,PRESSURE_POWER_IDENTITY))
						{
							ghSensorControl[chSensorCounter].chSensorStatusFlag=SAMPLE_STATE_RUN_LVL0;
						}
					}
					else if(ghSensorControl[chSensorCounter].chSensorStatusFlag==SAMPLE_STATE_RUN_LVL0)
					{
						if(fnCheckI2cAvailability()==RETURN_TRUE)
						{
							ACQUIRE_I2C_INTERFACE;
							ghSensorControl[chSensorCounter].chSensorStatusFlag=SAMPLE_STATE_RUN;
						}
					}
					else if(ghSensorControl[chSensorCounter].chSensorStatusFlag==SAMPLE_STATE_RUN)
					{
						nDummyValue=fnFetchChamberPressure();
					
						if(RETURN_FALSE!=nDummyValue)
						{
							ghSensorControl[chSensorCounter].nSensorAvgData += nDummyValue;
						
							if(++ghSensorControl[chSensorCounter].chSensorSampleAvgCounter>=ghSensorControl[chSensorCounter].chSensorSampleAvgTotalCount)
							{
								ghSensorControl[chSensorCounter].nSensorData[0]= (ghSensorControl[chSensorCounter].nSensorAvgData/ghSensorControl[chSensorCounter].chSensorSampleAvgTotalCount);
							}
						
							SEND_DEBUG_STRING("Pressure Sensor Data Available\n");
							ghSensorControl[chSensorCounter].chSensorStatusFlag = SAMPLE_STATE_OFF;		//This will indicate sampling is done
							fnPowerSourceManager(POWER_SOURCE_DISABLE,PRESSURE_POWER_IDENTITY);
							RELEASE_I2C_INTERFACE;
						}
					}
				break;
				
				case UPLINK_RADIO_RSSI:
					
					if(ghSensorControl[chSensorCounter].chSensorStatusFlag==SAMPLE_STATE_START)
					{
						ghSensorControl[chSensorCounter].nSensorAvgData += gchUplinkRSSI;

						if(++ghSensorControl[chSensorCounter].chSensorSampleAvgCounter>=ghSensorControl[chSensorCounter].chSensorSampleAvgTotalCount)
						{
							ghSensorControl[chSensorCounter].nSensorData[0]= (ghSensorControl[chSensorCounter].nSensorAvgData/ghSensorControl[chSensorCounter].chSensorSampleAvgTotalCount);
						}

						SEND_DEBUG_STRING("RSSI Measurement Data Available\n");
						ghSensorControl[chSensorCounter].chSensorStatusFlag = SAMPLE_STATE_OFF;		//This will indicate sampling is done
					}
				break;
				
				case BATTERY_TEMPERATURE:
				break;
				
				case TEG_TEMPERATURE_COLD:
				break;
				
				case TEG_TEMPERATURE_HOT:
				break;
				
				default:
					SEND_ERROR_CODE_OVER_RF(SENSOR_INVALID_EXECUTION_DEVELOPER_ERROR);
					SEND_DEBUG_ERROR_CODES(SENSOR_INVALID_EXECUTION_DEVELOPER_ERROR);
				break;
			}
		}
	}
	
	//This loop will check the sampling status of every sensors, If sampling is not done then it will return false
	for(chSensorCounter = RESET_COUNTER;chSensorCounter < gchTotalSensorEntry;chSensorCounter++)
	{
		if(ghSensorControl[chSensorCounter].chSensorStatusFlag != SAMPLE_STATE_OFF)
		{
			return RETURN_FALSE;
		}
	}
	
	return RETURN_TRUE;
}

//_____ fnData_Collection_Task ____________________________________________________________________
//
// @brief	This function performs all the data collection related operations on the basis of available tasking table
//			The entire mechanism can be explained in listed steps:
//			STEP1:
//				If last Data Download packet is not sent and request for new Data collection packet occurs than radio divisor overrun error will raised.
//
//			STEP-2: 
//				Every sensor's radio clock counter is compared with its radio clock divisor.
//				If match is not found than particular sensor is skipped from collection packet in existing clock cycle.
//				But If match found than data for that sensor will get appended in the collection packet.
//
//			STEP3:
//				Before filling the data into the SENSOR communication buffer function will check whether the sufficient space is available or not in current packet.
//				If sufficient space is available in the packet then it will simply store the data into the same packet.
//				if sufficient space is not available than new packet will get created for the remaining data.
//
//			STEP-4: 
//				After successful collection of the data packet Data Collection will get terminated till next radio clock availability. 
//
// @return	TRUE if data collection task is completed or packets are ready otherwise it returns FALSE

int8_t fnData_Collection_Task(void)
{
	uint8_t	chSensorCounter = RESET_COUNTER;
	uint8_t chLoopIndex = RESET_COUNTER;
	uint8_t chCheckFlag = RESET_FLAG;
	uint8_t chSensorAvail = RESET_FLAG;
	
	if(gchDataDownloadPacketReady)					//If data download task is already running than ignore this request
	{
		SEND_ERROR_CODE_OVER_RF(SENSOR_RADIO_DIVISOR_OVERRUN);
		SEND_DEBUG_ERROR_CODES(SENSOR_RADIO_DIVISOR_OVERRUN);
		return RETURN_TRUE;
	}
	
	//?????????????????
	//Fetch data from smart sensors before filing the data packets
	//?????????????????

	//Initialization of data index from where data filling can be start in the packet
	gchCollectionDataIndex		= PACKET_DATA_MESSAGE_INDEX;
	gchCollectionPacketIndex	= RESET_VALUE;			//reset packet index
	chCheckFlag		= RESET_FLAG;			//reset flag
	chSensorCounter	= RESET_COUNTER;		//reset counter
	chSensorAvail	= RESET_FLAG;			//reset  flag
	
	while(1)
	{
		//This will compare the current radio clock counter with radio clock divisor of particular sensor.
		if(++ghSensorControl[chSensorCounter].chRadioClockCounter >= ghSensorControl[chSensorCounter].chRadioClockDivisor)
		{
			chSensorAvail = SET_FLAG;
			ghSensorControl[chSensorCounter].chRadioClockCounter = RESET_COUNTER;
			
			//This will add the sensor ID before its data
			gchSensorCommBuff[gchCollectionPacketIndex][gchCollectionDataIndex++] = ghSensorControl[chSensorCounter].chSensorID;
			
			//This loop will fill the SENSOR communication buffer with the data
			for(chLoopIndex = RESET_COUNTER;chLoopIndex<ghSensorControl[chSensorCounter].chSensorDataLength;chLoopIndex++)
			{
				gchSensorCommBuff[gchCollectionPacketIndex][gchCollectionDataIndex++]=ghSensorControl[chSensorCounter].nSensorData[chLoopIndex] >> BIT_8_bp;
				gchSensorCommBuff[gchCollectionPacketIndex][gchCollectionDataIndex++]=ghSensorControl[chSensorCounter].nSensorData[chLoopIndex];
				ghSensorControl[chSensorCounter].nSensorData[chLoopIndex] = RESET_VALUE;
			}
			
			if(ghSensorControl[chSensorCounter].chSensorSampleAvgCounter>=ghSensorControl[chSensorCounter].chSensorSampleAvgTotalCount)
			{
				ghSensorControl[chSensorCounter].chSensorSampleAvgCounter = RESET_COUNTER;
				ghSensorControl[chSensorCounter].nSensorAvgData = RESET_VALUE;
			}
		}
		
		if(++chSensorCounter>=gchTotalSensorEntry)
		{
			if(chSensorAvail)
			{
				chCheckFlag = SET_FLAG;
			}
			else
			{
				return RETURN_TRUE;
			}
		}
		
		//If the packet is full and not able to fill the total data of sensor then fill the next coming packet
		if(chCheckFlag || (gchCollectionDataIndex + (ghSensorControl[chSensorCounter].chSensorDataLength*2) + 1 > PACKET_LAST_DATA_BYTE_INDEX))		//1 is used to add Offset of SensorID
		{
			if (ghMasterTaskTable.chDataDownloadChannel == RADIO_CH_FAST_DOWNLINK_CC2520)
			{
				gchSensorCommBuff[gchCollectionPacketIndex][PACKET_HEADER_INDEX]=FAST_DL_DATA_PACKET;	//fast down link
				ghSensorCommManager.hPacketDescriptor.chTransmitterID=RADIO_CH_FAST_DOWNLINK_CC2520;
			}
			else
			{
				gchSensorCommBuff[gchCollectionPacketIndex][PACKET_HEADER_INDEX]=SLOW_DL_DATA_PACKET;	//slow down link
				ghSensorCommManager.hPacketDescriptor.chTransmitterID=RADIO_CH_SLOW_DOWNLINK_CC1125;
			}
			
			if(chCheckFlag)
			{
				ghSensorCommManager.hPacketDescriptor.chLastPacket = SET_FLAG;	//last packet indication
			}
			else
			{
				ghSensorCommManager.hPacketDescriptor.chLastPacket = RESET_FLAG;
			}
			
			gchCollectionDataIndex--;
			ghSensorCommManager.hPacketDescriptor.chPacketSeqNo=gchCollectionPacketIndex+1;
			gchSensorCommBuff[gchCollectionPacketIndex][PACKET_DESCRIPTOR_INDEX]=ghSensorCommManager.hPacketDescriptor.chPacketDescriptor;
			gchSensorCommBuff[gchCollectionPacketIndex][PACKET_DATA_LENGTH_INDEX]=gchCollectionDataIndex;
			gchSensorCommBuff[gchCollectionPacketIndex][CC1125_DATA_PACKET_LENGTH]=gchCollectionDataIndex;
			gchSensorCommBuff[gchCollectionPacketIndex][PACKET_DOWNLOAD_DATA_SIZE]=gchCollectionDataIndex - PACKET_DATA_MESSAGE_INDEX + 2;	//download data size
			gchCollectionDataIndex=PACKET_DATA_MESSAGE_INDEX;
			
			if(chCheckFlag)
			{
				break;
			}
		}
	}
	
	ghSensorCommManager.chTotalPacketCount=gchCollectionPacketIndex+1;
	gchCollectionPacketIndex	= RESET_COUNTER;		//Resetting of packet index
	gchCollectionDataIndex		= RESET_COUNTER;			//Resetting of Data index
	gchDataDownloadPacketReady = SET_FLAG;
	
	SEND_DEBUG_STRING("Data Collection Completed\n");
	return RETURN_TRUE;
}

//_____ fnResetFirmwareResourceAllocations ____________________________________________________________________
//
// @brief	This function will get call when it is require to free all the shared resources which may occupied in middle and lower level during execution
//			Probably this function will executed when new tasking table is available with SENSOR MC
//			It vanish all the shared resources including,
//				1> Lower level interfaces: ADC, SPI, I2C
//				2> Power source management related definitions
//				3> Middle layer definitions	used for communicating with radio and sensors
//				4> Upper layer definitions used by system level task

void fnResetFirmwareResourceAllocations(void)
{
	//ADC Resources
	fnResetAdcResources();
	
	//SPI Resources
	fnResetSpiResources();
	
	//I2C Resources
	fnResetI2cResources();
	
	gchLvl1StepIndexI2C=RESET_VALUE;
	gchLvl2StepIndexI2C=RESET_VALUE;
	gchStepIndexE2PROM=RESET_VALUE;
	
	//Power Resources
	ghPowerManager.chCheckFlag=RESET_FLAG;
	ghPowerManager.chPowerUsageCounter=RESET_COUNTER;
	
	//Data Collection Task
	gchCollectionDataIndex=RESET_VALUE;
	gchCollectionPacketIndex=RESET_VALUE;
	
	//System Global Definitions
	gchDataDownloadPacketReady=RESET_FLAG;
	gchTotalSensorEntry=RESET_COUNTER;
	
	return;
}
