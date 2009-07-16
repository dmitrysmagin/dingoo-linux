//===========================================================================
//===========================================================================
//===========================================================================
//===========================================================================

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>		// REMOVE !!!

#include "fstype.h"

//===========================================================================
//===========================================================================
//===========================================================================
//===========================================================================

#define CRAMFS_SUPER_MAGIC			0x28cd3d45
#define CRAMFS_SUPER_MAGIC_BE		0x453dcd28

#define SQUASHFS_SUPER_MAGIC		0x73717368
#define SQUASHFS_SUPER_MAGIC_BE		0x68737173

//===========================================================================

#define EXT2_PRE_02B_MAGIC  0xEF51
#define EXT2_SUPER_MAGIC    0xEF53
#define EXT3_FEATURE_COMPAT_HAS_JOURNAL 0x0004

typedef struct _ext2_t {

	unsigned char 	s_dummy1			[56];
	unsigned char 	s_magic				[2];
	unsigned char	s_dummy2			[34];
	unsigned char	s_feature_compat	[4];
	unsigned char	s_feature_incompat	[4];
	unsigned char	s_feature_ro_compat	[4];
	unsigned char	s_uuid				[16];
	unsigned char 	s_volume_name		[16];
	unsigned char	s_dummy3			[88];
	unsigned char	s_journal_inum		[4];	/* ext3 only */

} __attribute__((packed)) ext2_t;

#define ext2magic(s)	assemble2le(s.s_magic)

//===========================================================================

#define MINIX_SUPER_MAGIC   0x137F		/* minix v1, 14 char names */
#define MINIX_SUPER_MAGIC2  0x138F		/* minix v1, 30 char names */
#define MINIX2_SUPER_MAGIC  0x2468		/* minix v2, 14 char names */
#define MINIX2_SUPER_MAGIC2 0x2478		/* minix v2, 30 char names */

typedef struct _minix_t {

	unsigned char   s_dummy [16];
	unsigned char   s_magic [2];

} __attribute__((packed)) minix_t;

#define minixmagic(s)	assemble2le(s.s_magic)

//===========================================================================

typedef struct _fat_t {

    unsigned char    s_dummy	[3];
    unsigned char    s_os		[8];	/* "MSDOS5.0" or "MSWIN4.0" or "MSWIN4.1" */
										/* mtools-3.9.4 writes "MTOOL394" */
    unsigned char    s_dummy2	[32];
    unsigned char    s_label	[11];	/* for DOS? */
    unsigned char    s_fs		[8];	/* "FAT12   " or "FAT16   " or all zero   */
										/* OS/2 BM has "FAT     " here. */
    unsigned char    s_dummy3	[9];
    unsigned char    s_label2	[11];	/* for Windows? */
    unsigned char    s_fs2		[8];	/* garbage or "FAT32   " */

} __attribute__((packed))fat_t;

//===========================================================================
//===========================================================================
//===========================================================================
//===========================================================================

static inline unsigned long assemble2le(unsigned char *p) {
	return (p[0] | (p[1] << 8));
}

static inline unsigned long assemble4le (unsigned char *p) {
	return (p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24));
}

static inline unsigned short swapped (unsigned short b) {
	return (b << 8) | (b >> 8);
}

const char *fstype (const char *dev) {

	int fd; const char *type = NULL;

	union {
		unsigned char		raw [512];
		minix_t				minix;
		ext2_t				ext2;
		fat_t				fat;
	} sect;

	fd = open(dev, O_RDONLY, 0);
	if (fd < 0) return NULL;

	if (lseek(fd, 0, SEEK_SET) != 0) goto exit;
	if (read(fd, sect.raw, sizeof(sect.raw)) != sizeof(sect.raw)) goto exit;

	// ========== First sector detection ==========

	// romfs
	if (memcmp(sect.raw, "-rom1fs-", 8) == 0) { type = "romfs"; goto exit; }

	// cramfs
	if (assemble4le(sect.raw) == CRAMFS_SUPER_MAGIC		||
		assemble4le(sect.raw) == CRAMFS_SUPER_MAGIC_BE	)
		{ type = "cramfs"; goto exit; }

	// squashfs
	if (assemble4le(sect.raw) == SQUASHFS_SUPER_MAGIC		||
		assemble4le(sect.raw) == SQUASHFS_SUPER_MAGIC_BE	)
		{ type = "squashfs"; goto exit; }

	// fat
	if ((	!strncmp((char *)sect.fat.s_os, "MSDOS", 5)		||
			!strncmp((char *)sect.fat.s_os, "MSWIN", 5)		||
			!strncmp((char *)sect.fat.s_os, "MTOOL", 5)		||
			!strncmp((char *)sect.fat.s_os, "IBM", 3)		||
			!strncmp((char *)sect.fat.s_os, "DRDOS", 5)		||
			!strncmp((char *)sect.fat.s_os, "mkdosfs", 7)	||
			!strncmp((char *)sect.fat.s_os, "kmkdosfs", 8)	||
			!strncmp((char *)sect.fat.s_os, "CH-FOR18", 8)
		) && (
			!strncmp((char *)sect.fat.s_fs, "FAT12   ", 8)	||
			!strncmp((char *)sect.fat.s_fs, "FAT16   ", 8)	||
			!strncmp((char *)sect.fat.s_fs2, "FAT32   ", 8)
		)) { type = "vfat"; goto exit; }

	// ========== Second sector detection ==========
	
	if (lseek(fd, 1024, SEEK_SET) != 1024) goto exit;
	if (read(fd, sect.raw, sizeof(sect.raw)) != sizeof(sect.raw)) goto exit;

	// ext2 / ext3
	if (ext2magic(sect.ext2) == EXT2_SUPER_MAGIC			||
		ext2magic(sect.ext2) == EXT2_PRE_02B_MAGIC			||
		ext2magic(sect.ext2) == swapped(EXT2_SUPER_MAGIC)	)
	{
		type = "ext2";
		if (assemble4le(sect.ext2.s_feature_compat) & EXT3_FEATURE_COMPAT_HAS_JOURNAL)
			if (sect.ext2.s_journal_inum != 0)
				type = "ext3";
		goto exit;
	}

	// minix
	if (minixmagic(sect.minix) == MINIX_SUPER_MAGIC				||
		minixmagic(sect.minix) == MINIX_SUPER_MAGIC2			||
		minixmagic(sect.minix) == swapped(MINIX_SUPER_MAGIC2)	||
		minixmagic(sect.minix) == MINIX2_SUPER_MAGIC			||
		minixmagic(sect.minix) == MINIX2_SUPER_MAGIC2			)
		{ type = "minix"; goto exit; }

exit:

	close(fd);
	return type;
}

//===========================================================================
//===========================================================================
//===========================================================================
//===========================================================================

