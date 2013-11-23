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
	if (_buffer >= _file_buffer + _file_length) return 0;
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

void SkipSpriteData(uint8_t type, int32_t num)
{
	if (type & 2) {
		SkipBytes(num);
	} else {
		/* Note: num is signed. Invalid formats will result in num < 0 and abort the loop. */
		while (num > 0) {
			int8_t i = ReadByte();
			if (i >= 0) {
				int size = (i == 0) ? 0x80 : i;
				num -= size;
				SkipBytes(size);
			} else {
				i = -(i >> 3);
				num -= i;
				ReadByte();
			}
		}
	}
}

inline uint32_t Swap32(uint32_t x)
{
	return ((x >> 24) & 0xFF) | ((x >> 8) & 0xFF00) | ((x << 8) & 0xFF0000) | ((x << 24) & 0xFF000000);
}

static const char header[] = {
	'\x00', '\x00',                 // End-of-file marker for old OTTDp versions
	'G',    'R',    'F',    '\x82', // Container version 2
	'\x0D', '\x0A', '\x1A', '\x0A', // Detect garbled transmission
};

inline uint32_t ReadSize(int grfcontversion)
{
	return grfcontversion == 2 ? ReadDWord() : ReadWord();
}


const char *GetGrfID(const char *filename, uint32_t *grfid)
{
	*grfid = 0;

	FILE *f = fopen(filename, "rb");
	if (f == NULL) return "Unable to open file";

	/* Get the length of the file */
	fseek(f, 0, SEEK_END);
	_file_length = ftell(f);
	fseek(f, 0, SEEK_SET);

	/* Map the file into memory */
	_file_buffer = (uint8_t*)mmap(NULL, _file_length, PROT_READ, MAP_PRIVATE, fileno(f), 0);
	_buffer = _file_buffer;

	int grfcontversion = 1;

	if (_file_length > sizeof(header) && memcmp(_buffer, header, sizeof(header)) == 0) {
		grfcontversion = 2;
		_buffer += sizeof(header) + 4 + 1; // Header + offset till data + compression
	}

	/* Check the magic header, or what there is of one */
	if (ReadSize(grfcontversion) != 0x04 || ReadByte() != 0xFF) return "No magic header";

	/* Number of sprites. */
	ReadDWord();

	while (_buffer < _file_buffer + _file_length) {
		uint32_t num = ReadSize(grfcontversion);
		if (num == 0) break;

		if (_buffer + num > _file_buffer + _file_length) return "Corrupt GRF; would read beyond buffer";

		uint8_t type = ReadByte();
		if (type == 0xFF) {
			/* Pseudo sprite */
			uint8_t action = ReadByte();
			if (action == 0x08) {
				/* GRF Version, do not care */
				ReadByte();
				/* GRF ID */
				*grfid = Swap32(ReadDWord());
				/* No more, we don't care */
				break;
			} else {
				SkipBytes(num - 1);
			}
		} else if (grfcontversion == 2 && type== 0xfd) {
			/* Skip sprite offset */
			ReadDWord();
		} else if (grfcontversion == 1) {
			SkipBytes(7);
			/* Skip sprite data */
			SkipSpriteData(type, num - 8);
		} else {
			/* Invalid format, skip to end */
			_buffer = _file_buffer + _file_length;
		}
	}

	munmap(_file_buffer, _file_length);
	fclose(f);

	return (*grfid == 0) ? "File valid but no GrfID found" : NULL;
}

const char *GetMD5(const char *filename, md5_state_t *md5)
{
	FILE *f = fopen(filename, "rb");
	if (f == NULL) return "Unable to open file";

	/* Get the length of the file */
	fseek(f, 0, SEEK_END);
	_file_length = ftell(f);
	fseek(f, 0, SEEK_SET);

	/* Map the file into memory */
	_file_buffer = (uint8_t*)mmap(NULL, _file_length, PROT_READ, MAP_PRIVATE, fileno(f), 0);
	_buffer = _file_buffer;

	size_t read_length = _file_length;
	if (_file_length > sizeof(header) && memcmp(_buffer, header, sizeof(header)) == 0) {
		_buffer += sizeof(header);
		/* Reduce the length to contain only the data, but not the sprites. */
		read_length = sizeof(header) + 4 + ReadDWord();
		if (read_length > _file_length) return "Invalid sprite location offset";
	}

	md5_append(md5, _file_buffer, read_length);

	munmap(_file_buffer, _file_length);
	fclose(f);

	return NULL;
}

int main(int argc, char **argv)
{
	if (argc < 2 || strcmp(argv[1], "-h") == 0 || (strcmp(argv[1], "-m") == 0 && argc < 3)) {
		printf(
			"GRFID " VERSION " - Copyright (C) 2009 by Peter Nelson\n"
			"\n"
			"Usage:\n"
			"    GRFID <NewGRF-File>\n"
			"        Get the GRF ID from the NewGRF file\n"
			"    GRFID -m <NewGRF-File>\n"
			"        Get the MD5 checksum of the NewGRF file\n"
			"    GRFID -v\n"
			"        Get the version of GRFID\n"
			"\n"
			"You may copy and redistribute it under the terms of the GNU General Public\n"
			"License, as stated in the file 'COPYING'.\n"
			"Uses MD5 implementation of L. Peter Deutsch.\n");
		return 1;
	}
	if (strcmp(argv[1], "-v") == 0) {
		printf("GRFID " VERSION " - Copyright (C) 2009 by Peter Nelson\n");
		return 0;
	}

	const char *err;
	if (strcmp(argv[1], "-m") == 0) {
		md5_state_t md5;
		md5_init(&md5);
		err = GetMD5(argv[2], &md5);
		if (err == NULL) {
			md5_byte_t digest[16];
			md5_finish(&md5, digest);
			for (size_t i = 0; i < sizeof(digest); i++) {
				printf("%02x", digest[i]);
			}
			printf("\n");
			return 0;
		}
	} else {
		uint32_t grfid;
		err = GetGrfID(argv[1], &grfid);

		if (err == NULL) {
			printf("%08x\n", grfid);
			return 0;
		}
	}

	printf("Unable to get requested information: %s\n", err);
	return 1;
}

