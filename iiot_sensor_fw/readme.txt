Note: This file containts code snippet of various main functions desgined for various demo test applications.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void fnGayroScopeProtocol(void)
{
	uint8_t		chBuff[10];
	uint8_t		chCounter=0;
	volatile uint32_t lDummyCounter=0;
	
	while(RETURN_TRUE!=fnFetchGyrometerMeasurements(chBuff))
	{
		if(++lDummyCounter>10000000)
		{
			chCounter=1;
			break;
		}
	}
	
	if(!chCounter)
	{
		for(chCounter=0;chCounter<6;chCounter++)
		{
			SEND_HIGH_LEVEL_DEBUG_BYTE(chBuff[chCounter]);
		}
	}
	else
	{
		SEND_DEBUG_STRING("GYROSCOPE FETCH FAIL\n");
	}
	
	fnWait_uSecond(1000);
	MC_GP_LED1_OFF;
	MC_GP_LED0_OFF;
	MC_HEARTBEAT_LED_OFF;
	fnWait_uSecond(1000);
	return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void fnFetchChamberPressure(void)
{
	uint8_t		chDebugMsg[30];		//debug message print
	int8_t		chStatus;
	volatile uint32_t lDummyCounter=0;
	
	ghI2cCommData.chCommBuff[0]='A';
	ghI2cCommData.chCommBuff[1]='A';
	ghI2cCommData.chCommBuff[2]='A';
	ghI2cCommData.chCommBuff[3]='A';
	
	chStatus = fnI2cSendReceiveOperation(PRESSURE_SENSOR_I2C_ADDR,0,4);	//start condition + address send+ master read mode
	if(chStatus == -1)
	{
		SEND_DEBUG_STRING("FAIL\n");
	}
	
	while(fnI2cFreeForOperation()==-1)
	{
		if(++lDummyCounter>1000000)
		{
			break;
		}
	}
	
	ghPressureSensor.chStausByte=(ghI2cCommData.chCommBuff[0]>>6);
	
	if(ghPressureSensor.chStausByte == 0x00)
	{
		ghPressureSensor.nPressureRes= ((uint16_t)(ghI2cCommData.chCommBuff[0] & 0x3f)<<8) | ghI2cCommData.chCommBuff[1];
		ghPressureSensor.nCompansateTempRes= ((uint16_t)ghI2cCommData.chCommBuff[2]<<4) | ((uint16_t)ghI2cCommData.chCommBuff[3]>>4);
		ghPressureSensor.fTempValue= (((float)ghPressureSensor.nCompansateTempRes/2047) * 200) -50;
		ghPressureSensor.fPressureValue= ((float)((ghPressureSensor.nPressureRes - PRESSURE_MIN_RESOLUTION_VALUE) * (PRESSURE_MAX_VALUE - PRESSURE_MIN_VALUE)) / (PRESSURE_MAX_RESOLUTION_VALUE - PRESSURE_MIN_RESOLUTION_VALUE)) + PRESSURE_MIN_VALUE;
		
		fnFloatToString(ghPressureSensor.fPressureValue,chDebugMsg);
		SEND_DEBUG_STRING((char*)chDebugMsg);
		SEND_DEBUG_STRING(" - Pressure Value        ");
		fnFloatToString(ghPressureSensor.fTempValue,chDebugMsg);
		SEND_DEBUG_STRING((char*)chDebugMsg);
		SEND_DEBUG_STRING(" - Temprature Value\n");
	}
	else
	{
		SEND_DEBUG_STRING("Pressure Sensor Fetching Failed\n");
	}
	
	fnWait_uSecond(100);
	MC_GP_LED1_OFF;
	MC_GP_LED0_OFF;
	MC_HEARTBEAT_LED_OFF;
	fnWait_uSecond(100);
	return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//_____fnFetchChamberTemperature ____________________________________________________________________
//
//	This function will return chamber temperature data.
//	@return	-1: If sensor is busy in data conversion or
//			Pure Hex value of converted temperature
//
int16_t fnFetchChamberTemperature(void)
{
	volatile uint16_t nValue;
	volatile float fTemperature;
	volatile float fInAdcValue;
	char	chDebugMsg[30];
	
	//Check if ADCB is in sampling or not?
	if(gchADCBch1Flag == 0)
	{
		return RETURN_FALSE;
	}
	
	nValue=ADCB.CH1.RES;				//ADCB-CH0 input value(In resolution word)
	
	fnFloatToString(nValue,(uint8_t*)chDebugMsg);
	SEND_DEBUG_STRING("Chamber Temp Resolution   : ");
	SEND_DEBUG_STRING(chDebugMsg);
	SEND_DEBUG_STRING("\n");
	
	//For 2048 count -> 2.0625v then voltage at nValue
	fInAdcValue=(2.0625 * ((float)nValue/2048));
	
	fnFloatToString(fInAdcValue,(uint8_t*)chDebugMsg);
	SEND_DEBUG_STRING("Chamber Temp Voltage level: ");
	SEND_DEBUG_STRING(chDebugMsg);
	SEND_DEBUG_STRING("V\n");
	
	//The voltage input to the ADC for a chamber temperature is 4.999 mV / °K
	fTemperature = ((fInAdcValue * 1000)/4.999);				//In kelvin
	fnFloatToString(fTemperature,(uint8_t*)chDebugMsg);
	SEND_DEBUG_STRING("Temperature Value         : ");
	SEND_DEBUG_STRING(chDebugMsg);
	SEND_DEBUG_STRING(" K\n");
	
	//T (°C) = T (K) ? 273.2.
	fTemperature -= 273.2;
	fnFloatToString(fTemperature,(uint8_t*)chDebugMsg);
	SEND_DEBUG_STRING("Temperature Value         : ");
	SEND_DEBUG_STRING(chDebugMsg);
	SEND_DEBUG_STRING("°C\n");
	SEND_DEBUG_STRING("\n");
	
	return nValue;		//return pure hex value 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float fnFetchDistanceSensorValue(void)
{
	//All definitions are used for mathematical operations to convert Analog Output of AR2500 to distance measure value
	volatile uint16_t nValue;
	volatile float fDistance;
	volatile float fInAdcValue;
	char	chDebugMsg[30];
	
	//Check if ADCB is in sampling or not?
	if(fnCheckDistanceSensorStatus())
	{
		return RETURN_FALSE;
	}
	
	nValue=ADCB.CH0.RES;	//ADCB-CH0 input value(In resolution word)
	
	fnFloatToString(nValue,(uint8_t*)chDebugMsg);
	SEND_DEBUG_STRING("AR2500 Resolution: ");
	SEND_DEBUG_STRING(chDebugMsg);
	SEND_DEBUG_STRING("\n");
	
	fInAdcValue=(2.0625 * ((float)nValue/2048)) + lOffset; 
	fnFloatToString(fInAdcValue,(uint8_t*)chDebugMsg);
	SEND_DEBUG_STRING("AR2500 Voltage level: ");
	SEND_DEBUG_STRING(chDebugMsg);
	SEND_DEBUG_STRING("V\n");
	
	//AR2500 provides 4mA-20mA analog output in valid distance measure and ADC can measure the voltage level on ADC input pin
	//So it is mandatory to use Shunt Resistor and use ohm's law (V=IR) to find the current value for voltage level avail on pin (We are using 100 ohm Resistor)
	fInAdcValue=fInAdcValue/lResistance;
	
	fnFloatToString(fInAdcValue,(uint8_t*)chDebugMsg);
	SEND_DEBUG_STRING("AR2500 Current level: ");
	SEND_DEBUG_STRING(chDebugMsg);
	SEND_DEBUG_STRING("A\n");
	
	//AR2500 provides 4mA-20mA analog output in valid distance measure event so if this limit exceeds than it indicates error in sensor measurement operation
	if((fInAdcValue > 0.02009) || (fInAdcValue < 0.00400))
	{
		SEND_DEBUG_STRING("Value exceeds the limit of 0.004A-0.020A\n");
		SEND_DEBUG_STRING("\n");
		return -2;
	}
	
	//Find the distance with the help of analog output current value
	//AR2500 is configured to provide 0.0m-30.0m distance which is directly propositional to 4mA-20mA analog output
	//For more details please refer AR2500 user manual
	fDistance=((fInAdcValue-0.004)*30)/0.016;
	
	fnFloatToString(fDistance,(uint8_t*)chDebugMsg);
	SEND_DEBUG_STRING("AR2500 Distance Value: ");
	SEND_DEBUG_STRING(chDebugMsg);
	SEND_DEBUG_STRING("m\n");
	
	SEND_DEBUG_STRING("\n");
	
	return fDistance;
}

