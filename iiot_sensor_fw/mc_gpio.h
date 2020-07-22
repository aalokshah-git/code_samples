/* -------------------------------------------------------------------------
Filename: mc_gpio.h

Purpose: Contains required GPIO related definitions
Date Created: 10/08/2014

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

#ifndef MC_GPIO_H_
#define MC_GPIO_H_

	//_____  I N C L U D E S ______________________________________________________________

	#include "mc_system.h"			// Basic functionality for ATXMEGA MCU system

	//_____ H A R D W A R E   R E G I S T E R S / B I T   D E F I N A T I O N S ____________________________________________________________________

	//PORT-A Pins
	#define PA_B1_VREF				(1 << 0)
	#define PA_B2_VREF				(1 << 1)
	#define PA_B3_VREF				(1 << 2)
	#define PA_B4_VREF				(1 << 3)
	#define PA_BTY_CURRENT_OUT		(1 << 4)
	#define PA_BTY_CURRENT_IN		(1 << 5)
	#define PA_BTY_TEMP				(1 << 6)
	#define PA_TEST_POINT			(1 << 7)

	//PORT-B Pins
	#define PB_TEG_TEMP_HOT			(1 << 0)
	#define PB_TEG_TEMP_COLD		(1 << 1)
	#define PB_TEST_POINT			(1 << 2)
	#define PB_CHAMBER_TEMP			(1 << 3)
	/* B4-B7 : JTAG */

	//PORT-C Pins
	#define PC_RADIO_SELECT			(1 << 0)
	#define PC_RADIO_RESET			(1 << 1)
	#define PC_UART_RXD0			(1 << 2)
	#define PC_UART_TXD0			(1 << 3)
	#define PC_SPIC_SS				(1 << 4)
	#define PC_SPIC_MOSI			(1 << 5)
	#define PC_SPIC_MISO			(1 << 6)
	#define PC_SPIC_SCK				(1 << 7)

	//PORT-D Pins
	#define PD_SPID_S0				(1 << 0)
	#define PD_SPID_S1				(1 << 1)
	#define PD_TEST_POINT			(1 << 2)
	#define PD_SPID_S2				(1 << 3)
	#define PD_SPID_SS				(1 << 4)
	#define PD_SPID_MOSI			(1 << 5)
	#define PD_SPID_MISO			(1 << 6)
	#define PD_SPID_SCK				(1 << 7)

	//PORT-E Pins
	#define PE_SPIE_S0				(1 << 0)
	#define PE_SPIE_S1				(1 << 1)
	#define PE_VDD_5V_GD			(1 << 2)
	#define PE_SPIE_S2				(1 << 3)
	#define PE_SPIE_SS				(1 << 4)
	#define PE_SPIE_MOSI			(1 << 5)
	#define PE_SPIE_MISO			(1 << 6)
	#define PE_SPIE_SCK				(1 << 7)

	//PORT-F Pins
	#define PF_I2CF_SDA				(1 << 0)
	#define PF_I2CF_SCL				(1 << 1)
	#define PF_BTY_CHARGE_POSITIVE	(1 << 2)
	#define PF_BTY_CHARGE_NEGATIVE	(1 << 3)
	#define PF_SPIF_SS				(1 << 4)
	#define PF_SPIF_MOSI			(1 << 5)
	#define PF_SPIF_MISO			(1 << 6)
	#define PF_SPIF_SCK				(1 << 7)

	//PORT-H Pins
	#define PH_EEPROM_WR_CONTROL	(1 << 0)
	#define PH_EEPROM_E2_ADDR_BIT	(1 << 1)
	#define PH_CC1125_GPIO0			(1 << 2)
	#define PH_CC1125_GPIO1			(1 << 3)
	#define PH_CC1125_GPIO2			(1 << 4)
	#define PH_CC1125_GPIO3			(1 << 5)
	#define PH_3V3_25MA_PWR			(1 << 6)
	#define PH_RADIO_PWR_EN			(1 << 7)

	//PORT-J Pins
	#define PJ_SPIF_S0				(1 << 0)
	#define PJ_SPIF_S1				(1 << 1)
	#define PJ_TXD_MOD_FREQ			(1 << 2)
	#define PJ_SPIF_S2				(1 << 3)
	#define PJ_GP_LED0				(1 << 4)		//Green LED
	#define PJ_GP_LED1				(1 << 5)		//Red LED
	#define PJ_CC2591_HGM			(1 << 6)
	#define PJ_CC2591_RXTX			(1 << 7)

	//PORT-K Pins
	#define PK_SYS_HB_LED			(1 << 0)
	#define PK_RADIO2_CHIP_EN		(1 << 1)
	#define PK_RADIO2_PWR_EN		(1 << 2)
	#define PK_BTY_TEST_EN			(1 << 3)
	#define PK_BTY_TEST_S0			(1 << 4)
	#define PK_BTY_TEST_S1			(1 << 5)
	#define PK_SENSOR_RESET			(1 << 6)
	#define PK_SAMPLE_CLOCK			(1 << 7)

	//PORT-Q Pins
	#define PQ_CC2520_GPIO0			(1 << 0)
	#define PQ_CC2520_GPIO1			(1 << 1)
	#define PQ_CC2520_GPIO2			(1 << 2)
	#define PQ_CC2520_GPIO3			(1 << 3)

	//_____ M A C R O S ____________________________________________________________________

	//Inline MACRO definitions for setting and resetting the GPIO
	#define SET_PORT_OUTPUT(PORT_VALUE)					PORT_VALUE.DIRSET = 0xff
	#define SET_PORT_INPUT(PORT_VALUE)					PORT_VALUE.DIRCLR = 0xff
	#define SET_PORT_HIGH(PORT_VALUE)					PORT_VALUE.OUTSET = 0xff
	#define SET_PORT_LOW(PORT_VALUE)					PORT_VALUE.OUTCLR = 0xff
	#define GET_PORT_VALUE(PORT_VALUE)					PORT_VALUE.IN

	#define SET_PINS_INPUT(PORT_VALUE,PIN_NOS)			PORT_VALUE.DIRCLR = PIN_NOS
	#define SET_PINS_OUTPUT(PORT_VALUE,PIN_NOS)			PORT_VALUE.DIRSET = PIN_NOS
	#define SET_PINS_HIGH(PORT_VALUE,PIN_NOS)			PORT_VALUE.OUTSET = PIN_NOS
	#define SET_PINS_LOW(PORT_VALUE,PIN_NOS)			PORT_VALUE.OUTCLR = PIN_NOS
	#define GET_PIN_VALUE(PORT_VALUE,PIN_NO)			(PORT_VALUE.IN & PIN_NO)

	// General Purpose LEDs
	#define MC_HEARTBEAT_LED_ON			SET_PINS_LOW(PORTK,PK_SYS_HB_LED)			//Enable controller heartbeat LED
	#define MC_HEARTBEAT_LED_OFF		SET_PINS_HIGH(PORTK,PK_SYS_HB_LED)			//Disable controller heartbeat LED
	#define MC_GP_LED0_ON				SET_PINS_LOW(PORTJ,PJ_GP_LED0)				//Enable general purpose LED0
	#define MC_GP_LED0_OFF				SET_PINS_HIGH(PORTJ,PJ_GP_LED0)				//Disable general purpose LED0
	#define MC_GP_LED1_ON				SET_PINS_LOW(PORTJ,PJ_GP_LED1)				//Enable general purpose LED1
	#define MC_GP_LED1_OFF				SET_PINS_HIGH(PORTJ,PJ_GP_LED1)				//Disable general purpose LED1

	//CC1125 and CC2520 Radio Controlling
	#define SELECT_CC1125_RADIO			SET_PINS_LOW(PORTC,PC_RADIO_SELECT)			//CC1125 Activate
	#define SELECT_CC2520_RADIO			SET_PINS_HIGH(PORTC,PC_RADIO_SELECT)		//CC2520 Activate
	#define ACTIVATE_RADIO_RESET		SET_PINS_LOW(PORTC,PC_RADIO_RESET)			//Radio Reset Enable
	#define DEACTIVATE_RADIO_RESET		SET_PINS_HIGH(PORTC,PC_RADIO_RESET)			//Radio Reset Disable
	#define ENABLE_RADIO_CC2520_CHIP	SET_PINS_HIGH(PORTK,PK_RADIO2_CHIP_EN)		//Enable CC2520 for Radio2 communication
	#define DISABLE_RADIO_CC2520_CHIP	SET_PINS_LOW(PORTK,PK_RADIO2_CHIP_EN)		//Disable CC2520 for Radio2 communication

	//Power Management
	#define ENABLE_5VOLT_POWER			SET_PINS_HIGH(PORTH,PH_RADIO_PWR_EN)		//Enable Power to Radio1- CC1125
	#define DISABLE_5VOLT_POWER			SET_PINS_LOW(PORTH,PH_RADIO_PWR_EN)			//Disable Power to Radio1- CC1125
	#define ENABLE_CC2520_RADIO_POWER	SET_PINS_HIGH(PORTK,PK_RADIO2_PWR_EN)		//Enable Power to Radio2- CC2520
	#define DISABLE_CC2520_RADIO_POWER	SET_PINS_LOW(PORTK,PK_RADIO2_PWR_EN)		//Disable Power to Radio2- CC2520
	#define ENABLE_VDD_3V3_25MA_PWR		SET_PINS_HIGH(PORTH,PH_3V3_25MA_PWR)		//Enable 3V3-25MA power source
	#define DISABLE_VDD_3V3_25MA_PWR	SET_PINS_LOW(PORTH,PH_3V3_25MA_PWR)			//Disable 3V3-25MA power source

	//E2PROM control pins
	#define E2PROM_WR_CONTROL_ACTIVE	SET_PINS_LOW(PORTH,PH_EEPROM_WR_CONTROL)	//Enable write on E2PROM
	#define E2PROM_WR_CONTROL_DEACTIVE	SET_PINS_HIGH(PORTH,PH_EEPROM_WR_CONTROL)	//Disable write on E2PROM
	#define E2PROM_ADDR_BIT3_HIGH		SET_PINS_HIGH(PORTJ,PH_EEPROM_E2_ADDR_BIT)
	#define E2PROM_ADDR_BIT3_LOW		SET_PINS_LOW(PORTJ,PH_EEPROM_E2_ADDR_BIT)

	//CC2591 Module Control pins
	#define CC2591_HGM_HIGH				SET_PINS_HIGH(PORTJ,PJ_CC2591_HGM)
	#define CC2591_HGM_LOW				SET_PINS_LOW(PORTJ,PJ_CC2591_HGM)
	#define CC2591_RXTX_HIGH			SET_PINS_HIGH(PORTJ,PJ_CC2591_RXTX)
	#define CC2591_RXTX_LOW				SET_PINS_LOW(PORTJ,PJ_CC2591_RXTX)

	//Battery Controlling pins
	#define BATTERY_TEST_ACTIVATE		SET_PINS_LOW(PORTK,PK_BTY_TEST_EN)
	#define BATTERY_TEST_DEACTIVATE		SET_PINS_HIGH(PORTK,PK_BTY_TEST_EN)
	#define BATTERY_TEST_SEL0_HIGH		SET_PINS_HIGH(PORTK,PK_BTY_TEST_S0)
	#define BATTERY_TEST_SEL0_LOW		SET_PINS_LOW(PORTK,PK_BTY_TEST_S0)
	#define BATTERY_TEST_SEL1_HIGH		SET_PINS_HIGH(PORTK,PK_BTY_TEST_S1)
	#define BATTERY_TEST_SEL1_LOW		SET_PINS_LOW(PORTK,PK_BTY_TEST_S1)

	//Sensor Related
	#define ACTIVATE_SENSOR_RESET				SET_PINS_LOW(PORTK,PK_SENSOR_RESET)						//Activate Reset on connected Smart Sensors
	#define DEACTIVATE_SENSOR_RESET				SET_PINS_HIGH(PORTK,PK_SENSOR_RESET)					//Deactivate Reset on connected Smart Sensors
	#define ENABLE_SMART_SENSOR_SAMPLE_CLOCK	SET_PINS_HIGH(PORTK,PK_SAMPLE_CLOCK)					//Notify Smart Sensors to start sampling
	#define DISABLE_SAMPLING_CLOCK				SET_PINS_LOW(PORTK,PK_SAMPLE_CLOCK)						//Notify Smart sensors to stop after completion of current sampling

	//Smart Sensor Selection Pins
	#define SET_SPID0_HIGH				SET_PINS_HIGH(PORTD,PD_SPID_S0)
	#define SET_SPID0_LOW				SET_PINS_LOW(PORTD,PD_SPID_S0)
	#define SET_SPID1_HIGH				SET_PINS_HIGH(PORTD,PD_SPID_S1)
	#define SET_SPID1_LOW				SET_PINS_LOW(PORTD,PD_SPID_S1)
	#define SET_SPID2_HIGH				SET_PINS_HIGH(PORTD,PD_SPID_S2)
	#define SET_SPID2_LOW				SET_PINS_LOW(PORTD,PD_SPID_S2)

	#define SET_SPIE0_HIGH				SET_PINS_HIGH(PORTE,PE_SPIE_S0)
	#define SET_SPIE0_LOW				SET_PINS_LOW(PORTE,PE_SPIE_S0)
	#define SET_SPIE1_HIGH				SET_PINS_HIGH(PORTE,PE_SPIE_S1)
	#define SET_SPIE1_LOW				SET_PINS_LOW(PORTE,PE_SPIE_S1)
	#define SET_SPIE2_HIGH				SET_PINS_HIGH(PORTE,PE_SPIE_S2)
	#define SET_SPIE2_LOW				SET_PINS_LOW(PORTE,PE_SPIE_S2)

	#define SET_SPIF0_HIGH				SET_PINS_HIGH(PORTJ,PJ_SPIF_S0)
	#define SET_SPIF0_LOW				SET_PINS_LOW(PORTJ,PJ_SPIF_S0)
	#define SET_SPIF1_HIGH				SET_PINS_HIGH(PORTJ,PJ_SPIF_S1)
	#define SET_SPIF1_LOW				SET_PINS_LOW(PORTJ,PJ_SPIF_S1)
	#define SET_SPIF2_HIGH				SET_PINS_HIGH(PORTJ,PJ_SPIF_S2)
	#define SET_SPIF2_LOW				SET_PINS_LOW(PORTJ,PJ_SPIF_S2)

	//_____ G L O B A L   D E F I N I T I O N S ______________________________________________________________

	//Declared in mc_gpio.c
	extern volatile uint8_t		gchDataCommFlagCC1125;

	//_____ F U N C T I O N   D E F I N I T I O N S ____________________________________________

	//_____ fnInitializeGPIO ____________________________________________________________________
	//
	// @brief	Function initialize all the port pins as per hardware requirement.
	//			Following steps are performed to do the same:
	//				1> Individual port pins are configured as input or output on the basis of their functionality.
	//				2> All the general purpose pins configurted as output is initialized for High or Low state on the basis of its role in SENSOR MC.
	//				3> GPIO related interrupts and necessary internal pull up and pull down are also configured.

	void fnInitializeGPIO(void);

#endif /* MC_GPIO_H_ */