#ifndef _PNGSPRIT_H
#define _PNGSPRIT_H
#ifdef WITH_PNG

#include "pcxsprit.h"
#include <png.h>
#include <vector>

class pngwrite: public pcxwrite {
	public:
	pngwrite(multifile *mfile);
	~pngwrite();

	void filestart();
	void filedone(int final);

	protected:
	void encodebytes(CommonPixel byte, int num);
	void encodebytes(CommonPixel buffer[], int num);

	private:
	pngwrite(const pngwrite&);
	void operator=(const pngwrite&);

	private:
	png_struct *png;
	png_info *info;
	std::vector<U8> cache;
};

class pngread: public pcxread {
	public:
	pngread(singlefile * mfile);
	~pngread();

	void filestart();
	void setline(CommonPixel *band);

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
