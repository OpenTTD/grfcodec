/*
 * This file is part of GRFCodec.
 * GRFCodec is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * GRFCodec is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "md5.h"

#ifndef WIN32
#include <sys/mman.h>
#else
#include <stdlib.h>
#define PROT_READ 0
#define MAP_PRIVATE 0
/* Lets fake mmap for Windows, please! */
void *mmap (void *ptr, long size, long prot, long type, long handle, long arg) {
	char *mem = (char*)malloc(size + 1);
	mem[size] = 0;
	FILE *in = fdopen(handle, "rb");
	if (fread(mem, size, 1, in) != 1) {
		fclose(in);
		free(mem);
		return NULL;
	}
	fclose(in);
	return mem;
}
long munmap (void *ptr, long size) {
	free(ptr);
	return 0;
}
#endif

#include "version.h"

size_t _file_length;
uint8_t *_file_buffer;
uint8_t *_buffer;

inline void SkipBytes(size_t count)
{
	_buffer += count;
}

inline uint8_t ReadByte()
{
	return *(_buffer++);
}

inline uint16_t ReadWord()
{
	uint16_t v = ReadByte();
	return v | (ReadByte() << 8);
}

inline uint32_t ReadDWord()
{
	uint32_t v = ReadWord();
	return v | (ReadWord() << 16);
}

static const char header[] = {
	'\x00', '\x00',                 // End-of-file marker for old OTTDp versions
	'G',    'R',    'F',    '\x82', // Container version 2
	'\x0D', '\x0A', '\x1A', '\x0A', // Detect garbled transmission
};


const char *Strip(const char *origin, const char *dest, uint32_t allowed)
{
	FILE *fin = fopen(origin, "rb");
	if (fin == NULL) return "Unable to open origin file";

	FILE *fout = fopen(dest, "wb");
	if (fout == NULL) return "Unable to open destination file";

	/* Get the length of the file */
	fseek(fin, 0, SEEK_END);
	_file_length = ftell(fin);
	fseek(fin, 0, SEEK_SET);

	/* Map the file into memory */
	_file_buffer = (uint8_t*)mmap(NULL, _file_length, PROT_READ, MAP_PRIVATE, fileno(fin), 0);
	uint8_t *end = _file_buffer + _file_length;

	if (_file_length <= sizeof(header) || memcmp(_file_buffer, header, sizeof(header)) != 0) {
		return "No GRF with container version 2.";
	}

	_buffer = _file_buffer + sizeof(header);
	uint32_t len = ReadDWord();
	_buffer = _file_buffer + sizeof(header) + len + 4;

	if (_buffer >= end) return "Invalid GRF";

	if (fwrite(_file_buffer, 1, _buffer - _file_buffer, fout) != (size_t)(_buffer - _file_buffer)) {
		return "Could not write to file";
	}

	for (;;) {
		uint8_t *begin = _buffer;
		uint32_t id = ReadDWord();
		if (id == 0) {
			if (fwrite(begin, 1, end - begin, fout) != (size_t)(end - begin)) {
				return "Could not write to file";
			}
			break;
		}

		uint32_t size = ReadDWord();
		uint8_t info = ReadByte();
		uint8_t zoom = ReadByte();

		int offset = ((info & 0x7) == 4 ? 0 : 16) + zoom;

		if (info == 0xFF || (allowed & (1 << offset)) != 0) {
			/* Copy */
			if (fwrite(begin, 1, size + 8, fout) != (size_t)(size + 8)) {
				return "Could not write to file";
			}
		}

		/* Skip sprite. */
		SkipBytes(size - 2);
		if (_buffer >= end) return "Invalid GRF";
	}


	munmap(_file_buffer, _file_length);
	fclose(fin);

	return NULL;
}

int main(int argc, char **argv)
{
	if (argc < 3 || strcmp(argv[1], "-h") == 0) {
		printf(
			"GRFSTRIP " VERSION " - Copyright (C) 2009 by Remko Bijker\n"
			"\n"
			"Usage:\n"
			"    GRFSTRIP <origin> <dest> [<depth> <zoom>]\n"
			"        Strip real sprites that are not in the set \"normal 8bpp and the ones\n"
			"        specified at the command line\" from origin into dest.\n"
			"    GRFSTRIP -v\n"
			"        Get the version of GRFSTRIP\n"
			"\n"
			"You may copy and redistribute it under the terms of the GNU General Public\n"
			"License, as stated in the file 'COPYING'.\n");
		return 1;
	}
	if (strcmp(argv[1], "-v") == 0) {
		printf("GRFSTRIP " VERSION " - Copyright (C) 2012 by Remko Bijker\n");
		return 0;
	}

	uint32_t allowed = 1;
	for (int i = 3; i + 1 < argc; i++) {
		int depth_offset;
		if (strcmp(argv[i], "32bpp") == 0) {
			depth_offset = 16;
		} else if (strcmp(argv[i], "8bpp") == 0) {
			depth_offset = 0;
		} else {
			printf("Invalid depth \"%s\"\n", argv[i]);
			return 1;
		}

		static const char *zoom[] = { "normal", "zi4", "zi2", "zo2", "zo4", "zo8" };
		int zoom_offset = 0xFF;
		for (int j = 0; j < 6; j++) {
			if (strcmp(argv[i + 1], zoom[j]) == 0) {
				zoom_offset = j;
				break;
			}
		}

		if (zoom_offset == 0xFF) {
			printf("Invalid zoom \"%s\"\n", argv[i]);
		}
		allowed |= 1 << (depth_offset + zoom_offset);
	}

	const char *err = Strip(argv[1], argv[2], allowed);
	if (err == NULL) {
		printf("Stripped %s into %s successfully\n", argv[1], argv[2]);
		return 0;
	}

	printf("Unable to get requested information: %s\n", err);
	return 1;
}

