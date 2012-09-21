#ifndef _SPRITES_H
#define _SPRITES_H

/*****************************************\
*                                         *
* SPRITES.H - A couple of routines to     *
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

#include <stdio.h>
#include <stdarg.h>
#include <string>

using namespace std;

#define ZOOM_LEVELS (6)
extern const char *zoom_levels[ZOOM_LEVELS];

#define DEPTHS (3)
extern const char *depths[DEPTHS];

static const int DEPTH_8BPP  = 0;
static const int DEPTH_32BPP = 1;
static const int DEPTH_MASK  = 2;

#include "pcxfile.h"
#include "typesize.h"

// Define some of the bits in SpriteInfo::info
#define DONOTCROP(info) (info & 64)
#define HASTRANSPARENCY(info) (info & 8)
#define SIZEISCOMPRESSED(info) (info & 2)


// minimum and maximum overlap to search for in the compression routines
#define MINOVERLAP 3
#define MAXOVERLAP 15	// must be <= 15 b/o how it's encoded

/** Information about a single sprite. */
struct SpriteInfo {
	U8 info;  ///< Info byte; bit 1: size is compressed size, bit 3: tile transparancy, value 0xFF: special sprite.
	U8 depth; ///< The "depth" of the image.
	U8 zoom;  ///< The zoom level.
	U16 ydim; ///< Number of lines in the sprite.
	U16 xdim; ///< Number of columns in the sprite.
	S16 xrel; ///< Horizontal offset
	S16 yrel; ///< Vertical offset

	string name;
	int xpos,ypos,imgsize;
	bool forcereopen;

	void writetobuffer(U8 *buffer, int grfcontversion);
	void readfromfile(const char *action, int grfcontversion, FILE *grf, int spriteno);
	static int Size(int grfcontversion) { return grfcontversion == 2 ? 10 : 8; }
};

class spriteinfowriter {
	public:
	~spriteinfowriter() {}
	virtual void addsprite(bool /* first */, const char * /* filename */, int /* y */, int /*x*/, SpriteInfo /*info*/) { };
	virtual void adddata(uint /*size*/, U8 * /*data*/) { };
};

class spritestorage {
	public:
	virtual ~spritestorage(){}
	virtual void newsprite() {};
	virtual void setsize(int /*sx*/, int /*sy*/) {};
	virtual int  curspritex() {return 0;};
	virtual int  curspritey() {return 0;};
	virtual const char *filename(){return NULL;};
	virtual void newrow() {};
	virtual void nextpixel(CommonPixel /*colour*/) {};
	virtual void spritedone(int /*sx*/, int /*sy*/) {};
	virtual void spritedone() {};
};


extern int maxx, maxy, maxs;

int decodesprite(FILE *grf, spritestorage *imgpal, spritestorage *imgrgba, spriteinfowriter *writer, int spriteno, U32 *dataoffset, int grfcontversion);

long getlasttilesize();
long encodetile(U8 **compressed_data, long *uncompressed_size, const CommonPixel *image, long imgsize, int sx, int sy, SpriteInfo inf, int docompress, int spriteno, bool has_mask, bool rgba, int grfcontversion);
long encoderegular(U8 **compressed_data, const U8 *image, long imgsize, SpriteInfo inf, int docompress, int spriteno, int grfcontversion);
void writesprite(FILE *grf, const U8 *compressed_data, int compressed_size, int uncompressed_size, SpriteInfo inf, int spriteno, int grfcontversion);
void writespritesize(const char *action, unsigned int spritesize, int grfcontversion, FILE *grf);
void writeword(const char *action, unsigned int value, FILE *grf);
void writedword(const char *action, unsigned int value, FILE *grf);

unsigned int readspritesize(const char *action, int grfcontversion, FILE *grf);
U16 readword(const char *action, FILE *grf);
U32 readdword(const char *action, FILE *grf);

#endif /* _SPRITES_H */
