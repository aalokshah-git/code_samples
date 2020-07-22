/*-------------------------------------------------------------------------
Filename: sensor_protocol.c

Job#: 20473
Date Created: 11/12/2014

Purpose: To manages all the required mechanism to maintain the reliable communication over RF for Data Download and TT Request Task.

Functions:
fnData_ET_Request_Task			Execution Table Request task (It will execute when SENSOR in IDLE mode)
fnRadioTxRxHandler				It includes all the required scenarios to manage the communication over RF
fnData_Download_Task			Data Download Task (It will execute when SENSOR in Data mode)
fnET_Upload_Task				It verifies the received tasking table and initialize the allocated memory resources for the same

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

#include "sensor_protocol.h"				//All definitions required for TT Request and Data Download task execution

//_____ G L O B A L   D E F I N A T I O N S _______________________________________________

//Holds the various fields required for RF communication
volatile SENSOR_COMM_MANAGER			ghSensorCommManager;

//Memory resources for the data with multi packet support
uint8_t					gchSensorCommBuff[MAX_COMM_PACKET_COUNT][MAX_COMM_PACKET_SIZE];

//Holds current state of RF communication
volatile Radio_Handler_Step_Enum_t		gchRadioCommStepMode;

//Flag used to signal the data download task that data collection task is completed and data packet are ready
volatile uint8_t					gchDataDownloadPacketReady;

//Holds the value for no of sensors available in current tasking table
volatile uint8_t					gchTotalSensorEntry;

//Flag will get set from communication wait timer to indicate the time out condition
volatile uint8_t					gchCommunicationTimeOut;

//Flag to indicate SENSOR loop back is ON or OFF
volatile Loop_Back_Step_Enum_t		gchSensorLoopBack;

//Variable used to support the communication wait time doubling scenarios
volatile uint8_t					gchCommWaitTimeDoubleCounter;

//Holds default value of communication wait time
volatile uint16_t					gnDefaultCommWaitTimeValue;

//Variable used to indicate the reply type - ACK/NACK after receiving the data from RFC
uint8_t								gchAckReplyType;

//____fnET_Upload_Task _________________________________________________________________
//
// @brief	Function fill up the memory resources assigned to hold the tasking table details of master controller as well as individual sensors. 
//			It validates received tasking table for its value and execute accordingly for the data collection processes.
//			The functionality of this task can be explained as below:
//				Whenever the new TT is available all the shared resources are released and sample clock is disabled to avoid the conflict. 
//				It will also perform boundary checks for all the received data bytes for sample clock, radio clock divisor, comm wait time, etc.. 
//				If any mismatch will get found in the checking than this function will return and task manager of SENSOR MC firmware start executing Execution Table request task with default tasking table.
//				If all the fields are within boundary than data structures will get filled with the new tasking table data and task manager will start executing as per requirement.
//				In the case of mismatch it will also raise the overrun errors indicating for the same.
//				After successfully loading the Master Controller this function will load all the sensor's individual tasking table in the same way.
// @param	pchSensorRxBuff	Pointer to memory resources used to hold the received data over RF
// @return	FALSE if boundary mismatch in any of the parameter will get found in received data otherwise returns TRUE

inline int8_t fnET_Upload_Task(uint8_t *pchSensorRxBuff)
{
	uint8_t chSensorCounter;
	
	//Disable Sample Clock
	fnStopSampleClock();
	
	//Release all the shared resources to avoid conflict
	fnResetFirmwareResourceAllocations();
		
	if(pchSensorRxBuff[PACKET_MASTER_ID_INDEX] != ID_MASTER_CONTROLLER)		//Master Controller ID
	{
		return RETURN_FALSE;
	}
	
	//Sample Clock Frequency
	ghMasterTaskTable.nSampleClock = ((uint16_t)pchSensorRxBuff[PACKET_SAMPLE_CLOCK_FREQ_BYTE1] << BIT_8_bp) | pchSensorRxBuff[PACKET_SAMPLE_CLOCK_FREQ_BYTE2];	
	
	if(ghMasterTaskTable.nSampleClock== RESET_VALUE)		//If Sample Clock is zero than continue TT Request operation
	{
		return RETURN_FALSE;
	}
	else if(ghMasterTaskTable.nSampleClock>MAX_SAMPLE_CLOCK_FREQ)
	{
		ghMasterTaskTable.nSampleClock=MAX_SAMPLE_CLOCK_FREQ;
	}
	
	//Check for Data Download Channel
	if(pchSensorRxBuff[PACKET_CONTROL_BYTE1] & DOWNLINK_BM)
	{
		ghMasterTaskTable.chDataDownloadChannel=RADIO_CH_FAST_DOWNLINK_CC2520;
	}
	else
	{
		ghMasterTaskTable.chDataDownloadChannel=RADIO_CH_SLOW_DOWNLINK_CC1125;
	}
	
	//Radio Clock Divisor
	ghMasterTaskTable.nRadioClockDivisor =((uint16_t)(pchSensorRxBuff[PACKET_CONTROL_BYTE1] & BIT_0_bm) << BIT_8_bp) | pchSensorRxBuff[PACKET_CONTROL_BYTE2];				
	
	if(ghMasterTaskTable.nRadioClockDivisor<MIN_RADIO_CLOCK_DIVISOR)
	{
		SEND_ERROR_CODE_OVER_RF(SENSOR_RADIO_DIVISOR_OVERRUN);
		SEND_DEBUG_ERROR_CODES(SENSOR_RADIO_DIVISOR_OVERRUN);
		return RETURN_FALSE;
	}
	
	//Message Response Wait Time
	ghMasterTaskTable.nCommTimeout = ((uint16_t) pchSensorRxBuff[PACKET_WAIT_TIME_BYTE1] << BIT_8_bp) | pchSensorRxBuff[PACKET_WAIT_TIME_BYTE2];
	
	if(ghMasterTaskTable.nCommTimeout < MIN_COMM_WAIT_TIME_OUT)
	{
		SEND_ERROR_CODE_OVER_RF(SENSOR_COMM_WAIT_TIME_MISMATCH);
		SEND_DEBUG_ERROR_CODES(SENSOR_COMM_WAIT_TIME_MISMATCH);
		return RETURN_FALSE;
	}
	else
	{
		gnDefaultCommWaitTimeValue=ghMasterTaskTable.nCommTimeout;
	}
	
	//If sample clock is greater then minimum sample clock value to keep 5v source on then 5v source remains on
	if(ghMasterTaskTable.nSampleClock >= MIN_SAMPLE_CLOCK_VALUE_FOR_KEEP_5V_ON)
	{
		gchPowerSourceMode=POWER_SOURCE_ALWAYS_ON;
	}
	else
	{
		if(gchPowerSourceMode==POWER_SOURCE_ALWAYS_ON)
		{
			//Release Power Resources- ???????????
		}
		
		gchPowerSourceMode=POWER_SOURCE_AT_REQUIRE;
	}
	
	for(chSensorCounter=SENSORS_START_INDEX; chSensorCounter<pchSensorRxBuff[PACKET_LENGTH_INDEX] ;chSensorCounter += SENSOR_ENTRY_OFFSET)
	{
		//Skip Sensors with the Sample Clock divisor value zero
		if(pchSensorRxBuff[chSensorCounter + SAMPLE_CLOCK_DIVISOR_OFFSET] == RESET_VALUE)
		{
			continue;
		}
		
		ghSensorControl[gchTotalSensorEntry].chSensorID					= pchSensorRxBuff[chSensorCounter + SENSOR_EXECUTION_TABLE_ID_OFFSET];			//sensor id
		ghSensorControl[gchTotalSensorEntry].chSensorCtrlByte 			= pchSensorRxBuff[chSensorCounter + SENSOR_CONTROL_BYTE_OFFSET];				//sensor control byte
		ghSensorControl[gchTotalSensorEntry].chSampleClockDivisor 		= pchSensorRxBuff[chSensorCounter + SAMPLE_CLOCK_DIVISOR_OFFSET];				//sample clock divisor
		ghSensorControl[gchTotalSensorEntry].chRadioClockDivisor 		= pchSensorRxBuff[chSensorCounter + RADIO_CLOCK_DIVISOR_OFFSET];				//radio clock divisor
		ghSensorControl[gchTotalSensorEntry].chSensorSampleAvgTotalCount= pchSensorRxBuff[chSensorCounter + SAMPLES_IN_AVERAGE_OFFSET];				//sensor sample clock averages
	
		ghSensorControl[gchTotalSensorEntry].chSensorDataLength 		= fnFetchSensorDataLength(ghSensorControl[gchTotalSensorEntry].chSensorID);	//sensor data length
		
		if(ghSensorControl[gchTotalSensorEntry].chSensorSampleAvgTotalCount == RESET_VALUE || ghSensorControl[gchTotalSensorEntry].chSensorSampleAvgTotalCount>MAX_SAMPLE_AVERAGE_SUPPORTED)
		{
			ghSensorControl[gchTotalSensorEntry].chSensorSampleAvgTotalCount = MIN_SAMPLE_AVG_COUNT;
			SEND_ERROR_CODE_OVER_RF(SENSOR_SAMPLE_AVERAGE_COUNT_EXCEEDS);		
			SEND_DEBUG_ERROR_CODES(SENSOR_SAMPLE_AVERAGE_COUNT_EXCEEDS);
		}
		
		//Reset various sequential counters to its initial value
		ghSensorControl[gchTotalSensorEntry].chSampleClockCounter 		= ghSensorControl[gchTotalSensorEntry].chSampleClockDivisor;	//sample all requested sensors for the first time
		ghSensorControl[gchTotalSensorEntry].chRadioClockCounter		= RESET_VALUE;
		ghSensorControl[gchTotalSensorEntry].chSensorSampleAvgCounter	= RESET_VALUE;
		ghSensorControl[gchTotalSensorEntry].nSensorAvgData				= RESET_VALUE;
		ghSensorControl[gchTotalSensorEntry].chSensorStatusFlag 		= SAMPLE_STATE_OFF;
		gchTotalSensorEntry++;	//Increment sensor counter on every successful fetch
	}
	
	//Configure the sample clock scenarios with the new available data
	fnConfigureSampleClock(ghMasterTaskTable.nSampleClock);
	SEND_DEBUG_STRING("TT Uploaded Successfully\n");
	
	return RETURN_TRUE;
}

//____fnRadioTxRxHandler _________________________________________________________________
//
// @brief	Function is designed to handle all the RF communication between SENSOR and RFCM.
//			Entire communication is divided in several steps as listed below:
//				1> Turn On the power for Radio chip
//				2> Initialize the Radio for communication operations
//				3> Transmit the packet to the Radio chip
//				4> Wait for chip to complete the transmission
//				5> Turn On the communication wait timer and wait for the reception till time out ocurs
//				6> If time out occurs than follow the steps again till retry out condittion
//				7> Segregate the received data and if necessary then send back the ACK or NACK
//				8> Turn OFF the radio power
// @return	TRUE if all the steps were executed properly or TT Request/Data Download task which calls this function needs termination otherwise returns FALSE

inline int8_t fnRadioTxRxHandler(void)
{
	uint8_t chNextPacket			= RESET_FLAG;							//Used to execute multiple iteration of loop where it requires to execute multiple steps at once
	uint8_t chRxBytes				= RESET_VALUE;
	uint8_t chStatusByte			= RESET_VALUE;
	uint8_t chSensorRxBuff[MAX_COMM_PACKET_SIZE];								//Resources to hold receive buffer for RF communication
	
	do
	{
		//If Loop back mode is ON than execution needs to stay in current do_while loop
		if(gchSensorLoopBack != LB_STEP_0)
		{
			//Disable Watchdog during Loop back Mode
			RESET_WDT;
			chNextPacket = SET_FLAG;
		}
		else
		{
			//Will get reset in every iteration and get set when multiple steps required to execute
			chNextPacket = RESET_FLAG;
		}
		
		switch (gchRadioCommStepMode)
		{
			//Power ON the Radio Chip
			case RADIO_PWR_CHECK_MODE:
				if(fnPowerSourceManager(POWER_SOURCE_ENABLE,CC1125_POWER_IDENTITY)==RETURN_TRUE)
				{
					gchRadioCommStepMode = RADIO_LINK_SELECT_MODE;
					chNextPacket = SET_FLAG;
				}
			break;
			
			//Select between CC1125 and CC2520 chip
			case RADIO_LINK_SELECT_MODE:
				if(ghMasterTaskTable.chDataDownloadChannel == RADIO_CH_FAST_DOWNLINK_CC2520)
				{
					SELECT_CC2520_RADIO;
					//CHECK FOR CC2520 VOLTAGE STABILIZATION - ???????
				}
				else
				{
					SELECT_CC1125_RADIO;
					chStatusByte=fnRadioCC1125Initialization();
					
					if(chStatusByte == SENSOR_NO_ERROR)
					{
						gchRadioCommStepMode=RADIO_TX_MODE;
						chNextPacket= SET_FLAG;
					}
					else
					{
						SEND_DEBUG_ERROR_CODES(chStatusByte);
						SEND_ERROR_CODE_OVER_RF(chStatusByte);
					}
				}
			break;
			
			//Transmit data over RF
			case RADIO_TX_MODE:
			
				//Put Radio in IDLE mode before performing the send operation
				fnCC112xSendReceiveHandler(RADIO_COMMAND_STROBE,CC112X_SFTX,1,NULL);
				fnWait_uSecond(5);
				fnCC112xSendReceiveHandler(RADIO_COMMAND_STROBE,CC112X_SIDLE,1,NULL);
				fnWait_uSecond(5);
				
				//Fill up the Send Retry for the same packet which will going to send next
				if(gchSensorLoopBack==LB_STEP_0)
				{
					gchSensorCommBuff[ghSensorCommManager.chPacketCounter][PACKET_ERROR_CONTROL_INDEX]=ghSensorCommManager.hPacketCheck.chPacketCheck;
					SEND_ERROR_CODE_OVER_RF(SENSOR_NO_ERROR);		//Error Sending Successful
				}
				
				//Start sending data to Radio Chip
				fnCC112xSendReceiveHandler(RADIO_TRANSMIT_BYTES,CC112X_TXFIFO,gchSensorCommBuff[ghSensorCommManager.chPacketCounter][CC1125_DATA_PACKET_LENGTH]+1,gchSensorCommBuff[ghSensorCommManager.chPacketCounter]);		//Transmit bytes to TX FIFO of Radio chip
			
				//Check for the status of transmit operation
				fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,CC112X_MARCSTATE,1,&chStatusByte);		//Check the Radio chip status
				if((chStatusByte & CC1125_FIFO_ERR_CHECK_BM) == CC112X_STATE_TXFIFO_ERROR)
				{
					//If error related to TX FIFO than flush the FIFO
					fnCC112xSendReceiveHandler(RADIO_COMMAND_STROBE,CC112X_SFTX,1,NULL);
					SEND_DEBUG_ERROR_CODES(SENSOR_CC1125_TX_FIIO_ERR);
				}
				else
				{
					//Notify CC1125 chip to start send operation by providing the "STX" strobe
					fnCC112xSendReceiveHandler(RADIO_COMMAND_STROBE,CC112X_STX,1,NULL);
				
					//Start communication wait timer for transmit operation
					fnStartCommunicationTimer(CC1125_TX_COMPLETE_TIMEOUT_WAIT);
					gchRadioCommStepMode=TX_TIME_OUT_MODE;
				}
			break;
			
			//Time out for transmit operation of Radio chip
			case TX_TIME_OUT_MODE:
				
				//Check for the interrupt from CC1125 indicating data transmission complete
				if(fnCC112xSendDataComplete()==RETURN_TRUE)
				{
					gchRadioCommStepMode=RADIO_RX_MODE;
					fnRadioCommunicationTimerDisable();									//Data send complete before time out so disable the communication timer
				
					if (gchSensorLoopBack==LB_STEP_0)
					{
						fnStartCommunicationTimer(ghMasterTaskTable.nCommTimeout);			//Start communication timer and wait for data from RFCM
					}
					else
					{
						fnStartCommunicationTimer(LOOPBACK_WAIT_TIMEOUT);
					}
				}
				else
				{
					//Check time out flag of communication wait timer
					if(gchCommunicationTimeOut)
					{
						//Flush the TX FIFO to avoid unwanted exception in next transmission
						fnCC112xSendReceiveHandler(RADIO_COMMAND_STROBE,CC112X_SFTX,1,NULL);		//FLUSH The TX FIFO of CC1125.
					
						//Increment the retry counter and send the same packet again if supported retry value not reached otherwise terminate the operation
						if(++ghSensorCommManager.chPacketSendRetryCounter < MAX_RF_COMM_RETRY)
						{
							SEND_DEBUG_ERROR_CODES(SENSOR_CC1125_TX_GPIO_INTTERUPT_FAIL);
							SEND_DEBUG_STRING("Next Retry-TX\n");
							chNextPacket = SET_FLAG;
							gchRadioCommStepMode = RADIO_TX_MODE;
						}
						else
						{
							SEND_DEBUG_ERROR_CODES(SENSOR_CC1125_TX_GPIO_INTTERUPT_FAIL);
							SEND_DEBUG_STRING("Retry Out-TX\n");
							return RETURN_TRUE;			//Terminate the operation
						}
					}
				}
			break;
			
			//Receive data over RF
			case RADIO_RX_MODE:
			
				//Check for the interrupt from CC1125 indicating new data received
				if (RETURN_TRUE == fnCC112xReceiveDataAvail())
				{
					//As data received successfully disable the communication wait timer
					fnRadioCommunicationTimerDisable();
				
					//Get count of total bytes received
					fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,CC112X_NUM_RXBYTES,1,&chRxBytes);
				
					//Fill the SENSOR receive buffer with the received data
					fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,CC112X_RXFIFO,chRxBytes,chSensorRxBuff);
				
					//Variable length mode is used in CC1125 so the received data will be like:
					//First received byte will be data length
					//Data will be appended after second byte
					//Second last byte will be RSSI value
					//Last byte will be control flag with the indication of CRC check
				
					if(!(chSensorRxBuff[chRxBytes-1] & CC1125_CRC_ERR_CHECK_BM))				//Check for CRC
					{
						//Increment the retry counter and send the same packet again if supported retry value not reached otherwise terminate the operation
						if(++ghSensorCommManager.chPacketSendRetryCounter < MAX_RF_COMM_RETRY)
						{
							SEND_DEBUG_ERROR_CODES(SENSOR_CC1125_CRC_MISMATCH_ERR);
							SEND_DEBUG_STRING("Next Retry-CRC\n");
							chNextPacket = SET_FLAG;
							gchRadioCommStepMode = RADIO_TX_MODE;
						}
						else
						{
							SEND_DEBUG_ERROR_CODES(SENSOR_CC1125_CRC_MISMATCH_ERR);
							SEND_DEBUG_STRING("Retry Out-CRC\n");
							return RETURN_TRUE;			//Terminate the operation
						}
					}
					else
					{
						//Fill up the current RSSI value it may require in sensor tasking table
						gchUplinkRSSI= chSensorRxBuff[chRxBytes-2];
						ghSensorCommManager.chPacketSendRetryCounter=0;		//Reset Retry Counter on Successful Reception
					
						//If SENSOR loop back mode is enabled then skip the remaining execution
						if (gchSensorLoopBack != LB_STEP_0)
						{
							SEND_DEBUG_STRING("Testing Loopback Mode with Packet: ");
							gchRadioCommStepMode = RADIO_SENSOR_LOOP_BACK_MODE;
						}
						else
						{
							//On successful reception of the packet check for the received packet type
							switch(chSensorRxBuff[1])
							{
								//Acknowledge received for the last downloaded data packet
								case ACK_RECEIPT_LAST_DATA_PACKET:
							
									//If multi packet query than move ahead and send the next one
									if(++ghSensorCommManager.chPacketCounter < ghSensorCommManager.chTotalPacketCount)
									{
										SEND_DEBUG_STRING("ACk- Sending Next Packet\n");
										gchRadioCommStepMode=RADIO_TX_MODE;		//Send Next Packet
									}
									else
									{
										SEND_DEBUG_STRING("ACK- Data Download Complete\n");
										return RETURN_TRUE;						//Terminate the task as execution completed
									}
								break;
							
								//Mismatch in the last downloaded packet
								case NACK_OUT_OF_SEQUENCE:
								case NACK_INVALID_PACKET:
								case NACK_INVALID_MSG_FORMAT:
							
									//Increment the retry counter and send the same packet again if supported retry value not reached otherwise terminate the operation
									if(++ghSensorCommManager.chPacketSendRetryCounter < MAX_RF_COMM_RETRY)
									{
										SEND_DEBUG_ERROR_CODES(SENSOR_CC1125_NACK_RECEIVED);
										SEND_DEBUG_STRING("Next Retry-NACK\n");
										chNextPacket = SET_FLAG;
										gchRadioCommStepMode = RADIO_TX_MODE;
									}
									else
									{
										SEND_DEBUG_ERROR_CODES(SENSOR_CC1125_NACK_RECEIVED);
										SEND_DEBUG_STRING("Retry Out-NACK\n");
										return RETURN_TRUE;			//Terminate the operation
									}
								break;
							
								//Stop sending data download packet and move ahead in the operation
								case NACK_STOP_SENDING_DATA_MESSAGE:
							
									SEND_DEBUG_STRING("Request for Terminating Data Download\n");
									return RETURN_TRUE;				//Terminate the operation
								break;
							
								//Terminate the on going data operations
								case TERMINATE_DATA_DOWNLOAD:
							
									SEND_DEBUG_STRING("Request for Terminating Data Download\n");
									fnDefaultExecutionTableInit();
									return RETURN_TRUE;					//Terminate the operation
								break;
							
								//Execution table upload
								case NEW_EXECUTION_TABLE_PACKET:
							
									//Implement it for multi packet scenarios - ????????????
									SEND_DEBUG_STRING("New Execution Table Query Received\n");
							
									//Fill up the memory resources assigned for master controller and sensor operations with the received buff
									if(RETURN_TRUE==fnET_Upload_Task(chSensorRxBuff))	//passing address of SENSOR receive buffer
									{
										//Send ACK if uploaded data are fine
										gchAckReplyType= SET_FLAG;
									}
									else
									{
										//Send NACK if uploaded data are not fine
										gchAckReplyType= RESET_FLAG;
									}
							
									gchRadioCommStepMode=RADIO_ET_REPLY_MODE;
									chNextPacket = SET_FLAG;
								break;
							
								//No Execution Table available to execute
								case NO_NEW_ET_AVAILABLE:
							
									//New tasking table is not available so keep doing the TT request query at periodic wake up
									SEND_DEBUG_STRING("Execution Table is not Available\n");
									return RETURN_TRUE;			//Terminate the operation
								break;
							
								//Look Back Mechanism
								case SENSOR_LOOP_BACK_PACKET_SLOW:		//SENSOR loop back on SLOW Down link
								case SENSOR_LOOP_BACK_PACKET_FAST:		//SENSOR loop back on FAST Down Link
									
									SEND_DEBUG_STRING("Start Loop Back Mode\n");
									fnDisableWDT();
									gchSensorLoopBack = LB_STEP_1;				//flag to indicate SENSOR loop back is on
									gchAckReplyType	= SET_FLAG;					//Reply ACK on getting START loop back command
									gchRadioCommStepMode = RADIO_ET_REPLY_MODE;
								break;
							
								default:

									//The received query is undefined - Requires checking
									SEND_DEBUG_ERROR_CODES(SENSOR_PACKET_HEADER_UNDEFINED);
								return RETURN_TRUE;			//Terminate the operation
								break;
							}
						}
					}
				}
				else
				{
					//Check time out flag of communication wait timer
					if(gchCommunicationTimeOut)
					{
						if (gchSensorLoopBack != LB_STEP_0)
						{
							SEND_DEBUG_STRING("Stopping Loopback Mode: Timeout Condittion\n");
							gchSensorLoopBack = LB_STEP_0;
							fnDefaultExecutionTableInit();
							fnEnableWDT();
							return RETURN_TRUE;
						}
					
						//Increment the retry counter and send the same packet again if supported retry value not reached otherwise terminate the operation
						if(++ghSensorCommManager.chPacketSendRetryCounter < MAX_RF_COMM_RETRY)
						{
							SEND_DEBUG_ERROR_CODES(SENSOR_CC1125_COMMUNICATION_WAIT_TIMEOUT);
							SEND_DEBUG_STRING("Next Retry-TimeOut\n");
							chNextPacket = SET_FLAG;
							gchRadioCommStepMode = RADIO_TX_MODE;
						}
						else
						{
							SEND_DEBUG_ERROR_CODES(SENSOR_CC1125_COMMUNICATION_WAIT_TIMEOUT);
							SEND_DEBUG_STRING("Retry Out-TimeOut\n");
						
							if(gchCommWaitTimeDoubleCounter++ < COMM_WAIT_TIME_DOUBLE_RETRY)
							{
								SEND_DEBUG_STRING("Increasing Comm Wait Time\n");
								ghMasterTaskTable.nCommTimeout *= 2;
								chNextPacket = SET_FLAG;
								gchRadioCommStepMode = RADIO_TX_MODE;
								ghSensorCommManager.chPacketSendRetryCounter=RESET_COUNTER;
								SEND_ERROR_CODE_OVER_RF(SENSOR_COMM_WAIT_TIME_MISMATCH);
							
								if(ghMasterTaskTable.nCommTimeout>MAX_COMM_WAIT_TIME_VALUE)
								{
									ghMasterTaskTable.nCommTimeout=MAX_COMM_WAIT_TIME_VALUE;
								}
							}
							else
							{
								return RETURN_TRUE;			//Terminate the operation
							}
						}
					}
				}
			break;
			
			//SENSOR Loop back mechanism
			case RADIO_SENSOR_LOOP_BACK_MODE:
			
				fnRadioCommunicationTimerDisable();
			
				// Get response to load IDLE TT
				if(chSensorRxBuff[1] == STOP_LOOP_BACK_LOAD_IDLE)
				{
					gchSensorLoopBack = LB_STEP_2;
					fnEnableWDT();
					fnDefaultExecutionTableInit();
					SEND_DEBUG_STRING("Stop Loop Back with IDLE TT\n");
					gchAckReplyType	= SET_FLAG;											//Reply ACK on getting STOP loop back command
					gchRadioCommStepMode = RADIO_ET_REPLY_MODE;
				}
			
				//If start active tasking table command received
				else if(chSensorRxBuff[1] == STOP_LOOP_BACK_LOAD_ACTIVE)
				{
					gchSensorLoopBack = LB_STEP_2;
					fnEnableWDT();
					gchAckReplyType	= SET_FLAG;											//Reply ACK on getting STOP loop back command
					gchRadioCommStepMode = RADIO_ET_REPLY_MODE;
					SEND_DEBUG_STRING("Stop Loopback with Active TT\n");
				}
			
				else
				{
					fnMemCopy((uint8_t*)gchSensorCommBuff[BASE_INDEX],chSensorRxBuff,chSensorRxBuff[CC1125_DATA_PACKET_LENGTH]+1);	//copy all received data into transmit buffer
					SEND_DEBUG_DATA_BYTES(&chSensorRxBuff[1],chSensorRxBuff[CC1125_DATA_PACKET_LENGTH]);
					SEND_DEBUG_STRING("\n");
					ghSensorCommManager.chPacketCounter = RESET_COUNTER;
					gchRadioCommStepMode = RADIO_TX_MODE;							//enable transmit mode
				}
			break;
			
			//Send ACK/NACK for the received Execution Table upload query
			case RADIO_ET_REPLY_MODE:
			
				//Put Radio in IDLE mode before performing the send operation
				fnCC112xSendReceiveHandler(RADIO_COMMAND_STROBE,CC112X_SIDLE,1,NULL);
			
				//Occasional delay
				fnWait_uSecond(5);
			
				ghSensorCommManager.hPacketDescriptor.chPacketSeqNo	= SET_COUNTER;	//Packet sequence no
				ghSensorCommManager.hPacketDescriptor.chLastPacket	= SET_FLAG;		//Last packet indicator
			
				//Check for Reply Type
				if(gchAckReplyType==SET_FLAG)
				{
					gchSensorCommBuff[BASE_INDEX][PACKET_HEADER_INDEX]=ACK_RECEIPT_OF_LAST_ET_PACKET;
				}
				else
				{
					gchSensorCommBuff[BASE_INDEX][PACKET_HEADER_INDEX]=NACK_RECEIPT_OF_LAST_ET_PACKET;
				}
			
				//Query consist of packet header of 4 bytes
				gchSensorCommBuff[BASE_INDEX][PACKET_DATA_LENGTH_INDEX]	= 4;
				gchSensorCommBuff[BASE_INDEX][PACKET_DESCRIPTOR_INDEX]	= ghSensorCommManager.hPacketDescriptor.chPacketDescriptor;
				gchSensorCommBuff[BASE_INDEX][CC1125_DATA_PACKET_LENGTH]	= gchSensorCommBuff[0][PACKET_DATA_LENGTH_INDEX];
				gchSensorCommBuff[BASE_INDEX][PACKET_ERROR_CONTROL_INDEX]	= ghSensorCommManager.hPacketCheck.chPacketCheck;
			
				//Start sending data to Radio Chip
				fnCC112xSendReceiveHandler(RADIO_TRANSMIT_BYTES,CC112X_TXFIFO,gchSensorCommBuff[BASE_INDEX][CC1125_DATA_PACKET_LENGTH]+1,gchSensorCommBuff[BASE_INDEX]);
			
				//Check for the status of transmit operations
				fnCC112xSendReceiveHandler(RADIO_RECEIVE_BYTES,CC112X_MARCSTATE,1,&chStatusByte);
				if((chStatusByte & CC1125_FIFO_ERR_CHECK_BM) == CC112X_STATE_TXFIFO_ERROR)
				{
					//If error related to TX FIFO of Radio chip than flush the FIFO
					SEND_DEBUG_ERROR_CODES(SENSOR_CC1125_TX_FIIO_ERR);
				
					//Flush the TX FIFO of Radio chip
					fnCC112xSendReceiveHandler(RADIO_COMMAND_STROBE,CC112X_SFTX,1,NULL);
					return RETURN_TRUE;			//Terminate the operation
				}
				else
				{
					//Notify CC1125 chip to start send operation by providing the "STX" strobe
					fnCC112xSendReceiveHandler(RADIO_COMMAND_STROBE,CC112X_STX,1,NULL);
				
					//Start communication wait timer for transmit operation
					fnStartCommunicationTimer(CC1125_TX_COMPLETE_TIMEOUT_WAIT);
					gchRadioCommStepMode=RADIO_ET_REPLY_TIME_OUT_MODE;
				}
			break;

			//Time out for transmit operation of Radio chip
			case RADIO_ET_REPLY_TIME_OUT_MODE:
			
				//Check for the interrupt from CC1125 indicating data transmission complete
				if(RESET_VALUE == fnCC112xSendDataComplete())
				{
					fnRadioCommunicationTimerDisable();						//Data send complete before time out so disable the communication timer
				
					switch (gchSensorLoopBack)
					{
						case LB_STEP_1:
							gchRadioCommStepMode = RADIO_RX_MODE;
							SEND_DEBUG_STRING("Request for Start Loopbak: ACK Sent\n");
							fnStartCommunicationTimer(LOOPBACK_WAIT_TIMEOUT);
						break;
					
						case LB_STEP_2:
							gchSensorLoopBack=LB_STEP_0;
							return RETURN_TRUE;
						break;
					
						default:
							if(gchAckReplyType==SET_FLAG)
							{
								gchTasks_Enable &= (~EXECUTION_TABLE_REQ_TASK);
								gchTasks_Active = DISABLE_ALL_TASKS;
								gchTasks_Enable |= DATA_SAMPLING_TASK | DATA_COLLECTION_TASK | DATA_DOWNLOAD_TASK;
							}
							else
							{
								fnDefaultExecutionTableInit();
							}
							return RETURN_TRUE;
						break;
					}
				
				}
				else
				{
					//Check time out flag of communication wait timer
					if(gchCommunicationTimeOut)
					{
						//Flush the TX FIFO to avoid unwanted exception in next transmission
						fnCC112xSendReceiveHandler(RADIO_COMMAND_STROBE,CC112X_SFTX,1,NULL);
						SEND_DEBUG_ERROR_CODES(SENSOR_CC1125_TX_GPIO_INTTERUPT_FAIL);
						return RETURN_TRUE;																//Terminate the operation
					}
				}
			break;
			
			default:
				SEND_DEBUG_ERROR_CODES(SENSOR_CC1125_TXRX_UNDEFINED_STATE);	//Terminate the operation
				return RETURN_TRUE;
			break;
		}
		
	}while(chNextPacket);		//Check for the next iteration
	
	return RETURN_FALSE;
}

//____fnData_Download_Task _________________________________________________________________
//
// @brief	Function will get call from the task manager itself when it wants to send data packets over RF on the basis of various Radio Clocks.
//			Most of the contents of the packet which needs to send will get created in the data collection task.
//			The only operation this task has to perform is to send those packets reliably over RF with the help of CC1125.
// @return	TRUE if all the steps required to perform Data Download operations in fnRadioTxRxHandler completed successfully

int8_t fnData_Download_Task(void)
{
	//Flag used to make data collection and data download task sequential
	//It will get set from data collection task to notify that the packet is ready for the downloading
	if(gchDataDownloadPacketReady)
	{
		//First initial step for the data download operation
		if(gchRadioCommStepMode == COMM_ENTRY_POINT)
		{
			SEND_DEBUG_STRING("Data Download Task Entry\n");
			gchRadioCommStepMode=RADIO_PWR_CHECK_MODE;
			
			//Reset counter related fields
			ghSensorCommManager.chPacketCounter			= RESET_COUNTER;
			ghSensorCommManager.chPacketSendRetryCounter	= RESET_COUNTER;
			
			if(gchCommWaitTimeDoubleCounter>=COMM_WAIT_TIME_DOUBLE_RETRY)
			{
				gchCommWaitTimeDoubleCounter=RESET_COUNTER;
				ghMasterTaskTable.nCommTimeout=gnDefaultCommWaitTimeValue;
				SEND_DEBUG_STRING("Comm Wait Time initialized to default\n");
			}
		}
				
		if(fnRadioTxRxHandler()==RETURN_TRUE)
		{
			//Disable the Radio Power and initialize necessary fields for the future operation
			gchRadioCommStepMode = COMM_ENTRY_POINT;
			fnPowerSourceManager(POWER_SOURCE_DISABLE,CC1125_POWER_IDENTITY);
			gchDataDownloadPacketReady= RESET_FLAG;
			SEND_DEBUG_STRING("Data Download Task Exit\n");
			return RETURN_TRUE;			//Task is complete
		}
	}
	else
	{
		//If Data Collection is task is not active it means no data packets are available to send
		if(!(gchTasks_Active & DATA_COLLECTION_TASK))
		{
			return RETURN_TRUE;			//Task is complete
		}
	}
	return RETURN_FALSE;				//Task is running
}

//____fnData_ET_Request_Task _________________________________________________________________
//
// @brief	Function is called from the task manager itself when it wants to request for the new tasking table to RFC Console.
//			The packet for the same is created first and than fnRadioTxRxHandler handles the rest of the communication steps.
// @return	TRUE if all the steps required to perform TT Request operations in fnRadioTxRxHandler completed successfully

int8_t fnData_ET_Request_Task(void)
{
	//First initial step for the tasking table request operation
	if(gchRadioCommStepMode == COMM_ENTRY_POINT)
	{
		SEND_DEBUG_STRING("TT Req Task Entry\n");
		
		//Create the packet for TT Request
		ghSensorCommManager.chTotalPacketCount				= SET_COUNTER;	//Total no of packets
		ghSensorCommManager.hPacketDescriptor.chPacketSeqNo	= SET_COUNTER;	//Packet sequence no
		ghSensorCommManager.hPacketDescriptor.chLastPacket	= SET_FLAG;		//Last packet indicator
		ghSensorCommManager.hPacketDescriptor.chTransmitterID	= RADIO_CH_SLOW_DOWNLINK_CC1125;	//communication link
		
		gchSensorCommBuff[BASE_INDEX][PACKET_HEADER_INDEX]		= REQUEST_NEW_EXECUTION_TABLE;		//0x81
		gchSensorCommBuff[BASE_INDEX][PACKET_DATA_LENGTH_INDEX]	= ET_REQ_PACKET_LENGTH;
		gchSensorCommBuff[BASE_INDEX][CC1125_DATA_PACKET_LENGTH]	= ET_REQ_PACKET_LENGTH;
		gchSensorCommBuff[BASE_INDEX][PACKET_DESCRIPTOR_INDEX]	= ghSensorCommManager.hPacketDescriptor.chPacketDescriptor;
		
		gchRadioCommStepMode=RADIO_PWR_CHECK_MODE;
		
		//Reset counter related fields
		ghSensorCommManager.chPacketCounter			= RESET_COUNTER;
		ghSensorCommManager.chPacketSendRetryCounter	= RESET_COUNTER;
		
		if(gchCommWaitTimeDoubleCounter>=COMM_WAIT_TIME_DOUBLE_RETRY)
		{
			gchCommWaitTimeDoubleCounter=RESET_COUNTER;
			ghMasterTaskTable.nCommTimeout=gnDefaultCommWaitTimeValue;
			SEND_DEBUG_STRING("Comm Wait Time initialized to default\n");
		}
	}
	
	if(fnRadioTxRxHandler()==RETURN_TRUE)
	{
		//Disable the Radio Power and initialize necessary fields for the future operation
		gchRadioCommStepMode = COMM_ENTRY_POINT;
		fnPowerSourceManager(POWER_SOURCE_DISABLE,CC1125_POWER_IDENTITY);
		SEND_DEBUG_STRING("TT Req Task Exit\n");
		return RETURN_TRUE;				//Task is complete
	}
	
	return RETURN_FALSE;				//Task is running
}

