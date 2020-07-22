/* -------------------------------------------------------------------------
Filename: mc_timer.c

Date Created: 10/08/2014

Purpose: TIMER related functionality including initialization and configurations.

Functions:
fnInitializeUartDelayTimer				Initialization function for configuring the UART frame delay timer
fnInitializeCommunicationTimer			Initialization of communication timer
fnInitializeVolStableTimer				Initialization of voltage stability timer
fnInitializeWaitTimer					Initialization of execution wait timer
fnUartDelayTimerEnable					Enable UART frame delay timer
fnUartDelayTimerDisable					Disable UART frame delay timer
fnRadioCommunicationTimerEnable			It will start communication timer
fnRadioCommunicationTimerDisable		It will stop communication timer
fnStartCommunicationTimer				Function to start communication timer
fnStartVolStableTimer					Function to start voltage stability timer

Interrupts:
TCE0_OVF_vect							ISR for TIMER-CE0 overflow (UART frame delay)
TCC1_OVF_vect							ISR for TIMER-CC1 overflow (Comm Wait Timer)
TCE1_OVF_vect							ISR for TIMER-CE1 overflow (Voltage Stabilization Timer)
TCF1_OVF_vect							ISR for TIMER-CF1 overflow (System Delay Timer)


Author: Aalok Shah, 

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

#include "mc_timer.h"			// TIMER functionality for ATXMEGA MCU system

//_____ G L O B A L   D E F I N I T I O N S _________________________________________________

//Counter fields related to UART Frame delay Timer
static volatile uint8_t		gchMaxDelayValue;
static volatile uint8_t		gchCurrentDelayValue;

//Flag to indicate successful reception
volatile uint8_t			gchRxAvail;

//Counter fields related to Communication Timer
static volatile uint8_t		gchCommunicationCurrentDelayValue;
static volatile uint8_t		gchCommunicationMaxDelayValue;

//Flag to indicate communication timeout condition
volatile uint8_t			gchCommunicationTimeOut;

//Flag to indicate completion of time duration for Voltage stabilization
volatile uint8_t			gchVoltageStableTimerFlag;		

//Counter fields related to system execution wait time generation 
static volatile uint8_t		gchCurrentCounterDelayValue;
static volatile uint8_t		gchMaxCounterDelayValue;

//Flag to indicate completion of wait time
volatile uint8_t			gchCounterDelayTimeOut;

//_____ fnInitializeUartDelayTimer ____________________________________________________________________
//
// @brief	UART is designed to terminate reception on specific inter-char frame delay (40ms) to support unsolicited message handling.
//			Timer is initialized with 5ms in following manner to help UART reception:
//				1> Assume that UART is configured for the baudrate of 9600 with 1-Start and 8-Char bits.
//					So typical time frame for one byte to transmit is ((1/9600)*10).
//					Application terminates the reception when inter char delay exceeds the value 10*typical time frame (approx.)
//				2> If total frame delay wait value is 40ms than design the timer with appropriate scalar and period register values
//				3> Enable the timer related interrupts

inline void fnInitializeUartDelayTimer(void)
{
	//Normal mode of timer operation
	TCE0.CTRLB = RESET_VALUE;
	TCE0.CTRLE = RESET_VALUE;
	TCE0.CTRLC = RESET_VALUE;
	TCE0.CTRLD = RESET_VALUE;
	
	//Calculation:
	//Clock=16MHZ, Prescalar=256, Desired Time Delay Value=5ms
	//Desired Time Delay Value=(1/(Clock/Prescalar)) * Period_Reg
	
	//Fill_up the period register
	TCE0.CNT = RESET_COUNTER;
	TCE0.PER = 312;				 
	
	// Disable ABCD capture/compare interrupts
	TCE0.INTCTRLB = RESET_VALUE;

	//Reset interrupt status register
	TCE0.INTFLAGS = RESET_VALUE;
	
	//Enable timer interrupt
	ENABLE_UART_DELAY_TIMER_INTERUPT;
	
	return;
}

//_____fnInitializeCommunicationTimer_______________________________________________________________
//
// @brief	This function initialize communication timer TCC1 which is used to wait for the reception for 150 to 65535ms after sending packet to RFC.

inline void fnInitializeCommunicationTimer(void)
{
	//Normal mode of timer operation
	TCC1.CTRLB = RESET_VALUE;
	TCC1.CTRLC = RESET_VALUE;
	TCC1.CTRLD = RESET_VALUE;
	TCC1.CTRLE = RESET_VALUE;
	
	// Disable ABCD capture/compare interrupts
	TCC1.INTCTRLB = RESET_VALUE;
	
	//Reset interrupt status register
	TCC1.INTFLAGS = RESET_VALUE;
	
	//Enable timer interrupt
	ENABLE_COMMUNICATION_TIMER_INTERUPT;
	
	return;
}

//_____fnInitializeVolStableTimer_______________________________________________________________
//
// @brief	This function initialize voltage regulator stability timer TCE1 which is used to generate the duration required for of 5.5V and 3.3V regulator to get stabilized.

inline void fnInitializeVolStableTimer(void)
{
	//Normal mode of timer operation
	TCE1.CTRLB = RESET_VALUE;
	TCE1.CTRLC = RESET_VALUE;
	TCE1.CTRLD = RESET_VALUE;
	TCE1.CTRLE = RESET_VALUE;
	
	// Disable ABCD capture/compare interrupts
	TCE1.INTCTRLB = RESET_VALUE;
	
	//Reset interrupt status register
	TCE1.INTFLAGS = RESET_VALUE;
	
	//Enable timer interrupt
	ENABLE_VOL_STABLE_TIMER_INTERUPT;
	
	return;
}

//_________ fnInitializeWaitTimer ___________________________________________________________
//
// @brief	Timer is used to provide delay during program execution

inline void fnInitializeWaitTimer(void)
{
	//Normal mode of timer operation
	TCF1.CTRLB = RESET_VALUE;
	TCF1.CTRLE = RESET_VALUE;
	TCF1.CTRLC = RESET_VALUE;
	TCF1.CTRLD = RESET_VALUE;
	
	// Disable ABCD capture/compare interrupts
	TCF1.INTCTRLB = RESET_VALUE;

	//Reset interrupt status register
	TCF1.INTFLAGS = RESET_VALUE;
	
	//Enable timer interrupt
	ENABLE_WAIT_TIMER_INTERUPT;
	
	return;
}

//_________ fnTimersInit ___________________________________________________________
//
//	@brief	This function is called from fnHardwareInit function
//			It manages to call all the timer functions for their initialization

void fnTimersInit(void)
{
	//Initialize UART frame delay management timer
	fnInitializeUartDelayTimer();
	
	//Initialize communication timer
	fnInitializeCommunicationTimer();
	
	//Initialize voltage stability timer
	fnInitializeVolStableTimer();
	
	//Initialize wait timer
	fnInitializeWaitTimer();
	
	return;
}

//_____ fnUartDelayTimerEnable ____________________________________________________________________
//
// @brief	Enable the UART Frame Delay Timer

void fnUartDelayTimerEnable(void)
{
	ENABLE_UART_DELAY_TIMER;
	return;
}

//_____ fnUartDelayTimerDisable ____________________________________________________________________
//
// @brief	Disable the UART Frame Delay Timer

void fnUartDelayTimerDisable(void)
{
	DISABLE_UART_DELAY_TIMER;
	return;
}

//_____ fnRadioCommunicationTimerEnable ____________________________________________________________________
//
// @brief	Enable the Radio Comm. Delay Timer

void fnRadioCommunicationTimerEnable(void)
{
	ENABLE_COMMUNICATION_TIMER;
	return;
}

//_____ fnRadioCommunicationTimerDisable ____________________________________________________________________
//
// @brief	Disable the Radio Comm. Delay Timer

void fnRadioCommunicationTimerDisable(void)
{
	DISABLE_COMMUNICATION_TIMER;
	return;
}

//_____ fnDisableSystemDelayTimer ____________________________________________________________________
//
// @brief	Disable system delay timer

void fnDisableSystemDelayTimer(void)
{
	DISABLE_WAIT_TIMER;
	return;
}

//_____ fnEnableSystemDelayTimer ____________________________________________________________________
//
// @brief	Enable system delay timer

void fnEnableSystemDelayTimer(void)
{
	ENABLE_WAIT_TIMER;
	return;
}

//_____fnStartSystemDelayTimer_______________________________________________________________
//
// @brief	Will start the system delay timer for specified time delay value generation
//			It supports minimum of 0.5uSec delay generation
// @param	fDelayCount		Value for which to initialize the Timer

void fnStartSystemDelayTimer(float fDelayCount)
{
	uint16_t nPeriod = RESET_COUNTER;
	
	fDelayCount /= 1000000;						//Convert uSec to Sec
	gchMaxCounterDelayValue = SET_COUNTER;		//Maximum Value
	
	//Calculation:
	//Prescalar=8, Clock=16MHZ, Max CNT Value=65535
	//Min Delay Value=(1/(Clock/Prescalar))*65535
	while(fDelayCount > 0.0327675f)				// Check for Max value can be generated using internal period counter
	{
		//Fetch efficient Internal and External period values
		fDelayCount /= 2;					
		gchMaxCounterDelayValue *= 2;
	}
	
	//2000000 count in 1 second hence how much count in fDelayCount second-??
	nPeriod = fDelayCount*2000000;
	
	//Fill_up the period register
	TCF1.CNT = RESET_COUNTER;
	TCF1.PER = nPeriod;
	
	gchCurrentCounterDelayValue = RESET_VALUE;
	gchCounterDelayTimeOut = RESET_VALUE;
	
	ENABLE_WAIT_TIMER;
	
	return;
}

//_________fnStartCommunicationTimer________________________________________________
//
// @brief	It will start the Radio Communication Timer with the desired value. Timer is designed to support wait time between 150 to 65535 ms
//			It indicates the time for which SENSOR MC should get reception after successful transmission
// @param	fDelayCount		Value for which to initialize the Timer

void fnStartCommunicationTimer(float fDelayCount)
{
	uint16_t nPeriod = RESET_COUNTER;
	
	fDelayCount /=1000;			//Convert mSec to Sec
	gchCommunicationMaxDelayValue = SET_COUNTER;
	
	//Calculation:
	//Prescalar=1024, Clock=16MHZ, Max CNT Value=65535
	//Min Delay Value=(1/(Clock/Prescalar))*65535
	while(fDelayCount >4.19424f)				// Check for Max value can be generated using internal period counter
	{
		//Fetch efficient Internal and External period values
		fDelayCount /=2;
		gchCommunicationMaxDelayValue *= 2;
	}
	
	//15625 count in 1 second hence how much count in lDelayCount second-??
	nPeriod =fDelayCount * 15625;
	
	//Fill_up the period register
	TCC1.CNT = RESET_COUNTER;
	TCC1.PER = nPeriod;
	
	//Initialize current delay value
	gchCommunicationCurrentDelayValue = SET_COUNTER;
	
	//Reset communication time out flag
	gchCommunicationTimeOut = RESET_FLAG;
	
	//Enable timer
	ENABLE_COMMUNICATION_TIMER;
	
	return;
}

//_____fnStartVolStableTimer_______________________________________________________________
//
// @brief	This function start voltage regulator stability timer which is used to generate the time delay for 5.5V and 3.3V regulator to get stabilized
// @param	chVal	Flag indicating stabilization time required to get generated for 5V and 3.3V regulators.

void fnStartVolStableTimer(uint8_t chVal)
{
	//Fill_up the period register
	TCE1.CNT = RESET_COUNTER;
	
	if(chVal == WAIT_5_VOL_TIMER)
	{
		TCE1.PER = 1251;			//Set period register for 80ms + 60us delay 
	}
	else
	{
		TCE1.PER = 1;				//Set period register for 60us delay
	}
	
	gchVoltageStableTimerFlag = RESET_FLAG;		//Reset voltage stable flag
	ENABLE_VOL_STABLE_TIMER;					//Start voltage stable timer
	
	return;
}

//_____ I S R - U A R T  T I M E R ____________________________________________________________________
//
// @brief	ISR for UART Frame Delay Timer:
//			This is the simple TIMER with increment counter mechanism.
//			When desired count value will reach, timer will get disabled and appropriate decision making flags will get set/reset.

ISR(TCE0_OVF_vect)
{
	if(++gchCurrentDelayValue > gchMaxDelayValue)
	{
		gchNewInterrupt = SET_NEW_ISR_FLAG;
		gchRxAvail=SET_FLAG;						//Set this flag to indicate successful reception
		DISABLE_UART_DELAY_TIMER;
	}
}

//_____ I S R - C O M M U N I C A T I O N  T I M E R____________________________________________________
//
// @brief	ISR for communication delay timer:
//			This is the simple TIMER with increment counter mechanism.
//			When desired count value will reach, timer will get disabled and appropriate decision making flags will get set/reset.

ISR(TCC1_OVF_vect)
{
	if(++gchCommunicationCurrentDelayValue > gchCommunicationMaxDelayValue)
	{
		gchNewInterrupt = SET_NEW_ISR_FLAG;
		gchCommunicationTimeOut = SET_FLAG;				//Set this flag to indicate communication time out
		DISABLE_COMMUNICATION_TIMER;				
	}
}

//_____ I S R - V O L T A G E S T A B I L I T Y T I M E R____________________________________________________
//
// @brief	This ISR will be executed when voltage stability timer overflow will occur.
//			It will set the flag indicating voltage is stable in regulators.

ISR(TCE1_OVF_vect)
{
	gchNewInterrupt = SET_NEW_ISR_FLAG;
	gchVoltageStableTimerFlag=SET_FLAG;					//Set this flag to indicate voltage regulator is stable
	DISABLE_VOL_STABLE_TIMER;				
}

//_____ I S R - C O U N T E R  D E L A Y____________________________________________________
//
// @brief	ISR for communication delay timer:
//			This is the simple TIMER with increment counter mechanism.
//			When desired count value will reach, timer will get disabled and appropriate decision making flags will get set/reset.

ISR(TCF1_OVF_vect)
{
	if(++gchCurrentCounterDelayValue > gchMaxCounterDelayValue)
	{
		gchCounterDelayTimeOut=SET_FLAG;				//Set this flag to indicate time delay acheived
		DISABLE_WAIT_TIMER;
	}
}