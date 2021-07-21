#ifndef __is_included__6da3675c_e5fd_11eb_9c52_70f3950389a2
#define __is_included__6da3675c_e5fd_11eb_9c52_70f3950389a2 1

extern float refCenterline[4];
extern uint16_t currentADCValues[4];
extern float currentMovingAverage[4];
extern float currentMovingDeviation[4];
extern unsigned long int adcMovingAverageCapCenterline;
extern bool adcTriggered; /* This is reset from the main loop but set from our interrupt handler ... */

void adcStartCalibration();
void adcInit();

#endif
