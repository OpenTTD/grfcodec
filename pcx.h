#ifndef _PCX_H
#define _PCX_H

/*****************************************\
*                                         *
* PCX.H - A class to combine several      *
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

struct pcxheader {
	U8 manuf;	// 10
	U8 version;	// 5
	U8 encoding;	// 1
	U8 bpp;		// 1, 2, 4 or 8 (per plane)
	S16 window[4];	// x1,y1,x2,x2
	S16 dpi[2];	// x,y
	U8 cmap[48];	// colormap
	U8 reserved;	// 0
	U8 nplanes;
	S16 bpl;	// bytes per line (per plane)
	S16 palinfo;	// 1=colour
	S16 screen[2];	// x,y
	U8 filler[54];
};

class multifile {
	public:
	multifile() {};
	virtual ~multifile() {};
	virtual FILE *curfile()  { return NULL; };
	virtual FILE *nextfile() { return NULL; };
	virtual const char *filename() { return NULL; };
	virtual const char *getdirectory() { return NULL; };
};

class singlefile : virtual public multifile {
	public:
	singlefile(const char *filename, const char *mode, const char *dir);
	singlefile(FILE *file, const char *dir);
	virtual ~singlefile();

	void setfile(FILE *file);

	virtual FILE *curfile()  { return thefile; };
	virtual const char *filename() { return thefilename; };
	virtual const char *getdirectory() { return directory; };

	private:
	char *thefilename, *directory;
	FILE *thefile;
	int autoclose;
};

class bandnotify {	// used when a band is written to the PCX file
	public:
	virtual ~bandnotify(){}
	virtual void newband(class pcxfile* /*pcx*/) { };
};

class pcxfile {
	public:
	virtual ~pcxfile();

	virtual const char *filename() { return mfile->filename(); };
	virtual FILE *getnextfile() { return mfile->nextfile(); };
	virtual FILE *getcurfile() { return mfile->curfile(); };
	virtual void filedone(int /*final*/) { };
	virtual void filestart() { };

	void setfile(multifile *mfile);

	void newfile(int sx);
	void newheader(int sx);
	void startimage(int sx, int sy, int bandx, int bandy, bandnotify *notify);
	void alloclines(int newlines);
	void expirelines(int oldlines);
	void initline(int y);
	virtual void setline(U8* /*band*/) { };
	void endimage();

	virtual void startsubimage(int /*x*/, int /*y*/, int /*sx*/, int /*sy*/) { };
	void newline();
	void streamputpixel(U8 colour);
	void streamputpixel(U8 *buffer, unsigned long datasize);
	U8 streamgetpixel();
	void streamgetpixel(U8 *buffer, unsigned long datasize);

	void installreadmap(int *map);
	void installwritemap(int *map);

	void putpixel(int x, int y, U8 colour);
	//U8 getpixel(int x, int y);
	int  subimagex()
		{ return subx + dx; }
	int  subimagey()
		{ return totaly + dy; }
	void endsubimage();

	const char *getdirectory() { return mfile->getdirectory(); };

	protected:
	pcxfile();

	void newband();

	void startdecoding();
	void startencoding();
	void encodebytes(U8 byte, int num);
	void encodebytes(U8 buffer[], int num);
	void decodebytes(U8 buffer[], int num);
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
	bandnotify *notify;
	class colourmap{
	public:
		colourmap():map(new int[256]),deletemap(true){
			for(int i=0;i<256;i++)
				map[i]=i;
		}
		~colourmap(){
			if(deletemap) delete map;
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

	private:

	multifile *mfile;
	U8 **band;

	U8 last;
	U8 run;
};


#endif /* _PCX_H */
