#ifndef __is_included__dda99e8c_f29f_4b2a_900f_7fd3256deb65
#define __is_included__dda99e8c_f29f_4b2a_900f_7fd3256deb65 1

#include <stdint.h>

#ifdef __cplusplus
	extern "C" {
#endif

#ifndef __cplusplus
	#ifndef true
		typedef unsigned char bool;
		#define true 1
		#define false 0
	#endif
#endif

#ifndef sysUuid_Defined
	struct sysUuid {
		uint32_t p1;
		uint16_t p2[3];
		uint8_t p3[6];
	};

	#define sysUuid_Defined 1

	int sysUuidCompare(
		struct sysUuid* lpID1,
		struct sysUuid* lpID2
	);

#endif

#ifdef __cplusplus
	} /*	extern "C" */
#endif

#endif
