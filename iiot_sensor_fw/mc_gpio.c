/* -------------------------------------------------------------------------
Filename: mc_gpio.c

Date Created: 10/08/2014

Purpose: To hold all the GPIO related functionality including initialization of ports/pins

Functions:
fnInitializeGPIO		Perform initialization procedures on all the MCU pins for their role in SENSOR MC

Interrupts:
PORTH_INT0_vect			ISR for PORTH-PIN2 falling edge (Configured for CC1125-GPIO0: Transmit/Receive Complete Interrupt)


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

#include "mc_gpio.h"			// GPIO functionality for ATXMEGA MCU system

//_____  G L O B A L   D E F I N I T I O N S ______________________________________________________________

//It will get set when CC1125-GPIO0 Transmit/Receive complete interrupt arise.
//It needs to get reset from firmware.
volatile uint8_t		gchDataCommFlagCC1125;

//_____ fnInitializeGPIO ____________________________________________________________________
//
// @brief	Function initialize all the port pins as per hardware requirement.
//			Following steps are performed to do the same:
//				1> Individual port pins are configured as input or output on the basis of their functionality.
//				2> All the general purpose pins configurted as output is initialized for High or Low state on the basis of its role in SENSOR MC.
//				3> GPIO related interrupts and necessary internal pull up and pull down are also configured.

void fnInitializeGPIO(void)
{
	/****************************************************
	//			PORT-A Pins
	//
	//	0		BTY1_VREF					INPUT-PULL_UP
	//	1		BTY2_VREF					INPUT-PULL_UP
	//	2		BTY3_VREF					INPUT-PULL_UP
	//	3		BTY4_VREF					INPUT-PULL_UP
	//	4		BTY_CURRENT_OUT				INPUT-PULL_UP
	//	5		BTY_CURRENT_INTERRUPT		INPUT
	//	6		BTY_TEMPRATURE				INPUT-PULL_UP
	//	7		RADIO CLOCK				?????
	*****************************************************/
	
	//SET_PINS_INPUT(PORTA,PA_B1_VREF | PA_B2_VREF | PA_B3_VREF | PA_B4_VREF | PA_BTY_CURRENT_IN | PA_BTY_CURRENT_OUT | PA_BTY_TEMP);
	//PORTA.PIN0CTRL=PORT_OPC_PULLUP_gc;
	//PORTA.PIN1CTRL=PORT_OPC_PULLUP_gc;
	//PORTA.PIN2CTRL=PORT_OPC_PULLUP_gc;
	//PORTA.PIN3CTRL=PORT_OPC_PULLUP_gc;
	//PORTA.PIN4CTRL=PORT_OPC_PULLUP_gc;
	//PORTA.PIN6CTRL=PORT_OPC_PULLUP_gc;
	SET_PINS_OUTPUT(PORTA,PA_TEST_POINT);
	/****************************************************
	//			PORT-B Pins
	//
	//	0		TEG_TEMP_HOT				INPUT-PULL_UP
	//	1		TEG_TEMP_COLD				INPUT-PULL_UP
	//	2		CLOCK PHASES				???
	//	3		CHAMBER_TEMP				INPUT-PULL_UP
	//	4-7		JTAG						-----
	*****************************************************/

	SET_PINS_INPUT(PORTB,PB_CHAMBER_TEMP | PB_TEG_TEMP_HOT | PB_TEG_TEMP_COLD);
	SET_PINS_OUTPUT(PORTB,PB_TEST_POINT);
	//PORTB.PIN0CTRL=PORT_OPC_PULLUP_gc;
	//PORTB.PIN1CTRL=PORT_OPC_PULLUP_gc;
	//PORTB.PIN2CTRL=PORT_OPC_PULLUP_gc;
	//PORTB.PIN3CTRL=PORT_OPC_PULLUP_gc;
	
	/****************************************************
	//			PORT-C Pins
	//
	//	0		RADIO_SELECT				OUTPUT-LOW
	//	1		RADIO_RESET					OUTPUT-HIGH
	//	2		UART_RXD0					INPUT
	//	3		UART_TXD0					OUTPUT
	//	4		SPIC_SS						OUTPUT-HIGH
	//	5		SPIC_MOSI					OUTPUT
	//	6		SPIC_MISO					INPUT-PULL_UP
	//	7		SPIC_SCK					OUTPUT
	*****************************************************/
	
	SET_PINS_INPUT(PORTC,PC_SPIC_MISO | PC_UART_RXD0);
	SET_PINS_OUTPUT(PORTC,PC_RADIO_SELECT | PC_RADIO_RESET | PC_UART_TXD0 | PC_SPIC_SS | PC_SPIC_SCK | PC_SPIC_MOSI);
	SET_PINS_HIGH(PORTC,PC_RADIO_RESET | PC_SPIC_SS);
	SET_PINS_LOW(PORTC,PC_RADIO_SELECT);
	PORTC.PIN6CTRL=PORT_OPC_PULLUP_gc;
	
	/****************************************************
	//			PORT-D Pins
	//
	//	0		SPID_S0						OUTPUT-LOW
	//	1		SPID_S1						OUTPUT-LOW
	//	2		TEST_POINT_3				?????
	//	3		SPID_S2						OUTPUT-LOW
	//	4		SPID_SS						OUTPUT-HIGH
	//	5		SPID_MOSI					OUTPUT
	//	6		SPID_MISO					INPUT
	//	7		SPID_SCK					OUTPUT
	*****************************************************/
	
	SET_PINS_INPUT(PORTD,PD_SPID_MISO);
	SET_PINS_OUTPUT(PORTD,PD_SPID_S0 | PD_SPID_S1 | PD_SPID_S2 | PD_SPID_SS | PD_SPID_MOSI |PD_SPID_SCK);
	SET_PINS_HIGH(PORTD,PD_SPID_SS);
	SET_PINS_LOW(PORTD,PD_SPID_S0 | PD_SPID_S1 | PD_SPID_S2);
	
	/****************************************************
	//			PORT-E Pins
	//
	//	0		SPIE_S0						OUTPUT-LOW
	//	1		SPIE_S1						OUTPUT-LOW
	//	2		VDD_5V_GD					INPUT
	//	3		SPIE_S2						OUTPUT-LOW
	//	4		SPIE_SS						OUTPUT-HIGH
	//	5		SPIE_MOSI					OUTPUT
	//	6		SPIE_MISO					INPUT
	//	7		SPIE_SCK					OUTPUT
	*****************************************************/
	
	SET_PINS_INPUT(PORTE,PE_SPIE_MISO | PE_VDD_5V_GD);
	SET_PINS_OUTPUT(PORTE,PE_SPIE_S0 | PE_SPIE_S1 | PE_SPIE_S2 | PE_SPIE_SS | PE_SPIE_MOSI |PE_SPIE_SCK);
	SET_PINS_HIGH(PORTE,PE_SPIE_SS);
	SET_PINS_LOW(PORTE,PE_SPIE_S0 | PE_SPIE_S1 | PE_SPIE_S2);
	
	/****************************************************
	//			PORT-F Pins
	//
	//	0		I2CF_SDA					WIRED_AND
	//	1		I2CF_SCL					WIRED_AND
	//	2		BTY_CHARGE_POSITIVE			INPUT
	//	3		BTY_CHARGE_NEGATIVE			INPUT
	//	4		SPIF_SS						OUTPUT-HIGH
	//	5		SPIF_MOSI					OUTPUT
	//	6		SPIF_MISO					INPUT
	//	7		SPIF_SCK					OUTPUT
	*****************************************************/
	
	SET_PINS_INPUT(PORTF,PF_SPIF_MISO);// | PF_BTY_CHARGE_NEGATIVE | PF_BTY_CHARGE_POSITIVE);
	SET_PINS_OUTPUT(PORTF,PF_SPIF_SS | PF_SPIF_MOSI |PF_SPIF_SCK);
	SET_PINS_HIGH(PORTF,PF_SPIF_SS);
	PORTF.PIN0CTRL=PORT_OPC_WIREDAND_gc;
	PORTF.PIN1CTRL=PORT_OPC_WIREDAND_gc;
	
	/****************************************************
	//			PORT-H Pins
	//
	//	0		EEPROM_WR_CONTROL			OUTPUT-HIGH
	//	1		EEPROM_E2_ADDR_BIT			OUTPUT-HIGH
	//	2		CC1125_GPIO0				INPUT (Falling Edge Interrupt)
	//	3		CC1125_GPIO1				INPUT
	//	4		CC1125_GPIO2				INPUT
	//	5		CC1125_GPIO3				INPUT
	//	6		3V3_25MA_PWR				OUTPUT-LOW
	//	7		RADIO_PWR_EN				OUTPUT-LOW
	*****************************************************/
	
	SET_PINS_INPUT(PORTH,PH_CC1125_GPIO0);// | PH_CC1125_GPIO1 | PH_CC1125_GPIO2 | PH_CC1125_GPIO3);
	SET_PINS_OUTPUT(PORTH,PH_EEPROM_WR_CONTROL | PH_EEPROM_E2_ADDR_BIT | PH_RADIO_PWR_EN | PH_3V3_25MA_PWR);
	SET_PINS_HIGH(PORTH,PH_EEPROM_WR_CONTROL | PH_EEPROM_E2_ADDR_BIT);
	SET_PINS_LOW(PORTH, PH_RADIO_PWR_EN  | PH_3V3_25MA_PWR);
	
	PORTH.INT0MASK = PH_CC1125_GPIO0;
	PORTH.PIN2CTRL= PORT_ISC_FALLING_gc;	//Configure falling edge interrupt
	PORTH.INTFLAGS |= PH_CC1125_GPIO0;		//Clear interrupt flag
	PORTH.INTCTRL = PORT_INT0LVL_LO_gc;		//Configure interrupt for low level

	/****************************************************
	//			PORT-J Pins
	//
	//	0		SPIF_S0						OUTPUT-LOW
	//	1		SPIF_S1						OUTPUT-LOW
	//	2		TXD_MOD_FREQ				OUTPUT-LOW
	//	3		SPIF_S2						OUTPUT-LOW
	//	4		GP_LED0						OUTPUT-HIGH
	//	5		GP_LED1						OUTPUT-HIGH
	//	6		CC2591_HGM					OUTPUT-LOW
	//	7		CC2591_RXTX					OUTPUT-LOW
	*****************************************************/
	
	SET_PORT_OUTPUT(PORTJ);
	SET_PINS_HIGH(PORTJ,PJ_GP_LED0 | PJ_GP_LED1);
	SET_PINS_LOW(PORTJ,PJ_SPIF_S0 | PJ_SPIF_S1 | PJ_SPIF_S2);// | PJ_TXD_MOD_FREQ | PJ_CC2591_HGM | PJ_CC2591_RXTX);
	
	/****************************************************
	//			PORT-K Pins
	//
	//	0		SYS_HB_LED					OUTPUT-HIGH
	//	1		RADIO2_CHIP_EN				OUTPUT-LOW
	//	2		RADIO2_PWR_EN				OUTPUT-LOW
	//	3		BTY_TEST_EN					OUTPUT-HIGH
	//	4		BTY_TEST_S0					OUTPUT-LOW
	//	5		BTY_TEST_S1					OUTPUT-LOW
	//	6		SENSOR_RESET				OUTPUT-HIGH
	//	7		SAMPLE_CLOCK				OUTPUT-LOW
	*****************************************************/
	
	SET_PORT_OUTPUT(PORTK);
	SET_PINS_HIGH(PORTK,PK_SYS_HB_LED | PK_BTY_TEST_EN | PK_SENSOR_RESET);
	SET_PINS_LOW(PORTK,PK_RADIO2_CHIP_EN | PK_RADIO2_PWR_EN | PK_BTY_TEST_S0 | PK_BTY_TEST_S1 | PK_SAMPLE_CLOCK);
	
	/****************************************************
	//			PORT-Q Pins
	//
	//	0		CC2520_GPIO0				INPUT
	//	1		CC2520_GPIO1				INPUT
	//	2		CC2520_GPIO2				INPUT
	//	3		CC2520_GPIO3				INPUT
	*****************************************************/
	
	//SET_PINS_INPUT(PORTQ,PQ_CC2520_GPIO0 | PQ_CC2520_GPIO1 | PQ_CC2520_GPIO2 | PQ_CC2520_GPIO3);
	return;
}

//_____ I S R - P O R T H - F A L L I N G  E D G E ____________________________________________________________________
//
// @brief	ISR for PORTH (PIN-4) Falling Edge:
//			Radio chip-CC112x  provides facility to get interruption on reception and transmission complete event.
//			So this facility is used in SENSOR MC design to introduce reliability in communication mechanism and avoid polling while waiting fort data operations to complete in CC112x.

ISR(PORTH_INT0_vect)
{
	PORTH.INTFLAGS |= PH_CC1125_GPIO0;		//Reset Interrupt Flag
	gchNewInterrupt = SET_NEW_ISR_FLAG;
	gchDataCommFlagCC1125=1;				//Flag to indicate the completion of transmit/receive operation in CC1125
}