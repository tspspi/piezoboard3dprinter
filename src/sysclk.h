#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include <util/twi.h>
#include <stdint.h>

#ifndef __cplusplus
	#ifndef true
		typedef int bool;
		#define true 1
		#define false 0
	#endif
#endif

#define SYSCLK_TIMER_OVERFLOW_MICROS	(64L * 256L * (F_CPU / 1000000L))
#define SYSCLK_MILLI_INCREMENT			(SYSCLK_TIMER_OVERFLOW_MICROS / 1000)
#define SYSCLK_MILLIFRACT_INCREMENT		((SYSCLK_TIMER_OVERFLOW_MICROS % 1000) >> 3)
#define SYSCLK_MILLIFRACT_MAXIMUM		(1000 >> 3)


#ifdef __cplusplus
	extern "C" {
#endif

/*@
	requires \valid(&SREG);
	assigns SREG;
	assigns TCCR0A;
	assigns TCCR0B;
	assigns TIMSK0;
*/
void systickInit();

/*@
	assigns TIMSK0;
*/
void systickDisable();

/*@
	requires \valid(&SREG);
	assigns SREG;
*/
unsigned long int millis();

/*@
	requires \valid(&SREG);
	assigns SREG;
*/
unsigned long int micros();

/*@
	requires millisecs >= 0;
	requires \valid(&SREG);
	assigns SREG;
*/
void delay(unsigned long millisecs);

/*@
	requires microDelay >= 13;
	assigns \nothing;
*/
void delayMicros(unsigned int microDelay);
