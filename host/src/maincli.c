#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "./i2c.h"
#include "./piezoboard.h"

int main(int argc, char* argv[]) {
	struct i2cBus* lpBus;
	struct piezoboard* lpPzb;
	enum i2cError ei2c;
	enum piezoboardError e;

	ei2c = i2cConnect(&lpBus, NULL);
	if(ei2c != i2cE_Ok) {
		printf("%s:%u Failed to connect with I2C device (%u)\n", __FILE__, __LINE__, ei2c);
		return 1;
	}

	e = piezoboardConnect(&lpPzb, lpBus, 0x11, 0);
	if(e != piezoE_Ok) {
		printf("%s:%u Failed to attach piezo driver to I2C device (%u)\n", __FILE__, __LINE__, e);
		lpBus->vtbl->release(lpBus);
		return 1;
	}


	lpPzb->vtbl->release(lpPzb);
	lpBus->vtbl->release(lpBus);
	return 0;
}
