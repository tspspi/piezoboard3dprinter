/*
	Analog digital module.

	This module is responsible for gathering data for all ADCs. This will
	be done continuously.
*/

ISR(ADC_vect) {
	uint8_t oldMux = ADMUX;

	/*
		Store current raw value in ringbuffer

		On cycle finish set cycle finished flag so synchronous recalculation
		is triggered
	*/

	/* Select next MUX value for after the next iteration */
	ADMUX = (oldMux & 0xE0) | (((oldMux & 0x1F) + 1) & 0x03);
}

void adcInit() {
	uint8_t sregOld = SREG;
	#ifndef FRAMAC_SKIP
		cli();
	#endif
	{
		PRR0 = PRR0 & ~(0x01);
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
