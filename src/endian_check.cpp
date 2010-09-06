/**
 * @file endian_check.cpp Get the endianness of a platform.
 *
 * After that it outputs the contents of an include files (endian.h)
 *  that says or GRFCODEC_LITTLE_ENDIAN, or GRFCODEC_BIG_ENDIAN. Makefile takes
 *  care of the real writing to the file.
 */

#include <stdio.h>
#include <string.h>

/**
 * Main call of the endian_check program
 * @param argc argument count
 * @param argv arguments themselves
 * @return exit code
 */
int main (int argc, char *argv[])
{
	union {
		unsigned int i;
		char c[4];
	} bint = {0x01020304};

	bool little_endian = (bint.c[0] != 1);

	if (argc > 1 && strcmp(argv[1], "BE") == 0) little_endian = false;
	if (argc > 1 && strcmp(argv[1], "LE") == 0) little_endian = true;

	printf("#ifndef ENDIAN_H\n#define ENDIAN_H\n#define GRFCODEC_%s_ENDIAN\n#endif /* ENDIAN_H */\n",
		little_endian ? "LITTLE" : "BIG");

	return 0;
}
