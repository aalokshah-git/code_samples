/* -------------------------------------------------------------------------
Filename: data_structure.h

Job#: 20473
Purpose: This file contains definitions of all the data structures designed to handle the task manager
Date Created: 11/29/2014

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

Note: See document 000xxxx for complete project requirements
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

#ifndef DATA_STRUCTURE_H_
#define DATA_STRUCTURE_H_

	//_________________________________________ M A C R O S ________________________________________________

	#define MAX_SENSOR_DATA_LENGTH					5				//Maximum size of data bytes considered for particular sensor
	#define MAX_COMM_PACKET_COUNT					8				//Maximum no of packets supported in SENSOR protocol
	#define MAX_COMM_PACKET_SIZE					135				//Maximum bytes considered for individual packet: Data:128, Data Length:1, RSSI + CRC Status:2
	#define MAX_SENSOR_COUNT						127				//Maximum no of sensors supported in design
	#define MAX_RF_COMM_RETRY						3				//Supported no of retry in case of NACK or Timeout in SENSOR to RFC communication
	#define MIN_SAMPLE_CLOCK_VALUE_FOR_KEEP_5V_ON	1				//TBD-???
	
	#define MIN_RADIO_CLOCK_DIVISOR					3				//Minimum Radio sample clock divisor
	#define MIN_COMM_WAIT_TIME_OUT					150				//Minimum Communication wait time out
	#define MAX_MSG_SEQ_NO_VALUE					15				//Maximum message sequence value
	#define CC1125_TX_COMPLETE_TIMEOUT_WAIT			500				//The maximum time considered for CC1125 to complete the transmission
	#define LOOPBACK_WAIT_TIMEOUT					15000			//The maximum time considered between consecutive loop back query
	#define MAX_SAMPLE_AVERAGE_SUPPORTED			50				//The maximum samples supported for performing the averaging operations

	//Default TT definitions
	#define DEFAULT_ET_SAMPLE_CLOCK					1				//Default Master Sample Clock Value
	#define DEFAULT_ET_RADIO_CLOCK					10				//Default Master Radio Clock Divisor Value
	#define DEFAULT_ET_COMM_WAIT_TIME				1000			//Default Execution Table wait time

	//Radio Channel Identity
	#define	RADIO_CH_UPLINK_CC1125					0x01				//CC1125 Uplink channel
	#define RADIO_CH_SLOW_DOWNLINK_CC1125			0x02				//CC1125 Slow Downlink channel
	#define RADIO_CH_FAST_DOWNLINK_CC2520			0x03				//CC1125 Fast Downlink channel

	//__________________________________E N U M E R A T I O N S ___________________________
	
	//Various Power Levels available in firmware
	typedef enum
	{
		POWER_STATE_OFF,							//5V Power is OFF
		POWER_STATE_5V_ON,							//5V Power is ON
		POWER_STATE_5V_3V3_ON						//5V Power is ON along with the required 3.3V for CC2520
	}Power_Source_Enum_t;
	
	//__________________________________D A T A   S T R U C T U R E S ___________________________

	//Fields related to master controller tasking table
	typedef struct
	{
		uint16_t	nRadioClockCounter;				//Sequential counter to support radio clock generation
		uint16_t	nRadioClockDivisor;				//SENSOR MC Radio clock divisor value
		uint8_t		chDataDownloadChannel;			//Radio Channel Identity
		uint16_t	nSampleClock;					//SENSOR MC Sample clock frequency value
		uint16_t	nCommTimeout;					//SENSOR to RF communication- Message response wait time (in ms)
	}MASTER_CONTROLLER_STRUCT;

	//Fields related to sensor management and sensor tasking table
	typedef struct
	{
		uint8_t		chSensorID;								//Sensor ID
		uint8_t		chSensorCtrlByte;						//Sensor control byte (Specifically for smart sensor management)
		uint8_t		chSampleClockDivisor;					//Sensor- Sample clock divisor value
		uint8_t		chSampleClockCounter;					//Sequential counter to support sample clock generation
		uint8_t		chRadioClockDivisor;					//Sensor- Radio clock divisor value
		uint8_t		chRadioClockCounter;					//Sequential counter to support radio clock generation
		uint8_t		chSensorStatusFlag;						//To hold the current state of sensor's operation
		uint8_t		chSensorDataLength;						//Maximum length of data the sensor can have
		uint16_t	nSensorData[MAX_SENSOR_DATA_LENGTH];	//Memory resources to store the sampled data of sensor
		uint32_t	nSensorAvgData;							//Storage for Averaging value
		uint8_t		chSensorSampleAvgTotalCount;			//No of samples include in average
		uint8_t		chSensorSampleAvgCounter;				//Counter to manage average of samples
	}SMART_SENSORS_STRUCT;

	//Fields required for packet based communication over RF
	typedef struct
	{
		union
		{
			uint8_t chPacketDescriptor;
			struct
			{
				uint8_t chReserved:2;
				uint8_t chPacketSeqNo:3;			//Indicate packet sequence no when query for communication is divided in multi packet scenario
				uint8_t chTransmitterID:2;			//Radio channel identity
				uint8_t chLastPacket:1;				//Flag to indicate the last packet in multi packet query (Value will be 1 for last packet)
			};
		}hPacketDescriptor;
	
		union
		{
			uint8_t chPacketCheck;				
			struct
			{
				uint8_t chMsgSeqNo:3;				//Message Sequence No
				uint8_t chOverrunErrorID:5;			//Over run error ID
			};
			struct
			{
				uint8_t chSensorPacketACK:3;			//Packet ACK info
				uint8_t chPacketReceiveCounter:5;	//Packet receive counter
			};
		}hPacketCheck;
	
		uint8_t		chPacketSendRetryCounter;		//Sequential counter to manage the retry count
		uint8_t		chTotalPacketCount;				//Holds the value of total no of packets to send in multi packet query
		uint8_t		chPacketCounter;				//Sequential counter to manage the multi packet  query
	}SENSOR_COMM_MANAGER;

	//Data structure to manage the power functionality
	typedef struct
	{
		uint8_t					chCheckFlag;					//Variable used as a flag to help designed algorithm during power ON condition
		uint8_t					chIdentity;						//Holds the source value which first requests to enable the 5V power
		int8_t					chPowerUsageCounter;			//Counter to manage the total no of requests for enabling and disabling the power
		Power_Source_Enum_t		chPowerLevelIndicator;			//Variable to hold the current power level state
	}PWR_5V_MANAGER;

#endif /* DATA_STRUCTURE_H_ */
