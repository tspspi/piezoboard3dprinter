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

| OpCode | Data length | Content                                                                         | Response data                                                 |
| ------ | ----------- | ------------------------------------------------------------------------------- | ------------------------------------------------------------- |
| 0x01   | 0           | Query device UUID                                                               | 16 byte UUID, Checksum                                        |
| 0x02   | 0           | Query current threshold                                                         | 1 Byte current threshold, 1 Byte checksum                     |
| 0x03   | 1           | Set current threshold                                                           | None                                                          |
| 0x04   | 0           | Query current sensor readings (raw data)                                        | 8 byte sensor readings (2 bytes per channel), 1 byte checksum |
| 0x05   | 0           | Query current running averages                                                  | 8 byte sensor readings (2 bytes per channel), 1 byte checksum |
| 0x06   | 1           | Set trigger mode (0: Piezos with veto, 1: Only Piezo, 2: Use capacitive probe)  | None                                                          |
| 0x07   | 0           | Get trigger mode                                                                | 1 Byte data, 1 Byte checksum                                  |
| 0x08   | 0           | Reset board                                                                     | None                                                          |
| 0x09   | 0           | Calibrate centerline for piezos                                                 | None                                                          |
| 0x0A   | 0           | Store settings to EEPROM                                                        | None                                                          |
