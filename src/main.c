#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include <math.h>
#include <util/twi.h>
#include <stdint.h>

#include "./main.h"
#include "./sysclk.h"
#include "./i2c.h"
#include "./adc.h"

/*
	Pin mapping

	Arduino pin

	A0	Piezo 1				Analog in				PC0
	A1	Piezo 2				Analog in				PC1
	A2	Piezo 3				Analog in				PC2
	A3	Piezo 4				Analog in				PC3
	9	Digital out			Digital out, push/pull	PB1
	10	Inductive probe in	Digital in				PB2
*/

#ifdef __cplusplus
	extern "C" {
#endif



#ifndef SETTINGS_EEPROM_LOCATION
	#define SETTINGS_EEPROM_LOCATION 0
#endif

struct eepromSettings currentSettings;
static uint32_t debounceCounter;
static unsigned long int debounceStart;

static void eepromSave() {
	unsigned long int i;
	uint8_t chkSum;

	chkSum = 0;
	for(i = 0; i < sizeof(struct eepromSettings)-2; i=i+1) {
		chkSum = chkSum ^ ((char*)(&currentSettings))[i];
	}

	currentSettings.xorChecksum = chkSum;
	currentSettings.negChecksum = (chkSum ^ 0xFF);

	eeprom_write_block(&currentSettings, SETTINGS_EEPROM_LOCATION, sizeof(struct eepromSettings));
}

static void eepromDefaults() {
	currentSettings.trigMode 							= PIEZOBOARD_DEFAULT__TRIGGERMODE;
	currentSettings.movingAverage.thresholdFactor 		= PIEZOBOARD_DEFAULT__THRESHOLD;
	currentSettings.movingAverage.dMovingAverageAlpha 	= PIEZOBOARD_DEFAULT__MOVINGAVERAGEALPHA;
	currentSettings.movingAverage.dwInitSamples 		= PIEZOBOARD_DEFAULT__INITSAMPLES;
	currentSettings.debounceLength 						= PIEZOBOARD_DEFAULT__DEBOUNCELENGTH;

	currentSettings.movingAverageRefCenterline[0]		= 0.0f;
	currentSettings.movingAverageRefCenterline[1]		= 0.0f;
	currentSettings.movingAverageRefCenterline[2]		= 0.0f;
	currentSettings.movingAverageRefCenterline[3]		= 0.0f;

	eepromSave();
}

static void eepromLoad() {
	unsigned long int i;
	uint8_t chkSum;

	eeprom_read_block(&currentSettings, SETTINGS_EEPROM_LOCATION, sizeof(struct eepromSettings));

	chkSum = 0;
	for(i = 0; i < sizeof(struct eepromSettings)-2; i=i+1) {
		chkSum = chkSum ^ ((char*)(&currentSettings))[i];
	}

	if((chkSum == currentSettings.xorChecksum) && ((chkSum ^ 0xFF) == currentSettings.negChecksum)) {
		refCenterline[0] = currentSettings.movingAverageRefCenterline[0];
		refCenterline[1] = currentSettings.movingAverageRefCenterline[1];
		refCenterline[2] = currentSettings.movingAverageRefCenterline[2];
		refCenterline[3] = currentSettings.movingAverageRefCenterline[3];
		return;
	}

	/*
		Initialize to default values and invoke store settings ...
	*/
	eepromDefaults();
}

/*@
	axiomatic hardware_registers {
		axiom valid_PCICR: \valid(&PCICR);
		axiom valid_PCMSK1: \valid(&PCMSK1);
		axiom valid_TCCR0A: \valid(&TCCR0A);
		axiom valid_TCCR0B: \valid(&TCCR0B);
		axiom valid_TCNT0: \valid(&TCNT0);
		axiom valid_TIMSK2: \valid(&TIMSK0);
		axiom valid_OCR0A: \valid(&OCR0A);
		axiom valid_TCCR2A: \valid(&TCCR2A);
		axiom valid_TCCR2B: \valid(&TCCR2B);
		axiom valid_TCNT2: \valid(&TCNT2);
		axiom valid_TIMSK0: \valid(&TIMSK2);
		axiom valid_OCR2A: \valid(&OCR2A);
		axiom valid_PORTB: \valid(&PORTB);	axiom valid_DDRB: \valid(&DDRB);	axiom valid_PINB: \valid(&PINB);
		axiom valid_PORTC: \valid(&PORTC);	axiom valid_DDRC: \valid(&DDRC);	axiom valid_PINC: \valid(&PINC);
		axiom valid_PORTD: \valid(&PORTD);	axiom valid_DDRD: \valid(&DDRD);	axiom valid_PIND: \valid(&PIND);
		axiom valid_UCSR0B: \valid(&UCSR0B);
		axiom valid_TWAR: \valid(&TWAR);	axiom valid_TWCR: \valid(&TWCR);	axiom valid_TWDR: \valid(&TWDR);
	}
*/
/*@
	requires \valid(&TCCR0A) && \valid(&TCCR0B) && \valid(&TIMSK0);
	requires \valid(&PORTB) && \valid(&DDRB);
	requires \valid(&UCSR0B);

	ensures UCSR0B == 0;
*/
int main() {
	systickInit();
	sei();

	#ifdef DEBUG
		/* In debug mode signal bootup via LED */
		DDRB = 0x02;
		PORTB = 0x02;
		delay(1000);
		PORTB = PORTB ^ 0x02;
		delay(1000);
		PORTB = PORTB ^ 0x02;
		delay(1000);
		PORTB = PORTB ^ 0x02;
		delay(1000);
	#endif

	/* PB2 is our output pin ... we do not assert now */
	DDRB = DDRB | 0x02;
	PORTB = PORTB & (~0x02);

	/* PB1 is our input pin without pullup or pulldown */
	DDRB = DDRB & (~0x04);

	/* Disable serial (enabled by bootloader) */
	UCSR0B = 0;

	/* Initialize the I2C port ... */
	i2cSlaveInit(PIEZO_I2C_ADDRESS);

	/* Load settings from EEPROM */
	eepromLoad();

	#if 0
		/*
			This code is used only for debugging purposes and asserts the output.
			The output is deasserted by an accompanying piece of code in the ADC
			ISR.
		*/
		PORTB = PORTB | 0x02;
	#endif

	/* Intiialize ADC */
	adcInit();

	for(;;) {
		i2cMessageLoop();

		switch(currentSettings.trigMode) {
			case triggerMode_PiezoOnly:
			{
				if((adcTriggered != false) && (debounceCounter == 0)) {
					adcTriggered = false;
					PORTB = PORTB | 0x02;
					debounceCounter = currentSettings.debounceLength;
					debounceStart = millis();
				}
				break;
			}
			case triggerMode_PiezoVeto:
			{
				if((adcTriggered != false) && (debounceCounter == 0) && ((PINB & 0x04) != 0)) {
					adcTriggered = false;
					PORTB = PORTB | 0x02;
					debounceCounter = currentSettings.debounceLength;
					debounceStart = millis();
				}
				break;
			}
			case triggerMode_Capacitive:
			{
				if((PINB & 0x04) != 0) {
					PORTB = PORTB | 0x02;
					debounceCounter = currentSettings.debounceLength;
					debounceStart = millis();
				}
				break;
			}
			case triggerMode_PiezoOrCapacitive:
			{
				if(((adcTriggered != false) && (debounceCounter == 0)) || ((PINB & 0x04) != 0)) {
					adcTriggered = false;
					PORTB = PORTB | 0x02;
					debounceCounter = currentSettings.debounceLength;
					debounceStart = millis();
				}
				break;
			}
		}

		if(debounceCounter > 0) {
			unsigned long int debounceEnd;
			unsigned long int milCurrent = millis();
			debounceEnd = debounceStart + debounceCounter;

			if(debounceEnd < debounceStart) {
				if((milCurrent > debounceEnd) && (milCurrent < debounceStart)) {
					debounceCounter = 0;
					PORTB = PORTB & (~0x02);
				}
			} else {
				if((milCurrent > debounceEnd) || (milCurrent < debounceStart)) {
					debounceCounter = 0;
					PORTB = PORTB & (~0x02);
				}
			}
		}
	}
}

static uint8_t handleI2CMessage_Response_IDAndVersion[16+1] = {
	0xca, 0x26, 0x13, 0x06, 0xd7, 0x64, 0x11, 0xeb, 0x94, 0x24, 0xb4, 0x99, 0xba, 0xdf, 0x00, 0xa1,
	0x01
};

/*
	Handle an I2C message. The message is contained in an ringbuffer and it's
	checksum has already been checked (it's not included in the message size).
	The first byte is always the OpCode, the second again the length of the
	message (one shouldn't really use this value)
*/
void handleI2CMessage(
    volatile uint8_t* lpRingbuffer,
    unsigned long int dwBufferSize,

    unsigned long int dwBase,
    unsigned long int dwMessageSize
) {
	uint8_t opCode = lpRingbuffer[dwBase];

	switch(opCode) {
		case i2cCmd_GetIDAndVersion:
		{
			i2cTransmitPacket(handleI2CMessage_Response_IDAndVersion, i2cCmd_GetIDAndVersion, sizeof(handleI2CMessage_Response_IDAndVersion));
			break;
		}
		case i2cCmd_GetThreshold:
		{
			uint8_t bResponse[1];
			bResponse[0] = (uint8_t)(currentSettings.movingAverage.thresholdFactor);
			i2cTransmitPacket(bResponse, i2cCmd_GetThreshold, sizeof(bResponse));
			break;
		}
		case i2cCmd_SetThreshold:
		{
			/* Decode message ... */
			if(dwBufferSize < 3) {
				break; /* Invalid message ... */
			}
			uint8_t bNewThreshold = lpRingbuffer[dwBase+2];

			currentSettings.movingAverage.thresholdFactor = bNewThreshold;
			/* ToDo: Write into EEPROM? */

			break;
		}
		case i2cCmd_ReadCurrentValues:
		{
			uint16_t bufferedCounts[4];

			{
				uint8_t oldSREG = SREG;
				#ifndef FRAMAC_SKIP
					cli();
				#endif

				bufferedCounts[0] = currentADCValues[0];
				bufferedCounts[1] = currentADCValues[1];
				bufferedCounts[2] = currentADCValues[2];
				bufferedCounts[3] = currentADCValues[3];

				SREG = oldSREG;
			}

			uint8_t bResponse[4*2];
			bResponse[0] = ((uint8_t)(bufferedCounts[0] & 0xFF));
			bResponse[1] = ((uint8_t)((bufferedCounts[0] >> 8) & 0xFF));
			bResponse[2] = ((uint8_t)(bufferedCounts[1] & 0xFF));
			bResponse[3] = ((uint8_t)((bufferedCounts[1] >> 8) & 0xFF));
			bResponse[4] = ((uint8_t)(bufferedCounts[2] & 0xFF));
			bResponse[5] = ((uint8_t)((bufferedCounts[2] >> 8) & 0xFF));
			bResponse[6] = ((uint8_t)(bufferedCounts[3] & 0xFF));
			bResponse[7] = ((uint8_t)((bufferedCounts[3] >> 8) & 0xFF));

			i2cTransmitPacket(bResponse, i2cCmd_ReadCurrentValues, sizeof(bResponse));
			break;
		}
		case i2cCmd_ReadCurrentAverages:
		{
			float bufferedAverages[4];
			{
				uint8_t oldSREG = SREG;
				#ifndef FRAMAC_SKIP
					cli();
				#endif

				bufferedAverages[0] = currentMovingAverage[0];
				bufferedAverages[1] = currentMovingAverage[1];
				bufferedAverages[2] = currentMovingAverage[2];
				bufferedAverages[3] = currentMovingAverage[3];

				SREG = oldSREG;
			}

			uint8_t bResponse[4*2];

			bResponse[0] = (uint8_t)((((uint16_t)bufferedAverages[0])     ) & 0xFF);
			bResponse[1] = (uint8_t)((((uint16_t)bufferedAverages[0]) >> 8) & 0xFF);
			bResponse[2] = (uint8_t)((((uint16_t)bufferedAverages[1])     ) & 0xFF);
			bResponse[3] = (uint8_t)((((uint16_t)bufferedAverages[1]) >> 8) & 0xFF);
			bResponse[4] = (uint8_t)((((uint16_t)bufferedAverages[2])     ) & 0xFF);
			bResponse[5] = (uint8_t)((((uint16_t)bufferedAverages[2]) >> 8) & 0xFF);
			bResponse[6] = (uint8_t)((((uint16_t)bufferedAverages[3])     ) & 0xFF);
			bResponse[7] = (uint8_t)((((uint16_t)bufferedAverages[3]) >> 8) & 0xFF);

			i2cTransmitPacket(bResponse, i2cCmd_ReadCurrentAverages, sizeof(bResponse));
			break;
		}
		case i2cCmd_GetTriggerMode:
		{
			uint8_t bResponse[1];
			bResponse[0] = (uint8_t)currentSettings.trigMode;
			i2cTransmitPacket(bResponse, i2cCmd_GetTriggerMode, sizeof(bResponse));
			break;
		}
		case i2cCmd_SetTriggerMode:
		{
			if(dwBufferSize < 3) {
				break; /* Invalid message */
			}
			uint8_t newMode = lpRingbuffer[dwBase + 2];

			switch(newMode) {
				case triggerMode_PiezoOnly:					currentSettings.trigMode = triggerMode_PiezoOnly; 				break;
				case triggerMode_PiezoVeto: 				currentSettings.trigMode = triggerMode_PiezoVeto; 				break;
				case triggerMode_Capacitive: 				currentSettings.trigMode = triggerMode_Capacitive; 				break;
				case triggerMode_PiezoOrCapacitive:	currentSettings.trigMode = triggerMode_PiezoOrCapacitive;	break;
				default: 																																											break; /* Invalid message */
			}
			break;
		}
		case i2cCmd_Reset:
			eepromDefaults();
			adcStartCalibration();
			break;
		case i2cCmd_Recalibrate:
			adcStartCalibration();
			break;
		case i2cCmd_StoreSettings:
			eepromSave();
			break;
		case i2cCmd_GetAlphaValue:
		{
			uint8_t bResponse[1];
			bResponse[0] = (uint8_t)(currentSettings.movingAverage.dMovingAverageAlpha * 100.0);
			i2cTransmitPacket(bResponse, i2cCmd_GetAlphaValue, sizeof(bResponse));
			break;
		}
		case i2cCmd_SetAlphaValue:
		{
			if(dwBufferSize < 3) {
				break; /* Invalid message */
			}
			uint8_t alphaPct = lpRingbuffer[dwBase + 2];
			if(alphaPct > 100) { alphaPct = 100; }

			currentSettings.movingAverage.dMovingAverageAlpha = ((float)alphaPct) / 100.0;
			break;
		}
		default:
			/* Unknown operation - ignore */
			break;
	}
	return;
}


#ifdef __cplusplus
	} /* extern "C" { */
#endif
