//==============================================================================

#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>

#include <linux/kdev_t.h>

#ifndef MS_MOVE
#define MS_MOVE 8192
#endif

#ifndef MNT_DETACH
#define MNT_DETACH 2
#endif

#ifndef MNT_FORCE
#define MNT_FORCE 1
#endif

#include "loop.h"
#include "fstype.h"

//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

static void _msg (const char *prefix, const char *format, va_list v) {
	fprintf(stderr, "%s: ", prefix);
	vfprintf(stderr, format, v);
	fprintf(stderr, " (%i)\n", errno);
}

static void _warn (const char *format, ...) {
	va_list v;
	va_start(v, format);
	_msg("WARNING", format, v);
	va_end(v);
}
/*
static void _error (const char *format, ...) {
	va_list v;
	va_start(v, format);
	_msg("ERROR", format, v);
	va_end(v);
}
*/
static void _fatal (const char *format, ...) {
	va_list v;
	va_start(v, format);
	_msg("FATAL", format, v);
	va_end(v);
	sleep(3);
	exit(1);
}

//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//
//	Make a path of directories (equivalent to mkdir -p).
//

static int _mkpath (const char *path, int mode, int fatal) {
	int i, r; char buf [256];
	if (!path[0]) return 0;
	for (buf[0] = path[0], i = 1; buf[i] = '\0', path[i]; buf[i] = path[i], i++) {
		if (path[i] != '/') continue;
		r = mkdir(buf, mode); 
		if (r < 0 && errno != EEXIST) {
			if (fatal) _fatal("cannot create directory %s", buf);
			return r;
		}
	}
	return 0;
}

//
//	Make a single directory.
//

static int _mkdir (const char *path, int mode, int fatal) {
	int r;
	r = _mkpath(path, mode, fatal); if (r < 0) return r;
	r = mkdir(path, mode);
	if (r < 0 && errno != EEXIST) {
		if (fatal) _fatal("cannot create directory %s", path);
		return r;
	}
	return 0;
}

//
//	Mount a device.
//

static int _mount (
	const char *source,		// Source device
	const char *target,		// Destination directory (will be made if necessary)
	const char *type,		// Filesystem type (NULL to guess)
	unsigned long flags,	// Mount flags
	const void *data,		// Special data (depens on filesystem)
	int fatal
) {
	int r = _mkdir(target, 0755, fatal); if (r < 0) return r;

	if (type == NULL && !(flags & MS_MOVE)) {	// Don't find out if moving
		type = fstype(source);
		if (type == NULL) {
			if (fatal) _fatal("cannot not guess %s filesystem type", source);
			return -1;
		}
	}

	r = mount(source, target, type, flags, data);
	if (r < 0 && fatal) _fatal("mounting %s on %s failed", source, target);

	return r;
}

//
//	Unmount a device.
//

static void _umount (const char *target) {
	if (umount2(target, MNT_DETACH) < 0) {
		_warn("forced unmount of %s", target);
		umount2(target, MNT_FORCE);
	}
}

//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//
//	Read a single line from a file.
//

static int _read_line (const char *path, char *buf, int len, int fatal) {

	int r, f; char *s;

	f = open(path, O_RDONLY);
	if (f < 0) {
		if (fatal) _fatal("cannot open %s", path);
		return f;
	}

	r = read(f, buf, len - 1); close(f);
	if (r < 0) {
		if (fatal) _fatal("cannot read %s", path);
		return r;
	}

	buf[r] = '\0'; if ((s = strchr(buf, '\n')) != NULL) *s = '\0';

	return 0;
}

//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//
//	Setup a loop device.
//

static int _losetup (
	const char *loop,			// Loop device
	const char *file,			// Target file (or block device)
	int fatal
) {
	int r, loopfd = -1, filefd = -1, mode;

	r = filefd = open(file, mode = O_RDWR);
	if (r < 0) {
		r = filefd = open(file, mode = O_RDONLY);
		if (r < 0) { if (fatal) _fatal("cannot open %s", file); goto exit; }
	}

	r = loopfd = open(loop, mode);
	if (r < 0) {
		r = loopfd = open(loop, mode = O_RDONLY);
		if (r < 0) { if (fatal) _fatal("cannot open %s", loop); goto exit; }
	}

	r = losetup(loopfd, filefd, file);
	if (r < 0 && fatal) _fatal("cannot setup loop device %s", loop);

exit:

	close(loopfd);
	close(filefd);
	return r;
}

//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//
//	Split the passed buffer as parameters.
//

static int _mkparam (char *buf, char **paramv, int maxparam) {

	int paramc; char *s;

	for (paramc = 0, s = buf;; *s++ = '\0') {

		for (; *s != '\0' && isspace(*s); s++);
		if (*s == '\0') return paramc;

		paramv[paramc++] = s; if (paramc >= maxparam) return paramc;

		for (; *s != '\0' && !isspace(*s); s++);
		if (*s == '\0') return paramc;
	}
}

//
//	Multiple, comma separated mount
//

static int _multimount (
	char *source,			// Source device
	const char *target,		// Destination directory (will be made if necessary)
	const char *type,		// Filesystem type (NULL to guess)
	unsigned long flags,	// Mount flags
	const void *data,		// Special data (depens on filesystem)
	int retries,
	int fatal
) {
	int try; char c, *s, *t;

	for (try = 0; try < retries; usleep(100000), try++) {

		for (c = ',', s = source; c == ','; *s++ = c) {

			for (t = s; *s != ',' && *s != '\0'; s++); c = *s; *s = '\0';

			if (access(t, R_OK) >= 0 && _mount(t, target, type, flags, data, 0) >= 0) {
				printf("Mounting %s on %s\n", t, target);
				*s = c;
				return 0;
			}
		}
	}

	if (fatal) _fatal("cannot mount %s on %s\n", source, target);
	return -1;
}

//
//	Main function.
//

int main (int argc, char **argv) {

	int i, f, boot = 0;

	char loop_dev [] = "/dev/loop0";
	char cbuf [4096];
	char sbuf [256];

	int paramc; char * paramv [64];

	const char *inits [] = { "/sbin/init", "/etc/init", "/bin/init", "/bin/sh", NULL };

	printf("\n\n\nmininit 1.0.0 by Ignacio Garcia Perez <iggarpe@gmail.com>\n");

	//
	// Setup a minimal working environment
	//

	printf("Mounting /proc /sys /dev\n");
	_mount(NULL, "/proc", "proc", 0, NULL, 1);
	_mount(NULL, "/sys", "sysfs", 0, NULL, 1);

	//
	// Get arguments from kernel command line and real-root-dev
	// (note that paramv[0] and paramv[paramc] are reserved)
	//

	printf("Reading kernel command line\n");

	_read_line("/proc/cmdline", cbuf, sizeof(cbuf), 1);
	paramc = 1 + _mkparam(cbuf, paramv + 1, sizeof(paramv) / sizeof(paramv[0]) - 2);

	//
	// Process "boot" parameter (ONLY ONE, ALLOW COMMA SEPARATED LIST)
	//
	// Note that we specify 20 retries (2 seconds), just in case it is
	// a hotplug device which takes some time to detect and initialize.
	//

	for (i = 1; i < paramc; i++) {
		if (strncmp(paramv[i], "boot=", 5)) continue;
		_multimount(paramv[i] + 5, "/boot", NULL, MS_RDONLY, NULL, 20, 1);
		boot = 1;
		break; /* only one */
	}

	//
	// Process "loop" parameter (MULTIPLE)
	//

	for (i = 1; i < paramc; i++) {

		if (strncmp(paramv[i], "loop", 4) || paramv[i][5] != '=') continue;

		loop_dev[9] = paramv[i][4];

		printf("Setting up loopback %s %s\n", loop_dev, paramv[i] + 6);
		_losetup(loop_dev, paramv[i] + 6, 1);
	}

	//
	// Process "root" parameter (ONLY ONE, ALLOW COMMA SEPARATED LIST)
	//
	// Note that we specify 20 retries (2 seconds), just in case it is
	// a hotplug device which takes some time to detect and initialize.
	//

	for (i = 1; i < paramc; i++) {
		if (strncmp(paramv[i], "root=", 5)) continue;
		_multimount(paramv[i] + 5, "/root", NULL, MS_RDONLY, NULL, 20, 1);
		break; /* only one */
	}

	if (i >= paramc)
		_fatal("root parameter not found");	// TODO: use real-root-dev ?




	//
	// Move /boot if mounted
	//

	if (boot) {
		printf("Moving boot mount\n");
		_mount("/boot", "/root/boot", NULL, MS_MOVE, NULL, 1);
	}




	//
	//	Switch to new root
	//

	printf("Switching root\n");

	// Change to new root directory
	if (chdir("/root") < 0) _fatal("changing directory");

	// Reopen the console device at the new location (must already exist !!!)
	f = open("./dev/console", O_RDWR, 0);
	if (f < 0) _fatal("opening console");
	if (dup2(f, 0) != 0 || dup2(f, 1) != 1 || dup2(f, 2) != 2)
		_fatal("duplicating console handle");
	if (f > 2) close(f);

	// Keep old root open (purpose?, Red Hat's nash does it...)
	f = open("/", O_RDONLY, 0);

	// Unmount all we previously mounted
	_umount("/sys");
	_umount("/proc");

	// Do the root switch	
	_mount(".", "/", NULL, MS_MOVE, NULL, 1);
	if (chroot(".") < 0) _fatal("chroot failed");
	if (chdir("/") < 0) _fatal("chdir failed");

	// Release old root
	close(f);

	// Prepare argv[0] which is the init program itself and add NULL terminator
	paramv[0] = sbuf; sbuf[0] = '\0';
	paramv[paramc] = NULL;

	// Get init parameter from command line
	for (i = 1; i < paramc; i++)
		if (!strncmp(paramv[i], "init=", 5)) { strcpy(sbuf, paramv[i] + 5); }

	// If no init= in command line, try to locate it
	if (sbuf[0] == '\0') {
		for (i = 0; inits[i] != NULL && access(inits[i], X_OK) < 0; i++);
		if (inits[i] == NULL) _fatal("cannot locate init");
		strcpy(sbuf, inits[i]);
	}

	// Execute init
	execv(paramv[0], paramv); _fatal("exec or init failed");

	return 0;
}

//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

