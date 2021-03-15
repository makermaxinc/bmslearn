#ifndef _MCP342X_H_
#define _MCP342X_H_

#include "stm32f3xx.h"

// I2C Address of device
// MCP3421, MCP3425 & MCP3426 are factory programed for any of 0x68 thru 0x6F
#define MCP342X_DEFAULT_ADDRESS	0x6E//0x6E
#define MCP342X_DEFAULT_ADDRESS_LEFT_SHIFTED 0xDC//0xDC

  /**
     * Status of function.
     */
typedef enum {
	MCP_SUCCESS,               /**< The function processed successfully. */
	MCP_ERROR_I2C_READ,        /**< Error related to I2C read. */
	MCP_ERROR_I2C_WRITE,       /**< Error related to I2C write. */
	MCP_ERROR,                 /**< General Error */
} Status;

/**
* Conversion mode setting.
*/
typedef enum {
	CONTINUOUS,            /**< Continuous conversion mode. Default. */
	ONE_SHOT,              /**< One-shot conversion mode. */
} ConversionMode;

/**
 * Data ready status.
 */
typedef enum {
	DATA_NOT_UPDATED,       /**< Output register has not been updated. */
	DATA_UPDATED,           /**< Output register has been updated with the latest conversion result. */
} DataStatus;

/**
 * Measurement trigger command.
 */
typedef enum {
	TRIGGER,              /**< Initiate a new conversion. */
	NONE,                 /**< No effect. */
} MeasurementTrigger;

/**
 * Sample rate and resolution setting.
 */
typedef enum {
	SAMPLE_240HZ_12BIT,       /**< 240 sample per second with 12 bit data. Default. */
	SAMPLE_60HZ_14BIT,        /**< 60 sample per second with 14 bit data. */
	SAMPLE_15HZ_16BIT,        /**< 15 sample per second with 16 bit data. */
} SampleSetting;

/**
 * ADC channel selection.
 */
typedef enum {
	ADC_CH1 = 0,                 /**< Channel 1, default. */
	ADC_CH2 = 1,                 /**< Channel 2 */
	ADC_CH3 = 2,                 /**< Channel 3, MCP3428 only, treated as channel 1 by the MCP3426/MCP3427. */
	ADC_CH4 = 3,                 /**< Channel 4, MCP3428 only, treated as channel 2 by the MCP3426/MCP3427. */
} AdcChannel;

/**
 * Programmable Gain Amplifier setting.
 */
typedef enum {
	PGA_1X,                  /**< Gain 1x, Default. */
	PGA_2X,                  /**< Gain 2x. */
	PGA_4X,                  /**< Gain 4x. */
	PGA_8X,                  /**< Gain 8x. */
} PgaSetting;

/**
 * ADC result.
 */
typedef struct {
	DataStatus st;
	int16_t value;  /**< ADC value. The value takes from -2^11 to (2^11 - 1) when 12 bit sample mode, from -2^13 to (2^13 - 1) when 14 bit sample mode, from -2^15 to (2^15 - 1) when 16bit sample mode. */
} Data;

typedef struct {
	MeasurementTrigger   measurementTrigger;
	DataStatus           dataStatus;
	ConversionMode       conversionMode;
	SampleSetting        sampleSetting;
	AdcChannel           adcChannel;
	PgaSetting           pgaSetting;
} Config;


Status MCP342X(I2C_HandleTypeDef *hi2c);
/**
 * Reads the data registers including the configuration register.
 * @param val Pointer to the buffer which stores ADC value.
 * @param currentConfig Pointer to the structure which stores the current configuration.
 * @return SUCCESS when succeeded. Other value will be returned when error.
 */
Status readData(int16_t *val, Config *config);

/**
 * Sets the configuration register.
 * @param Configuration to be set.
 * @return SUCCESS when succeeded. Other value will be returned when error.
 */
Status setConfig(const Config *config);

/**
 * Decodes a configuration register value and put them into the specified Config structure.
 * @param config Pointer to a Config structure to store the result.
 * @param regVal Register value of the configuration register.
 */
void decodeConfigurationRegister(Config *config, uint8_t regVal);

/**
* Sets a ADC channel.
* @param ch ADC channel which to be the input.
* @return SUCCESS when succeeded. Other value will be returned when error.
*/
Status setChannel(AdcChannel ch);

/**
* Gets the current selected ADC channel.
* @return ADC channel currently set.
*/
AdcChannel getChannel();

/**
* Sets a conversion mode.
* @param mode Conversion mode which to be set.
* @return SUCCESS when succeeded. Other value will be returned when error.
*/
Status setConversionMode(ConversionMode mode);

/**
* Gets the current conversion mode.
* @return Current conversion mode.
*/
ConversionMode getConversionMode();

/**
* Sets sample setting, i.e. sampling frequency and resolution bits.
* @param s Sample setting to be set.
* @return SUCCESS when succeeded. Other value will be returned when error.
*/
Status setSampleSetting(SampleSetting s);

/**
* Gets the current sample setting.
* @return Current sample setting.
*/
SampleSetting getSampleSetting();

/**
* Sets the gain of Programmable Gain Amplifier (PGA).
* @param s PGA seeting to be set.
* @return SUCCESS when succeeded. Other value will be returned when error.
*/
Status setPgaSetting(PgaSetting s);

/**
* Gets the current Programmable Gain Amplifier (PGA) setting.
* @return Current PGA setting.
*/
PgaSetting getPgaSetting();

/**
* Gets the AD value.
* @return AD value.
*/
Status getData(Data *pt);

float convertDataToVoltage(Data data);
float convertDataToDischargeCurrent(Data data);
float convertDataToChargeCurrent(Data data);
float convertDataToTemp(Data data);

/**
* Trigger AD conversion. In continuous measurement mode, this function has no effect.
* @return SUCCESS when succeeded. Other value will be returned when error.
*/
Status trigger();

#endif /* _MCP342X_H_ */
