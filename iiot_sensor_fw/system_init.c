/* -------------------------------------------------------------------------
Filename: system_init.c

Job#: 20473
Date Created: 10/08/2014

Purpose: Initialization of SENSOR MC firmware as well as hardware

Functions:
fnSystemInitTask				Responsible for the initialization of hardware as well as firmware
fnDefaultTaskingTableInit		Initialize default taking table
fnResetSource					Detect Reset source at power on
fnHardwareInit					Initialize SENSOR MC hardware at start up
fnSoftwareInit					Initialize SENSOR MC firmware at start up
fnPowerSourceManager			Manage resources required to maintain the 5V power 

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

//_____  I N C L U D E S _____________________________________________________________________________________________

#include "system_globals.h"						//Contains definitions related to various system level task
#include "hardware_abstraction_layer.h"			//Contains headers of hardware dependent programming functionality

//_____ G L O B A L   D E F I N I T I O N S ______________________________________________________________

//Flag to indicate the occurrence of new interrupt (It is used to avoid missing interrupt while putting controller to sleep mode)
volatile uint8_t	gchNewInterrupt;

//To indicate the task manager to go in to deep sleep mode (Will get set when receiving "OFF" from Sidewall Serial Interface)						
volatile uint8_t	gchControllerOff;

//Every bit of this variable mapped with a task which maintain its enable/disable state
volatile uint8_t	gchTasks_Enable;	

//Every bit of this variable mapped with a task which maintain its hold/active state				
volatile uint8_t	gchTasks_Active;						

//It points to the current power state on SENSOR MC (For 5V-Radio Power) 			
volatile uint8_t	gchPowerSourceMode;		

//Data structure which manages all the memory resources required to perform operations on the basis of tasking table
volatile MASTER_CONTROLLER_STRUCT	ghMasterTaskTable;		

//Group of flags required to manage the 5V power resources on SENSOR MC board
volatile PWR_5V_MANAGER ghPowerManager;

//____fnResetSource _________________________________________________________________
//
// @brief	This function is used to detect the cause of reset at every power cycle. 
//			It is implemented just to provide extensive debugging at system start up.

inline void fnResetSource(void)
{
	//To detect the source of reset
	if(RST.STATUS & RESET_BY_WDT)
	{
		SEND_DEBUG_STRING("Reset Source: Watchdog\n");
		RST.STATUS |= RESET_BY_WDT;		
	}
	else if (RST.STATUS & RESET_BY_SOFTWARE)
	{
		SEND_DEBUG_STRING("Reset Source: Software\n");
		RST.STATUS |= RESET_BY_SOFTWARE;	
	}
	else if(RST.STATUS & RESET_BY_PWRON)
	{
		SEND_DEBUG_STRING("Reset Source: PowerOn\n");
		RST.STATUS |= RESET_BY_PWRON;		
	}
	else if(RST.STATUS & RESET_BY_EXT)
	{
		SEND_DEBUG_STRING("Reset Source: External\n");
		RST.STATUS |= RESET_BY_EXT;			
	}
	
	return;
}

//____ fnHardwareInit  _________________________________________________________________
//
// @brief	This function performs all the necessary steps to initialize the XMEGA MCU for full functional SENSOR MC hardware
//			It initializes all the occupied MCU interfaces including Clock, GPIO, Timer, UART, SPI, I2C and RTC.

inline void fnHardwareInit(void)
{
	//Disable/Enable necessary interrupt levels
	DISABLE_MIDDLE_LAYER_INTERRUPT;
	DISABLE_UPPER_LAYER_INTERRUPT;
	ENABLE_LOWER_LAYER_INTERRUPT;
	
	//Enable global interrupts
	ENABLE_GLOBAL_INTERRUPTS;
	
	//Initialize clock - 16MHZ (internal)
	fnInitializeClock();
	
	//Initialize GPIO
	fnInitializeGPIO();
	
	MC_GP_LED1_ON;
	
	//Initialize UART
	fnUartInitialization();
	SEND_DEBUG_STRING("UART Modules Initialized\n");
	SEND_DEBUG_STRING("GPIO Modules Initialized\n");

	//Initialization of all the required timers
	fnTimersInit();
	SEND_DEBUG_STRING("TIMER Modules Initialized\n");
	
	//Initialize SPI Module for CC112X/CC252x Communication
	fnSpiInitialization();
	SEND_DEBUG_STRING("SPI Modules Initialized\n");
	
	//Initialize ADC Modules
	fnInitializeADC();
	SEND_DEBUG_STRING("ADC Modules Initialized\n");
	
	//Initialize I2C
	fnI2cInitialization();
	SEND_DEBUG_STRING("I2C Modules Initialized\n");
	
	MC_GP_LED0_ON;
	
	fnWait_uSecond(1000000);	//1Sec Delay
	
	MC_HEARTBEAT_LED_OFF;
	MC_GP_LED0_OFF;
	MC_GP_LED1_OFF;
	
	return;
}

//____ fnSoftwareInit _________________________________________________________________
//
// @brief	This function performs all the necessary steps to initialize the global definitions required to make Task Manager working
//			It will reset all the shared resources used in lower and middle layer of the firmware

inline void fnSoftwareInit(void)
{
	fnResetFirmwareResourceAllocations();
	
	//Global definition initializations
	gchTasks_Enable=DISABLE_ALL_TASKS;
	gchTasks_Active=DEACTIVATE_ALL_TASKS;		//0: Task Done | 1: Task Active
	
	gchControllerOff=RESET_FLAG;
	gchPowerSourceMode=POWER_SOURCE_ALWAYS_ON;
	
	return;
}

//____fnSystemInitTask _________________________________________________________________
//
// @brief	This is the main initialization task. It is responsible for initializing the SENSOR MMC firmware and hardware for operations.
//			It initializes the hardware by calling fnHardwareInit() function.
//			It initializes the firmware by calling fnSoftwareInit() and fnDefaultTaskingTableInit() functions.

void fnSystemInitTask(void)
{
	//Hardware Initialization function. It will also initializes the ISRs used in the system.
	fnHardwareInit();
	
	//To detect reset source while debugging.
	fnResetSource();
	
	//Software Initialization function.
	fnSoftwareInit();
	
	//It will initialize the default tasking table at application start up
	fnDefaultTaskingTableInit();
	
	SEND_DEBUG_STRING("Initialization Successful - Starting Task Manager...\n\n");
	fnWait_uSecond(100000);	//100ms occasional delay
	return;
}

//____fnDefaultTaskingTableInit _________________________________________________________________
//
// @brief	This function can be considered as a part of Firmware Initialization function
//			It initializes the default Tasking Table with which SENSOR MC has to operate
//			Following settings are considered for default tasking table:
//				Sample Clock: 1HZ
//				Radio Clock Divisor: 5
//				Comm Wait tTime: 500mz
//				Active Tasks: TT Request Task
//				No Sensor Tasking Table is available at startup			

void fnDefaultTaskingTableInit(void)
{
	gchTasks_Enable |=  TASKING_TABLE_REQ_TASK;
	gchTasks_Enable &=  ~(DATA_SAMPLING_TASK | DATA_COLLECTION_TASK | DATA_DOWNLOAD_TASK);
	gchTasks_Active =	DISABLE_ALL_TASKS;
	
	fnStopSampleClock();
		
	//Holds default value of communication wait time in case of no response
	gnDefaultCommWaitTimeValue=DEFAULT_TT_COMM_WAIT_TIME;				
	ghMasterTaskTable.nCommTimeout=DEFAULT_TT_COMM_WAIT_TIME;
	
	ghMasterTaskTable.nRadioClockDivisor=DEFAULT_TT_RADIO_CLOCK;
	ghMasterTaskTable.chDataDownloadChannel=RADIO_CH_SLOW_DOWNLINK_CC1125;
	fnConfigureSampleClock(DEFAULT_TT_SAMPLE_CLOCK);
	return;
}

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

int8_t fnPowerSourceManager(uint8_t chOperation,uint8_t chSourceIdentity)
{
	//If the sample clock frequency is greater then 10Hz then power source remains on.
	if((gchPowerSourceMode==POWER_SOURCE_ALWAYS_ON) && (ghPowerManager.chPowerLevelIndicator==POWER_STATE_5V_ON))
	{
		return RETURN_TRUE;
	}
	
	if(chOperation==POWER_SOURCE_DISABLE)
	{
		//if 5v power source is not used by any sensor or radio then this logic will disable power source. gchPowerUsageCounter variable manages number of sensor currently using 5v power source.
		if(--ghPowerManager.chPowerUsageCounter <= RESET_COUNTER)
		{
			ACTIVATE_RADIO_RESET;
			fnWait_uSecond(10);
			DISABLE_5VOLT_POWER;
			ghPowerManager.chPowerLevelIndicator=POWER_STATE_OFF;
			ghPowerManager.chPowerUsageCounter=RESET_COUNTER;							//For Safety
			SEND_DEBUG_STRING("5V Source Disable\n");
		}
	}
	else
	{
		if(ghPowerManager.chPowerLevelIndicator==POWER_STATE_5V_ON)
		{
			ghPowerManager.chPowerUsageCounter++;
		}
		else
		{
			if(!ghPowerManager.chCheckFlag)						//This flag prevent entry for other sensor when one sensor is pending to serve the requirement of power source.
			{
				ghPowerManager.chPowerUsageCounter++;			//This will maintain no of sensor has requested for the power source.
				ghPowerManager.chIdentity=chSourceIdentity;		//This will store which sensor has requested for the power source.
				
				ENABLE_5VOLT_POWER;								//Enable power source.
				fnStartVolStableTimer(WAIT_5_VOL_TIMER);		//Start 80ms timer used to give time to stable 5v regulator
				ghPowerManager.chCheckFlag= SET_FLAG;
				return RETURN_FALSE;
			}
			else
			{
				//If the source identity is mismatched means another sensor has called this function before previous request is served successfully then it will return false.
				if(ghPowerManager.chIdentity!=chSourceIdentity)
				{
					return RETURN_FALSE;
				}
				
				//This flag is set by ISR of voltage stable timer.
				if(gchVoltageStableTimerFlag)
				{
					ghPowerManager.chPowerLevelIndicator=POWER_STATE_5V_ON;			//Update the flag with new power state.
					ghPowerManager.chCheckFlag= RESET_FLAG;							//Reset the flag
					DEACTIVATE_RADIO_RESET;
					SEND_DEBUG_STRING("5V Source Enable\n");
				}
				else
				{
					return RETURN_FALSE;
				}
			}
		}
	}
	
	return RETURN_TRUE;
}