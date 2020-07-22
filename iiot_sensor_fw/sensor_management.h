/* -------------------------------------------------------------------------
Filename: sensor_management.h

Job#: 20473
Purpose: all the sensors related functionality
Date Created: 11/04/2014

(NOTE: latest version is the top version)

Author: , Aalok Shah
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

#ifndef SENSOR_MANAGEMENT_H_
#define SENSOR_MANAGEMENT_H_

	//_____ I N C L U D E S ______________________________________________________________

	#include "hardware_abstraction_layer.h"			//Contains headers of hardware dependent programming functionality

	//_________________ M A C R O S _______________________________________________
	
	#define SPI_SMART_SENSOR_COMM_BUF_SIZE		40

	//I2C Addresses
	//E2PROM ADDRESSES ARE VARIABLE (BUT ALWAYS SET E2 BIT HIGH)
	#define PRESSURE_SENSOR_I2C_ADDR			0x28
	#define GYRO_METER_I2C_ADDR					0x59		//R48 is connected on SENSOR MC Hardware
	#define E2PROM_MEMORY_I2C_ADDR				0x54		//A16=A17=0
	#define E2PROM_PAGE_I2C_ADDR				0x5C		//A16=A17=0

	//Only useful register addresses are mentioned in here for communicating with the Gyroscope
	//Gyroscope is used in Normal Mode- Values will be read from Gyro Data Register
	#define GYRO_MAX21000_DEVICE_ID				0x20
	#define GYRO_MAX21000_BANK_SEL				0x21
	#define GYRO_MAX21000_SYS_STATUS			0x22
	#define GYRO_MAX21000_SENSE_CNFG0			0x00
	#define GYRO_MAX21000_SENSE_CNFG1			0x01
	#define GYRO_MAX21000_SENSE_CNFG2			0x02
	#define GYRO_MAX21000_SENSE_CNFG3			0x03
	#define GYRO_MAX21000_DR_IFG				0x13
	#define GYRO_MAX21000_IO_CFG				0x14
	#define GYRO_MAX21000_I2C_CFG				0x15
	#define GYRO_MAX21000_DATA_START_ADDR		0x23

	//Pressure Sensor
	#define PRESSURE_MAX_RESOLUTION_VALUE		14745		//90% of 2^14 (14bit Resolution of Pressure Sensor)
	#define PRESSURE_MIN_RESOLUTION_VALUE		1638		//10% of 2^14 (14bit Resolution of Pressure Sensor)
	#define PRESSURE_MAX_VALUE					100
	#define PRESSURE_MIN_VALUE					0

	//Smart Sensor Scenarios
	#define MAX_SMART_SENSOR_GROUPS				3
	#define SMART_SENSOR_SPID_GROUP				0
	#define SMART_SENSOR_SPIE_GROUP				1
	#define SMART_SENSOR_SPIF_GROUP				2

	//SPID (SPID0-SPID4)
	#define SMART_SENSOR_SPID0				0x01
	#define SMART_SENSOR_SPID1				0x02
	#define SMART_SENSOR_SPID2				0x03
	#define SMART_SENSOR_SPID3				0x04
	#define SMART_SENSOR_SPID4				0x05

	//SPIE (SPIE0-SPIE4)
	#define SMART_SENSOR_SPIE0				0x01
	#define SMART_SENSOR_SPIE1				0x02
	#define SMART_SENSOR_SPIE2				0x03
	#define SMART_SENSOR_SPIE3				0x04
	#define SMART_SENSOR_SPIE4				0x05

	//SPIF (SPIF0-SPIF4)
	#define SMART_SENSOR_SPIF0				0x01
	#define SMART_SENSOR_SPIF1				0x02
	#define SMART_SENSOR_SPIF2				0x03
	#define SMART_SENSOR_SPIF3				0x04
	#define SMART_SENSOR_SPIF4				0x05
	#define SMART_SENSOR_SPIF5				0x06

	//_____ D A T A   S T R U C T U R E S _________________________________________________

	//Holds sensor static details including data length and sensor ID of all the supported sensors on SENSOR MC
	typedef struct
	{
		uint8_t chSensorID;
		uint8_t chSensorDataLen;
	}SENSOR_DETAILS;
	
	//_____ E N U M E R A T I O N S _________________________________________________
	
	//List of sensors Supported in system
	typedef enum
	{
		CHAMBER_TEMPERATURE=1,
		CHAMBER_PRESSURE,
		BATTERY_TEMPERATURE,
		TEG_TEMPERATURE_COLD,
		TEG_TEMPERATURE_HOT,
		UPLINK_RADIO_RSSI,
		GYRO_METER,
		RANGE_MEASUREMENT,
		BATTERY_STAVE_B1_UNLOADED=16,
		BATTERY_STAVE_B1_LOADED,
		BATTERY_STAVE_B2_UNLOADED,
		BATTERY_STAVE_B2_LOADED,
		BATTERY_STAVE_B3_UNLOADED,
		BATTERY_STAVE_B3_LOADED,
		BATTERY_STAVE_B4_UNLOADED,
		BATTERY_STAVE_B4_LOADED
	}SENSOR_MC_SENSOR_LIST;

	//_____ G L O B A L   D E F I N I T I O N S ______________________________________________________________

	extern SENSOR_MC_SENSOR_LIST ghSensorList;
	
	//Shared step indexes to perform I2C communication among various Sensors
	extern volatile uint8_t gchLvl1StepIndexI2C;
	extern volatile uint8_t gchLvl2StepIndexI2C;
	extern volatile uint8_t gchStepIndexE2PROM;

	//_____ F U N C T I O N   D E F I N I T I O N S ___________________________________________________________

	//_____ fnFetchSensorDataLength ____________________________________________________________________
	//
	// @brief	It will perform linear search in all the entries of SENSOR_DETAILS to find out the data length of the sensor passed in argument
	// @param	chSensorID		Sensor ID for which searching for the data length
	// @return	FALSE if no entry found for given Sensor ID otherwise returns data length specific to provided Sensor ID
	
	int8_t fnFetchSensorDataLength(uint8_t chSensorID);

	//_____ fnCheckI2cAvailability ____________________________________________________________________
	//
	// @brief It will just call	lower level function fnCheckI2CStatus() to find out I2C availability for operation

	int8_t fnCheckI2cAvailability(void);

	//_____ fnFetchChamberPressure ____________________________________________________________________
	//
	// @brief	This function will fetch the pressure value by communicating to sensor over I2C interface
	// @return	FALSE if pressure data are not fetched successfully otherwise returns Pressure Data fetched from sensor
	
	int16_t fnFetchChamberPressure(void);

	//_____ fnInitializeGyrometer ____________________________________________________________________
	//
	// @brief	It will initialize the Gyrometer for operations
	//			At power on Gyroscope is not operational so this function must be execution to instruct the Gyroscope for start sampling
	// @return	FASLE if initialization procedure is not completed successfully otherwise returns TRUE
	
	int8_t fnInitializeGyrometer(void);

	//_____ fnFetchGyrometerMeasurements ____________________________________________________________________
	//
	// @brief	It will fetch current measurements of Gyroscope
	//			fnInitializeGyrometer() function must be executed once before calling this function so that Gyroscope can start sampling the real time data
	//			This function will check for the Gyroscope status to avoid mismatch in data fetching
	// @return	FASLE if Gyroscope data fetching is not completed successfully otherwise returns TRUE
	
	int8_t fnFetchGyrometerMeasurements(uint16_t *pnBuff);

	//__________fnStartChamberTemperature______________________
	//
	// @brief	It will call the lower level ADC function to start sampling the ADC input of chamber temperature sensor
	// @return	FALSE if no ADC resources are free otherwise returns TRUE
	
	int8_t fnStartChamberTemperature(void);

	//_____ fnFetchChamberTemperature ____________________________________________________________________
	//
	// @brief	This function will fetch the sampled chamber temperature data from ADC
	//			fnStartChamberTemperature() must be execued first to start sampling for chamber temperature
	// @return	FALSE if data sampling is running otherwise sampled data value
	//			In case of mismatch it will return 0 as sampled data
	
	int16_t fnFetchChamberTemperature(void);

	//_____ fnE2PROMWriteOperation ____________________________________________________________________
	//
	// @brief	Use this function to write block of data sequentially in to E2PROM
	// @param	pchBuff		Pointer to the memory resources to hold the data bytes needed to write in E2PROM
	//			chLength	Specifies value of count for which to perform write operation
	//			nAddress	Base address in E2PROM from where to perform sequential write of data
	// @return	FALSE if operation is not completed otherwise sampled data value
	//			In the case of overflow function will return SENSOR_I2C_COMM_BUFF_OVERFLOW
	
	SENSOR_MC_ERROR_CODES fnE2PROMWriteOperation(uint8_t *pchBuff, uint8_t chLength, uint16_t nAddress);

	//_____ fnE2PROMReadOpeartion ____________________________________________________________________
	//
	// @brief	Use this function to read block of data sequentially from E2PROM
	// @param	pchBuff		Pointer to the memory resources to hold the data bytes after reading from the E2PROM
	//			chLength	Specifies value of count for which to perform read operation
	//			nAddress	Base address in E2PROM from where to perform sequential read of data
	// @return	FALSE if operation is not completed otherwise sampled data value
	//			In the case of overflow function will return SENSOR_I2C_COMM_BUFF_OVERFLOW
	
	SENSOR_MC_ERROR_CODES fnE2PROMReadOpeartion(uint8_t *pchBuff, uint8_t chLength, uint16_t nAddress);

	//_____ fnStartSmartSensorSampling ____________________________________________________________________
	//
	// @brief	This function will notify the Smart Sensor to start sampling with the provided sensor tasking table
	//			fnSetResetSmartSensorSelectlines() must needs to be executed to make communication path (SPI interface) between MCU and Smart Sensor through various select lines
	//			?????
	// @param	chSensorGroup	It indicates the group/interface to which this Sensor is connected
	//			chSensorValue	It indicates the Sensor to which communication will be established
	// @return FALSE if Sensor Group or Sensor Value is invalid otherwise returns TRUE
	
	int8_t fnStartSmartSensorSampling(uint8_t chSensorGroup,uint8_t chSensorValue);

	//_____ fnSmartSensorsDataCollection ____________________________________________________________________
	//
	// @brief	This function will fetch the sampled data from the Smart Sensor
	//			fnSetResetSmartSensorSelectlines() must needs to be executed to make communication path (SPI interface) between MCU and Smart Sensor through various select lines
	//			?????
	// @param	chSensorGroup	It indicates the group/interface to which this Sensor is connected
	//			chSensorValue	It indicates the Sensor to which communication will be established
	//			pchSampledData	Pointer to memory resources where wants to put the sample data after receiving
	//			chSampledlength	No of sampled data to fetch from the Smart Sensor
	// @return FALSE if Sensor Group or Sensor Value is invalid otherwise returns TRUE
	
	int8_t fnSmartSensorsDataCollection(uint8_t chSensorGroup,uint8_t chSensorValue,uint8_t *pchSampledData,uint8_t chSanpledlength);

#endif /* SENSOR_MANAGEMENT_H_ */