#if !defined(__cplusplus) && !defined(true)
	typedef int bool;
	#define true 1
	#define false 0
#endif

#ifndef PIEZOBOARD_DEFAULT__TRIGGERMODE
	#define PIEZOBOARD_DEFAULT__TRIGGERMODE triggerMode_PiezoOrCapacitive
#endif
#ifndef PIEZOBOARD_DEFAULT__THRESHOLD
	#define PIEZOBOARD_DEFAULT__THRESHOLD 10
#endif
#ifndef PIEZOBOARD_DEFAULT__MOVINGAVERAGEALPHA
	#define PIEZOBOARD_DEFAULT__MOVINGAVERAGEALPHA 0.4f
#endif
#ifndef PIEZOBOARD_DEFAULT__INITSAMPLES
	#define PIEZOBOARD_DEFAULT__INITSAMPLES 100
#endif
#ifndef PIEZOBOARD_DEFAULT__DEBOUNCELENGTH
	#define PIEZOBOARD_DEFAULT__DEBOUNCELENGTH 125
#endif

#ifdef __cplusplus
    extern "C" {
#endif

enum triggerMode {
	triggerMode_PiezoVeto					= 0x00,		/* Trigger whenever piezos trigger and the external probe is active */
	triggerMode_PiezoOnly					= 0x01,		/* Ignore external probe and only trigger on piezos */
	triggerMode_Capacitive					= 0x02,		/* Only trigger on external probe and ignore piezos */
	triggerMode_PiezoOrCapacitive			= 0x03,		/* In case any probe is active trigger - piezo or external (default mode for failsafe fallback operation) */
};

struct eepromSettings {
	enum triggerMode						trigMode;
	struct {
		uint32_t							thresholdFactor;			/* ADC counts that moving average has to be above the centerline */
		float								dMovingAverageAlpha; 	/* Transmitted * 1000 over the wire */
		uint32_t							dwInitSamples; 				/* Number of samples to use for centerline measurement */
	} movingAverage;
	uint16_t								debounceLength;



	/* These two have to be the last bytes */
	uint8_t 								xorChecksum;				/* All previous bytes xor'ed */
	uint8_t									negChecksum; 				/* Negated checksum */
};

void handleI2CMessage(
    volatile uint8_t* lpRingbuffer,
    unsigned long int dwBufferSize,

    unsigned long int dwBase,
    unsigned long int dwMessageSize
);

#ifdef __cplusplus
    } /* extern "C" { */
#endif
