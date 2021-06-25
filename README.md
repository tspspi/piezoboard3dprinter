# Piezo board for 3D printer


## I2C Commands

All I2C commands start with the pattern ```0xAA, 0x55, 0xAA, 0x55```
that is immediatly followed by an OpCode as well as an length byte. The length
includes only data following the Length byte - so a simple request can have
a length of 0. It does _not_ include the checksum byte. One can add an arbitrary
number of ```0xAA, 0x55``` sequences to perform resynchronization.
The whole packet is followed by the checksum byte - a XOR over all bytes should
return 0. The checksum does not include the synchronization pattern but does
include opcode and length.

All packets are protected by a simple XOR based checksum and have fixed length.
Invalid packets will be silently dropped.

| OpCode | Data length | Content | Response data |
| ------ | ----------- | ------- | ------------- |
| 0x01   | 0           | Query device UUID | 8 Byte sync sequence, 16 byte UUID, Checksum |
| 0x02   | 0           | Query current threshold |
| 0x03   | 0           | Query current sensor readings including rolling minimum and maximum |


## I2C Responses
