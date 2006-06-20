/*****************************************\
*                                         *
* SPRITES.C - A couple of routines to     *
*             decode and encode TTD .GRF  *
*             sprites                     *
*                                         *
*                                         *
* Copyright (C) 2000 by Josef Drexler     *
*               <jdrexler@julian.uwo.ca>  *
*                                         *
* Permission granted to copy and redist-  *
* ribute under the terms of the GNU GPL.  *
* For more info please read the file      *
* COPYING which should have come with     *
* this file.                              *
*                                         *
\*****************************************/

#include <stdlib.h>
#include "error.h"

#define _SPRITES_C
#include "sprites.h"

// Define some of the bits in info[0]
#define HASTRANSPARENCY(info) (info[0] & 8)
#define SIZEISCOMPRESSED(info) (info[0] & 2)

void cfread (void *ptr, size_t size, size_t n, FILE *stream)
{
	unsigned int read = fread(ptr, 1, size * n, stream);

	if (read != size * n) {
		fperror("\nError in cfread, got %d, wanted %d, at %ld", read, size * n,
			ftell(stream));
		exit(2);
	}
}

void cfwrite (void *ptr, size_t size, size_t n, FILE *stream)
{
	unsigned int written = fwrite(ptr, 1, size * n, stream);

	if (written != size * n) {
		fperror("\nError in cfwrite, wrote %d, wanted %d, at %ld",
			written, size * n, ftell(stream));
		exit(2);
	}
}

long fcopy(FILE *one, FILE *two, long bytes)
{
	void *buffer = malloc(8192);
	long total = 0;

	if (!buffer) {
		printf("\nError copying file\n");
		exit(2);
	}

	while (bytes) {
		long chunk = (bytes > 8192) ? 8192 : bytes;
		long read = fread(buffer, 1, chunk, one);

		if (read != chunk)
			break;

		long written = fwrite(buffer, 1, read, two);

		total += written;

		if (written != read)
			break;

		bytes -= written;
	}

	free(buffer);
	return total;
}

int decodetile(U8 *buffer, int sx, int sy, spritestorage *store)
{
	U16 *ibuffer = (U16*) buffer;

	for (int y=0; y<sy; y++) {
		long offset = ibuffer[4+y] + 8;

		long x, islast, chunkstart=0;
		do {
			islast   = buffer[offset]   & 0x80;
			long len = buffer[offset++] & 0x7f;
			long ofs = buffer[offset++];

			// fill from beginning of last chunk to start
			// of this one, with "background" colour
			for (x=chunkstart; x<ofs; x++)
				store->nextpixel(0);

			// then copy the number of actual bytes
			for (x=0; x<len; x++) {
				int col = buffer[offset++];
				store->nextpixel(col);
				cused[col/8] |= ( 1 << (col&7) );
			}
			chunkstart = ofs + len;

		} while (!islast);

		// and fill from the end of the last chunk to the
		// end of the line
		for (x=chunkstart; x<sx; x++)
			store->nextpixel(0);

		store->newrow();
	}

	return 1;
}

int decoderegular(U8 *buffer, int sx, int sy, spritestorage *store)
{
	long offset = 8;
	for (int y=0; y<sy; y++) {
		for (int x=0; x<sx; x++)
			store->nextpixel(buffer[offset++]);
		store->newrow();
	}

	return 1;
}

long uncompress(unsigned long size, U8* in, unsigned long *insize, U8* out, unsigned long outsize)
{
	unsigned long inused, datasize, compsize, *testsize;

	memcpy(out, in, 8);

	testsize = &datasize;
	if (SIZEISCOMPRESSED(in))	// size is the compressed size
		testsize = &compsize;

	compsize = 8;		// initially we only have the info data
	datasize = 8;
	in += 8;
	inused = 8;

	while (*testsize < size) {
		S8 code = * ( (S8*) in++);
		if (++inused > *insize)
			break;

		compsize++;

		if (code < 0) {
			U8 ofs = *(in++);
			inused++;

			compsize++;

			unsigned long count = -(code >> 3);
			unsigned long offset = ( (code & 7) << 8) | ofs;

			if (datasize < offset) {
				printf("\nOffset too large!\n");
				exit(2);
			}

			if (datasize + count > outsize)
				return -2*(datasize + count);

			memmove(out+datasize, out + datasize - offset, count);

			datasize += count;
		} else {
			unsigned long skip = code;
			if (code == 0)
				skip = 128;

			if (datasize + skip > outsize)
				return -2*(datasize + skip);

			memmove(out+datasize, in, skip);
			in += skip;
			inused += skip;

			compsize += skip;
			datasize += skip;
		}
	}

	if (inused > *insize) {
		printf("\nNot enough input data for decompression\n");
		exit(2);
	}

	*insize = inused;
	return datasize;
}

int decodesprite(FILE *grf, spritestorage *store, spriteinfowriter *writer)
{
	unsigned long size, orgsize, datasize, inbufsize, outbufsize, startpos;
	U16 wsize;
	U8 info[8];
	U8 *inbuffer, *outbuffer;
	int sx, sy;

	if (!writer || !store) {
		printf("\nparameter is NULL!\n");
		exit(2);
	}

	store->newsprite();

	cfread(&wsize, 2, 1, grf);
	size = wsize;

	if (!size)
		return 0;

	startpos = ftell(grf);
	cfread(info, 1, 1, grf);

	if (info[0] == 0xff) {
		store->setsize(1, 0);
		outbuffer = (U8*) malloc(size);
		//outbuffer[0] = 0xff;
		fread(outbuffer, 1, size, grf);
		writer->adddata(size, outbuffer/*+1*/);
		store->spritedone();
		return 1;
	}

	cfread(info+1, 1, 7, grf);

	orgsize = size;

	// output buffer: y * x_low + x_high<<8 + 8 bytes info
	sx = info[2] + (info[3] << 8);
	sy = info[1];
	outbufsize = 0L + sx * sy + 8;
	store->setsize(sx, sy);

	if (SIZEISCOMPRESSED(info)) {	// compressed size stated
		inbufsize = size;
		// assume uncompressed is max. twice the compressed
		outbufsize <<= 1;
	} else {
		// assume compressed is max 12% larger than uncompressed (overhead etc.)
		inbufsize = size + (size >> 3);
	}

	if (HASTRANSPARENCY(info)) {
		outbufsize += 0L + info[1] * 4;
	}

	inbuffer = (U8*) malloc(inbufsize);
	outbuffer = (U8*) malloc(outbufsize);
	if (!inbuffer || !outbuffer) {
		printf("\nError allocating sprite buffer, want %ld\n", inbufsize + outbufsize);
		exit(2);
	}

	long result;
	do {
		fseek(grf, startpos, SEEK_SET);
		fread(inbuffer, 1, inbufsize, grf);
		result = uncompress(size, inbuffer, &inbufsize, outbuffer, outbufsize);
		if (result < 0) {
			outbufsize = -result;
			outbuffer = (U8*) realloc(outbuffer, outbufsize);
			if (!outbuffer) {
				printf("\nError increasing sprite buffer size\n");
				exit(2);
			}
		}
	} while (result < 0);
	datasize = result;
	fseek(grf, startpos + inbufsize, SEEK_SET);

	writer->addsprite(store->curspritex(), info);

	if (HASTRANSPARENCY(info))	// it's a tile
		result = decodetile(outbuffer, sx, sy, store);
	else
		result = decoderegular(outbuffer, sx, sy, store);

	store->spritedone(sx, sy);


	if (sy > maxy)
		maxy = sy;
	if (sx > maxx)
		maxx = sx;
	if (datasize > (unsigned long) maxs)
		maxs = datasize;

	free(inbuffer);
	free(outbuffer);

	return result;
}

long fakecompress(const U8 *in, long insize, U8 *out, long outsize, U16 *compsize)
{
	long needsize = insize + ((insize + 0x7f) / 0x7f);
	if (outsize < needsize) {
		needsize += needsize / 8;
		return -needsize;
	}

	long inpos = 0, outpos = 0;
	while (inpos < insize) {
		long rest = insize - inpos;
		if (rest > 0x7f)
			rest = 0x7f;

		out[outpos++] = rest;
		memmove(&(out[outpos]), &(in[inpos]), rest);
		outpos += rest;
		inpos += rest;
	}
	if (outpos > 65535) {
		printf("\nSorry, uncompressed sprite too large\n");
		exit(2);
	}
	*compsize = outpos;

	return 1;
}

#ifdef OLDSTRATEGY
// different compression strategies for identifying repetition
multitype strategy1(const U8* data, unsigned int datasize, unsigned int datamax)
{
	int overlap;
	multitype ret;
	unsigned int ofs, foundofs = GREP_NOMATCH;

	// can only find up to 11 bits back
	if (datasize >= (1 << 11)) {
		ofs = datasize - (1 << 11) + 1;
		data+=ofs;
		datasize-=ofs;
	}
	for (overlap=MAXOVERLAP;overlap >= MINOVERLAP;overlap--) {
		// see if we can find a repetition of length overlap

		// allow overlapping memory blocks?
#if 0
		foundofs = grepmem(data, datasize+overlap-1, data+datasize, overlap, 0);
#else
		foundofs = grepmem(data, datasize, data+datasize, overlap, 0);
#endif

		if (foundofs != GREP_NOMATCH) {		// found one
			if (foundofs > datasize) {
				foundofs = GREP_NOMATCH;	// not really
				continue;
			}
			ofs = datasize - foundofs;
			if (ofs + overlap > datasize)
				overlap = datasize - ofs;
			if (datasize + overlap > datamax)
				overlap = datamax - datasize;
			break;
		}
	}

	ret.u16[0] = ofs;
	ret.u8[2] = overlap;
	ret.u8[3] = (foundofs != GREP_NOMATCH) && (overlap >= MINOVERLAP);
	return ret;
}
#endif

// use strategy from LZ77 algorithm
//
// data is the beginning of the sprite data
// datasize is how much has been processed so far, and can be used for
//	the repetition finding
// datamax is the entire size of the sprite data
multitype strategy2(const U8* data, unsigned int datasize, unsigned int datamax)
{
	unsigned int overlap = 0, newoverlap, remain, minoverlap, maxoverlap;
	multitype ret;
	int foundofs = -1;
	const U8 *found;

	// can only find up to 11 bits back
	if (datasize >= (1 << 11)) {
		unsigned int ofs = datasize - (1 << 11) + 1;
		data+=ofs;
		datasize-=ofs;
		datamax-=ofs;
	}

	remain = datasize;
	found = data-1;
	maxoverlap = MAXOVERLAP;

	while (overlap < maxoverlap) {

		// minoverlap is minimum overlap we need to find minus one
		minoverlap = overlap;
		if (minoverlap < MINOVERLAP-1)
			minoverlap = MINOVERLAP-1;

		if (remain <= minoverlap)
			break;		// don't have more data to find longer overlap

		int i;
		for (i=remain-minoverlap; i; i--)
			if ( *(++found) == data[datasize])
				break;

		if (!i)
			break;

		foundofs = (int) (found - data);
		remain = datasize - foundofs - 1;

		maxoverlap = datasize - foundofs;
		if (maxoverlap > MAXOVERLAP)
			maxoverlap = MAXOVERLAP;		// can't store more anyway
		if (maxoverlap > datamax - datasize)
			maxoverlap = datamax - datasize;	// can't use more

		newoverlap = 1;
		while ( (newoverlap < maxoverlap) && (found[newoverlap] == data[datasize+newoverlap]) )
			newoverlap++;

		if (newoverlap < MINOVERLAP)
			continue;

		if (newoverlap > overlap) {	// a longer chunk
			ret.u16[0] = datasize - foundofs;
			overlap = newoverlap;
		}
	}
	ret.u8[2] = overlap;
	ret.u8[3] = (overlap >= MINOVERLAP);
	return ret;
}

multitype strategy3(const U8* data, unsigned int datasize, unsigned int datamax);
// in assembly optimized source if used

#ifdef _MSC_VER
#pragma warning(disable:4701)//chunk may be used uninitialized
#endif

long realcompress(const U8 *in, long insize, U8 *out, long outsize, U16 *compsize)
{
	long inpos = 0, outpos = 0;
	long inposm8, outposm8;
	U8 *lastcodepos = out;
	multitype chunk;

	int stats[16];
	memset(stats, 0, 16*sizeof(int));	// for debugging

	out[outpos++] = 1;
	out[outpos++] = in[inpos++];

	while (inpos < insize) {
		inposm8 = inpos - 8;	// for debugging only
		outposm8 = outpos - 8;

		// search for where the first repetition of >= 3 chars occurs
		int overlap;
		int ofsh,ofsl;

		//	chunk = strategy1(in, inpos, insize);
		chunk = strategy2(in, inpos, insize);

		if (chunk.u8[3]) {		//  Yay! Found one!
			if (!*lastcodepos)	// there's a zero length verbatim chunk -> delete it
				outpos--;

			ofsl = chunk.u8[0];
			ofsh = chunk.u8[1];
			overlap = chunk.u8[2];

			out[outpos++] = (-overlap << 3) | ofsh;
			out[outpos++] = ofsl;

			out[outpos] = 0;	// start new interim verbatim chunk
			lastcodepos = &(out[outpos++]);
			inpos += overlap;
			stats[overlap]++;
		} else {	//  no we didn't. Increase length of verbatim chunk
			if (*lastcodepos == 0x7f) {	// maximum length 127
				lastcodepos = &(out[outpos++]);	// start new one
				*lastcodepos = 0;
			}
			(*lastcodepos)++;
			out[outpos++] = in[inpos++];
			stats[0]++;
		}
		if (outpos + 2 >= outsize) {
			// buffer too small, estimate expected size
			long needed = (long) outpos * (long) insize / (long) inpos;
			needed += needed >> 3;	// add 12% extra
			return -needed;
		}
	}

	if (inpos != insize) {
		printf("\nError: compressed %ld bytes too much: %ld not %ld!",
			inpos - insize, inpos, insize);
		printf("\nLast chunk was repetition=%d, len=%d, ofs=%d\n",
			chunk.u8[3], chunk.u8[2], chunk.u16[0]);
		exit(2);
	}

	if (!*lastcodepos)
		outpos--;

	*compsize = outpos;

	return 1;
}

#ifdef _MSC_VER
#pragma warning(default:4701)
#endif

U16 lasttilesize;

U16 getlasttilesize()
{
	return lasttilesize;
}

U16 encodetile(FILE *grf, const U8 *image, long imgsize, U8 background, int sx, int sy, const U8 inf[8], int docompress)
{
	long tilesize = imgsize + 16L * sy;

	while (1) {	// repeat in case we didn't allocate enough memory
		U8 *tile = (U8*) malloc(tilesize);
		if (!tile) {
			printf("\nError: can't allocate %ld bytes for tile memory\n",
				tilesize);
			exit(2);
		}

		long tileofs = 2L * sy;		// first sy (int) offsets, then data

		U16 *lineofs = (U16*) tile;
		int y;

		for (y=0; y<sy; y++) {
			int x1 = 0, x2 = 0;

			lineofs[y] = tileofs;
			long lastlenofs = tileofs;

			while ( (x1 < sx) && (tileofs + 2 + sx < tilesize) ) {
				// find where next non-transparent part starts
				while ( (x1 < sx) && (image[y*sx+x1] == background) )
					x1++;

				if (x1 < sx) {
					int len = 1;
					// ...and where it ends
					x2 = x1 + 1;
					while ( (x2 < sx) && (len < 0x7f) && (image[y*sx+x2] != background) ) {
						len++;
						x2++;
					}


					lastlenofs = tileofs;
					tile[tileofs++] = len;
					tile[tileofs++] = x1;
					memmove( &(tile[tileofs]), &(image[y*sx+x1]), len);

					tileofs += len;
				} else {	// transparent to end of line
					if (x2 == 0) {	// totally empty line
						tile[tileofs++] = 0;
						tile[tileofs++] = 0;
					}
					x2 = x1;
				}
				x1 = x2;
			}
			tile[lastlenofs] |= 0x80;
			if (tileofs + 2 + sx >= tilesize)
				break;
		}

		if (tileofs + 2 + sx >= tilesize) {
			// tile didn't hold all data, estimate real size
			// and enlarge it
			free(tile);
			long imgofs = y*sx + 1L;
			tilesize = tilesize * imgsize / imgofs;
			tilesize += (tilesize >> 3) + 16L;
			continue;
		}

		lasttilesize = tileofs;

		int result = encoderegular(grf, tile, tileofs, inf, docompress);
		free(tile);
		return result;
	}
}

U16 encoderegular(FILE *grf, const U8 *image, long imgsize, const U8 inf[8], int docompress)
{
	long compsize = imgsize + 24 + 8, uncompsize = compsize + 8;
	unsigned int size;

	U8 *compr = (U8*) malloc(compsize);
	U8 *uncomp = (U8*) malloc(uncompsize);
	if (!compr || !uncomp) {
		printf("\nError: can't allocate %ld bytes for compressed buffer\n",
			compsize + uncompsize);
		exit(2);
	}

	long result;
	U16 realcompsize;
	while (1) {
		memcpy(compr, inf, 8);
		if (docompress)
			result = realcompress(image, imgsize, compr+8, compsize-8, &realcompsize);
		else
			result = fakecompress(image, imgsize, compr+8, compsize-8, &realcompsize);

		if (SIZEISCOMPRESSED(inf))	// write compressed size
			size = realcompsize + 8;
		else
			size = imgsize + 8;

		if (result > 0) {
			do {	// everything was good

				// check that the compression is correct, by uncompressing again
				unsigned long insize = realcompsize + 8;
				result = uncompress(size, compr, &insize, uncomp, uncompsize);
				if (result < 0) {
					uncompsize = -result;
					uncomp = (U8*) realloc(uncomp, uncompsize);
					if (!uncomp) {
						printf("\nError increasing sprite buffer size\n");
						exit(2);
					}
				}
				// and verifying
				if ((result-imgsize-8) || memcmp(uncomp+8, image, imgsize)) {
					printf("\nError: invalid compression, ");
					if (result-imgsize-8)
						printf("length diff %ld, ", result-imgsize-8);
					else {
						int i;
						for (i=0; uncomp[i+8]==image[i]; i++);
						printf("data diff at %d of %ld bytes, ", i, imgsize);
					}
					if (docompress) {
						puts("trying without it for this sprite\n");
						docompress = 0;
					} else {
						printf("even uncompressed, aborting.\n");
						exit(2);
					}
					result = 0;
				}
			} while (result < 0);
			if (result)
				break;
		} else if (result < 0) {	// buffer was too small
			compsize = -result;
			compr = (U8*) realloc(compr, compsize);
			if (!compr) {
				printf("\nError: can't allocate %ld bytes for compressed buffer\n",
					compsize);
				exit(2);
			}
		} else {
			printf("\nError: unknown error while compressing\n");
			exit(2);
		}
	}

	fwrite(&size, 1, 2, grf);
	fwrite(compr, 1, realcompsize + 8, grf);

	free(compr);
	free(uncomp);

	return realcompsize;
}

