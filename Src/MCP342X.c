#include "MCP342X.h"
#include "stm32f3xx_hal.h"

#define LEN_DATA_REGISTER  3

static Config currentConfig;
static I2C_HandleTypeDef *i2c_conn;
uint16_t devAdd = MCP342X_DEFAULT_ADDRESS;
uint32_t devAddLeftShifted = MCP342X_DEFAULT_ADDRESS_LEFT_SHIFTED;
uint32_t timeout = 1500; //ms

/******************************************
 * Default constructor, uses default I2C address.
 * @see MCP342X_DEFAULT_ADDRESS
 */
Status MCP342X(I2C_HandleTypeDef *hi2c) {
	i2c_conn = hi2c;

	Status status;
	int16_t val = 0;
	//Left shift the ADC address as it is needed by the HAL functions
	devAddLeftShifted = (devAdd << 1);

	// Reads the current configuration.
	if ((status=readData(&val, &currentConfig)) != MCP_SUCCESS) {
		return status;
	}

	currentConfig.measurementTrigger = NONE;

	return status;
}

Status readData(int16_t *val, Config *config) {
	uint8_t buf[LEN_DATA_REGISTER];

    //I2C READ DATA
    HAL_StatusTypeDef status = HAL_I2C_Master_Receive(i2c_conn, devAddLeftShifted, buf, LEN_DATA_REGISTER, timeout);

    if(status != HAL_OK)
    {
    	return MCP_ERROR_I2C_READ;
    }

    // Decodes configuration register data.
    decodeConfigurationRegister(config, buf[2]);

    // Converts AD value
    *val = (int16_t)((buf[0] << 8) | buf[1]);

    return MCP_SUCCESS;
}

void decodeConfigurationRegister(Config *config, uint8_t regVal) {
    uint8_t tmp = 0;

    // For the meaning of each bit, see the section 5.2 in the datasheet.

    // Decodes Ready Bit (~RDY), Bit 7
    tmp = ((0x80 & regVal) >> 7);
    if (tmp == 0x00) {
        config->dataStatus = DATA_UPDATED;
    } else {
        config->dataStatus = DATA_NOT_UPDATED;
    }

    // Decodes Channel Selection Bits, Bit 6-5
    tmp = ((0x60 & regVal) >> 5);
    if (tmp == 0x00) {
        config->adcChannel = ADC_CH1;
    } else if (tmp == 0x01) {
        config->adcChannel = ADC_CH2;
    } else if (tmp == 0x02) {
        config->adcChannel = ADC_CH3;
    } else {  //  ch == 0x03
        config->adcChannel = ADC_CH4;
    }

    // Decodes Conversion Mode Bit, Bit 4
    tmp = ((0x10 & regVal) >> 4);
    if (tmp == 0x01) {
        config->conversionMode = CONTINUOUS;
    } else {
        config->conversionMode = ONE_SHOT;
    }

    // Decodes Sample Rate Selection Bit
    tmp = ((0x0C & regVal) >> 2);
    if (tmp == 0x00) {
        config->sampleSetting = SAMPLE_240HZ_12BIT;
    } else if (tmp == 0x01) {
        config->sampleSetting = SAMPLE_60HZ_14BIT;
    } else {
        config->sampleSetting = SAMPLE_15HZ_16BIT;
    }

    // Decodes PGA Gain Selection Bits
    tmp = (0x03 & regVal);
    if (tmp == 0x00) {
        config->pgaSetting = PGA_1X;
    } else if (tmp == 0x01) {
        config->pgaSetting = PGA_2X;
    } else if (tmp == 0x02) {
        config->pgaSetting = PGA_4X;
    } else {
        config->pgaSetting = PGA_8X;
    }
}


Status setChannel(AdcChannel ch) {
    currentConfig.adcChannel = ch;
    return setConfig(&currentConfig);
}

AdcChannel getChannel() {
    return currentConfig.adcChannel;
}

Status setConversionMode(ConversionMode mode) {
    currentConfig.conversionMode = mode;
    return setConfig(&currentConfig);
}

ConversionMode getConversionMode() {
    return currentConfig.conversionMode;
}

Status setSampleSetting(SampleSetting s) {
    currentConfig.sampleSetting = s;
    return setConfig(&currentConfig);
}

SampleSetting getSampleSetting() {
    return currentConfig.sampleSetting;
}

Status setPgaSetting(PgaSetting s) {
    currentConfig.pgaSetting = s;
    return setConfig(&currentConfig);
}

PgaSetting getPgaSetting() {
    return currentConfig.pgaSetting;
}

Status getData(Data *pt) {

	Status status;

	int16_t val = 0;
	if ((status=readData(&val, &currentConfig)) != MCP_SUCCESS) {
		return status;
	}

	pt->st = currentConfig.dataStatus;
	pt->value = val;

    return MCP_SUCCESS;
}

float convertDataToVoltage(Data data)
{
   float voltage=data.value/32767.0;
   voltage=voltage*2.048;
   voltage=voltage/0.3197;
   return voltage;
}

float convertDataToDischargeCurrent(Data data)
{
	float dchgcurrent=data.value*1000.0;
	dchgcurrent=dchgcurrent/32767.0;
	dchgcurrent=dchgcurrent*2.048;
	dchgcurrent=dchgcurrent/0.2; //shunt value
	return dchgcurrent;
}

float convertDataToChargeCurrent(Data data)
{
	float chgcurrent=data.value*1000.0;
	chgcurrent=chgcurrent/32767.0;
	chgcurrent=chgcurrent*2.048;
	chgcurrent=chgcurrent/0.2;  //shunt value
	return chgcurrent;
}

float convertDataToTemp(Data data)
{
	float temp=data.value/32767.0;
	temp=temp*2.048;
	return temp;
}

Status trigger(){
    Status status;

    currentConfig.measurementTrigger = TRIGGER;
    status = setConfig(&currentConfig);
    currentConfig.measurementTrigger = NONE;

    return status;
}

/******************************************
 * Set the configuration shadow register
 */
Status setConfig(const Config *config) {
      uint8_t val = 0;

      // Measurement trigger
      if (config->measurementTrigger == TRIGGER) {
          val |= 0x80;
      } else {
          val |= 0x00;
      }

      // Channel Selection
      if (config->adcChannel == ADC_CH1) {
          val |= 0x00;
      } else if (config->adcChannel == ADC_CH2) {
          val |= 0x20;
      } else if (config->adcChannel == ADC_CH3) {
          val |= 0x40;
      } else {  // config->adcChannel == ADC_CH4
          val |= 0x60;
      }

      // Conversion Mode
      if (config->conversionMode == CONTINUOUS) {
          val |= 0x10;
      } else if (config->conversionMode == ONE_SHOT) {
          val |= 0x00;
      }

      // Sample Rate
      if (config->sampleSetting == SAMPLE_240HZ_12BIT) {
          val |= 0x00;
      } else if (config->sampleSetting == SAMPLE_60HZ_14BIT) {
          val |= 0x04;
      } else { //config->sampleSetting == SAMPLE_15HZ_16BIT
          val |= 0x08;
      }

      // PGA Gain Selection
      if (config->pgaSetting == PGA_1X) {
          val |= 0x00;
      } else if (config->pgaSetting == PGA_2X) {
          val |= 0x01;
      } else if (config->pgaSetting == PGA_4X) {
          val |= 0x02;
      } else { // config->pgaSetting == PGA_8X) {
          val |= 0x03;
      }

      //I2C WRITE DATA
  	  HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(i2c_conn, MCP342X_DEFAULT_ADDRESS_LEFT_SHIFTED, &val, I2C_MEMADD_SIZE_8BIT, 500);

  	  if(status != HAL_OK)
  		  return MCP_ERROR_I2C_READ;

      return MCP_SUCCESS;
  }




