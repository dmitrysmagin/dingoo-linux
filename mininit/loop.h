//===========================================================================

#ifndef __loop_h__
#define __loop_h__

//===========================================================================

int losetup (
	int loopfd,
	int filefd,
	const char *filename,
	const char *encryption,
	const void *key,
	unsigned int key_size
);

int lodelete (int loopfd);

//===========================================================================
#endif

