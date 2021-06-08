# Piezo board for 3D printer


## I2C Commands

All I2C commands start with the pattern ```0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55```
that is immediatly followed by an OpCode. One can add an arbitrary number of ```0xAA, 0x55```
sequences to perform resynchronization.

All packets are protected by a simple XOR based checksum and have fixed length.
Invalid packets will be silently dropped.

| OpCode | Data length | Content | Response data |
| ------ | ----------- | ------- | ------------- |
| 0x01   | 0           | Query device UUID | 8 Byte sync sequence, 16 byte UUID, Checksum |
| 0x02   | 0           | Query current sensitivity threashold |
| 0x03   | 0           | Query current sensor readings including rolling minimum and maximum |


## I2C Responses
