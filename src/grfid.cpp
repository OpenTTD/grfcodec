/*
 * This file is part of GRFCodec.
 * GRFCodec is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * GRFCodec is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdbool>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

#include "md5.h"

#include "version.h"

struct FileReader {
	std::vector<uint8_t> file_buffer;
	std::vector<uint8_t>::iterator buffer;

	/**
	 * Open a file for reading.
	 * @param filename Filename of file to read.
	 * @return true iff the file was successfully opened for reading.
	 */
	bool Open(const std::string &filename)
	{
		/* Open file at the end -- this puts us in the right place to get the file size. */
		std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
		if (!ifs) return false;

		/* Get file size. */
		auto end = ifs.tellg();
		ifs.seekg(0, std::ios::beg);
		auto size = std::size_t(end - ifs.tellg());

		/* Don't load empty file. */
		if (size == 0) return false;

		this->file_buffer.resize(size);

		/* Slurp the file into the buffer. */
		if (!ifs.read((char *)this->file_buffer.data(), this->file_buffer.size())) return false;

		this->buffer = this->file_buffer.begin();
		return true;
	}

	/**
	 * Skip bytes in the buffer.
	 * @param count bytes to skip.
	 */
	inline void SkipBytes(size_t count)
	{
		this->buffer += count;
	}

	/**
	 * Read byte from buffer.
	 * @return byte from buffer.
	 */
	inline uint8_t ReadByte()
	{
		if (this->buffer >= this->file_buffer.end()) return 0;
		return *(this->buffer++);
	}

	/**
	 * Read 16-bit little-endian word from buffer.
	 * @return 16-bit word from buffer.
	 */
	inline uint16_t ReadWord()
	{
		uint16_t v = ReadByte();
		return v | (ReadByte() << 8);
	}

	/**
	 * Read 32-bit little-endian doubleword from buffer.
	 * @return 32-bit doubleword from buffer.
	 */
	inline uint32_t ReadDWord()
	{
		uint32_t v = ReadWord();
		return v | (ReadWord() << 16);
	}
};

void SkipSpriteData(FileReader &file, uint8_t type, int32_t num)
{
	if (type & 2) {
		file.SkipBytes(num);
	} else {
		/* Note: num is signed. Invalid formats will result in num < 0 and abort the loop. */
		while (num > 0) {
			int8_t i = file.ReadByte();
			if (i >= 0) {
				int size = (i == 0) ? 0x80 : i;
				num -= size;
				file.SkipBytes(size);
			} else {
				i = -(i >> 3);
				num -= i;
				file.ReadByte();
			}
		}
	}
}

inline uint32_t Swap32(uint32_t x)
{
	return ((x >> 24) & 0xFF) | ((x >> 8) & 0xFF00) | ((x << 8) & 0xFF0000) | ((x << 24) & 0xFF000000);
}

static const uint8_t header[] = {
	0x00, 0x00,             // End-of-file marker for old OTTDp versions
	'G',  'R',  'F',  0x82, // Container version 2
	0x0d, 0x0a, 0x1a, 0x0a, // Detect garbled transmission
};

inline uint32_t ReadSize(FileReader &file, int grfcontversion)
{
	return grfcontversion == 2 ? file.ReadDWord() : file.ReadWord();
}

const char *GetGrfID(const char *filename, uint32_t *grfid)
{
	*grfid = 0;

	FileReader file;
	if (!file.Open(filename)) return "Unable to open file";

	int grfcontversion = 1;

	if (file.file_buffer.size() > sizeof(header) && std::equal(std::begin(header), std::end(header), file.buffer)) {
		grfcontversion = 2;
		file.buffer += sizeof(header) + 4 + 1; // Header + offset till data + compression
	}

	/* Check the magic header, or what there is of one */
	if (ReadSize(file, grfcontversion) != 0x04 || file.ReadByte() != 0xFF) return "No magic header";

	/* Number of sprites. */
	file.ReadDWord();

	while (file.buffer < file.file_buffer.end()) {
		uint32_t num = ReadSize(file, grfcontversion);
		if (num == 0) break;

		if (file.buffer + num > file.file_buffer.end()) return "Corrupt GRF; would read beyond buffer";

		uint8_t type = file.ReadByte();
		if (type == 0xFF) {
			/* Pseudo sprite */
			uint8_t action = file.ReadByte();
			if (action == 0x08) {
				/* GRF Version, do not care */
				file.ReadByte();
				/* GRF ID */
				*grfid = Swap32(file.ReadDWord());
				/* No more, we don't care */
				break;
			} else {
				file.SkipBytes(num - 1);
			}
		} else if (grfcontversion == 2 && type== 0xfd) {
			/* Skip sprite offset */
			file.ReadDWord();
		} else if (grfcontversion == 1) {
			file.SkipBytes(7);
			/* Skip sprite data */
			SkipSpriteData(file, type, num - 8);
		} else {
			/* Invalid format, skip to end */
			file.buffer = file.file_buffer.end();
		}
	}

	return (*grfid == 0) ? "File valid but no GrfID found" : nullptr;
}

const char *GetMD5(const char *filename, md5_state_t &md5)
{
	FileReader file;
	if (!file.Open(filename)) return "Unable to open file";

	size_t read_length = file.file_buffer.size();
	if (file.file_buffer.size() > sizeof(header) && std::equal(std::begin(header), std::end(header), file.buffer)) {
		file.buffer += sizeof(header);
		/* Reduce the length to contain only the data, but not the sprites. */
		read_length = sizeof(header) + 4 + file.ReadDWord();
		if (read_length > file.file_buffer.size()) return "Invalid sprite location offset";
	}

	md5_append(&md5, file.file_buffer.data(), (int)read_length);

	return nullptr;
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
		err = GetMD5(argv[2], md5);
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

		if (err == nullptr) {
			printf("%08x\n", grfid);
			return 0;
		}
	}

	fprintf(stderr, "Unable to get requested information: %s (%s)\n", err, std::strerror(errno));
	return 1;
}

