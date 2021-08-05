#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "./i2c.h"
#include "./piezoboard.h"

static void printfUUID(struct sysUuid* lpUUID) {
	printf(
		"%08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x",
		lpUUID->p1,
		lpUUID->p2[0],
		lpUUID->p2[1],
		lpUUID->p2[2],
		lpUUID->p3[0],
		lpUUID->p3[1],
		lpUUID->p3[2],
		lpUUID->p3[3],
		lpUUID->p3[4],
		lpUUID->p3[5]
	);
}

static void printUsage(int argc, char* argv[]) {
	printf("Usage: %s [OPTIONS] [COMMANDS]\n\n", argv[0]);

	printf("Supported options:\n");
	printf("\t-port FILENAME\n");
	printf("\t\tSets the used I2C port (ex.: /dev/iic1)\n");
	printf("\t-tries NUMBER\n");
	printf("\t\tSets the number of retries (default 3)\n");

	printf("\nSupported commands:\n");

	printf("\tid\n\t\tIdentifies the board ID and version\n");

	printf("\tgetth\n\t\tGet the current set threshold\n");
	printf("\tsetth THRESHOLD\n\t\tSet the current threshold (integer)\n");

	printf("\tgettrig\n\t\tGet current trigger mode\n");
	printf("\tsettrig MODE\n\t\tSets the current trigger mode\n");
	printf("\t\t0\tPiezo with external veto\n");
	printf("\t\t1\tOnly piezo disks\n");
	printf("\t\t2\tOnly external trigger\n");
	printf("\t\t3\tPiezo or external trigger\n");

	printf("\tgetalpha\n\t\tGet the current alpha value\n");
	printf("\tsetalpha THRESHOLD\n\t\tSet the current alpha value (integer 0-100)\n");

	printf("\trst\n\t\tReset the board and erase EEPROM\n");
	printf("\tcal\n\t\tExecute recalibration for baseline\n");

	printf("\tst\n\t\tStore settings in EEPROM\n");
}

int main(int argc, char* argv[]) {
	struct i2cBus* lpBus;
	struct piezoboard* lpPzb;
	enum i2cError ei2c;
	enum piezoboardError e;

	unsigned long int i;

	char* lpPortName = NULL;
	unsigned long int dwRetryCount = 3;

	if(argc < 2) {
		printUsage(argc, argv);
		return 1;
	}

	/*
		Command line parsing.

		This is done in two steps:
			* First we do check if there is a supplied port (like /dev/)
			  or a specification for retries. If so we appply these independent
			  of the position on the commandline
			* All other commands are then processed in a loop later on.
	*/
	for(i = 1; i < argc; i=i+1) {
		if(strcmp(argv[i], "-port") == 0) {
			if(argc <= (i+1)) { printf("Missing port name\n"); printUsage(argc, argv); return 1; }
			lpPortName = argv[i+1];
			i = i + 1;
		} else if(strcmp(argv[i], "-tries") == 0) {
			if(argc <= (i+1)) { printf("Missing number of retries\n"); printUsage(argc, argv); return 1; }
			if(sscanf(argv[i+1], "%lu", &dwRetryCount) != 1) { printf("Invalid retry count %s\n", argv[i+1]); printUsage(argc, argv); return 1; }
			i = i + 1;
		} else if(strcmp(argv[i], "id") == 0) { continue; }
		else if(strcmp(argv[i], "getth") == 0) { continue; }
		else if(strcmp(argv[i], "setth") == 0) { i = i + 1; continue; }
		else if(strcmp(argv[i], "getalpha") == 0) { continue; }
		else if(strcmp(argv[i], "setalpha") == 0) { i = i + 1; continue; }
		else if(strcmp(argv[i], "gettrig") == 0) { continue; }
		else if(strcmp(argv[i], "settrig") == 0) { i = i + 1; continue; }
		else if(strcmp(argv[i], "rst") == 0) { continue; }
		else if(strcmp(argv[i], "cal") == 0) { continue; }
		else if(strcmp(argv[i], "st") == 0) { continue; }
		else {
			printf("Unknown command %s\n", argv[i]);
			printUsage(argc, argv);
			return 1;
		}
	}

	ei2c = i2cConnect(&lpBus, lpPortName);
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

	int r = 0;

	for(i = 1; i < argc; i=i+1) {
		if(strcmp(argv[i], "-port") == 0) {
			i = i + 1;
			continue;
		} else if(strcmp(argv[i], "-tries") == 0) {
			i = i + 1;
			continue;
		} else if(strcmp(argv[i], "id") == 0) {
			struct sysUuid lpBoardUUID;
			uint8_t boardVersion;

			e = lpPzb->vtbl->identify(lpPzb, &lpBoardUUID, &boardVersion);
			if(e != piezoE_Ok) {
				printf("%s:%u Failed to query board UUID and version (%u)\n", __FILE__, __LINE__, e);
				r = 2;
				break;
			}

			printf("Board UUID: "); printfUUID(&lpBoardUUID); printf("\n");
			printf("Board Version: %u\n", boardVersion);

			usleep(100*1000);
		} else if(strcmp(argv[i], "getth") == 0) {
			uint8_t currentThreshold;

			e = lpPzb->vtbl->getThreshold(lpPzb, &currentThreshold);
			if(e != piezoE_Ok) {
				printf("%s:%u Failed to query current threshold (%u)\n", __FILE__, __LINE__, e);
				r = 2;
				break;
			}

			printf("Current threshold: %u\n", currentThreshold);

			usleep(100*1000);
		} else if(strcmp(argv[i], "setth") == 0) {
			unsigned long int readValue;
			uint8_t nextThreshold;

			if(argc <= (i+1)) {
				printf("Missing new threshold value\n");
				printUsage(argc, argv);
				r = 1;
				break;
			}
			if(sscanf(argv[i+1], "%lu", &readValue) != 1) {
				printf("Invalid new threshold value %s\n", argv[i+1]);
				printUsage(argc, argv);
				r = 1;
				break;
			}
			if(readValue > 255) {
				printf("Invalid new threshold value %lu\n", readValue);
				printUsage(argc, argv);
				r = 1;
				break;
			}
			nextThreshold = (uint8_t)readValue;

			e = lpPzb->vtbl->setThreshold(lpPzb, nextThreshold);
			if(e != piezoE_Ok) {
				printf("%s:%u Failed to set threshold to %u (code %u)\n", __FILE__, __LINE__, nextThreshold, e);
				r = 2;
				break;
			}

			printf("Set new threshold value %u\n", nextThreshold);
			i = i + 1;

			usleep(100*1000);
		} else if(strcmp(argv[i], "getalpha") == 0) {
			uint8_t currentAlpha;

			e = lpPzb->vtbl->getAlpha(lpPzb, &currentAlpha);
			if(e != piezoE_Ok) {
				printf("%s:%u Failed to query current alpha (%u)\n", __FILE__, __LINE__, e);
				r = 2;
				break;
			}

			printf("Current alpha: %u\n", currentAlpha);

			usleep(100*1000);
		} else if(strcmp(argv[i], "setalpha") == 0) {
			unsigned long int readValue;
			uint8_t nextAlpha;

			if(argc <= (i+1)) {
				printf("Missing new alpha value\n");
				printUsage(argc, argv);
				r = 1;
				break;
			}
			if(sscanf(argv[i+1], "%lu", &readValue) != 1) {
				printf("Invalid new alpha value %s\n", argv[i+1]);
				printUsage(argc, argv);
				r = 1;
				break;
			}
			if(readValue > 100) {
				printf("Invalid new alpha value %lu\n", readValue);
				printUsage(argc, argv);
				r = 1;
				break;
			}
			nextAlpha = (uint8_t)readValue;

			e = lpPzb->vtbl->setAlpha(lpPzb, nextAlpha);
			if(e != piezoE_Ok) {
				printf("%s:%u Failed to set alpha to %u (code %u)\n", __FILE__, __LINE__, nextAlpha, e);
				r = 2;
				break;
			}

			printf("Set new alpha value %u\n", nextAlpha);
			i = i + 1;

			usleep(100*1000);
		} else if(strcmp(argv[i], "gettrig") == 0) {
			enum piezoTriggerMode currentTriggerMode;

			e = lpPzb->vtbl->getTriggerMode(lpPzb, &currentTriggerMode);
			if(e != piezoE_Ok) {
				printf("%s:%u Failed to query current trigger mode (%u)\n", __FILE__, __LINE__, e);
				r = 2;
				break;
			}

			switch(currentTriggerMode) {
				case piezoTriggerMode_PiezoVeto:			printf("Trigger mode: Piezo with veto\n"); break;
				case piezoTriggerMode_PiezoOnly:			printf("Trigger mode: Piezo only\n"); break;
				case piezoTriggerMode_Capacitive:			printf("Trigger mode: External only\n"); break;
				case piezoTriggerMode_PiezoOrCapacitive:	printf("Trigger mode: Piezo or external\n"); break;
				default:									printf("Trigger mode: Unknown\n"); break;
			}

			usleep(100*1000);
		} else if(strcmp(argv[i], "settrig") == 0) {
			unsigned long int readValue;
			enum piezoTriggerMode newMode;

			if(argc <= (i+1)) {
				printf("Missing new trigger mode value\n");
				printUsage(argc, argv);
				r = 1;
				break;
			}
			if(sscanf(argv[i+1], "%lu", &readValue) != 1) {
				printf("Invalid new trigger mode value %s\n", argv[i+1]);
				printUsage(argc, argv);
				r = 1;
				break;
			}

			switch(readValue) {
				case 0:			newMode = piezoTriggerMode_PiezoVeto; break;
				case 1:			newMode = piezoTriggerMode_PiezoOnly; break;
				case 2:			newMode = piezoTriggerMode_Capacitive; break;
				case 3:			newMode = piezoTriggerMode_PiezoOrCapacitive; break;
				default:
				{
					printf("Unknown trigger mode %lu\n", readValue);
					printUsage(argc, argv);
					r = 1;
					break;
				}
			}
			if(r == 1) { break; }

			e = lpPzb->vtbl->setTriggerMode(lpPzb, newMode);
			if(e != piezoE_Ok) {
				printf("%s:%u Failed to set trigger modde to %u (code %u)\n", __FILE__, __LINE__, newMode, e);
				r = 2;
				break;
			}

			printf("Set new trigger mode value %u\n", newMode);
			i = i + 1;

			usleep(100*1000);
		} else if(strcmp(argv[i], "rst") == 0) {
			e = lpPzb->vtbl->reset(lpPzb);
			if(e != piezoE_Ok) {
				printf("%s:%u Failed to perform reset (%u)\n", __FILE__, __LINE__, e);
				r = 2;
				break;
			}
			printf("Performed reset\n");

			usleep(100*1000);
		} else if(strcmp(argv[i], "cal") == 0) {
			e = lpPzb->vtbl->recalibrate(lpPzb);
			if(e != piezoE_Ok) {
				printf("%s:%u Failed to perform calibration (%u)\n", __FILE__, __LINE__, e);
				r = 2;
				break;
			}
			printf("Performed recalibration\n");

			usleep(100*1000);
		} else if(strcmp(argv[i], "st") == 0) {
			e = lpPzb->vtbl->storeSettings(lpPzb);
			if(e != piezoE_Ok) {
				printf("%s:%u Failed to store settings (%u)\n", __FILE__, __LINE__, e);
				r = 2;
				break;
			}
			printf("Stored settings\n");

			usleep(100*1000);
		} else {
			printf("Unknown command %s\n", argv[i]);
			printUsage(argc, argv);
			return 1;
		}
	}

	lpPzb->vtbl->release(lpPzb);
	lpBus->vtbl->release(lpBus);
	return r;
}
