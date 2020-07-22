/* -------------------------------------------------------------------------
Filename: system_timing.c

Job#: 20473
Date Created: 11/12/2014

Purpose: Holds scenarios to manage the timing constraints in the SENSOR MC firmware design.

Functions:
fnConfigureSampleClock		Start sample clock functionality
fnStopSampleClock			Stop sample clock functionality
fnSystem_Timing_Task		System timing task to manage the SENSOR MC timing constraints

Interrupts:
RTC_OVF_vect				Interrupts periodically at sample_clock/8 value and enables the system timing task


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

#include "system_globals.h"						//Contains definitions related to various system level task
#include "hardware_abstraction_layer.h"			//Contains headers of hardware dependent programming functionality

//_____ M A C R O S ____________________________________________________________________

//RTC Management
#define DISABLE_RTC_MODULE					RTC.CTRL = RTC_PRESCALER_OFF_gc
#define ENABLE_RTC_MODULE					RTC.CTRL = RTC_PRESCALER_DIV1_gc					//Configured for 250us RTC_Clock
#define TOTAL_SAMPLE_CLOCK_PHASE			8

//RTC Interrupt related
#define ENABLE_RTC_INTERRUPT				RTC.INTCTRL = RTC_OVFINTLVL0_bm
#define DISABLE_RTC_INTERRUPT				RTC.INTCTRL = RTC_OVFINTLVL_OFF_gc

//_____ E N U M E R A T I O N S _________________________________________________

///List of Sample Clock Phases
typedef enum
{
	RTC_PHASE0=0,
	RTC_PHASE1,
	RTC_PHASE2,
	RTC_PHASE3,
	RTC_PHASE4,
	RTC_PHASE5,
	RTC_PHASE6,
	RTC_PHASE7
}RTC_PHASES;

//_____ G L O B A L   D E F I N I T I O N S ______________________________________________________________

//It is basically an increment counter which counts to 7 and than start again from 0 to manage the 8 phases of sample clock
volatile uint8_t	gchSampleClockPhase;

//List of RTC Phases
RTC_PHASES			ghRtcphases;

//To generate sample clock on test points
uint8_t				gchClockPhase;

//____fnConfigureSampleClock _________________________________________________________________
//
// @brief	Function will configure the RTC for provided sample frequency. It has to follow certain steps to achieve the same:
//			1> Release occupancy of all the resources which are indirectly dependant on sample clock
//			2> As SENSOR MC task manager is dependant on sample clock so reset the dependant task manager resources
//			3> Compute the time value for which to configure the RTC by dividing sample clock with the 8 phases
//			4> Initialize the RTC and enable the sample clock scenarios
// @param	chClockFreq	Sample clock value on which RTC needs to configure

void fnConfigureSampleClock(uint8_t chClockFreq)
{
	DISABLE_RTC_INTERRUPT;
	
	//Point to the first RTC Phase
	gchSampleClockPhase=RTC_PHASE0;
	gchSampleClockIndicator=RESET_FLAG;
	gchRadioClockIndicator=RESET_FLAG;
	
	//Time divided into 8 phase
	chClockFreq=(1000/(chClockFreq * TOTAL_SAMPLE_CLOCK_PHASE));	//1000 is used to convert second into millisecond
	
	//Before setting the period register wait for RTC to sync
	while(RTC.STATUS & BIT_0_bm);
	
	//Configure the RTC with the value of single phase
	RTC.PER= chClockFreq;
	RTC.CNT=RESET_COUNTER;
	
	//Enable RTC interruption
	ENABLE_RTC_INTERRUPT;
	
	//Start RTC Module
	ENABLE_RTC_MODULE;
	return;
}

//____fnStopSampleClock _________________________________________________________________
//
// @brief	It will stop the RTC so indirectly it will stop the sample clock generation

void fnStopSampleClock(void)
{
	DISABLE_RTC_MODULE;
	return;
}

//____fnSystem_Timing_Task _________________________________________________________________________________________________________________________________________________________________________________
//
// @brief	Function will get call at every RTC interruption.
//			As SENSOR MC design is based on 8 phases of sample clock this function will perform all necessary steps to raise the necessary flags for various tasks of SENSOR MC.
//			It also manages the sampling and radio clock related timing constraints of SENSOR MC design.
//			All operations performed by this function can be distributed in various phases maintained with the help of gchSampleClockPhase as listed below:

//			RTC_PHASE0:	Data sampling task
//				During phase-0 this function will check whether the sampling task is enabled. If sampling task is enabled than function will put Data Sampling task in undone state.
//				This will also start the smart sensor sample clock.
//
//			RTC_PHASE1:	TT request task
//				During phase-1 this function will check whether the new TT request task is enabled. If the TT request task is enabled than this function will put TT request task in the undone state.
//
//			RTC_PHASE6:	Data collection task
//				During phase-6 this function will check whether data collection task is enabled. If the task is enabled and radio clock divisor value has been reach than this function will put Data Collection task in undone state.
//
//			RTC_PHASE7:	Data download task
//				During phase-7 this function will check whether the data download task is enabled. If the task is enabled and data collection task is enabled than this function will put Data Download task in undone state.

inline void fnSystem_Timing_Task(void)
{
	if(gchClockPhase)
	{
		gchClockPhase=0;
		SET_PINS_HIGH(PORTB,PB_TEST_POINT);
	}
	else
	{
		gchClockPhase=1;
		SET_PINS_LOW(PORTB,PB_TEST_POINT);
	}
	
	switch(gchSampleClockPhase)
	{
		//Phase-0: Data Sampling Task
		case RTC_PHASE0:
		
			if(gchTasks_Enable & DATA_SAMPLING_TASK)
			{
				SET_PINS_LOW(PORTA,PA_TEST_POINT);
				ENABLE_SMART_SENSOR_SAMPLE_CLOCK;
				gchSampleClockIndicator=SET_FLAG;				//Used in data sampling task to indicate the new interruption of RTC
			
				//Enable sampling task for execution
				gchTasks_Active |= DATA_SAMPLING_TASK;			//Activate the task
			}
		break;
		
		//Phase-1: Tasking Table Request Task
		case RTC_PHASE1:
			DISABLE_SAMPLING_CLOCK;
			MC_HEARTBEAT_LED_ON;
					
			if(gchTasks_Enable & TASKING_TABLE_REQ_TASK)
			{
				if(++ghMasterTaskTable.nRadioClockCounter >= ghMasterTaskTable.nRadioClockDivisor)		//Value must be >1 for nRadioClockDivisor
				{
					ghMasterTaskTable.nRadioClockCounter=RESET_COUNTER;			//Counter value zero when data collection task will get active so use it in checking of data download task
				
					//Enable tasking table request task for execution
					gchTasks_Active |= TASKING_TABLE_REQ_TASK;					//Activate the task
				}
			}
		break;
		
		//Phase-6: Data Collection Task
		case RTC_PHASE6:
			MC_HEARTBEAT_LED_OFF;
					
			if(gchTasks_Enable & DATA_COLLECTION_TASK)
			{
				if(++ghMasterTaskTable.nRadioClockCounter >= ghMasterTaskTable.nRadioClockDivisor)	//Value must be >1 for nRadioClockDivisor
				{
					ghMasterTaskTable.nRadioClockCounter=RESET_COUNTER;			//Counter value zero when data collection task will get active so use it in checking of data download task
					gchRadioClockIndicator=SET_FLAG;							//Used in data collection task to indicate the new interruption of RTC
				
					//Enable data collection task for execution
					gchTasks_Active |= DATA_COLLECTION_TASK;					//Activate the task
				}
			}
		break;
		
		//Phase-7: Data Download Task
		case RTC_PHASE7:
			if((ghMasterTaskTable.nRadioClockCounter==RESET_COUNTER) && (gchTasks_Enable & DATA_DOWNLOAD_TASK))
			{
				//Enable data download task for execution
				gchTasks_Active |= DATA_DOWNLOAD_TASK;						//Activate the task
				SET_PINS_HIGH(PORTA,PA_TEST_POINT);
			}
		break;
		
		default:
			//Nothing to perform in rest of the phases
		break;
	}
	
	//This will increment the phase counter and reset the counter value when it exceeds the maximum phase value.
	if(++gchSampleClockPhase > RTC_PHASE7)									//Increment the phase counter
	{
		gchSampleClockPhase=RTC_PHASE0;
	}

	return;
}

//_____ I S R - R T C   O V E R F L O W ____________________________________________________________________
//
// @brief	ISR for RTC Overflow:
//			RTC is configured with the phase=sample clock/8 value.
//			SENSOR MC timing constraints are managed by this interrupt only because it enables the system timing task on which entire task manager is dependent.
//			In background it also wake ups the controller from power saving  sleep mode.

ISR(RTC_OVF_vect)
{
	gchNewInterrupt = SET_NEW_ISR_FLAG;
	fnSystem_Timing_Task();
}