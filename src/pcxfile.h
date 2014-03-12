#ifndef _PCXFILE_H
#define _PCXFILE_H

/*****************************************\
*                                         *
* PCXFILE.H - A class to combine several  *
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
#include <stdio.h>
#include <string.h>

#include "typesize.h"
#include "file.h"

/** Definition of a common pixel in GRFCodec's realm. */
struct CommonPixel {
	U8 r;  ///< Red-channel
	U8 g;  ///< Green-channel
	U8 b;  ///< Blue-channel
	U8 a;  ///< Alpha-channel
	U8 m;  ///< Remap-channel

	explicit CommonPixel(U8 r = 0, U8 g = 0, U8 b = 0, U8 a = 0, U8 m = 0) : r(r), g(g), b(b), a(a), m(m) {}

	bool IsTransparent(bool rgba) const
	{
		return rgba ? this->a == 0 : this->m == 0;
	}

	U8 *Encode(U8 *buffer, bool has_mask, bool rgba) const
	{
		if (rgba) {
			*buffer++ = this->r;
			*buffer++ = this->g;
			*buffer++ = this->b;
			*buffer++ = this->a;
		}
		if (has_mask) *buffer++ = this->m;
		return buffer;
	}

	const U8 *Decode(const U8 *buffer, bool has_mask, bool rgba)
	{
		if (rgba) {
			this->r = *buffer++;
			this->g = *buffer++;
			this->b = *buffer++;
			this->a = *buffer++;
		}
		if (has_mask) this->m = *buffer++;
		return buffer;
	}

	void MakeTransparent()
	{
		this->r = 0;
		this->g = 0;
		this->b = 0;
		this->a = 0;
		this->m = 0;
	}
};

struct pcxheader {
	U8 manuf;	// 10
	U8 version;	// 5
	U8 encoding;	// 1
	U8 bpp;		// 1, 2, 4 or 8 (per plane)
	U16 window[4];	// x1,y1,x2,x2
	U16 dpi[2];	// x,y
	U8 cmap[48];	// colormap
	U8 reserved;	// 0
	U8 nplanes;
	S16 bpl;	// bytes per line (per plane)
	S16 palinfo;	// 1=colour
	S16 screen[2];	// x,y
	U8 filler[54];
};

class pcxfile {
	public:
	virtual ~pcxfile();

	virtual const char *filename() { return mfile->filename(); };
	virtual FILE *getnextfile() { return mfile->nextfile(); };
	virtual FILE *getcurfile() { return mfile->curfile(); };
	virtual void filedone(int /*final*/) { };
	virtual void filestart(bool)
		{ fseek(curfile, sizeof(header), SEEK_SET); };

	void setfile(multifile *mfile);

	void newfile(bool paletted, int sx);
	void newheader(int sx);
	void startimage(bool paletted, int sx, int sy, int bandx, int bandy);
	void alloclines(int newlines);
	void expirelines(int oldlines);
	void initline(int y);
	virtual void setline(CommonPixel* /*band*/) { };
	void endimage();

	virtual void startsubimage(int /*x*/, int /*y*/, int /*sx*/, int /*sy*/) { };
	void newline();
	void streamputpixel(CommonPixel colour);
	void streamputpixel(CommonPixel *buffer, unsigned long datasize);
private:
	CommonPixel streamgetpixel();
public:
	void streamgetpixel(CommonPixel *buffer, unsigned long datasize);

	void installreadmap(int *map);
	void installwritemap(int *map);

	void putpixel(int x, int y, CommonPixel colour);
	//U8 getpixel(int x, int y);
	int  subimagex()
		{ return subx + dx; }
	int  subimagey()
		{ return totaly + dy; }
	void endsubimage();

	const char *getdirectory() { return mfile->getdirectory(); };

	void be_swapheader(pcxheader& header);

	protected:
	pcxfile();

	void newband();

	void startdecoding();
	void startencoding();
	virtual void encodebytes(const CommonPixel &pixel, int num);
	virtual void encodebytes(const CommonPixel *buffer, int num);
	void decodebytes(CommonPixel buffer[], int num);
	void endencoding();

	int subofsx(int x, int checkbound);
	int subofsy(int y, int checkbound);

	FILE *curfile;
	pcxheader header;
	int sx, sy,			// size of the PCX image
	    subx,			// current subimage x offset
	    px, py,			// subimage size x, y
	    cx, cy,			// subimage stream ptr x, y
	    dx, dy,			// subimage pos x, y (rel to subx, 0)
	    bandx, bandy, thisbandy,	// default band x, y, current band y
	    totaly;			// total lines written so far
	int bandlines;
	int codecing;
	bool paletted;
	class colourmap{
	public:
		colourmap():map(new int[256]),deletemap(true){
			for(int i=0;i<256;i++)
				map[i]=i;
		}
		~colourmap(){
			if(deletemap) delete[] map;
		}
		const colourmap&operator=(int*p){
			if(deletemap) delete map;
			deletemap=false;
			map=p;
			return*this;
		}
		int operator[](int x){return map[x];}
	private:
		int*map;
		bool deletemap;
		void operator=(const colourmap&);// Not assignable
		colourmap(const colourmap&);// not copyable
	} getcolourmap, putcolourmap;

	CommonPixel **band;

	private:

	multifile *mfile;

	U8 last;
	U8 run;
};


#endif /* _PCXFILE_H */
