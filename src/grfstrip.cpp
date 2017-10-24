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
	const char *ret = NULL;
	uint32_t len;
	uint8_t *end;
	FILE *fout;
	FILE *fin;
	
	fin = fopen(origin, "rb");
	if (fin == NULL) {
		ret = "Unable to open origin file";
		goto fail_ret;
	}

	fout = fopen(dest, "wb");
	if (fout == NULL) {
		ret = "Unable to open destination file";
		goto fail_close_fin;
	}

	/* Get the length of the file */
	fseek(fin, 0, SEEK_END);
	_file_length = ftell(fin);
	fseek(fin, 0, SEEK_SET);

	/* Map the file into memory */
	_file_buffer = (uint8_t*)mmap(NULL, _file_length, PROT_READ, MAP_PRIVATE, fileno(fin), 0);
	end = _file_buffer + _file_length;

	if (_file_length <= sizeof(header) || memcmp(_file_buffer, header, sizeof(header)) != 0) {
		return "No GRF with container version 2.";
		goto fail;
	}

	_buffer = _file_buffer + sizeof(header);
	len = ReadDWord();
	_buffer = _file_buffer + sizeof(header) + len + 4;

	if (_buffer >= end) {
		return "Invalid GRF";
		goto fail;
	}

	if (fwrite(_file_buffer, 1, _buffer - _file_buffer, fout) != (size_t)(_buffer - _file_buffer)) {
		ret = "Could not write to file";
		goto fail;
	}

	for (;;) {
		uint8_t *begin = _buffer;
		uint32_t id = ReadDWord();
		if (id == 0) {
			if (fwrite(begin, 1, end - begin, fout) != (size_t)(end - begin)) {
				ret = "Could not write to file";
				goto fail;
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
				ret = "Could not write to file";
				goto fail;
			}
		}

		/* Skip sprite. */
		SkipBytes(size - 2);
		if (_buffer >= end) { 
			ret = "Invalid GRF";
			goto fail;
		}
	}

fail:
	munmap(_file_buffer, _file_length);
	fclose(fout);
fail_close_fin:
	fclose(fin);
fail_ret:
	return ret;
}

int main(int argc, char **argv)
{
	static const char * const depth[] = { "8bpp", "32bpp" };
	static const int num_depths = sizeof(depth) / sizeof(depth[0]);
	static const char * const zoom[] = { "normal", "zi4", "zi2", "zo2", "zo4", "zo8" };
	static const int num_zooms = sizeof(zoom) / sizeof(zoom[0]);

	if (argc < 3 || strcmp(argv[1], "-h") == 0) {
		printf(
			"GRFSTRIP " VERSION " - Copyright (C) 2009 by Remko Bijker\n"
			"\n"
			"Usage:\n"
			"    GRFSTRIP <origin> <dest> (<depth> <zoom>)*\n"
			"        Strip real sprites that are not in the set \"normal 8bpp and the ones\n"
			"        specified at the command line\" from origin into dest.\n"
			"        Known depths: ");
		for (int j = 0; j < num_depths; j++) {
			if (j != 0) printf(", ");
			printf("%s", depth[j]);
		}
		printf("\n        Known zooms: ");
		for (int j = 0; j < num_zooms; j++) {
			if (j != 0) printf(", ");
			printf("%s", zoom[j]);
		}
		printf( "\n"
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
	for (int i = 3; i + 1 < argc; ) {
		int depth_offset = 0xFF;
		for (int j = 0; j < num_depths; j++) {
			if (strcmp(argv[i], depth[j]) == 0) {
				depth_offset = 16 * j;
				break;
			}
		}
		if (depth_offset == 0xFF)  {
			printf("Invalid depth \"%s\"\n", argv[i]);
			return 1;
		}
		i++;

		int zoom_offset = 0xFF;
		for (int j = 0; j < num_zooms; j++) {
			if (strcmp(argv[i], zoom[j]) == 0) {
				zoom_offset = j;
				break;
			}
		}
		if (zoom_offset == 0xFF) {
			printf("Invalid zoom \"%s\"\n", argv[i]);
			return 1;
		}
		i++;

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

