#ifndef _PCXSPRIT_H
#define _PCXSPRIT_H


#include "pcxfile.h"
#include "sprites.h"

class pcxwrite : public pcxfile, public spritestorage {
	public:
	pcxwrite(multifile *mfile);

	virtual void filedone(int final);
	virtual void filestart(bool paletted);

	virtual void newsprite() {spriteno++; };
	virtual void startsubimage(int x, int y, int sx, int sy);
	virtual void setline(CommonPixel *band);

	virtual void setsize(int sx, int sy)
		{ startsubimage(-1, -1, sx, sy); };
	virtual int  curspritex() {return subimagex();};
	virtual int  curspritey() {return subimagey();};
	virtual const char *filename(){return pcxfile::filename();};
	virtual void newrow() { newline(); };
	virtual void nextpixel(CommonPixel colour) { streamputpixel(colour); };
	virtual void spritedone() { endsubimage(); };
	virtual void spritedone(int sx, int sy);


	void writeheader();
	void writepal();

	void setcolours(U8 bg, U8 bord, int skip);
	void setpalette(const U8 *palette);
	//void setpalette(FILE *palfile);

	protected:
	const U8 *palette;

	private:
	void showspriteno();

	CommonPixel background;
	CommonPixel border;
	int borderskip, spriteno, lastdigitx;

	pcxwrite(const pcxwrite&);//not copyable: pcxfile::colormap
	void operator=(const pcxwrite&);//not assignable: pcxfile::colormap
};

class pcxread : public pcxfile {
	public:
	pcxread(singlefile *mfile);

	virtual void filestart(bool paletted);

	virtual void startsubimage(int x, int y, int sx, int sy);
	virtual void setline(CommonPixel *band);

	void readheader();

	private:
	multifile *mfile;
	pcxread(const pcxread&); //not copyable: pcxfile::colormap
	void operator=(const pcxread&); //not assignable: pcx::colormap
};

#endif /* _PCXSPRIT_H */
