#ifndef __is_included__2784a374_f0d9_4f7c_9f48_f1ec8f55661d
#define __is_included__2784a374_f0d9_4f7c_9f48_f1ec8f55661d 1

#include "./sysuuid.h"

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

#define PIEZOBOARD_FLAG__BUS_CLOSE_ON_RELEASE						0x00000001

#define PIEZOBOARD_FLAG__VALIDFLAGS											(PIEZOBOARD_FLAG__BUS_CLOSE_ON_RELEASE)

enum piezoboardError {
	piezoE_Ok					= 0,

	piezoE_InvalidParam,
	piezoE_OutOfMemory,
	piezoE_ImplementationError,
};

enum piezoTriggerMode {
	piezoTriggerMode_PiezoVeto			= 0x00,		/* Piezos are triggering, external capacitive or other probe is used as "veto" - signal only triggers if both are active */
	piezoTriggerMode_PiezoOnly			= 0x01,		/* Trigger whenever piezos are triggering independent of external triggers */
	piezoTriggerMode_Capacitive			= 0x02,		/* Only trigger when external sensor triggers (just feed through) */
	piezoTriggerMode_PiezoOrCapacitive	= 0x03,		/* Trigger if any of the sensors triggers */
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
typedef enum piezoboardError (*lpfnPiezoboard_GetTriggerMode)(
	struct piezoboard* lpSelf,
	enum piezoTriggerMode* lpTriggerMode
);
typedef enum piezoboardError (*lpfnPiezoboard_SetTriggerMode)(
	struct piezoboard* lpSelf,
	enum piezoTriggerMode trigMode
);
typedef enum piezoboardError (*lpfnPiezoboard_Reset)(
	struct piezoboard* lpSelf
);
typedef enum piezoboardError (*lpfnPiezoboard_Recalibrate)(
	struct piezoboard* lpSelf
);
typedef enum piezoboardError (*lpfnPiezoboard_StoreSettings)(
	struct piezoboard* lpSelf
);

typedef enum piezoboardError (*lpfnPiezoboard_DebugCurrentSensorReadings)(
	struct piezoboard* lpSelf,
	uint16_t* lpOut[4]
);
typedef enum piezoboardError (*lpfnPiezoboard_DebugCurrentSensorAverages)(
	struct piezoboard* lpSelf,
	uint16_t* lpOut[4]
);


struct piezoboardVtbl {
	lpfnPiezoboard_Release										release;

	lpfnPiezoboard_Identify										identify;

	lpfnPiezoboard_SetThreshold								setThreshold;
	lpfnPiezoboard_GetThreshold								getThreshold;
	lpfnPiezoboard_GetTriggerMode							getTriggerMode;
	lpfnPiezoboard_SetTriggerMode							setTriggerMode;

	lpfnPiezoboard_Reset											reset;
	lpfnPiezoboard_Recalibrate								recalibrate;
	lpfnPiezoboard_StoreSettings							storeSettings;

	lpfnPiezoboard_DebugCurrentSensorReadings	getSensorReadings;
	lpfnPiezoboard_DebugCurrentSensorAverages	getSensorAverages;
};
struct piezoboard {
	struct piezoboardVtbl*								vtbl;
	void*																	lpReserved;
};



enum piezoboardError piezoboardConnect(
	struct piezoboard** lpBoardOut,
	struct i2cBus* lpBus,
	uint8_t boardAddress,
	uint32_t dwFlags
);

#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* #ifndef __is_included__2784a374_f0d9_4f7c_9f48_f1ec8f55661d */
