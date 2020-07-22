/* -------------------------------------------------------------------------
Filename: task_manager.h

Job#: 20473
Purpose:
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

#ifndef TASK_MANAGER_H_
#define TASK_MANAGER_H_

	//_____ F U N C T I O N   D E F I N I T I O N S ___________________________________________________________

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
	
	int8_t fnData_Sampling_Task(void);

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
	
	int8_t fnData_Collection_Task(void);

	//_____ fnResetFirmwareResourceAllocations ____________________________________________________________________
	//
	// @brief	This function will get call when it is require to free all the shared resources which may occupied in middle and lower level during execution
	//			Probably this function will executed when new tasking table is available with SENSOR MC
	//			It vanish all the shared resources including,
	//				1> Lower level interfaces: ADC, SPI, I2C
	//				2> Power source management related definitions
	//				3> Middle layer definitions	used for communicating with radio and sensors
	//				4> Upper layer definitions used by system level task
	
	void fnResetFirmwareResourceAllocations(void);
	
	//_____ fnDebug_Serial_Task ____________________________________________________________________
	//
	int8_t fnDebug_Serial_Task(void);

	//_____ fnWatch_Dog_Manage_Task ____________________________________________________________________
	//
	int8_t fnWatch_Dog_Manage_Task(void);

	//____fnConfigureSampleClock _________________________________________________________________
	//
	// @brief	Function will configure the RTC for provided sample frequency. It has to follow certain steps to achieve the same:
	//			1> Release occupancy of all the resources which are indirectly dependant on sample clock
	//			2> As SENSOR MC task manager is dependant on sample clock so reset the dependant task manager resources
	//			3> Compute the time value for which to configure the RTC by dividing sample clock with the 8 phases
	//			4> Initialize the RTC and enable the sample clock scenarios
	// @param	chClockFreq	Sample clock value on which RTC needs to configure

	void fnConfigureSampleClock(uint8_t chClockFreq);

	//____fnStopSampleClock _________________________________________________________________
	//
	// @brief	It will stop the RTC so indirectly it will stop the sample clock generation

	void fnStopSampleClock(void);

	//____fnSystemInitTask _________________________________________________________________
	//
	// @brief	This is the main initialization task. It is responsible for initializing the SENSOR MMC firmware and hardware for operations.
	//			It initializes the hardware by calling fnHardwareInit() function.
	//			It initializes the firmware by calling fnSoftwareInit() and fnDefaultExecutionTableInit() functions.
	
	void fnSystemInitTask(void);

	//____ fnPowerSourceManager _________________________________________________________________
	//
	// @brief	This function will manage the 5V power source requirement.
	//			It manages the power resources on the basis of provided operation request (chOperation) in argument.
	//			If 5V power is already ON and same request for the same will occur again than it will just increment the chPowerUsageCounter to track the no of resources using 5V power.
	//			If 5V power is not ON and request for turning on the power arise than it will turn on the power and start the timer to maintain the delay till the power will get stabilize.
	//			If 5V power is OFF and request for the same than it will do nothing as power is already OFF.
	//			If 5V power is not OFF and request for turning off the power arise than it will decrement the chPowerUsageCounter to track the no of resources using 5V power and if this counter reaches to 0 than turn the power OFF.
	// @param	chOperation			Request for turn ON or OFF the 5V power
	//			chSourceIdentity	The source requests the power ON request (Avoid mismatch while power is in stabilization)
	// @return	TRUE if request executed successfully otherwise returns FALSE
	
	int8_t fnPowerSourceManager(uint8_t chOperation,uint8_t chSourceIdentity);
	
	//____fnDefaultExecutionTableInit _________________________________________________________________
	//
	// @brief	This function can be considered as a part of Firmware Initialization function
	//			It initializes the default Execution Table with which SENSOR MC has to operate
	//			Following settings are considered for default tasking table:
	//				Sample Clock: 1HZ
	//				Radio Clock Divisor: 5
	//				Comm Wait tTime: 500mz
	//				Active Tasks: TT Request Task
	//				No Sensor Execution Table is available at startup
	
	void fnDefaultExecutionTableInit(void);
	
	//____fnData_Download_Task _________________________________________________________________
	//
	// @brief	Function will get call from the task manager itself when it wants to send data packets over RF on the basis of various Radio Clocks.
	//			Most of the contents of the packet which needs to send will get created in the data collection task.
	//			The only operation this task has to perform is to send those packets reliably over RF with the help of CC1125.
	// @return	TRUE if all the steps required to perform Data Download operations in fnRadioTxRxHandler completed successfully

	int8_t fnData_Download_Task(void);

	//____fnData_ET_Request_Task _________________________________________________________________
	//
	// @brief	Function is called from the task manager itself when it wants to request for the new tasking table to RFC Console.
	//			The packet for the same is created first and than fnRadioTxRxHandler handles the rest of the communication steps.
	// @return	TRUE if all the steps required to perform TT Request operations in fnRadioTxRxHandler completed successfully

	int8_t fnData_ET_Request_Task(void);
		
#endif /* TASK_MANAGER_H_ */