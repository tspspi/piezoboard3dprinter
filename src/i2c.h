#ifndef I2C_BUFFER_SIZE_RX
	#define I2C_BUFFER_SIZE_RX 64
#endif
#ifndef I2C_BUFFER_SIZE_TX
	#define I2C_BUFFER_SIZE_TX 64
#endif

#ifndef PIEZO_I2C_ADDRESS
	#define PIEZO_I2C_ADDRESS 0x11
#endif

#ifdef __cplusplus
    extern "C" {
#endif

enum i2cCommand {
	i2cCmd_GetIDAndVersion						= 1,

	i2cCmd_GetThreshold							= 2,
	i2cCmd_SetThreshold							= 3,

	i2cCmd_ReadCurrentValues					= 4,
	i2cCmd_ReadCurrentAverages					= 5,

	i2cCmd_SetTriggerMode						= 6,
	i2cCmd_GetTriggerMode						= 7,

	i2cCmd_Reset								= 8,
	i2cCmd_Recalibrate							= 9,

	i2cCmd_StoreSettings						= 10,

	i2cCmd_GetAlphaValue						= 11,
	i2cCmd_SetAlphaValue						= 12,
};

/*@
	requires \valid(&SREG) && \valid(&TWAR) && \valid(&TWCR);
	assigns SREG;
	assigns TWAR, TWCR;
	ensures TWCR == 0xC5;
	ensures (TWAR == address << 1) || (TWAR == ((address << 1) | 0x01));
*/
void i2cSlaveInit(
	uint8_t address
);

void i2cTransmitBytes(
	uint8_t* lpMessage,
	unsigned long int dwLength
);

void i2cTransmitPacket(
	uint8_t* lpPacket,
	uint8_t bOpCode,
	unsigned long int dwPayloadLength
);

void i2cQueuePreamble();

void i2cMessageLoop();

#ifdef __cplusplus
    } /* extern "C" { */
#endif
