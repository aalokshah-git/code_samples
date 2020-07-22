/* -------------------------------------------------------------------------
Filename: mc_adc.c

Date Created: 10/27/2014

Purpose:	ADC related functionality including initialization and support for parallel sampling operations over ADC channels

Functions:
fnInitializeAdcResources	Initializes ADC_MANAGER with mapping of all the ADC inputs for ADCA and ADCB channels
fnInitializeADC				Function is responsible behind initialization of all the ADC related resources (ADCA and ADCB of MCU)
fnADCFetchSampledData		Manages parallel access on all the channels of ADCA and ADCB and returns sampled data for provided ADC input
fnResetAdcResources			Reset all inter dependent ADC resources

Interrupts:


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

#include "mc_adc.h"			// ADC functionality for ATXMEGA MCU system

//_____ G L O B A L   D E F I N A T I O N S _______________________________________________

//All the 8-bits of this variable are mapped to virtual ADC channel defined in software. 
//For ADC availability bits can be checked explicitly.
volatile uint8_t gchFreeADC_Channels;

//All the bits point to individual ADC channel and can be used to check the current status of ADC channel.
volatile uint8_t gchAdcOpeartingState;

//XMEGA has two ADCs (ADCA & ADCB) and both can run 4-channels parallel
//All the ADC inputs are mapped to ADCA as well as ADCB and it can be configure for any of them
//Object of structure which holds mapping of data for the ADC inputs 
ADC_MANAGER  ghAdcManager[MAX_SAHRED_RESOURCES_ON_ADC];

//List of virtual software defined ADC channels
ADC_VIRTUAL_CHANNELS ghAdcChannels;

//_____ fnInitializeAdcResources ____________________________________________________________________
//
// @brief	This function initializes ADC_MANAGER with mapping of all the ADC inputs for ADCA and ADCB channels

inline void fnInitializeAdcResources(void)
{
	/* All ADC Input must need to have entry in this function */
	
	ghAdcManager[CHAMBER_TEMPERATURE_ADC_INDEX].chAdcMuxPosA=ADC_CH_MUXPOS_PIN11_gc;
	ghAdcManager[CHAMBER_TEMPERATURE_ADC_INDEX].chAdcMuxPosB=ADC_CH_MUXPOS_PIN3_gc;
	return;
}

//_____ fnInitializeADC ____________________________________________________________________
//
// @brief	This function initializes the ADCA and ADCB for initial level of operations by following the steps:
//				2> Confifure data rate on which to operate the ADC
//				3> Configure mode of operation and turn on the interrupt for ADC channels if required
//				4> Enable ADC Modules

void fnInitializeADC(void)
{
	ADCA.CTRLB |= (ADC_CONMODE_bm);
	ADCB.CTRLB |= (ADC_CONMODE_bm);
	
	// ADC Reference: VCC/1.6 is considered (2.0625V)
	ADCA.REFCTRL = ADC_REFSEL_INTVCC_gc;
	ADCB.REFCTRL = ADC_REFSEL_INTVCC_gc;
	
	// For 16MHZ system clock-ADC is configured to run with 1MHz clock settings
	ADCA.PRESCALER = ADC_PRESCALER_DIV16_gc;
	ADCB.PRESCALER = ADC_PRESCALER_DIV16_gc;
	
	//ADC CH0 is configured to use in single ended mode
	ADCB.CH0.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
	ADCA.CH0.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
	
	//ADC CH1 is configured to use in single ended mode
	ADCB.CH1.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
	ADCA.CH1.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
	
	//ADC CH2 is configured to use in single ended mode
	ADCB.CH2.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
	ADCA.CH2.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
	
	//ADC CH3 is configured to use in single ended mode
	ADCB.CH3.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
	ADCA.CH3.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
	
	ADCB.CH0.INTCTRL =	ADC_CH_INTLVL_LO_gc;
	ADCA.CH0.INTCTRL =	ADC_CH_INTLVL_LO_gc;
	
	ADCB.CH1.INTCTRL =	ADC_CH_INTLVL_LO_gc;
	ADCA.CH1.INTCTRL =	ADC_CH_INTLVL_LO_gc;
	
	ADCB.CH2.INTCTRL =	ADC_CH_INTLVL_LO_gc;
	ADCA.CH2.INTCTRL =	ADC_CH_INTLVL_LO_gc;
	
	ADCB.CH3.INTCTRL =	ADC_CH_INTLVL_LO_gc;
	ADCA.CH3.INTCTRL =	ADC_CH_INTLVL_LO_gc;
	
	fnInitializeAdcResources();
	
	//Enable ADC module
	ENABLE_ADCB_MODULE;
	ENABLE_ADCA_MODULE;
	
	return;
}

//_____ fnResetAdcResources ____________________________________________________________________
//
// @brief	Release all the inter dependent ADC resources

void fnResetAdcResources(void)
{
	gchFreeADC_Channels = ALL_ADC_CHANNELS_FREE;		//Initialize all ADC channels as free
	gchAdcOpeartingState = RESET_VALUE;					//Initialize ADC operating state as free.
	
	return;
}

//
// @brief	This function will get called if middle ware wants to samples any of the ADC input.
//			On request function will check the availability of ADC channel among 8 virtual channels of ADCA and ADCB as shown below:
//			Variable to indicate which ADC channel is free.
//			gchFreeADC_Channels:	|  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |			Bit Value:	1	Channel is busy
//									| CH3 | CH2 | CH1 | CH0 | CH3 | CH2 | CH1 | CH0 |						0	Channel is Free
//									|			ADCB		|	     ADCA		 	|
//
//									|			ADCB		|	     ADCA		 	|
//
//			The same virtual channel will be used to get the sampled data for that particular sensor after sampling completes.
// @param	chSensorEntryIndex	Index indicating ADC input data mapping in ADC_MANAGER
// @return	FALSE if all ADC channels are occupied

{
	uint8_t chLoopVar = RESET_COUNTER;
	uint8_t chNo = RESET_VALUE;
	
	//Check whether ADC channel is free or not
	if(gchFreeADC_Channels == ALL_ADC_CHANNELS_OCCUPIED)
	{
		return RETURN_FALSE;
	}
	
	chNo=0x01;	//Initialize channel no with the first virtual channel

	//This will check which channel is free and allocate that channel to the requested sensor.
	for (chLoopVar=RESET_COUNTER; chLoopVar<MAX_AVAILABLE_ADC_CHANNELS; chLoopVar++)
	{
		//If the channel is busy then check for the next channel
		if(gchFreeADC_Channels & chNo)
		{
			chNo=chNo<<1;						//Every bit of gchFreeADC_Channels mapped with one virtual channel
		}
		else
		{
			gchFreeADC_Channels |= chNo;		//Make ADC channel busy
			ghAdcManager[chSensorEntryIndex].chAdcActiveChannel=chNo;	//Map channel no with the Sensor for which this function is executed
		}
	}

	//Will start sampling for the allocated channel
	switch(ghAdcManager[chSensorEntryIndex].chAdcActiveChannel)
	{
		case 1:		//CH0_ADCA
			ADCA.CH0.MUXCTRL |= ghAdcManager[chSensorEntryIndex].chAdcMuxPosA;
			ADCA.CH0.CTRL |= ADC_CH_START_bm;
			gchAdcOpeartingState |= (ADC_CH_CONVERSATION_RUNNING <<ADC_VIRTUAL_CH1);
		break;
		
		case 2:		//CH1_ADCA
			ADCA.CH1.MUXCTRL |= ghAdcManager[chSensorEntryIndex].chAdcMuxPosA;
			ADCA.CH1.CTRL |= ADC_CH_START_bm;
			gchAdcOpeartingState |= (ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH2);
		break;
		
		case 4:		//CH2_ADCA
			ADCA.CH2.MUXCTRL |= ghAdcManager[chSensorEntryIndex].chAdcMuxPosA;
			ADCA.CH2.CTRL |= ADC_CH_START_bm;
			gchAdcOpeartingState |= (ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH3);
		break;
		
		case 8:		//CH3_ADCA
			ADCA.CH3.MUXCTRL |= ghAdcManager[chSensorEntryIndex].chAdcMuxPosA;
			ADCA.CH3.CTRL |= ADC_CH_START_bm;
			gchAdcOpeartingState |= (ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH4);
		break;
		
		case 16:	//CH0_ADCB
			ADCB.CH0.MUXCTRL |= ghAdcManager[chSensorEntryIndex].chAdcMuxPosB;
			ADCB.CH0.CTRL |= ADC_CH_START_bm;
			gchAdcOpeartingState |= (ADC_CH_CONVERSATION_RUNNING <<ADC_VIRTUAL_CH5);
		break;
		
		case 32:	//CH1_ADCB
			ADCB.CH1.MUXCTRL |= ghAdcManager[chSensorEntryIndex].chAdcMuxPosB;
			ADCB.CH1.CTRL |= ADC_CH_START_bm;
			gchAdcOpeartingState |= (ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH6);
		break;
		
		case 64:	//CH2_ADCB
			ADCB.CH2.MUXCTRL |= ghAdcManager[chSensorEntryIndex].chAdcMuxPosB;
			ADCB.CH2.CTRL |= ADC_CH_START_bm;
			gchAdcOpeartingState |= (ADC_CH_CONVERSATION_RUNNING <<ADC_VIRTUAL_CH7);
		break;
		
		case 128:	//CH3_ADCB
			ADCB.CH3.MUXCTRL |= ghAdcManager[chSensorEntryIndex].chAdcMuxPosB;
			ADCB.CH3.CTRL |= ADC_CH_START_bm;
			gchAdcOpeartingState |= (ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH8);
		break;
		
		default:
			SEND_DEBUG_ERROR_CODES(SENSOR_INVALID_EXECUTION_DEVELOPER_ERROR);			//Invalid ADC Input Index
			return RETURN_FALSE;
		break;
	}
	
	return RETURN_TRUE;
}

//_____ fnADCFetchSampledData ____________________________________________________________________
//
//	@brief	This function returns the sampled data if sampling is completed on the mapped ADC channel
//			This function executes in the steps as shown below:
//				1> Bit of gchAdcOpeartingState will get checked for the allocated channel
// @param	chSensorEntryIndex	Index indicating ADC input data mapping in ADC_MANAGER
//			In case of mismatch in resource management it will return ADC_DATA_COLLECTION_MISMATCH

int16_t fnADCFetchSampledData(uint8_t chSensorEntryIndex)
{
	//This will check whether any channel is selected for the sensor. If no channel is selected then return error code.
	if(ghAdcManager[chSensorEntryIndex].chAdcActiveChannel==ADC_NO_CHANNEL_SELECTED)
	{
		return RETURN_ADC_DATA_COLLECTION_MISMATCH;
	}
	
	//this will return the RETURN_ADC_DATA_COLLECTION_RUNNING otherwise converted data.
	switch(ghAdcManager[chSensorEntryIndex].chAdcActiveChannel)
	{
		case 1:		//CH0_ADCA
			if(!(gchAdcOpeartingState & (ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH1)))
			{
				gchFreeADC_Channels &= ~(ghAdcManager[chSensorEntryIndex].chAdcActiveChannel);
				return ADCA.CH0.RES;
			}
		break;
		
		case 2:		//CH1_ADCA
			if(!(gchAdcOpeartingState & (ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH2)))
			{
				gchFreeADC_Channels &= ~(ghAdcManager[chSensorEntryIndex].chAdcActiveChannel);
				return ADCA.CH1.RES;
			}
		break;
		
		case 4:		//CH2_ADCA
			if(!(gchAdcOpeartingState & (ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH3)))
			{
				gchFreeADC_Channels &= ~(ghAdcManager[chSensorEntryIndex].chAdcActiveChannel);
				return ADCA.CH2.RES;
			}
		break;
		
		case 8:		//CH3_ADCA
			if(!(gchAdcOpeartingState & (ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH4)))
			{
				gchFreeADC_Channels &= ~(ghAdcManager[chSensorEntryIndex].chAdcActiveChannel);
				return ADCA.CH3.RES;
			}
		break;
		
		case 16:	//CH0_ADCB
			if(!(gchAdcOpeartingState & (ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH5)))
			{
				gchFreeADC_Channels &= ~(ghAdcManager[chSensorEntryIndex].chAdcActiveChannel);
				return ADCB.CH0.RES;
			}
		break;
		
		case 32:	//CH1_ADCB
			if(!(gchAdcOpeartingState & (ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH6)))
			{
				gchFreeADC_Channels &= ~(ghAdcManager[chSensorEntryIndex].chAdcActiveChannel);
				return ADCB.CH1.RES;
			}
		break;
		
		case 64:	//CH2_ADCB
			if(!(gchAdcOpeartingState & (ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH7)))
			{
				gchFreeADC_Channels &= ~(ghAdcManager[chSensorEntryIndex].chAdcActiveChannel);
				return ADCB.CH2.RES;
			}
		break;
		
		case 128:	//CH3_ADCB
			if(!(gchAdcOpeartingState & (ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH8)))
			{
				gchFreeADC_Channels &= ~(ghAdcManager[chSensorEntryIndex].chAdcActiveChannel);
				return ADCB.CH3.RES;
			}
		break;
		
		default:
			SEND_DEBUG_ERROR_CODES(SENSOR_INVALID_EXECUTION_DEVELOPER_ERROR);
			return RETURN_ADC_DATA_COLLECTION_MISMATCH;
		break;
	}
	
	return RETURN_ADC_DATA_COLLECTION_RUNNING;
}

//_____ I S R - A D C A : C H 0  C O N V E R S I O N   C O M P L E T E ____________________________________________________________________
//
//	This ISR will clear the bit of gchAdcOpeartingState for the CH0 of ADCA

ISR(ADCA_CH0_vect)
{
	gchNewInterrupt = SET_NEW_ISR_FLAG;
	gchAdcOpeartingState &= ~(ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH1);
}

//_____ I S R - A D C A : C H 1  C O N V E R S I O N   C O M P L E T E ____________________________________________________________________
//
//	This ISR will clear the bit of gchAdcOpeartingState for the CH1 of ADCA

ISR(ADCA_CH1_vect)
{
	gchNewInterrupt = SET_NEW_ISR_FLAG;
	gchAdcOpeartingState &= ~(ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH2);
}

//_____ I S R - A D C A : C H 2  C O N V E R S I O N   C O M P L E T E ____________________________________________________________________
//
//	This ISR will clear the bit of gchAdcOpeartingState for the CH2 of ADCA

ISR(ADCA_CH2_vect)
{
	gchNewInterrupt = SET_NEW_ISR_FLAG;
	gchAdcOpeartingState &= ~(ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH3);
}

//_____ I S R - A D C A : C H 3  C O N V E R S I O N   C O M P L E T E ____________________________________________________________________
//
//	This ISR will clear the bit of gchAdcOpeartingState for the CH3 of ADCA

ISR(ADCA_CH3_vect)
{
	gchNewInterrupt = SET_NEW_ISR_FLAG;
	gchAdcOpeartingState &= ~(ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH4);
}

//_____ I S R - A D C B : C H 0  C O N V E R S I O N   C O M P L E T E ____________________________________________________________________
//
//	This ISR will clear the bit of gchAdcOpeartingState for the CH0 of ADCB

ISR(ADCB_CH0_vect)
{
	gchNewInterrupt = SET_NEW_ISR_FLAG;
	gchAdcOpeartingState &= ~(ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH5);
}

//_____ I S R - A D C B : C H 1  C O N V E R S I O N   C O M P L E T E ____________________________________________________________________
//
//	This ISR will clear the bit of gchAdcOpeartingState for the CH1 of ADCB

ISR(ADCB_CH1_vect)
{
	gchNewInterrupt = SET_NEW_ISR_FLAG;
	gchAdcOpeartingState &= ~(ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH6);
}

//_____ I S R - A D C B : C H 2  C O N V E R S I O N   C O M P L E T E ____________________________________________________________________
//
//	This ISR will clear the bit of gchAdcOpeartingState for the CH2 of ADCB

ISR(ADCB_CH2_vect)
{
	gchNewInterrupt = SET_NEW_ISR_FLAG;
	gchAdcOpeartingState &= ~(ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH7);
}

//_____ I S R - A D C B : C H 3  C O N V E R S I O N   C O M P L E T E ____________________________________________________________________
//
//	This ISR will clear the bit of gchAdcOpeartingState for the CH3 of ADCB

ISR(ADCB_CH3_vect)
{
	gchNewInterrupt = SET_NEW_ISR_FLAG;
	gchAdcOpeartingState &= ~(ADC_CH_CONVERSATION_RUNNING << ADC_VIRTUAL_CH8);
}


