/* -------------------------------------------------------------------------
Filename: system_globals.h

Job#: 20473
Purpose: Holds the global definitions used in task manager of SENSOR MC firmware
Date Created: 11/13/2014

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

#ifndef SYSTEM_GLOBALS_H_
#define SYSTEM_GLOBALS_H_

	//_____  I N C L U D E S ______________________________________________________________

	#include "mc_system.h"				//Basic functionality for ATXMEGA MCU system
	#include "task_manager.h"			//Definitions of all the upper level functions
	#include "data_structure.h"			//All structure definitions used across SENSOR MC design

	//_____ M A C R O S ____________________________________________________________________

	//Sensor status related macro
	#define SAMPLE_STATE_OFF					(0x00)		//collection done or pending
	#define SAMPLE_STATE_START					(0x01)		//collection done or pending
	#define SAMPLE_STATE_RUN					(0x02)		//sampling done or pending
	#define SAMPLE_STATE_RUN_LVL0				(0x03)		//sampling done or pending
	#define SAMPLE_STATE_RUN_LVL1				(0x04)		//sampling done or pending

	//Packet Header IDs: Instrumented Top Drive Sub
	//Contents of Byte 0x00 of Packet Header for RFC Transmissions
	#define MASTER_CONTROLLER_STATUS_PACKET		0x80
	#define REQUEST_NEW_EXECUTION_TABLE			0x81
	#define ACK_RECEIPT_OF_LAST_ET_PACKET		0x82
	#define NACK_RECEIPT_OF_LAST_ET_PACKET		0X83
	#define SLOW_DL_DATA_PACKET					0X84
	#define FAST_DL_DATA_PACKET					0X85
	#define DATA_MESSAGE_TERMINATED				0X86
	#define FAST_LOOP_BACK_PACKET_FROM_SENSOR		0x87
	#define SLOW_LOOP_BACK_PACKET_FROM_SENSOR		0x88
	#define RFCM_LOOP_BACK_PACKET				0X89
	#define RFCM_STATUS_PACKET					0X8A

	//Packet Header IDs: Rig Floor Console
	#define SENSOR_HARD_RESET_COMMAND_1			0x01
	#define REQUEST_SENSOR_STATUS					0x02
	#define NO_NEW_ET_AVAILABLE					0x03
	#define NEW_EXECUTION_TABLE_PACKET			0x04
	#define EXECUTION_TABLE_UPLOAD_TERMINATED		0x05
	#define TERMINATE_DATA_DOWNLOAD				0x06
	#define ACK_RECEIPT_LAST_DATA_PACKET		0x07
	#define NACK_INVALID_PACKET					0x08
	#define NACK_INVALID_MSG_FORMAT				0x09
	#define NACK_OUT_OF_SEQUENCE				0x0A
	#define NACK_STOP_SENDING_DATA_MESSAGE		0x0B
	#define SENSOR_LOOP_BACK_PACKET_SLOW			0x7D
	#define SENSOR_LOOP_BACK_PACKET_FAST			0x7E
	#define STOP_LOOP_BACK_LOAD_ACTIVE			0x77		//???? ID needs to change
	#define STOP_LOOP_BACK_LOAD_IDLE			0x76		//???? ID needs to change

	//Task Manager
	#define ENABLE_ALL_TASKS					0xFF		//Bit mask to check enabled task
	#define DISABLE_ALL_TASKS					0x00		//Bit mask to disable all task
	#define ACTIVATE_ALL_TASKS					0xFF		//To activate all task.
	#define DEACTIVATE_ALL_TASKS				0x00		//To deactivate all tasks
	#define DATA_SAMPLING_TASK					(1<<0)		//bit location for data sampling task
	#define DATA_COLLECTION_TASK				(1<<1)		//bit location for data collection task
	#define DATA_DOWNLOAD_TASK					(1<<2)		//bit location for data download task
	#define EXECUTION_TABLE_REQ_TASK				(1<<3)		//bit location for TT request task
	#define	DEBUG_SERIAL_TASK				(1<<4)		//bit location for side wall serial task
	#define	WATCHDOG_MANAGEMENT_TASK			(1<<5)		//bit location for watch dog management task

	//SENSOR-RF Communication Packet Indexes
	#define CC1125_DATA_PACKET_LENGTH			0
	#define PACKET_HEADER_INDEX					1
	#define PACKET_DATA_LENGTH_INDEX			2
	#define PACKET_DESCRIPTOR_INDEX				3
	#define PACKET_ERROR_CONTROL_INDEX			4
	#define PACKET_DOWNLOAD_DATA_SIZE			5
	#define PACKET_DATA_MESSAGE_INDEX			6
	#define PACKET_FIRST_SENSOR_ET_INDEX		12
	#define PACKET_LAST_DATA_BYTE_INDEX			128			//Maximum no of data can be stored in one packet

	//5V Power Control
	#define  POWER_SOURCE_ENABLE				0
	#define  POWER_SOURCE_DISABLE				1
	#define  POWER_SOURCE_CHECK					0
	#define  POWER_SOURCE_EXECUTE				1

	#define  POWER_SOURCE_ALWAYS_ON				1
	#define  POWER_SOURCE_AT_REQUIRE			0

	#define  CC1125_POWER_IDENTITY				1
	#define  TEMPERATURE_POWER_IDENTITY			2
	#define  PRESSURE_POWER_IDENTITY			3
	#define  CC2520_POWER_IDENTITY				4
	#define  SYSTEM_POWER_IDENTITY				5
	
	#define ALL_TASK_DONE						0
	
	//_____ I N L I N E   M A C R O S ____________________________________________________________
	
	#define SEND_ERROR_CODE_OVER_RF(ErrorID)					ghSensorCommManager.hPacketCheck.chOverrunErrorID = ErrorID
	
	//_____ G L O B A L   D E F I N I T I O N S _________________________________________________

	//Memory resources to generate the packet for RF communication
	extern uint8_t gchSensorCommBuff[MAX_COMM_PACKET_COUNT][MAX_COMM_PACKET_SIZE];
	
	//Memory resources required to manage the all sensor related data collection operations
	extern volatile SMART_SENSORS_STRUCT ghSensorControl[MAX_SENSOR_COUNT];
	
	//Memory resources required to manage the successful SENSOR to RFC communication
	extern volatile SENSOR_COMM_MANAGER ghSensorCommManager;
	
	//Memory resources required to manage the tasking table related functionality
	extern volatile MASTER_CONTROLLER_STRUCT ghMasterTaskTable;
	
	//Memory resources required to manage the 5V power source
	extern volatile PWR_5V_MANAGER ghPowerManager;
	
	//Every bit maps to individual task indicating its enable or disable status
	extern volatile uint8_t gchTasks_Enable;
	
	//Every bit maps to individual task indicating its running or idle status
	extern volatile uint8_t gchTasks_Active;
	
	//Indicates task manager to move towards deep power down sleep mode
	extern volatile uint8_t gchControllerOff;
	
	//To notify data download task when data collection is complete 
	extern volatile uint8_t	gchDataDownloadPacketReady;
	
	//Total no of sensor entries in tasking table
	extern volatile uint8_t gchTotalSensorEntry;			//This will indicate total no of sensors required by current tasking table
	
	//Indicates latest occurrence of sample clock
	extern volatile uint8_t gchSampleClockIndicator;
	
	//Indicates latest occurrence of radio clock
	extern volatile uint8_t gchRadioClockIndicator;
	
	//Holds fresh and valid RSSI value
	extern volatile uint8_t gchUplinkRSSI;
	
	//Holds current software defined power source mode
	extern volatile uint8_t	gchPowerSourceMode;
	
	//Holds default value of communication wait time
	extern volatile uint16_t	gnDefaultCommWaitTimeValue;
	
#endif /* SYSTEM_GLOBALS_H_ */