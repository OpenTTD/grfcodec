#ifndef _INFO_H
#define _INFO_H


#include <stdio.h>

#include "pcxsprit.h"
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
	void PrepareReal(const Real&);
	int getsprite(U8 *sprite);

	pcxread *imgfile;

	protected:

	const char *imgname;
	int *colourmap;
	AllocArray<Sprite> nfofile;
private:
	pcxread* MakeReader()const;
};

class infowriter :  public spriteinfowriter {
	public:
	infowriter(FILE *info, int maxboxes, int useplaintext);

	virtual void newband(pcxfile *pcx);
	virtual void addsprite(int x, SpriteInfo info);
	virtual void adddata(U16 size, U8 *data);

	void done(int count);

	private:
	FILE *info;

	void resize(int newmaxboxes);

	enum boxtype {isinvalid, issprite, isdata};

	struct box {
		boxtype type;
		union foo {
			struct boxsprite {
				int x;
				SpriteInfo info;
			} sprite;

			struct boxdata {
				U16 size;
				U8 *data;
			} data;
		} h;
	} *boxes;
	int spriteno, maxboxes, boxnum;
	int useplaintext;
};

#endif /* _INFO_H */
