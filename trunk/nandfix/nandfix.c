#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const char *_help =
"nandfix v1.0 by Ignacio Garcia Perez <iggarpe@gmail.com>\n"
"\n"
"Usage: nandfix <file>\n"
"\n"
"Fixes a 2K page size dump of the SPL eraseblock in order for\n"
"the IPL to recognize and load it.\n"
"\n"
"Linux places the ECC info at offset 28 in the OOB area of\n"
"each page, while the IPL expects it to be at offset 6.\n"
"\n"
"Also, for some unknown reason, the IPL expects bytes at\n"
"offsets 2 to 4 to be zero.\n"
"\n"
;

static struct {
	unsigned char data [2048];
	unsigned char oob  [64];
} __attribute__((packed)) _buf;

int main (int argc, char **argv) {

	FILE *f; long i, j, k, len, pos; size_t r;

	if (argc < 2) {
		fputs(_help, stderr);
		exit(1);
	}

	f = fopen(argv[1], "r+");
	if (f == NULL) {
		fprintf(stderr, "ERROR: cannot open file '%s'\n", argv[1]);
		exit(1);
	}

	fseek(f, 0, SEEK_END);
	len = ftell(f);

	if (len != 128 * (2048 + 64)) {
		fprintf(stderr, "ERROR: file size MUST be 128 pages (2048 data + 64 OOB)\n");
		exit(1);
	}

	printf("Fixing: ");

	for (i = 0; i < 128; i++) {

		pos = i * (2048 + 64);
		fseek(f, pos, SEEK_SET);
		r = fread(&_buf, 2048 + 64, 1, f);
		if (r < 1) {
			fprintf(stderr, "\nERROR: cannot read file\n");
			exit(1);
		}

		/* Skip erased pages */
		for (j = 0; j < 2048 && _buf.data[j] == 0xFF; j++);
		for (k = 0; k < 64 && _buf.oob[k] == 0xFF; k++);
		if (j >= 2048 && k >= 64) { printf("-"); continue; }

		memmove(_buf.oob + 6, _buf.oob + 28, 36);
		memset(_buf.oob + 6 + 36, 0xFF, 64 - 6 - 36);

		_buf.oob[2] = 0x00;	/* The IPL wants this for some unknown reason */
		_buf.oob[3] = 0x00;
		_buf.oob[4] = 0x00;
		_buf.oob[5] = 0xFF;

		fseek(f, pos + 2048, SEEK_SET);
		r = fwrite(_buf.oob, 64, 1, f);
		if (r < 1) {
			fprintf(stderr, "\nERROR: cannot write file\n");
			exit(1);
		}

		if (_buf.oob[0] != 0xFF || _buf.oob[1] != 0xFF) putchar('B');
		else putchar('F');
	}

	printf("\n");

	fclose(f);
	exit(0);
}

