#include "./sysuuid.h"

#ifdef __cplusplus
	extern "C" {
#endif

/*
	This is a minimal local implementation of the sysUuid type from sysLib
*/

int sysUuidCompare(
	struct sysUuid* lpID1,
	struct sysUuid* lpID2
) {
	unsigned long int i;

	if((lpID1 == NULL) && (lpID2 == NULL)) { return  0; }
	if((lpID1 == NULL) && (lpID2 != NULL)) { return  1; }
	if((lpID1 != NULL) && (lpID2 == NULL)) { return -1; }

	for(i = 0; i < sizeof(struct sysUuid); i=i+1) {
		if(((char*)lpID1)[i] > ((char*)lpID2)[i]) {
			return 1;
		} else if(((char*)lpID1)[i] < ((char*)lpID2)[i]) {
			return -1;
		}
	}

	return 0;
}


#ifdef __cplusplus
	} /* extern "C" { */
#endif
