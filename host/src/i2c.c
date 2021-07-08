#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "./i2c.h"

struct i2cBusImpl {
	struct i2cBus			obj;

	int						fd;
};


static enum i2cError i2cBusImpl_i2cRelease(
	struct i2cBus* lpBus
) {
	return i2cE_ImplementationError;
}
static enum i2cError i2cBusImpl_i2cRead(
	struct i2cBus* lpBus,
	uint32_t devAddr,
	char* lpOut,
	unsigned long int dwDataLength
) {
	return i2cE_ImplementationError;
}
static enum i2cError i2cBusImpl_i2cWrite(
	struct i2cBus* lpBus,
	uint32_t devAddr,
	char* lpData,
	unsigned long int dwDataLength
) {
	return i2cE_ImplementationError;
}
static enum i2cError i2cBusImpl_i2cScan(
	struct i2cBus* lpBus,
	i2cScan_ResultCallback lpfnCallbackDeviceFound
) {
	return i2cE_ImplementationError;
}

static struct i2cBusVTBL i2cDefaultVTBL = {
	&i2cBusImpl_i2cRelease,
	&i2cBusImpl_i2cRead,
	&i2cBusImpl_i2cWrite,
	&i2cBusImpl_i2cScan
};

static char* i2cDefaultDevices[] = {
	"/dev/iic1"
};

#define i2cDefaultDevices_LEN	sizeof(i2cDefaultDevices)/sizeof(char*)

enum i2cError i2cConnect(
	struct i2cBus** lpOut,
	char* lpBusName
) {
	struct i2cBusImpl* lpNew;

	if(lpOut == NULL) {
		return i2cE_InvalidParam;
	}

	lpNew = (struct i2cBusImpl*)malloc(sizeof(struct i2cBusImpl));
	if(lpNew == NULL) {
		return i2cE_OutOfMemory;
	}

	/* Open device file ... */
	if(lpBusName != NULL) {
		lpNew->fd = open(lpBusName, O_RDWR);
	} else {
		/* We try all known default names if NULL is passed as name ... */
		unsigned long int i;
		for(i = 0; i < i2cDefaultDevices_LEN; i=i+1) {
			if((lpNew->fd = open(i2cDefaultDevices[i], O_RDWR)) >= 0) {
				break;
			}
		}
	}

	if(lpNew->fd < 0) {
		free(lpNew);
		return i2cE_DeviceNotFound;
	}

	lpNew->obj.lpReserved = (void*)lpNew;
	lpNew->obj.vtbl = &i2cDefaultVTBL;

	(*lpOut) = (struct i2cBus*)(&(lpNew->obj));
	return i2cE_Ok;
}
