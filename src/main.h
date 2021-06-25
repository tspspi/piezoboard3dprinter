#if !defined(__cplusplus) && !defined(FRAMAC_SKIP)
	typedef int bool;
	#define true 1
	#define false 0
#endif

#ifdef __cplusplus
    extern "C" {
#endif

void handleI2CMessage(
    volatile uint8_t* lpRingbuffer,
    unsigned long int dwBufferSize,

    unsigned long int dwBase,
    unsigned long int dwMessageSize
);

#ifdef __cplusplus
    } /* extern "C" { */
#endif
