/* -------------------------------------------------------------------------
Filename: mc_adc.h

Purpose: ADC related basic functionality
Date Created: 10/27/2014

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

#ifndef MC_ADC_H_
#define MC_ADC_H_

	//_____  I N C L U D E S ______________________________________________________________

	#include "mc_system.h"			// Basic functionality for ATXMEGA MCU system

	//_____ M A C R O S ____________________________________________________________________

	//ADCB controlling
	#define ENABLE_ADCB_MODULE		ADCB.CTRLA = (ADC_ENABLE_bm | ADC_FLUSH_bm)
	#define DISABLE_ADCB_MODULE		ADCB.CTRLA &= ~(ADC_ENABLE_bm)

	//ADCA controlling
	#define ENABLE_ADCA_MODULE		ADCA.CTRLA = (ADC_ENABLE_bm | ADC_FLUSH_bm)
	#define DISABLE_ADCA_MODULE		ADCA.CTRLA &= ~(ADC_ENABLE_bm)

	#define MAX_SAHRED_RESOURCES_ON_ADC				10		//Maximum no of sensor share ADC resource
	#define MAX_AVAILABLE_ADC_CHANNELS				8
	#define ADC_CH_OCCUPIED							1
	#define ADC_CH_CONVERSATION_RUNNING				1
	#define ADC_NO_CHANNEL_SELECTED					0x00
	#define ALL_ADC_CHANNELS_FREE					0x00
	#define ALL_ADC_CHANNELS_OCCUPIED				0xff
	#define RETURN_ADC_DATA_COLLECTION_MISMATCH		5001	//Error code for data collection mismatch
	#define RETURN_ADC_DATA_COLLECTION_RUNNING		5002	//Error code for data collection running
	
	//Indexes defined for Various Sensors
	//All the ADC Inputs must needs their explicit entry
	#define CHAMBER_TEMPERATURE_ADC_INDEX			0		//chamber temperature buffer index no
	#define TEG_HOT_TEMPERATURE_ADC_INDEX			1		//TEG hot temperature buffer index no
	#define TEG_COLD_TEMPERATURE_ADC_INDEX			2		//TEG cold temperature buffer index no

	//__________ D A T A   S T R U C T U R E S ____________________________________________

	//Structure holds mapping of data for the ADC inputs 
	typedef struct
	{
		uint8_t chAdcActiveChannel;		//It will store no of active active ADC channel
		uint8_t chAdcMuxPosA;			//It will store MUXCTRL register value for ADCA
		uint8_t chAdcMuxPosB;			//It will store MUXCTRL register value for ADCB
	}ADC_MANAGER;

	//__________ E N U M E R A T I O N S ____________________________________________
	
	// List of software defined ADC channels
	typedef enum
	{
		ADC_VIRTUAL_CH1,
		ADC_VIRTUAL_CH2,
		ADC_VIRTUAL_CH3,
		ADC_VIRTUAL_CH4,
		ADC_VIRTUAL_CH5,
		ADC_VIRTUAL_CH6,
		ADC_VIRTUAL_CH7,
		ADC_VIRTUAL_CH8
	}ADC_VIRTUAL_CHANNELS;

	//_____ F U N C T I O N   D E F I N I T I O N S ___________________________________________________________

	//_____ fnInitializeADC ____________________________________________________________________
	//
	// @brief	This function initializes the ADCA and ADCB for initial level of operations by following the steps:
	//				1> Configure reference volatage and conversion mode - single ended signed mode
	//				2> Confifure data rate on which to operate the ADC
	//				3> Configure mode of operation and turn on the interrupt for ADC channels if required
	//				4> Enable ADC Modules
	
	void fnInitializeADC(void);

	//_____ fnADCStartConversion ____________________________________________________________________
	//
	// @brief	This function will get called if middle ware wants to samples any of the ADC input.
	//			On request function will check the availability of ADC channel among 8 virtual channels of ADCA and ADCB as shown below:
	//			Variable to indicate which ADC channel is free.
	//			gchFreeADC_Channels:	|  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |			Bit Value:	1	Channel is busy
	//									| CH3 | CH2 | CH1 | CH0 | CH3 | CH2 | CH1 | CH0 |						0	Channel is Free
	//									|			ADCB		|	     ADCA		 	|
	//
	//			Variable to indicate conversion is completed or not on particular channel.
	//			gchAdcOpeartingState:	|  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |			Bit Value:	1	Conversion complete
	//									| CH3 | CH2 | CH1 | CH0 | CH3 | CH2 | CH1 | CH0 |						0	Conversion Pending
	//									|			ADCB		|	     ADCA		 	|
	//
	//			If any ADC channel is free than it will be allocated as virtual channel to the avail ADC input for conversion.
	//			The same virtual channel will be used to get the sampled data for that particular sensor after sampling completes.
	// @param	chSensorEntryIndex	Index indicating ADC input data mapping in ADC_MANAGER
	// @return	FALSE if all ADC channels are occupied
	
	int8_t fnADCStartConversion(uint8_t chSensorEntryIndex);

	//_____ fnADCFetchSampledData ____________________________________________________________________
	//
	//	@brief	This function returns the sampled data if sampling is completed on the mapped ADC channel
	//			This function executes in the steps as shown below:
	//				1> Bit of gchAdcOpeartingState will get checked for the allocated channel
	//				2> If the conversion is completed on the channel then the bit feild mapped with the channel will get cleared in the ISR of that channel
	//				3> If bit value will found cleared than the conversion data will get returned to the middle ware layer in reply
	// @param	chSensorEntryIndex	Index indicating ADC input data mapping in ADC_MANAGER
	// @return	Sampled data (ADC Resolution) if conversion is completed otherwise returns ADC_DATA_COLLECTION_RUNNING
	//			In case of mismatch in resource management it will return ADC_DATA_COLLECTION_MISMATCH
	
	int16_t fnADCFetchSampledData(uint8_t chSensorEntryIndex);

	//_____ fnResetAdcResources ____________________________________________________________________
	//
	// @brief	Release all the inter dependent ADC resources
	
	void fnResetAdcResources(void);

#endif /* MC_ADC_H_ */
