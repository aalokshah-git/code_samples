/* -------------------------------------------------------------------------
Filename: sensor_protocol.h

Job#: 20473
Purpose: SENSOR Protocol supporting macros and enumeration
Date Created: 12/19/2014

(NOTE: latest version is the top version)

Author: Aalok Shah, 
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

#ifndef SENSOR_PROTOCOL_H_
#define SENSOR_PROTOCOL_H_

	//_____  I N C L U D E S ______________________________________________________________

	#include "system_globals.h"				//Contains global definitions required to handle the various system level task
	#include "radio_communication.h"		//Contains scenarios to manage the RF communication
	#include "sensor_management.h"			//Contains scenarios to manage the communication with various sensors available on SENSOR MC Hardware

	//________M A C R O S__________________________________________________________________

	#define MAX_SAMPLE_CLOCK_FREQ				125		//Maximum limit of sample clock freq
	#define MAX_COMM_WAIT_TIME_VALUE			65500
	#define COMM_WAIT_TIME_DOUBLE_RETRY			3
	#define ID_MASTER_CONTROLLER				0x00	//Master controller ID
	#define DOWNLINK_BM							0xC0	//Down link channel select bit mask
	#define MIN_SAMPLE_AVG_COUNT				1		//Minimum samples for the average

	//Indexes for Master Controller related fields in Tasking Table
	#define PACKET_LENGTH_INDEX					2		//Packet length index
	#define PACKET_MASTER_ID_INDEX				5		//Master controller TT id position
	#define PACKET_SAMPLE_CLOCK_FREQ_BYTE1		7		//Master controller sample clock frequency MSB position
	#define PACKET_SAMPLE_CLOCK_FREQ_BYTE2		8		//Master controller sample clock frequency LSB position
	#define PACKET_CONTROL_BYTE1				9
	#define PACKET_CONTROL_BYTE2				10
	#define PACKET_WAIT_TIME_BYTE1				11		//Response wait time MSB position
	#define PACKET_WAIT_TIME_BYTE2				12		//Response wait time LSB position

	//Indexes for Sensor related fields in Tasking Table
	#define SENSOR_ENTRY_OFFSET					6		//Gap between two sensor TT entry
	#define SENSORS_START_INDEX					13		//First Sensor entry in the Tasking Table
	#define SENSOR_TASKING_TABLE_ID_OFFSET		0		//Sensor ID position from the current value of index counter
	#define SENSOR_CONTROL_BYTE_OFFSET			2		//Sensor control byte position
	#define SAMPLE_CLOCK_DIVISOR_OFFSET			3		//Sensor sample clock divisor byte position
	#define SAMPLES_IN_AVERAGE_OFFSET			4		//Sensor sample in average position
	#define RADIO_CLOCK_DIVISOR_OFFSET			5		//Sensor's radio clock divisor

	//macros used in fnData_TT_Request_Task function
	#define TT_REQ_PACKET_LENGTH				4
	#define BASE_INDEX							0

	//________E N U M E R A T I O N S __________________________________________________________________

	//Identity of various steps required to execute for RF communication between SENSOR and RF Console
	typedef enum{
	
		COMM_ENTRY_POINT=0,							//RF communication starting point
		RADIO_PWR_CHECK_MODE,						//radio power check mode
		RADIO_LINK_SELECT_MODE,						//radio link select mode
		RADIO_TX_MODE,								//Radio transmit mode
		TX_TIME_OUT_MODE,							//transmission time out mode
		RADIO_RX_MODE,								//radio receive mode
		RADIO_TT_REPLY_MODE,						//send ACK/NACK mode
		RADIO_TT_REPLY_TIME_OUT_MODE,				//ACK/NACK send time out
		RADIO_SENSOR_LOOP_BACK_MODE,					//SENSOR loop back mode
		RADIO_SENSOR_LOOP_BACK_RESPONSE_MODE			//SENSOR loop back response wait mode
	}Radio_Handler_Step_Enum_t;

	//Steps for loop back mechanism
	typedef enum{
		LB_STEP_0=0,	//Loop back is off
		LB_STEP_1,		//Loop back is on
		LB_STEP_2,		
		LB_STEP_3
	}Loop_Back_Step_Enum_t;

#endif /* SENSOR_PROTOCOL_H_ */