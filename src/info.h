#ifndef _INFO_H
#define _INFO_H


#include <stdio.h>
#include <string>

#include "pcxsprit.h"
#include "pngsprit.h"
#include "sprites.h"
#include "nfosprite.h"
#include "allocarray.h"

class inforeader {
	public:
	inforeader(char *nf, int grfcontversion);
	~inforeader();
	const Sprite&operator[](int x)const{return *(nfofile[x]);}
	int size()const{return nfofile.size();}

	void installmap(int *map);

	long imgsize;
	int sx, sy;
	SpriteInfo inf;
	void PrepareReal(const SpriteInfo&info);
	int getsprite(CommonPixel *sprite);

	pcxread *imgfile;

	protected:

	string imgname;
	int *colourmap;
	AllocArray<Sprite> nfofile;
private:
	pcxread* MakeReader()const;
};

struct Box {
	enum boxtype {issprite, isspriteextension, isdata};
	Box(boxtype type) : type(type) {}
	virtual ~Box() {}
	boxtype type;
};

struct BoxData : Box {
	BoxData(uint size, U8 *data) : Box(isdata), size(size), data(data) {}
	uint size;
	U8 *data;
};

struct BoxSprite : Box {
	BoxSprite(bool first, const char *filename, int x, int y, SpriteInfo info) : Box(first ? issprite : isspriteextension), filename(strdup(filename)), x(x), y(y), info(info) {}
	~BoxSprite() { free(filename); }
	char *filename;
	int x;
	int y;
	SpriteInfo info;
};

class infowriter :  public spriteinfowriter {
	public:
	infowriter(FILE *info, int maxboxes, int useplaintext, const char *directory);
	virtual ~infowriter();

	void flush();
	virtual void addsprite(bool first, const char *filename, int x, int y, SpriteInfo info);
	virtual void adddata(uint size, U8 *data);

	void done(int count);

	private:
	FILE *info;
	const char *directory;
	void resize(int newmaxboxes);
	Box **boxes;

	int spriteno, maxboxes, boxnum;
	int useplaintext;
};

#endif /* _INFO_H */
