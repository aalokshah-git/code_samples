/* -------------------------------------------------------------------------
Filename: generic_macro.h

Job#: 20473
Purpose: List of generally used MACROs in effective firmware designing
Date Created: 12/19/2014

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

#ifndef GENERIC_MACRO_H_
#define GENERIC_MACRO_H_

	//_________________________________________ M A C R O S ________________________________________________
	
	#define		RETURN_TRUE							0
	#define		RETURN_FALSE						(-1)

	#define		SET_FLAG							1
	#define		RESET_FLAG							0
	
	#define		SET_COUNTER							1
	#define		RESET_COUNTER						0
	
	#define		SET_VALUE							1
	#define		RESET_VALUE							0

	#define		SET_NEW_ISR_FLAG					1
	#define		CLEAR_NEW_ISR_FLAG					0

	//__________________________________E N U M E R A T I O N S ___________________________
	
	//Enumeration for bit mask
	typedef enum
	{
		BIT_0_bm	= (1<<0),
		BIT_1_bm	= (1<<1),
		BIT_2_bm	= (1<<2),
		BIT_3_bm	= (1<<3),
		BIT_4_bm	= (1<<4),
		BIT_5_bm	= (1<<5),
		BIT_6_bm	= (1<<6),
		BIT_7_bm	= (1<<7),
		BIT_8_bm	= (1<<8),
		BIT_9_bm	= (1<<9),
		BIT_10_bm	= (1<<10),
		BIT_11_bm	= (1<<11),
		BIT_12_bm	= (1<<12),
		BIT_13_bm	= (1<<13),
		BIT_14_bm	= (1<<14),
		BIT_15_bm	= (1<<15)
	}Bit_Mask_t;

	//Enumeration for bit position
	typedef enum
	{
		BIT_0_bp = 0,
		BIT_1_bp,
		BIT_2_bp,
		BIT_3_bp,
		BIT_4_bp,
		BIT_5_bp,
		BIT_6_bp,
		BIT_7_bp,
		BIT_8_bp,		
		BIT_9_bp,
		BIT_10_bp,
		BIT_11_bp,
		BIT_12_bp,
		BIT_13_bp,
		BIT_14_bp,
		BIT_15_bp
	}Bit_Position_t;
	
	//Enumeration for algorithms related to step mode
	typedef enum
	{
		STEP_0_Val = 0,
		STEP_1_Val,
		STEP_2_Val,
		STEP_3_Val,
		STEP_4_Val,
		STEP_5_Val,
		STEP_6_Val
	}Step_Index_values;

#endif /* GENERIC_MACRO_H_ */