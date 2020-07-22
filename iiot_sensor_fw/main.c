/* -------------------------------------------------------------------------
Filename: main.c

Job#: 20473
Purpose: SENSOR Master controller application (Entry Point)
Date Created: 10/08/2014
Author: Aalok Shah


Authors: Aalok Shah

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

Requirements:
0006026_R8a - RF Module to SENSOR Protocol
0006095_R2	- Programming Guide, APS SENSOR MC
0006061R7	- Software Design Description, APS Master Controller
0005982_r2	- Schematic Diagram of SENSOR MC

Functionality:
(Note: Please refer all the documents specified in "Requirements" before preceding further)

First step is to initialize the SENSOR MC at power on. Which includes,
	Hardware_Initialization:
		1> Initialization of SPI interfaces for communicating with CC1125, CC2520 and Smart Sensors.
		2> Initialization of I2C intreface for E2PROM and various sensors including Gyroscope, Pressure Sensor, etc..
		3> Initialization of ADC interfaces for measure resolutions of various sensors.
		4> Initialization of Software watchdog to avoid hanging of firmware.
		5> Initialization of Power Saving Modes as SENSOR MC is designed to consume less power.
		6> Initialization of RTC as entire task manager of this firmware is dependant on RTC interruptions.
	Firmware_Initialization: 
		1> Reset all the shared resources of middle and lower level design as MCU interfaces are multiplexed among multiple devices.
		2> As SENSOR MC task manager excutes on the basis of tasking table scenarios initialize defalut tasking table at start up.

After successful initialization SENSOR MC firmware will start executing designed task manager by following all the scenarios described in documents.
Some of the major points are listed below which help to understand this firmware a better way:
	Task Manager:
		Task manager in this design consist of multiple independent task. Each of this task have multiple states which manages the execution of it.
		Entire task manager execution must follow timing constraints during execution. Please find below list of task currently available in task manager. 
			1> Data Sampling Task:
				It manages communication with the all the available sensors on SENSOR MC hardware. 
				For brief details and implementation of this task follow sample_collection.c.
			2> Data Collection Task:
				The main purpose of this task is to keep packets ready which needs to send over RF.
				For brief details and implementation of this task follow sample_collection.c.
			3> Data Download Task:
				It manages all the communication with the Remote side over RF by following predefined packet format.
				For brief details and implementation of this task follow sensor_protocol.c.
			5> Serial Debug Task:
				---- Not IMplemented ---
			6> Battery Management Task:
				---- Not IMplemented ---

	Execution Table:
		it can be referred as control settings. It manages entire data collection and download process on the basis of timing stamps.
		For brief details regarding available fields in tasking table follow data_structure.h.
	
	Timing Management:
		RTC is used to achieve all the timing constraints in SENSOR MC design. It is configured for periodic interruption with the sample clock value provided in execution table.
		On every interrupt it will notify one or another task to perform their execution so it can be considered as base for all the task.
		In firmware this unit is implemented under the name of System Timing task in system_timing.c.
	
	Power Saving System:
		During execution of task manager firmware is designed to spend as much time as possible in task manager. 
		Firmware is using all the XMEGA supported power saving modes in this design.
		Those modes are taken care with maximum accuracy to avoid mismatch in interrupt execution.  

Interrupt & Hardware Driven Tasks:
(Note: The listed entries indicate categories from which system might get interruption during execution)

RTC_Overlow_Interrupt							RTC is used for the generation of 8 phases of Sample Clock. So on every phase RTC will provide interruption.
I2C_Master_Communication_Interrupt				MCU behave as master while communication to sensors (slaves). So on every communication event over I2C bus will interrupt the firmware execution. 
GPIO_Falling_Edge_Interrupt						CC1125 chip is configured to provide interruption at transmit complete and receive avail event during communication.
Timer_Overflow_Interrupt						Various timers are used in SENSOR MC design for generation of specific timing duration.

Interrupt Handlers Used:
(Note: As firmware uses predefined notations of AVR studio to writing the interrupt user needs to find the interrupt routines with the listed names)

RTC_OVF_vect			system_timing.c		Interrupts periodically at sample_clock/8 value
PORTH_INT0_vect			mc_gpio.c			PORTH-PIN2 falling edge interrupt (Configured for CC1125-GPIO0: Transmit/Receive Complete Interrupt)
TWIF_TWIM_vect			mc_i2c.c			Manage I2C Master related communications in various software defined operation modes
TCE0_OVF_vect			mc_timer.c			ISR for TIMER-CE0 overflow (UART frame delay)
TCC1_OVF_vect			mc_timer.c			ISR for TIMER-CC1 overflow (Comm Wait Timer)
TCE1_OVF_vect			mc_timer.c			ISR for TIMER-CE1 overflow (Voltage Stabilization Timer)
TCF1_OVF_vect			mc_timer.c			ISR for TIMER-CF1 overflow (System Delay Timer)

----------------------------------------------------------------------------
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

//_____  I N C L U D E S ___________________________________________________________________

#include "system_globals.h"				//Contains global definitions required to handle the various system level task

//_____ F U N C T I O N S ____________________________________________________________________
//
// SENSOR_Master_Controller:	Main Entry
//
//  The main entry point for the SENSOR Master Controller Application

int main()
{
	//Initialization task to initialize SENSOR MC hardware and firmware for operations
	fnSystemInitTask();
	
	//Enable Watchdog timer with the time value of 8-seconds
	fnEnableWDT();
	
	//Main loop of full flange SENSOR MC firmware
	while(1)
	{
		gchNewInterrupt = SET_NEW_ISR_FLAG;							//Set new interrupt flag

		//Exit the execution of task list only when: Any enabled task is still active (not done) and There were no new interrupts on last pass through task list
		while (((gchTasks_Enable & gchTasks_Active) != ALL_TASK_DONE) && (gchNewInterrupt == SET_NEW_ISR_FLAG))
		{
			gchNewInterrupt = CLEAR_NEW_ISR_FLAG;						//Clear new interrupt flag

			// Data Sampling Task
			if (gchTasks_Active & DATA_SAMPLING_TASK)
			{
				//If task is executing and returns TRUE it means task is done with the operations otherwise it is blocking (waiting for something)
				if (RETURN_TRUE==fnData_Sampling_Task())
				{
					gchTasks_Active &= ~DATA_SAMPLING_TASK;					//Set Done bit
				}
			}

			// Data Collection Task
			if (gchTasks_Active & DATA_COLLECTION_TASK)
			{
				//If task is executing and returns TRUE it means task is done with the operations otherwise it is blocking (waiting for something)
				if (RETURN_TRUE==fnData_Collection_Task())
				{
					gchTasks_Active &= ~DATA_COLLECTION_TASK;			//Set Done bit
				}
			}
			
			// Data Download Task
			if (gchTasks_Active & DATA_DOWNLOAD_TASK)
			{
				//If task is executing and returns TRUE it means task is done with the operations otherwise it is blocking (waiting for something)
				if (RETURN_TRUE==fnData_Download_Task())
				{
					gchTasks_Active &= ~DATA_DOWNLOAD_TASK;			//Set Done bit
				}
			}
			
			// Execution Table Request Task
			if (gchTasks_Active & EXECUTION_TABLE_REQ_TASK)
			{
				//If task is executing and returns TRUE it means task is done with the operations otherwise it is blocking (waiting for something)
				if (RETURN_TRUE == fnData_ET_Request_Task())
				{
					gchTasks_Active &= ~EXECUTION_TABLE_REQ_TASK;			//Set Done bit
				}
			}

			// Debug Serial Communication Task
			if (gchTasks_Active & DEBUG_SERIAL_TASK)
			{
				//If task is executing and returns TRUE it means task is done with the operations otherwise it is blocking (waiting for something)
				if (RETURN_TRUE == fnDebug_Serial_Task())
				{
					gchTasks_Active &= ~DEBUG_SERIAL_TASK;			//Set Done bit
				}
			}

			// Watch Dog Timer Task
			if (gchTasks_Active & WATCHDOG_MANAGEMENT_TASK)
			{
				//If task is executing and returns TRUE it means task is done with the operations otherwise it is blocking (waiting for something)
				if (RETURN_TRUE==fnWatch_Dog_Manage_Task())
				{
					gchTasks_Active &= ~WATCHDOG_MANAGEMENT_TASK;			//Set Done bit
				}
			}
		}  //End loop processing of active and unblocked tasks

		//Disable interrupts to lock out New Interrupts
		DISABLE_GLOBAL_INTERRUPTS;
		
		//If all enabled tasks done & Master Controller in OFF Mode: Hibernate (Power Down Mode)
		if (((gchTasks_Enable & gchTasks_Active) ==  ALL_TASK_DONE) && (gchControllerOff == SET_FLAG))
		{
			fnDisableWDT();							//Watchdog will remain ON even in power down mode so disable it 
			ENABLE_GLOBAL_INTERRUPTS;
			ENABLE_POWER_DOWN_SLEEP_MODE;
			_SLEEP;
			DISABLE_POWER_DOWN_SLEEP_MODE;			//Will wake up on asynchronous interrupt
			fnEnableWDT();							//Turn on the Watchdog timer after coming out from the power down mode
		}

		//If all enabled tasks done & Master Controller not in OFF Mode: Sleep (Power Save Mode)
		else if ((gchTasks_Enable & gchTasks_Active) ==  ALL_TASK_DONE)
		{
			ENABLE_GLOBAL_INTERRUPTS;
			ENABLE_POWER_SAVING_SLEEP_MODE;
			_SLEEP;
			DISABLE_POWER_SAVING_SLEEP_MODE;		//Will wake up on RTC interrupt
		}

		//If no new interrupt just came in: Nap (Idle Power mode)
		else if (gchNewInterrupt == CLEAR_NEW_ISR_FLAG)
		{
			ENABLE_GLOBAL_INTERRUPTS;
			ENABLE_IDLE_POWER_SLEEP_MODE;
			_SLEEP;
			DISABLE_IDLE_POWER_SLEEP_MODE;			//Will wake up on any interrupt
		}
		
		//New interrupt crept in, repeat Task Loop
		else
		{
			ENABLE_GLOBAL_INTERRUPTS;
		}
		
		//Reset the watchdog timer (Inner while(1) must needs to break within 4-sec)
		RESET_WDT;
	}
}
