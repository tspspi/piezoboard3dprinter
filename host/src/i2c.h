#ifndef __is_included__c8d7190b_df68_11eb_ba9a_70f3950389a2
#define __is_included__c8d7190b_df68_11eb_ba9a_70f3950389a2 1

enum i2cError {
	i2cE_Ok					= 0,

	i2cE_InvalidParam,
	i2cE_OutOfMemory,
	i2cE_Failed,
	i2cE_DeviceNotFound,
};

struct i2cBus;
struct i2cBusVTBL;

typedef enum i2cError (*i2cRelease)(
	struct i2cBus* lpBus
);
typedef enum i2cError (*i2cRead)(
	struct i2cBus* lpBus,
	uint32_t devAddr,
	char* lpOut,
	unsigned long int dwDataLength
);
typedef enum i2cError (*i2cWrite)(
	struct i2cBus* lpBus,
	uint32_t devAddr,
	char* lpData,
	unsigned long int dwDataLength
);
typedef void (*i2cScan_ResultCallback)(
	struct i2cBus* lpBus,
	uint32_t devAddr
);
typedef enum i2cError (*i2cScan)(
	struct i2cBus* lpBus,
	i2cScan_ResultCallback lpfnCallbackDeviceFound
);

struct i2cBusVTBL {
	i2cRelease				release;
	i2cRead					read;
	i2cWrite				write;
	i2cScan					scan;
};
struct i2cBus {
	struct i2cBusVTBL*		vtbl;
	void*					lpReserved;
};

enum i2cError i2cConnect(
	struct i2cBus** lpOut,
	char* lpBusName
);





#endif /* #ifndef __is_included__c8d7190b_df68_11eb_ba9a_70f3950389a2 */
