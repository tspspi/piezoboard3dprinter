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







static struct piezoboardVtbl piezoboardImpl_DefaultVTBL = {
	&piezoboardImpl__Release,

	NULL,

	NULL,
	NULL,
	NULL,
	NULL,

	NULL,
	NULL,
	NULL
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
