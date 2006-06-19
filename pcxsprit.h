#ifndef _PCXSPRIT_H
#define _PCXSPRIT_H


#include "pcx.h"
#include "sprites.h"

class pcxwrite : public pcxfile, public spritestorage {
	public:
	pcxwrite(multifile *mfile);

	virtual void filedone(int final);
	virtual void filestart() { writeheader(); };

	virtual void newsprite() {spriteno++; };
	virtual void startsubimage(int x, int y, int sx, int sy);
	virtual void setline(U8 *band);

	virtual void setsize(int sx, int sy)
		{ startsubimage(-1, -1, sx, sy); };
	virtual int  curspritex() {return subimagex();};
	virtual void newrow() { newline(); };
	virtual void nextpixel(U8 colour) { streamputpixel(colour); };
	virtual void spritedone() { endsubimage(); };
	virtual void spritedone(int sx, int sy);


	void writeheader();
	void writepal();

	void setcolours(U8 bg, U8 bord, int skip);
	void setpalette(U8 *palette);
	//void setpalette(FILE *palfile);

	protected:

	private:
	void showspriteno();

	U8 *palette;

	int borderskip, spriteno, lastdigitx;
	U8 background;
	U8 border;

	pcxwrite(const pcxwrite&);//not copyable: pcxfile::colormap
	void operator=(const pcxwrite&);//not assignable: pcxfile::colormap
};

class pcxread : public pcxfile {
	public:
	pcxread(singlefile *mfile);

	virtual void filestart() { readheader(); };

	virtual void startsubimage(int x, int y, int sx, int sy);
	virtual void setline(U8 *band);

	void readheader();

	private:
	multifile *mfile;
	pcxread(const pcxread&); //not copyable: pcxfile::colormap
	void operator=(const pcxread&); //not assignable: pcx::colormap
};

#endif /* _PCXSPRIT_H */
