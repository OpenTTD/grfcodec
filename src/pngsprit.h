#ifndef _PNGSPRIT_H
#define _PNGSPRIT_H
#ifdef WITH_PNG

#include "pcxsprit.h"
#include <png.h>
#include <vector>

class pngwrite: public pcxwrite {
	public:
	pngwrite(multifile *mfile, bool paletted);
	~pngwrite();

	void filestart(bool);
	void filedone(int final);

	protected:
	void encodebytes(const CommonPixel &pixel, int num);
	void encodebytes(const CommonPixel *buffer, int num);

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

	void filestart(bool paletted);
	void setline(CommonPixel *band);

	private:
	pngread(const pngread&);
	void operator=(const pngread&);

	private:
	multifile *mfile;
	png_struct *png;
	png_info *info;
	bool paletted;

	U8 *line_buffer;
	U8 **whole_image;
	uint read_row;
};

#endif /* WITH_PNG */
#endif /* _PNGSPRIT_H */
