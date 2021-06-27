#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include <util/twi.h>
#include <stdint.h>

#include "./main.h"
#include "./sysclk.h"
#include "./i2c.h"

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

static uint8_t currentThreshold = 0x00;
static uint8_t vetoEnable = 0x01;





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
		DDRB = 0x20;
		PORTB = 0x20;
		delay(1000);
		PORTB = 0x00;
		delay(1000);
		PORTB = 0x20;
		delay(1000);
		PORTB = 0x00;
		delay(1000);
	#endif

	/* Disable serial (enabled by bootloader) */
	UCSR0B = 0;

	/* Initialize the I2C port ... */
	i2cSlaveInit(PIEZO_I2C_ADDRESS);

	for(;;) {
		i2cMessageLoop();
	}
}

static uint8_t handleI2CMessage_Response_IDAndVersion[16+2] = {
	0xca, 0x26, 0x13, 0x06, 0xd7, 0x64, 0x11, 0xeb, 0x94, 0x24, 0xb4, 0x99, 0xba, 0xdf, 0x00, 0xa1,
	0x01,
	0xe8 /* Checksum - update on any change! */
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
			i2cTransmitBytes(handleI2CMessage_Response_IDAndVersion, sizeof(handleI2CMessage_Response_IDAndVersion));
			break;
		}
		case i2cCmd_GetThreshold:
		{
			uint8_t bResponse[2];
			bResponse[0] = currentThreshold;
			bResponse[1] = 0x00 ^ currentThreshold;
			i2cTransmitBytes(bResponse, sizeof(bResponse));
			break;
		}
		case i2cCmd_SetThreshold:
		{
			/* Decode message ... */
			if(dwBufferSize < 3) {
				break; /* Invalid message ... */
			}
			uint8_t bNewThreshold = lpRingbuffer[dwBase+2];

			currentThreshold = bNewThreshold;
			/* ToDo: Write into EEPROM? */

			break;
		}
		case i2cCmd_ReadCurrentValues:
		case i2cCmd_ReadCurrentAverages:
		case i2cCmd_GetVetoEnable:
		{
			uint8_t bResponse[2];
			bResponse[0] = vetoEnable;
			bResponse[1] = 0x00 ^ vetoEnable;
			i2cTransmitBytes(bResponse, sizeof(bResponse));
			break;
		}
		case i2cCmd_SetVetoEnable:
		case i2cCmd_Reset:
		default:
			/* Unknown operation - ignore */
			break;
	}
	return;
}


#ifdef __cplusplus
	} /* extern "C" { */
#endif
