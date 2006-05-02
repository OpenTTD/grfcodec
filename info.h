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
	const Sprite&operator[](int x)const{return *(file[x]);}
	int size()const{return file.size();}

	void installmap(int *map);

	long imgsize;
	int sx, sy;
	const U8 *inf;
	void PrepareReal(const Real&);
	int getsprite(U8 *sprite);

        pcxread *pcx;

	protected:

	const char *pcxname;
	int *colourmap;
	AllocArray<Sprite> file;
};

class infowriter : virtual public spriteinfowriter {
	public:
	infowriter(FILE *info, int maxboxes, int useplaintext);

	virtual void newband(pcxfile *pcx);
	virtual void addsprite(int x, U8 info[8]);
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
				U8 info[8];
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

int makeint(U8 low, S8 high);

#endif /* _GRFCOMM_H */
