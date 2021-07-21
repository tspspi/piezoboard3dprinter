CPUFREQ=16000000L
FLASHDEV=/dev/ttyU0
I2CADR=0x11
SRCFILES=src/main.c \
	src/sysclk.c \
	src/i2c.c \
	src/adc.c
HEADFILES=src/main.h \
	src/sysclk.h \
	src/i2c.h \
	src/adc.h

all: bin/piezoboard.hex

tmp/piezoboard.bin: $(SRCFILES) $(HEADFILES)

	avr-gcc -Wall -Os -mmcu=atmega328p -DF_CPU=$(CPUFREQ) -DPIEZO_I2C_ADDRESS=$(I2CADR) -o tmp/piezoboard.bin $(SRCFILES)

bin/piezoboard.hex: tmp/piezoboard.bin

	avr-size -t tmp/piezoboard.bin
	avr-objcopy -j .text -j .data -O ihex tmp/piezoboard.bin bin/piezoboard.hex

flash: bin/piezoboard.hex

	sudo chmod 666 $(FLASHDEV)
	avrdude -v -p atmega328p -c avrisp -P $(FLASHDEV) -b 57600 -D -U flash:w:bin/piezoboard.hex:i

framac: $(SRCFILES)

	-rm bin/framacreport.csv
	frama-c -wp-verbose 0 -wp -rte -wp-rte -wp-dynamic -wp-timeout 300 -cpp-extra-args="-I/usr/home/tsp/framaclib/ -DF_CPU=16000000L -D__AVR_ATmega328P__ -DFRAMAC_SKIP" $(SRCFILES) -then -no-unicode -report -report-csv bin/framacreport.csv

clean:

	-rm tmp/*.bin

cleanall: clean

	-rm bin/*.hex
