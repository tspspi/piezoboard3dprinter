#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include <util/twi.h>
#include <stdint.h>

#include "./main.h"
#include "./i2c.h"

/*
	I2C buffered I/O
*/

static volatile uint8_t i2cBufferRX[I2C_BUFFER_SIZE_RX];
static volatile int i2cBufferRX_Head = 0;
static volatile int i2cBufferRX_Tail = 0;

static volatile uint8_t i2cBufferTX[I2C_BUFFER_SIZE_TX];
static volatile int i2cBufferTX_Head = 0;
static volatile int i2cBufferTX_Tail = 0;

/*@
	assigns \nothing;
*/
static inline void i2cEventBusError() {
	// Currently we force a reset by using the watchdog after 1s delay
	return;
}

/*@
	requires i2cBufferTX_Tail >= 0;
	requires i2cBufferTX_Tail < STEPPER_I2C_BUFFERSIZE_TX;
	behavior bufferUnderrun:
		assumes i2cBufferTX_Head == i2cBufferTX_Tail;
		assigns \nothing;
	behavior bufferDefault:
		assumes i2cBufferTX_Head != i2cBufferTX_Tail;
		assigns i2cBufferTX_Tail;
		assigns i2cBufferTX[i2cBufferTX_Tail];
		ensures i2cBufferTX_Tail >= 0;
		ensures i2cBufferTX_Tail < I2C_BUFFER_SIZE_TX;
		ensures i2cBufferTX_Tail == (\old(i2cBufferTX_Tail) + 1) % I2C_BUFFER_SIZE_TX;
	complete behaviors;
	disjoint behaviors;
*/
static inline uint8_t i2cEventTransmit() {
	if(i2cBufferTX_Head == i2cBufferTX_Tail) {
		/* Empty buffer - buffer underrun ... ToDo */
		return 0x00;
	} else {
		uint8_t r = i2cBufferTX[i2cBufferTX_Tail];
		i2cBufferTX_Tail = (i2cBufferTX_Tail + 1) % I2C_BUFFER_SIZE_TX;
		return r;
	}
}

/*@
	requires i2cBufferRX_Head >= 0;
	requires i2cBufferRX_Head < I2C_BUFFER_SIZE_RX;
	behavior bufferOverflow:
		assumes (i2cBufferRX_Head + 1) % I2C_BUFFER_SIZE_RX == i2cBufferRX_Tail;
		assigns \nothing;
	behavior bufferAvail:
		assumes (i2cBufferRX_Head + 1) % I2C_BUFFER_SIZE_RX != i2cBufferRX_Tail;
		assigns i2cBufferRX_Head;
		assigns i2cBufferRX[i2cBufferRX_Head];
		ensures i2cBufferRX_Head >= 0;
		ensures i2cBufferRX_Head < I2C_BUFFER_SIZE_RX;
		ensures i2cBufferRX_Head == \old((i2cBufferRX_Head + 1) % I2C_BUFFER_SIZE_RX);
	disjoint behaviors;
	complete behaviors;
*/
static inline void i2cEventReceived(uint8_t data) {
	// Do whatever we want with the received data
	if(((i2cBufferRX_Head + 1) % I2C_BUFFER_SIZE_RX) == i2cBufferRX_Tail) {
		// Buffer overflow. ToDo
		return;
	}
	i2cBufferRX[i2cBufferRX_Head] = data;
	i2cBufferRX_Head = (i2cBufferRX_Head + 1) % I2C_BUFFER_SIZE_RX;
}

void i2cSlaveInit(uint8_t address) {
	#ifndef FRAMAC_SKIP
		cli();
	#endif

	TWAR = (address << 1) | 0x00; // Respond to general calls and calls towards us
	TWCR = 0xC5; // Set TWIE (TWI Interrupt enable), TWEN (TWI Enable), TWEA (TWI Enable Acknowledgement), TWINT (Clear TWINT flag by writing a 1)

	#ifndef FRAMAC_SKIP
		sei();
	#endif
	return;
}

/*@
	requires \valid_read(&TWSR) && \valid_read(&TWDR) && \valid(&TWDR) && \valid(&TWCR);
	ensures TWCR == 0xC5;
*/
ISR(TWI_vect) {
	switch(TW_STATUS) { /* Note: TW_STATUS is an macro that masks status bits from TWSR) */
		case TW_SR_SLA_ACK:
			/*
				Slave will read, slave has been addresses and address
				has been acknowledged
			*/
			break;
		case TW_SR_DATA_ACK:
			/*
				We have received data. This is now contained in the TWI
				data register (TWDR)
			*/
			i2cEventReceived(TWDR);
			break;
		case TW_ST_SLA_ACK:
		case TW_ST_DATA_ACK:
			/*
				Either slave selected (SLA_ACK) and data requested or data
				transmitted, ACK received and next data requested
			*/
			TWDR = i2cEventTransmit();
			break;
		case TW_BUS_ERROR:
			i2cEventBusError();
			break;
		default:
			break;
	}
	TWCR = 0xC5; // Set TWIE (TWI Interrupt enable), TWEN (TWI Enable), TWEA (TWI Enable Acknowledgement), TWINT (Clear TWINT flag by writing a 1)
}

/*
    Synchronous message loop
*/
void i2cMessageLoop() {
    unsigned long int i;

    uint8_t rcvBytes = (i2cBufferRX_Tail <= i2cBufferRX_Head) ? (i2cBufferRX_Head - i2cBufferRX_Tail) : (I2C_BUFFER_SIZE_RX - i2cBufferRX_Tail + i2cBufferRX_Head);

    if(rcvBytes < 5) {
        return; /* Nothing has been received */
    }

    /* check for next sync pattern ... */
    while(rcvBytes > 4) {
        if((i2cBufferRX[i2cBufferRX_Tail] == 0xAA)
            && (i2cBufferRX[(i2cBufferRX_Tail+1) % I2C_BUFFER_SIZE_RX] == 0x55)
            && (i2cBufferRX[(i2cBufferRX_Tail+2) % I2C_BUFFER_SIZE_RX] == 0xAA)
            && (i2cBufferRX[(i2cBufferRX_Tail+3) % I2C_BUFFER_SIZE_RX] == 0x55)
            && (i2cBufferRX[(i2cBufferRX_Tail+4) % I2C_BUFFER_SIZE_RX] != 0xAA)) {
            /* Discovered start of packet */
            break;
        }

        /* Moving on till we discover the sync pattern ... */
        rcvBytes = rcvBytes - 1;
        i2cBufferRX_Tail = (i2cBufferRX_Tail + 1) % I2C_BUFFER_SIZE_RX;
    }
    if(rcvBytes <= 6) {
        return; /* Inconclusive */
    }

    /*
        Possible packet present ... check if it's fully available. If not
        continue waiting.

        Layout:
            +0              0xAA
            +1              0x55
            +2              0xAA
            +3              0x55
            +4              OpCode
            +5              Length
            ...
            +(5 + len + 1)  Checksum
    */
    uint8_t requiredPacketLength = 4 + 2 + 1 + i2cBufferRX[(i2cBufferRX_Tail + 5) % I2C_BUFFER_SIZE_RX];
    if(rcvBytes < requiredPacketLength) {
        return; /* We have to wait longer ... */
    }

    /*
		Perform checksum check ...

		The checksum includes OpCode and Length but not the synchronization pattern
		since the sync pattern could have an indefinite length
	*/
    uint8_t chksum = 0x00;
    for(i = 4; i < requiredPacketLength; i=i+1) {
        chksum = chksum ^ i2cBufferRX[(i2cBufferRX_Tail + i) % I2C_BUFFER_SIZE_RX];
    }

    if(chksum != 0) {
        /*
            Drop packet and resync ... ToDo: Should we count
            checksum mismatches or provide some kind of notification?

            We simply skip the synchronization pattern - the sync loop above
            will perform resynchronization anyways just in case there is
			a packet start somewhere shifted in between ...
        */
        i2cBufferRX_Tail = (i2cBufferRX_Tail + 4) % I2C_BUFFER_SIZE_RX;
        return;
    }

    /*
        We got a full packet that's correctly checksummed ...
    */
    handleI2CMessage(i2cBufferRX, I2C_BUFFER_SIZE_RX, i2cBufferRX_Tail+4, requiredPacketLength-4-1);
    i2cBufferRX_Tail = i2cBufferRX_Tail + requiredPacketLength; /* Drop data */
}

void i2cTransmitBytes(
    uint8_t* lpMessage,
    unsigned long int dwLength
) {
    unsigned long int i;

    if(lpMessage == 0) { return; }
    if(dwLength == 0) { return; }

    /*
        Check capacity
    */
    unsigned long int dwBufferedBytes = (i2cBufferTX_Tail <= i2cBufferTX_Head) ? (i2cBufferTX_Head - i2cBufferTX_Tail) : (I2C_BUFFER_SIZE_TX - i2cBufferTX_Tail + i2cBufferTX_Head);
    unsigned long int dwCapacity = I2C_BUFFER_SIZE_TX - dwBufferedBytes;

    if(dwLength > dwCapacity) { return; }

    for(i = 0; i < dwLength; i=i+1) {
        i2cBufferTX[i2cBufferTX_Head] = lpMessage[i];
        i2cBufferTX_Head = (i2cBufferTX_Head + 1) % I2C_BUFFER_SIZE_TX;
    }

    return;
}

void i2cTransmitPacket(
	uint8_t* lpPacket,
	uint8_t bOpCode,
	unsigned long int dwPayloadLength
) {
	unsigned long int i;
	uint8_t bChecksum = 0x00;

	i2cBufferTX[ i2cBufferTX_Head                        ] = 0xAA;
	i2cBufferTX[(i2cBufferTX_Head+1) % I2C_BUFFER_SIZE_TX] = 0x55;
	i2cBufferTX[(i2cBufferTX_Head+2) % I2C_BUFFER_SIZE_TX] = 0xAA;
	i2cBufferTX[(i2cBufferTX_Head+3) % I2C_BUFFER_SIZE_TX] = 0x55;
	i2cBufferTX[(i2cBufferTX_Head+4) % I2C_BUFFER_SIZE_TX] = bOpCode;
	i2cBufferTX[(i2cBufferTX_Head+5) % I2C_BUFFER_SIZE_TX] = ((uint8_t)dwPayloadLength+2); /* Includes opcode and length field */

	bChecksum = bChecksum ^ bOpCode;
	bChecksum = bChecksum ^ ((uint8_t)dwPayloadLength+2);

	for(i = 0; i < dwPayloadLength; i=i+1) {
		i2cBufferTX[(i2cBufferTX_Head+6+i) % I2C_BUFFER_SIZE_TX] = lpPacket[i];
		bChecksum = bChecksum ^ lpPacket[i];
	}

	i2cBufferTX[(i2cBufferTX_Head+6+i) % I2C_BUFFER_SIZE_TX] = bChecksum;

	return;
}
