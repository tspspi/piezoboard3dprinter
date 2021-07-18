#ifndef __is_included__2784a374_f0d9_4f7c_9f48_f1ec8f55661d
#define __is_included__2784a374_f0d9_4f7c_9f48_f1ec8f55661d 1

#ifdef __cplusplus
	extern "C" {
#endif

#ifndef __cplusplus
	#ifndef true
		typedef uint8_t bool;
		#define true 1
		#define false 0
	#endif
#endif

enum piezoboardError {
	piezoE_Ok					= 0,

	piezoE_InvalidParam,
};

struct piezoboard;
struct piezoboardVtbl;

typedef enum piezoboardError (*lpfnPiezoboard_Release)(
	struct piezoboard* lpSelf
);
typedef enum piezoboardError (*lpfnPiezoboard_Identify)(
	struct piezoboard* lpSelf,
	struct sysUuid* lpOut,
	uint8_t* lpVersionOut
);
typedef enum piezoboardError (*lpfnPiezoboard_SetThreshold)(
	struct piezoboard* lpSelf,
	uint8_t dThreshold
);
typedef enum piezoboardError (*lpfnPiezoboard_GetThreshold)(
	struct piezoboard* lpSelf,
	uint8_t* lpThreshold
);






enum piezoboardError piezoboardConnect(
	struct i2cBus* lpBus,
	uint8_t boardAddress
);

#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* #ifndef __is_included__2784a374_f0d9_4f7c_9f48_f1ec8f55661d */
