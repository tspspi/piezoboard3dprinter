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
		currentMovingDeviation[sampledValue] = (currentMovingAverage[sampledValue] > refCenterline[sampledValue]) ? currentMovingAverage[sampledValue] - refCenterline[sampledValue] : refCenterline[sampledValue] - currentMovingAverage[sampledValue];
		if(currentMovingDeviation[sampledValue] > currentSettings.movingAverage.thresholdFactor) {
			adcTriggered = true;
		}
	} else {
		refCenterline[sampledValue] = refCenterline[sampledValue] + (float)ADC;
		if(sampledValue == 3) {
			if((adcMovingAverageCapCenterline = adcMovingAverageCapCenterline - 1) == 0) {
				refCenterline[0] = ((float)refCenterline[0]) / (float)(currentSettings.movingAverage.dwInitSamples);
				refCenterline[1] = ((float)refCenterline[1]) / (float)(currentSettings.movingAverage.dwInitSamples);
				refCenterline[2] = ((float)refCenterline[2]) / (float)(currentSettings.movingAverage.dwInitSamples);
				refCenterline[3] = ((float)refCenterline[3]) / (float)(currentSettings.movingAverage.dwInitSamples);
				#if 0
					/*
						This code is used for debug purposes - it pulls the output
						high until the initialization has finished
					*/
					PORTB = PORTB & (~0x02);
				#endif
			}
		}
	}

	/* Select next MUX value for after the next iteration */
	ADMUX = (oldMux & 0xE0) | (((oldMux & 0x1F) + 1) & 0x03);
}

/*@
	assigns refCenterline[0..3];
	assigns adcMovingAverageCapCenterline;

	ensures refCenterline[0..3] == 0;
	ensures adcMovingAverageCapCenterline == currentSettings.movingAverage.dwInitSamples;
*/
void adcStartCalibration() {
	uint8_t i;
	uint8_t sregOld = SREG;
	#ifndef FRAMAC_SKIP
		cli();
	#endif

	/*@
		loop assigns refCenterline[0..3];

		loop invariant 0 <= i < 4;
	*/
	for(i = 0; i < sizeof(refCenterline)/sizeof(float); i=i+1) {
		refCenterline[i] = 0;
	}
	adcMovingAverageCapCenterline = currentSettings.movingAverage.dwInitSamples;

	SREG = sregOld;
}

/*@
	assigns adcMovingAverageCapCenterline;
	assigns adcTriggered;
	assigns refCenterline[0..3];
	assigns currentADCValues[0..3];
	assigns currentMovingAverage[0..3];
	assigns currentMovingDeviation[0..3];
	assigns PRR;
	assigns ADMUX;
	assigns ADCSRB;
	assigns ADCSRA;

	ensures adcMovingAverageCapCenterline == 0;
	ensures adcTriggered == false;
	ensures \forall integer iChannel; 0 <= iChannel < 4
		==> (refCenterline[iChannel] == 0) && (currentADCValues[iChannel] == 0) && (currentMovingAverage[iChannel] == 0) && (currentMovingDeviation[iChannel] == 0);
	ensures (PRR & 0x01) == 0;
	ensures ADMUX == 0x41;
	ensures ADCSRB == 0x00;
	ensures ADCSRA == (0xBF | 0x40);
*/
void adcInit() {
	uint8_t i;

	adcMovingAverageCapCenterline = 0;
	adcTriggered = false;

	/*@
		loop assigns refCenterline[0..3];
		loop assigns currentADCValues[0..3];
		loop assigns currentMovingAverage[0..3];
		loop assigns currentMovingDeviation[0..3];

		loop invariant 0 <= i < 4;
	*/
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
		/* In case we have no previous calibration start a new calibration sequence */
		if((refCenterline[0] == 0.0f) && (refCenterline[1] == 0.0f) && (refCenterline[2] == 0.0f) && (refCenterline[3] == 0.0f)) {
			adcStartCalibration();
		}

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
