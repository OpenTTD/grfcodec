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

#include "pcx.h"
#include "typesize.h"

#ifdef _SPRITES_C
#	define SPRITES_EXTERN
#	define SPRITES_INIT = 0
#else
#	define SPRITES_EXTERN extern
#	define SPRITES_INIT
#endif

// minimum and maximum overlap to search for in the compression routines
#define MINOVERLAP 3
#define MAXOVERLAP 15	// must be <= 15 b/o how it's encoded



class spriteinfowriter : virtual public bandnotify {
	public:
	virtual void addsprite(int /*x*/, U8 /*info*/[8]) { };
	virtual void adddata(U16 /*size*/, U8 * /*data*/) { };
};

class spritestorage {
	public:
	virtual ~spritestorage(){}
	virtual void newsprite() {};
	virtual void setsize(int /*sx*/, int /*sy*/) {};
	virtual int  curspritex() {return 0;};
	virtual void newrow() {};
	virtual void nextpixel(U8 /*colour*/) {};
	virtual void spritedone() {};
};


SPRITES_EXTERN U8 cused[256/8];
SPRITES_EXTERN int maxx SPRITES_INIT, maxy SPRITES_INIT, maxs SPRITES_INIT;

void cfread (void *ptr, size_t size, size_t n, FILE *stream);
void cfwrite (void *ptr, size_t size, size_t n, FILE *stream);
long fcopy(FILE *one, FILE *two, long bytes);

int decodetile(U8 *buffer, int sx, int sy, spritestorage *store, FILE *info);
int decoderegular(U8 *buffer, int sx, int sy, spritestorage *store, FILE *info);
int decodesprite(FILE *grf, spritestorage *store, spriteinfowriter *writer);

U16 getlasttilesize();
U16 encodetile(FILE *grf, U8 *image, long imgsize, U8 background, int sx, int sy, U8 inf[8], int docompress);
U16 encoderegular(FILE *grf, U8 *image, long imgsize, U8 inf[8], int docompress);

#endif /* _SPRITES_H */
