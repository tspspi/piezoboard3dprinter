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

void handleI2CMessage(
    volatile uint8_t* lpRingbuffer,
    unsigned long int dwBufferSize,

    unsigned long int dwBase,
    unsigned long int dwMessageSize
) {
	return;
}


#ifdef __cplusplus
	} /* extern "C" { */
#endif
