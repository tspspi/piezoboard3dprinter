#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdint.h>

#include "main.h"
#include "adc.h"

/*
	Analog digital module.

	This module is responsible for gathering data for all ADCs. This will
	be done continuously.
*/

extern struct eepromSettings currentSettings;

float refCenterline[4];
uint16_t currentADCValues[4];
float currentMovingAverage[4];
float currentMovingDeviation[4];
unsigned long int adcMovingAverageCapCenterline;

bool adcTriggered; /* This is reset from the main loop but set from our interrupt handler ... */

ISR(ADC_vect) {
	uint8_t oldMux = ADMUX;
	uint8_t sampledValue = ((oldMux & 0x03) + 5) & 0x03;

	if(adcMovingAverageCapCenterline == 0) {
		currentADCValues[sampledValue] = ADC;

		/* Update moving average */
		currentMovingAverage[sampledValue] = ((float)(currentMovingAverage[sampledValue]) * (1.0f - currentSettings.movingAverage.dMovingAverageAlpha) + (float)(currentADCValues[sampledValue]) * currentSettings.movingAverage.dMovingAverageAlpha);
		currentMovingDeviation[sampledValue] = (currentMovingAverage[sampledValue] > refCenterline[sampledValue]) ? currentMovingAverage[sampledValue] - ((float)(refCenterline[sampledValue])) : ((float)(refCenterline[sampledValue])) - currentMovingAverage[sampledValue];
		if(currentMovingDeviation[sampledValue] > currentSettings.movingAverage.thresholdFactor) { adcTriggered = true; }
	} else {
		refCenterline[sampledValue] = refCenterline[sampledValue] + ((float)ADC) / ((float)(currentSettings.movingAverage.dwInitSamples));
	}


	/* Select next MUX value for after the next iteration */
	ADMUX = (oldMux & 0xE0) | (((oldMux & 0x1F) + 1) & 0x03);
}

void adcStartCalibration() {
	uint8_t i;

	for(i = 0; i < sizeof(refCenterline)/sizeof(float); i=i+1) {
		refCenterline[i] = 0;
	}
	adcMovingAverageCapCenterline = currentSettings.movingAverage.dwInitSamples;
}

void adcInit() {
	uint8_t i;

	adcMovingAverageCapCenterline = 0;
	adcTriggered = false;

	for(i = 0; i < 4; i=i+1) {
		refCenterline[i] = 0;
		currentADCValues[i] = 0;
		currentMovingAverage[i] = 0;
		currentMovingDeviation[i] = 0;
	}

	uint8_t sregOld = SREG;
	#ifndef FRAMAC_SKIP
		cli();
	#endif
	{
		PRR = PRR & ~(0x01);
		ADMUX = 0x40; /* AVCC reference voltage, MUX 0, right aligned */
		ADCSRB = 0x00; /* Free running trigger mode, no comparator multiplexed */
		ADCSRA = 0xBF; /* /128 prescaler, interrupts enabled, autotriggering, enable; do not start */
	}
	SREG = sregOld;

	/*
		Now start conversion and advance the MUX value
	*/
	ADCSRA = ADCSRA | 0x40;
	ADMUX = (ADMUX & 0xE0) | ((ADMUX & 0x03) + 1);

	return;
}
