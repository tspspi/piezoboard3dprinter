struct i2cBusImpl {
	struct i2cBus			obj;

	int						fd;
};



static struct i2cBusVTBL i2cDefaultVTBL = {
	NULL,
	NULL,
	NULL,
	NULL
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

	(*lpOut) = (struct i2cBus)(lpNew->obj);
}
