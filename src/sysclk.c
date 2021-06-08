#include "./sysclk.h"

#ifdef __cplusplus
	extern "C" {
#endif
/*
	System tick timer
*/

volatile unsigned long int systemMillis					= 0;
volatile unsigned long int systemMilliFractional		= 0;
volatile unsigned long int systemMonotonicOverflowCnt	= 0;

/*@
	assigns systemMillis, systemMilliFractional, systemMonotonicOverflowCnt;
*/
ISR(TIMER0_OVF_vect) {
	unsigned long int m, f;

	m = systemMillis;
	f = systemMilliFractional;

	m = m + SYSCLK_MILLI_INCREMENT;
	f = f + SYSCLK_MILLIFRACT_INCREMENT;

	if(f >= SYSCLK_MILLIFRACT_MAXIMUM) {
		f = f - SYSCLK_MILLIFRACT_MAXIMUM;
		m = m + 1;
	}

	systemMonotonicOverflowCnt = systemMonotonicOverflowCnt + 1;

	systemMillis = m;
	systemMilliFractional = f;
}

/*
	Millis function used to calculate time delays
*/
/*@
	requires \valid(&SREG);
	requires \valid(&systemMillis);
	assigns SREG;
*/
unsigned long int millis() {
	unsigned long int m;

	/*
		Note that this is a hack.
		systemMillis is a multi-byte value so we disable interrupts to read
		consistently BUT this is implementation dependent on the compiler
	*/
	uint8_t srOld = SREG;
	#ifndef FRAMAC_SKIP
		cli();
	#endif
	m = systemMillis;
	SREG = srOld;

	return m;
}

unsigned long int micros() {
	uint8_t srOld = SREG;
	unsigned long int overflowCounter;
	unsigned long int timerCounter;

	#ifndef FRAMAC_SKIP
		cli();
	#endif
	overflowCounter = systemMonotonicOverflowCnt;
	timerCounter = TCNT0;

	/*
		Check for pending overflow that has NOT been handeled up to now
	*/
	if(((TIFR0 & 0x01) != 0) && (timerCounter < 255)) {
		overflowCounter = overflowCounter + 1;
	}

	SREG = srOld;

	return ((overflowCounter << 8) + timerCounter) * (64L / (F_CPU / 1000000L));
}

void delay(unsigned long millisecs) {
	//uint16_t lastMicro;
	unsigned int lastMicro;
	/*
		Busy waiting the desired amount of milliseconds ... by
		polling mircos
	*/
	lastMicro = (unsigned int)micros();
	/*@
		loop assigns lastMicro;
		loop assigns millisecs;
	*/
	while(millisecs > 0) {
		// uint16_t curMicro = (uint16_t)micros();
		unsigned int curMicro = micros();
		if(curMicro - lastMicro >= 1000)  {
			/* Every ~ thousand microseconds tick ... */
			lastMicro = lastMicro + 1000;
			millisecs = millisecs - 1;
		}
	}
	return;
}

void delayMicros(unsigned int microDelay) {
	#if F_CPU == 20000000L
		/*
			Burn two additional cycles - together with function
			calling overhead of avr-gcc and the subtraction below
			this should lead to 1 us delay (see assembly output!)
		*/
		__asm__ __volatile__ (
			"nop\n"
			"nop\n"
		);
		if((microDelay = microDelay - 1) == 0) {
			return;
		}

		/*
			Multiply by 5 - loop below takes 4 cycles.
			One cycle takes 1/20000000 seconds i.e. 5e-8 s i.e. 0.05 us.
			One loop iteration burns 4 cycles i.e. 0.2 us
			So we require 5 loop iterations per loop to reach 1 us
		*/
		microDelay = (microDelay << 2) + microDelay;
	#elif F_CPU == 16000000L
		/*
			Function calling, subtraction and conditional
			branch should be equal to approx. 1us (see assembly
			output to fine-tune).
		*/
		if((microDelay = microDelay - 1) == 0) {
			return;
		}

		/*
			Each cycle takes 1/16000000 seconds i.e. 6.25e-8 s i.e. 0.0625 us.
			One loop iteration burns 4 cycles i.e. 0.25 us
			So we require 4 loop iterations to reach 1 us
			This calculation takes us approx. 0.5 us (see assembly
			output to fine tune)
		*/
		microDelay = (microDelay << 2) - 2;
	#elif F_CPU == 8000000L
		if((microDelay = microDelay - 1) == 0) {
			return;
		}
		if((microDelay = microDelay - 1) == 0) {
			return;
		}

		/*
			Each loop iteration burns 0.5us,
			the calculation takes approx 0.5us
			(see assembly output to fine-tune again)
		*/
		microDelay = (microDelay << 1) - 1;
	#else
		#error No known delay loop calibration available for this F_CPU
	#endif

	/*
		Busy waiting loop.
		Takes 4 cycles. Micro Delay has been modified above
	*/
	#ifndef FRAMAC_SKIP
		/*@
			assigns microDelay;
			ensures microDelay == 0;
		*/
		__asm__ __volatile__ (
			"lp: sbiw %0, 1\n"
			"    brne lp"
			: "=w" (microDelay)
			: "0" (microDelay)
		);
	#else
		/*@
			loop assigns microDelay;
			loop invariant 0 <= microDelay;
		*/
		while(microDelay > 0) {
			microDelay = microDelay - 1;
		}
	#endif
	/*@ ghost  */
	return;
}

void systickInit() {
	uint8_t sregOld = SREG;

	/*
		Setup system clock (used for delay, etc.)
		We will use Timer0
	*/
	#ifndef FRAMAC_SKIP
		cli();
	#endif

	TCCR0A = 0x00;
	TCCR0B = 0x03;		/* /64 prescaler */
	TIMSK0 = 0x01;		/* Enable overflow interrupt */

	SREG = sregOld;
}

void systickDisable() {
	TIMSK0 = 0x00;
}

#ifdef __cplusplus
	} /* extern "C" { */
#endif
