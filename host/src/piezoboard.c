#include <stdint.h>
#include <stdlib.h>

#include "./i2c.h"
#include "./piezoboard.h"

#ifdef __cplusplus
	extern "C" {
#endif

struct piezoboardImpl {
	struct piezoboard						objBoard;

	struct i2cBus*							lpBus;
	uint32_t										dwFlags;
};

static enum piezoboardError piezoboardImpl__Release(
	struct piezoboard* lpSelf
) {
	struct piezoboardImpl* lpThis;

	if(lpSelf == NULL) { return piezoE_InvalidParam; }

	lpThis = (struct piezoboardImpl*)(lpSelf->lpReserved);

	if((lpThis->dwFlags & PIEZOBOARD_FLAG__BUS_CLOSE_ON_RELEASE) != 0) {
		lpThis->lpBus->vtbl->release(lpThis->lpBus);
		lpThis->lpBus = NULL;
	}

	free(lpThis);
	return piezoE_Ok;
}

static enum piezoboardError piezoboardImpl__Identify(
	struct piezoboard* lpSelf,
	struct sysUuid* lpOut,
	uint8_t* lpVersionOut
) {
	struct piezoboardImpl* lpThis;

	if(lpSelf == NULL) { return piezoE_InvalidParam; }

	lpThis = (struct piezoboardImpl*)(lpSelf->lpReserved);

	return piezoE_ImplementationError;
}
static enum piezoboardError piezoboardImpl__SetThreshold(
	struct piezoboard* lpSelf,
	uint8_t dThreshold
) {
	struct piezoboardImpl* lpThis;

	if(lpSelf == NULL) { return piezoE_InvalidParam; }

	lpThis = (struct piezoboardImpl*)(lpSelf->lpReserved);

	return piezoE_ImplementationError;
}
static enum piezoboardError piezoboardImpl__GetThreshold(
	struct piezoboard* lpSelf,
	uint8_t* lpThreshold
) {
	struct piezoboardImpl* lpThis;

	if(lpSelf == NULL) { return piezoE_InvalidParam; }

	lpThis = (struct piezoboardImpl*)(lpSelf->lpReserved);

	return piezoE_ImplementationError;
}
static enum piezoboardError piezoboardImpl__GetTriggerMode(
	struct piezoboard* lpSelf,
	enum piezoTriggerMode* lpTriggerMode
) {
	struct piezoboardImpl* lpThis;

	if(lpSelf == NULL) { return piezoE_InvalidParam; }

	lpThis = (struct piezoboardImpl*)(lpSelf->lpReserved);

	return piezoE_ImplementationError;
}
static enum piezoboardError piezoboardImpl__SetTriggerMode(
	struct piezoboard* lpSelf,
	enum piezoTriggerMode trigMode
) {
	struct piezoboardImpl* lpThis;

	if(lpSelf == NULL) { return piezoE_InvalidParam; }

	lpThis = (struct piezoboardImpl*)(lpSelf->lpReserved);

	return piezoE_ImplementationError;
}
static enum piezoboardError piezoboardImpl__Reset(
	struct piezoboard* lpSelf
) {
	struct piezoboardImpl* lpThis;

	if(lpSelf == NULL) { return piezoE_InvalidParam; }

	lpThis = (struct piezoboardImpl*)(lpSelf->lpReserved);

	return piezoE_ImplementationError;
}
static enum piezoboardError piezoboardImpl__Recalibrate(
	struct piezoboard* lpSelf
) {
	struct piezoboardImpl* lpThis;

	if(lpSelf == NULL) { return piezoE_InvalidParam; }

	lpThis = (struct piezoboardImpl*)(lpSelf->lpReserved);

	return piezoE_ImplementationError;
}
static enum piezoboardError piezoboardImpl__StoreSettings(
	struct piezoboard* lpSelf
) {
	struct piezoboardImpl* lpThis;

	if(lpSelf == NULL) { return piezoE_InvalidParam; }

	lpThis = (struct piezoboardImpl*)(lpSelf->lpReserved);

	return piezoE_ImplementationError;
}


static enum piezoboardError piezoboardImpl__DebugCurrentSensorReadings(
	struct piezoboard* lpSelf,
	uint16_t* lpOut[4]
) {
	struct piezoboardImpl* lpThis;

	if(lpSelf == NULL) { return piezoE_InvalidParam; }

	lpThis = (struct piezoboardImpl*)(lpSelf->lpReserved);

	return piezoE_ImplementationError;
}
static enum piezoboardError piezoboardImpl__DebugCurrentSensorAverages(
	struct piezoboard* lpSelf,
	uint16_t* lpOut[4]
) {
	struct piezoboardImpl* lpThis;

	if(lpSelf == NULL) { return piezoE_InvalidParam; }

	lpThis = (struct piezoboardImpl*)(lpSelf->lpReserved);

	return piezoE_ImplementationError;
}


static struct piezoboardVtbl piezoboardImpl_DefaultVTBL = {
	&piezoboardImpl__Release,

	&piezoboardImpl__Identify,

	&piezoboardImpl__SetThreshold,
	&piezoboardImpl__GetThreshold,
	&piezoboardImpl__GetTriggerMode,
	&piezoboardImpl__SetTriggerMode,

	&piezoboardImpl__Reset,
	&piezoboardImpl__Recalibrate,
	&piezoboardImpl__StoreSettings,

	&piezoboardImpl__DebugCurrentSensorReadings,
	&piezoboardImpl__DebugCurrentSensorAverages
};

enum piezoboardError piezoboardConnect(
	struct piezoboard** lpBoardOut,
	struct i2cBus* lpBus,
	uint8_t boardAddress,
	uint32_t dwFlags
) {
	struct piezoboardImpl* lpNew;

	if(lpBoardOut == NULL) { return piezoE_InvalidParam; }
	(*lpBoardOut) = NULL;

	if(lpBus == NULL) { return piezoE_InvalidParam; }
	if((dwFlags & (~PIEZOBOARD_FLAG__VALIDFLAGS)) != 0) { return piezoE_InvalidParam; }
	if((boardAddress & 0x80) != 0) { return piezoE_InvalidParam; }

	lpNew = (struct piezoboardImpl*)malloc(sizeof(struct piezoboardImpl));
	if(lpNew == NULL) { return piezoE_OutOfMemory; }

	lpNew->objBoard.vtbl = &piezoboardImpl_DefaultVTBL;
	lpNew->objBoard.lpReserved = (void*)lpNew;
	lpNew->dwFlags = dwFlags;
	lpNew->lpBus = lpBus;

	(*lpBoardOut) = &(lpNew->objBoard);
	return piezoE_Ok;
}


#ifdef __cplusplus
	} /* extern "C" { */
#endif
