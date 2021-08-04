#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef DEBUG
	#include <stdio.h>
#endif

#include "./i2c.h"
#include "./piezoboard.h"

#ifdef __cplusplus
	extern "C" {
#endif

enum piezoboardImpl_OpCode {
	opCode_GetIdAndVersion					= 0x01,
	opCode_GetThreshold						= 0x02,
	opCode_SetThreshold						= 0x03,
	opCode_ReadCurrentValues				= 0x04,
	opCode_ReadCurrentAverages				= 0x05,
	opCode_SetTriggerMode					= 0x06,
	opCode_GetTriggerMode					= 0x07,
	opCode_Reset							= 0x08,
	opCode_Recalibrate						= 0x09,
};

struct piezoboardImpl {
	struct piezoboard						objBoard;

	struct i2cBus*							lpBus;
	uint8_t									devAddress;
	uint32_t								dwFlags;
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

static uint8_t piezoboardImpl__Identify__Command[] = { 0xAA, 0x55, 0xAA, 0x55, opCode_GetIdAndVersion, 0x00, 0x01 };
static enum piezoboardError piezoboardImpl__Identify(
	struct piezoboard* lpSelf,
	struct sysUuid* lpOut,
	uint8_t* lpVersionOut
) {
	struct piezoboardImpl* lpThis;
	enum i2cError ei2c;
	uint8_t bResponse[16+1+4+2+1];

	if(lpSelf == NULL) { return piezoE_InvalidParam; }

	lpThis = (struct piezoboardImpl*)(lpSelf->lpReserved);

	/* Write request */
	ei2c = lpThis->lpBus->vtbl->write(lpThis->lpBus, lpThis->devAddress, piezoboardImpl__Identify__Command, sizeof(piezoboardImpl__Identify__Command));
	if(ei2c != i2cE_Ok) {
		#ifdef DEBUG
			printf("%s:%u Write failed (%u)\n", __FILE__, __LINE__, ei2c);
		#endif
		return piezoE_Failed;
	}

	/* Read response */
	ei2c = lpThis->lpBus->vtbl->read(lpThis->lpBus, lpThis->devAddress, bResponse, sizeof(bResponse));
	if(ei2c != i2cE_Ok) {
		#ifdef DEBUG
			printf("%s:%u Read failed (%u)\n", __FILE__, __LINE__, ei2c);
		#endif
		return piezoE_Failed;
	}

	/* validate response and write outputs ... */
	if((bResponse[0] != 0xAA) || (bResponse[1] != 0x55) || (bResponse[2] != 0xAA) || (bResponse[3] != 0x55) || (bResponse[4] != opCode_GetIdAndVersion) || (bResponse[5] != (sizeof(bResponse)-5))) {
		#ifdef DEBUG
			printf("%s:%u Packet format error\n", __FILE__, __LINE__);
		#endif
		return piezoE_CommunicationError;
	}

	/* Verify checksum */
	{
		uint8_t chkSum = 0x00;
		for(unsigned long int i = 4; i < sizeof(bResponse); i=i+1) {
			chkSum = chkSum ^ bResponse[i];
		}
		if(chkSum != 0x00) {
			#ifdef DEBUG
				printf("%s:%u Checksum format error\n", __FILE__, __LINE__);
			#endif
			return piezoE_ChecksumError;
		}
	}

	/* decode and write output */
	if(lpVersionOut != NULL) { (*lpVersionOut) = bResponse[6+16]; }
	if(lpOut != NULL) {
		memcpy((void*)lpOut, &(bResponse[6]), 16);
	}

	return piezoE_Ok;
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
	lpNew->devAddress = boardAddress;
	lpNew->dwFlags = dwFlags;
	lpNew->lpBus = lpBus;

	(*lpBoardOut) = &(lpNew->objBoard);
	return piezoE_Ok;
}


#ifdef __cplusplus
	} /* extern "C" { */
#endif
