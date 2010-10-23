#ifndef _PNGSPRIT_H
#define _PNGSPRIT_H
#ifdef WITH_PNG

#include "pcxsprit.h"
#include <png.h>

class pngread: public pcxread {
	public:
	pngread(singlefile * mfile);
	~pngread();

	void filestart();
	void setline(U8 *band);

	private:
	pngread(const pngread&);
	void operator=(const pngread&);

	private:
	multifile *mfile;
	png_struct *png;
	png_info *info;
};

#endif /* WITH_PNG */
#endif /* _PNGSPRIT_H */
