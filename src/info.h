#ifndef _INFO_H
#define _INFO_H


#include <stdio.h>

#include "pcxsprit.h"
#include "pngsprit.h"
#include "sprites.h"
#include "nfosprite.h"
#include "allocarray.h"

class inforeader {
	public:
	inforeader(char *nf);
	~inforeader();
	const Sprite&operator[](int x)const{return *(nfofile[x]);}
	int size()const{return nfofile.size();}

	void installmap(int *map);

	long imgsize;
	int sx, sy;
	SpriteInfo inf;
	void PrepareReal(const SpriteInfo&info);
	int getsprite(U8 *sprite);

	pcxread *imgfile;

	protected:

	const char *imgname;
	int *colourmap;
	AllocArray<Sprite> nfofile;
private:
	pcxread* MakeReader()const;
};

struct Box {
	enum boxtype {issprite, isspriteextension, isdata};
	Box(boxtype type) : type(type) {}
	boxtype type;
};

struct BoxData : Box {
	BoxData(U16 size, U8 *data) : Box(isdata), size(size), data(data) {}
	U16 size;
	U8 *data;
};

struct BoxSprite : Box {
	BoxSprite(bool first, int x, SpriteInfo info) : Box(first ? issprite : isspriteextension), x(x), info(info) {}
	int x;
	SpriteInfo info;
};

class infowriter :  public spriteinfowriter {
	public:
	infowriter(FILE *info, int maxboxes, int useplaintext);
	virtual ~infowriter();

	virtual void newband(pcxfile *pcx);
	virtual void addsprite(bool first, int x, SpriteInfo info);
	virtual void adddata(U16 size, U8 *data);

	void done(int count);

	private:
	FILE *info;

	void resize(int newmaxboxes);
	Box **boxes;

	int spriteno, maxboxes, boxnum;
	int useplaintext;
};

#endif /* _INFO_H */
