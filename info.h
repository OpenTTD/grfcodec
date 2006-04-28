#ifndef _INFO_H
#define _INFO_H


#include <stdio.h>

#include "pcxsprit.h"
#include "sprites.h"

class inforeader {
	public:
	inforeader(FILE *nf);
        ~inforeader();
	int next(int wantno);

	void installmap(int *map);

	// for dealing with verbatim data
	int verbatim;
	int verbatim_str;
	int nextverb();
	void uncommentstream(int &byte);
	char *bininclude;

	// and for regular sprites
	long imgsize;
	int intinf[8], sx, sy;
	U8 inf[8];
	int getsprite(U8 *sprite);

	// for both types
	U16 size;

	FILE *f;
	long filesize;
        pcxread *pcx;

	protected:
	int makebufferlarger();
	int readline(char *prepend = NULL);

	char *pcxname;
	char *buffer;
	int buffersize;
	int infover, lasty;
	int *colourmap;

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
