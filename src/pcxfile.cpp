/*****************************************\
*                                         *
* PCX.CC - A class to combine several     *
*         small images in one pcx file    *
*         as well as read them out        *
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
//#include <mem.h>

#include "pcxfile.h"
#include "error.h"


/***********************\
*						*
* class pcxfile			*
*						*
\***********************/

pcxfile::pcxfile()
{
	if (sizeof(pcxheader) != 128) {
		printf("Somebody's packing the PCX header, it's not 128 bytes large!");
		exit(254);
	}

	sx = -1;
	sy = -1;
	totaly = 0;
	subx = 0;
	px = 0;
	py = 0;
	bandy = -1;
	bandlines = 0;
	band = NULL;
	run = 0;
	curfile = NULL;
	mfile = NULL;
	header.window[2] = 65535;
	codecing = 0;
	paletted = true;

}

pcxfile::~pcxfile()
{
	delete(mfile);

	if (band) {
		for (int i=0; i<bandlines; i++)
			delete[](band[i]);
		delete[](band);
	}
}

void pcxfile::setfile(multifile *mfile)
{
	pcxfile::mfile = mfile;
	curfile = mfile->curfile();
}

void pcxfile::newfile(bool paletted, int sx)
{
	if (curfile) {    	// do we already have a file
		filedone(1);
	}

	// is it really a new file, or still the old one?
	if (getnextfile() || (header.window[2] == 65535)) {
		curfile = getcurfile();
		newheader(sx);
		totaly = 0;
		subx = 0;
		px = 0;
		py = 0;
	}

	this->paletted = paletted;
	filestart(paletted);
}

void pcxfile::newheader(int sx)
{
	const static pcxheader baseheader =
	{ 10, 5, 1, 8,		// manuf, version, encoding, bpp
	{0, 0, 65535, 65535},	// window[4]
	{72, 72},		// dpi[2]
	{0},			// cmap[48]
	0, 1, -1, 1,		// reserved, nplanes, bpl, palinfo
	{-1, -1},		// screen[2]
	{0 }};			// filler[54]

	header = baseheader;
	header.window[2] = sx - 1;
	header.screen[0] = sx - 1;
	header.bpl = sx;
}

void pcxfile::startimage(bool paletted, int sx, int sy, int bandx, int bandy)
{
	pcxfile::sx = sx;
	pcxfile::sy = sy;
	pcxfile::bandx = bandx;
	pcxfile::bandy = bandy;

	newfile(paletted, sx);

	if (bandy > bandlines)
		alloclines(bandy);

	thisbandy = bandy;
}

void pcxfile::alloclines(int newlines)
{
	CommonPixel **newband, **oldband = band;

	if (newlines <= bandlines)
		return;

	newband = new CommonPixel*[newlines];
	if (!newband) {
		printf("%s: Error allocating band array\n", this->filename());
		exit(2);
	}

	int i;
	for (i=0; i<bandlines; i++)
		newband[i] = oldband[i];

	for (i=bandlines; i<newlines; i++) {
		newband[i] = new CommonPixel[sx];
		if (!(newband[i])) {
			printf("%s: Error allocating new band lines\n", this->filename());
			exit(2);
		}
	}

	delete[](band);

	band = newband;
	i = bandlines;
	bandlines = newlines;

	for (; i<newlines; i++)
		initline(i);
}

void pcxfile::expirelines(int oldlines)
{
	for (int i=0; i<oldlines; i++) {
		CommonPixel *old = band[0];
		memmove( &(band[0]), &(band[1]), (bandlines-1)*sizeof(band[0]));
		band[bandlines-1] = old;

		// Attempt to prevent reading past the end of file
		if (i + totaly + bandlines < pcxfile::sy)
			initline(bandlines - 1);
	}
}

void pcxfile::initline(int y)
{
	if (y >= bandlines) {
		printf("%s: y=%d larger than bandlines=%d\n", this->filename(), y, bandlines);
		exit(2);
	}
	setline(band[y]);
};


void pcxfile::endimage()
{
	if (px)
		newband();

	if (run)
		endencoding();

	if (band) {
		int i;
		for (i=0; i<bandlines; i++)
			delete[](band[i]);
		delete[](band);
		band = NULL;
	}

	filedone(1);
}

void pcxfile::newline()
{
	cx = 0;
	cy++;
}

void pcxfile::streamputpixel(CommonPixel colour)
{
	int x = subofsx(cx, 1);
	int y = subofsy(cy, 1);

	if (putcolourmap[colour.m] == -1) {
		printf("%s: Agh! Putting colour %d but it has no map!\n", this->filename(), colour.m);
		exit(2);
	}

	band[y][x] = colour; // Colour mapping moved to pcxwrite::spritedone
	cx++;
}

void pcxfile::streamputpixel(CommonPixel *buffer, unsigned long datasize)
{
	for (unsigned long i=0; i<datasize; i++) {
		streamputpixel(buffer[i]);
		if ( (i % (px-1)) == (unsigned long) px-2)
			newline();
	}
}

CommonPixel pcxfile::streamgetpixel()
{
	int x = subofsx(cx, 0);
	int y = subofsy(cy, 0);

	cx++;
	if (x < 0 || y < 0)
		return CommonPixel(0, 0, 0, 0, 255);
	else
		return band[y][x];
}

extern bool _mapAll;

void pcxfile::streamgetpixel(CommonPixel *buffer, unsigned long datasize)
{
	unsigned long i=0;
	CommonPixel colour;
	bool maybeGlyph=!_mapAll;
	for (; i<datasize; i++) {
		colour = streamgetpixel();
		maybeGlyph &= (colour.m < 3);
		buffer[i] = colour;
		if ( (i % px) == (unsigned long) px-1)
			newline();
	}
	if (!maybeGlyph)
		for(i=0; i<datasize; i++)
			buffer[i].m = getcolourmap[buffer[i].m];
}

void pcxfile::putpixel(int x, int y, CommonPixel colour)
{
	if (putcolourmap[colour.m] == -1) {
		printf("%s: Agh! Putting colour %d but it has no map!\n", this->filename(), colour.m);
		exit(2);
	}

	colour.m = putcolourmap[colour.m];
	band[subofsy(y, 1)][subofsx(x, 1)] = colour;
}

/*U8 pcxfile::getpixel(int x, int y)
{
	int sx = subofsx(x, 0);
	int sy = subofsy(y, 0);
	int colour;
	if (sx < 0 || sy < 0)
		colour = 255;
	else
		colour = band[sy][sx];
	if (getcolourmap[colour] == -1) {
		printf("Agh! Getting colour %d but it has no map!\n", colour);
		exit(2);
	}

	return getcolourmap[colour];
}*/

void pcxfile::endsubimage()
{
}

void pcxfile::newband()
{
	int i, y;

	if ( (totaly + thisbandy > sy) && (thisbandy <= sy) ) {	// would be too large
		newfile(this->paletted, sx);
	}

	totaly += thisbandy;

	for (y=0; y<thisbandy; y++) {
		startencoding();
		encodebytes(band[y], sx);
		endencoding();
	}

	for (i=0; i<bandlines; i++)
		initline(i);

	subx=0;
	px=0;

	thisbandy = bandy;

	// while testing only...
//  filedone(0);
}

void pcxfile::startencoding()
{
	if (codecing == 1)
		return;

	run = 0;
	codecing = 1;
}

void pcxfile::startdecoding()
{
	if (codecing == 2)
		return;

	run = 0;
	codecing = 2;
}

void pcxfile::encodebytes(const CommonPixel *buffer, int num)
{
	int thisrun;
	U8 byte;

	if (codecing != 1) {
		printf("%s: I'm not encoding, but got a byte?\n", this->filename());
		exit(2);
	}

	while (num) {
		byte = (buffer++)->m;
		num--;

		thisrun = 1;
		while (num && (byte == buffer->m) && (thisrun < 0x3f)) {
			thisrun++;
			buffer++;
			num--;
		}

		if ((thisrun > 1) || ( (byte & 0xc0) == 0xc0) )
			fputc(thisrun | 0xc0, curfile);
		fputc(byte, curfile);
	}
}

void pcxfile::encodebytes(const CommonPixel &pixel, int num)
{
	U8 byte = pixel.m;
	int thisrun;

	if (codecing != 1) {
		printf("%s: I'm not encoding, but got a byte?\n", this->filename());
		exit(2);
	}

	while (num) {
		thisrun = num;
		if (thisrun > 0x3f)
			thisrun = 0x3f;

		if ((thisrun > 1) || ( (byte & 0xc0) == 0xc0) )
			fputc(thisrun | 0xc0, curfile);
		fputc(byte, curfile);
		num -= thisrun;
	}
}

void pcxfile::decodebytes(CommonPixel buffer[], int num)
{
	U8 byte = 0;
	int thisrun, used;

	if (codecing != 2) {
		printf("%s: I'm not decoding, but am supposed to return a byte?\n", this->filename());
		exit(2);
	}

	thisrun = run;
	if (thisrun) {
		used = run;
		if (used > num)
			used = num;
		for (int i = 0; i < used; i++) buffer[i].m = byte;
		buffer += used;
		num -= used;
		thisrun -= used;
	}

	while (num) {
		byte = fgetc(curfile);
		if ( (byte & 0xc0) == 0xc0) {	// it's a run of equal bytes
			thisrun = byte & 0x3f;
			byte = fgetc(curfile);

			used = thisrun;
			if (used > num)
				used = num;
			for (int i = 0; i < used; i++) buffer[i].m = byte;
			buffer += used;
			num -= used;
			thisrun -= used;
		} else {
			(buffer++)->m = byte;
			num--;
		}
	}
	run = thisrun;

	return;
}

void pcxfile::endencoding()
{
	//  if (codecing == 1)
	//	encodebytes(last - 1);
	//  run = 0;
	codecing = 0;
}

int pcxfile::subofsx(int x, int checkbound)
{
	int ofsx = subx + x + dx;
	if (ofsx >= sx) {
		if (checkbound) {
			printf("\n%s: ofsx too large: is %d=%d+%d+%d, sx=%d\n", this->filename(), ofsx, subx, x, dx, sx);
			exit(2);
		} else {
			return -1;
		}
	}
	return ofsx;
}

int pcxfile::subofsy(int y, int checkbound)
{
	int ofsy = y + dy;
	if (ofsy >= bandlines) {
		if (checkbound) {
			printf("\n%s: ofsy too large\n", this->filename());
			exit(2);
		} else {
			return -1;
		}
	}
	return ofsy;
}

void pcxfile::installreadmap(int *map)
{
	getcolourmap = map;
}

void pcxfile::installwritemap(int *map)
{
	putcolourmap = map;
}

#ifdef GRFCODEC_BIG_ENDIAN
void pcxfile::be_swapheader(pcxheader& header)
{
	header.window[0] = BE_SWAP16(header.window[0]);
	header.window[1] = BE_SWAP16(header.window[1]);
	header.window[2] = BE_SWAP16(header.window[2]);
	header.window[3] = BE_SWAP16(header.window[3]);
	header.dpi[0] = BE_SWAP16(header.dpi[0]);
	header.dpi[1] = BE_SWAP16(header.dpi[1]);
	header.bpl = BE_SWAP16(header.bpl);
	header.palinfo = BE_SWAP16(header.palinfo);
	header.screen[0] = BE_SWAP16(header.screen[0]);
	header.screen[1] = BE_SWAP16(header.screen[1]);
}
#else
void pcxfile::be_swapheader(pcxheader&){}
#endif
