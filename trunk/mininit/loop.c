//===========================================================================

#include <sys/ioctl.h>

#include <string.h>

#include "loop_info.h"
#include "loop.h"

//===========================================================================

int losetup (
	int loopfd,
	int filefd,
	const char *filename,
	const char *encryption,
	const void *key,
	unsigned int key_size
) {
	int r; struct loop_info64 lo;

	r = ioctl(loopfd, LOOP_SET_FD, filefd);
	if (r < 0) return r;

	memset(&lo, 0, sizeof(lo));
	strncpy((char *)lo.lo_file_name, filename, LO_NAME_SIZE - 1);

	if (encryption != NULL) {

		strncpy((char *)lo.lo_crypt_name, encryption, LO_NAME_SIZE - 1);

		// The SET_STATUS ioctl will fail unless key_size == LO_KEY_SIZE (!)
		lo.lo_encrypt_type = LO_CRYPT_CRYPTOAPI;
		lo.lo_encrypt_key_size = LO_KEY_SIZE;

		if (key_size > LO_KEY_SIZE) key_size = LO_KEY_SIZE;
		memcpy(lo.lo_encrypt_key, key, key_size);

	}

	return ioctl(loopfd, LOOP_SET_STATUS64, &lo);
}

//===========================================================================

int lodelete (int loopfd) {
	return ioctl(loopfd, LOOP_CLR_FD, 0);
}

//===========================================================================

