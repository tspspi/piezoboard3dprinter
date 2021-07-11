#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <dev/iicbus/iic.h>

#include "./i2c.h"

struct i2cBusImpl {
	struct i2cBus			obj;

	int						fd;
};


static enum i2cError i2cBusImpl_i2cRelease(
	struct i2cBus* lpBus
) {
	struct i2cBusImpl* lpThis;

	if(lpBus == NULL) {
		return i2cE_InvalidParam;
	}

	lpThis = (struct i2cBusImpl*)(lpBus->lpReserved);

	close(lpThis->fd);
	free(lpThis);

	return i2cE_Ok;
}
static enum i2cError i2cBusImpl_i2cRead(
	struct i2cBus* lpBus,
	uint32_t devAddr,
	uint8_t* lpOut,
	unsigned long int dwDataLength
) {
	struct i2cBusImpl* lpThis;
	struct iic_msg msg;
	struct iic_rdwr_data rdwr;

	if(lpBus == NULL) {
		return i2cE_InvalidParam;
	}

	lpThis = (struct i2cBusImpl*)(lpBus->lpReserved);

	msg.slave = devAddr << 1;
	msg.flags = IIC_M_RD;
	msg.len   = dwDataLength;
	msg.buf   = lpOut;

	rdwr.msgs = &msg;
	rdwr.nmsgs = 1;

	if(ioctl(lpThis->fd, I2CRDWR, &rdwr) < 0)  {
		return i2cE_Failed;
	}

	return i2cE_Ok;
}
static enum i2cError i2cBusImpl_i2cWrite(
	struct i2cBus* lpBus,
	uint32_t devAddr,
	uint8_t* lpData,
	unsigned long int dwDataLength
) {
	struct i2cBusImpl* lpThis;
	struct iic_msg msg;
	struct iic_rdwr_data rdwr;

	if(lpBus == NULL) {
		return i2cE_InvalidParam;
	}

	lpThis = (struct i2cBusImpl*)(lpBus->lpReserved);

	msg.slave = devAddr << 1;
	msg.flags = 0;
	msg.len   = dwDataLength;
	msg.buf   = lpData;

	rdwr.msgs = &msg;
	rdwr.nmsgs = 1;

	if(ioctl(lpThis->fd, I2CRDWR, &rdwr) < 0) {
		return i2cE_Failed;
	}

	return i2cE_Ok;
}
static enum i2cError i2cBusImpl_i2cScan(
	struct i2cBus* lpBus,
	i2cScan_ResultCallback lpfnCallbackDeviceFound
) {
	struct i2cBusImpl* lpThis;
	uint8_t buf[2] = { 0, 0 };
	unsigned long int i;
	struct iic_msg msg[2];
	struct iic_rdwr_data rdwr;

	if(lpBus == NULL) {
		return i2cE_InvalidParam;
	}

	lpThis = (struct i2cBusImpl*)(lpBus->lpReserved);

	msg[0].flags = !IIC_M_RD;
	msg[0].len   = sizeof(buf);
	msg[0].buf   = buf;

	msg[1].flags = IIC_M_RD;
	msg[1].len  = sizeof(buf);
	msg[1].buf  = buf;

	rdwr.nmsgs = 2;

	for(i = 1; i < 128; i++) {
		// Set address
		msg[0].slave = i << 1;
		msg[1].slave = i << 1;

		rdwr.msgs = msg;
		if(ioctl(lpThis->fd, I2CRDWR, &rdwr) >= 0) {
			lpfnCallbackDeviceFound(lpBus, i);
		}
	}

	return i2cE_Ok;
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
