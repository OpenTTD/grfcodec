#ifdef WITH_PNG

#include "pngsprit.h"
#include "ttdpal.h"
#include "grfcomm.h"

/***********************\
*                       *
* class pngwrite        *
*                       *
\***********************/

pngwrite::pngwrite(multifile *mfile, bool paletted): pcxwrite(mfile), png(NULL), info(NULL)
{
	this->paletted = paletted;
	// Hopefully 8MiB should suffice
	cache.reserve(8*1024*1024);
}
pngwrite::~pngwrite()
{
	// Make sure we clean up if grfcodec terminates prematurely
	if (png)
		png_destroy_write_struct(&png, &info);
}

void pngwrite::filestart(bool)
{
	// Create the libpng structs
	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info = png_create_info_struct(png);
	if (!png || !info) {
		printf("%s: Agh! Unable to initialize libpng!\n", this->filename());
		exit (254);
	}

	// Set the error out point
	if (setjmp(png_jmpbuf(png))) {
		exit (252);
	}
}

void pngwrite::filedone(int final)
{
	// Do not save the png until the grf file has been processed
	if (final && png && cache.size() > 0)
	{
		// Initialise libpng io
		png_init_io(png, curfile);

		if (paletted) {
			// Store the final image's size
			png_set_IHDR(png, info, sx, totaly, 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
				PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

			// Set the palette data
			png_set_PLTE(png, info, (png_color*)pcxwrite::palette, 256);
		} else {
			// Store the final image's size
			png_set_IHDR(png, info, sx, totaly, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
				PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		}

		// Write the the png header
		png_write_info(png, info);

		// Flush the header and every so many lines
		png_write_flush(png);
		png_set_flush(png, 64);

		// Write the image data
		for (unsigned int i = 0, j = cache.size(); i < j; i += sx * (paletted ? 1 : 4))
			png_write_row(png, (png_byte*)&cache[i]);

		// Finalise writing
		png_write_end(png, info);

		// Cleanup incase we are writing multiple files
		png_destroy_write_struct(&png, &info);
		cache.clear();
		png = NULL;
		info = NULL;
	}
}

// Hooks the writing of lines out to file,
//   which we will be buffering internally
void pngwrite::encodebytes(const CommonPixel &pixel, int num)
{
	for (int i = 0; i < num; i++) {
		if (paletted) {
			cache.push_back(pixel.m);
		} else {
			cache.push_back(pixel.r);
			cache.push_back(pixel.g);
			cache.push_back(pixel.b);
			cache.push_back(pixel.a);
		}
	}
}

void pngwrite::encodebytes(const CommonPixel *buffer, int num)
{
	for (int i = 0; i < num; i++)
		this->encodebytes(buffer[i], 1);
}

/***********************\
*                       *
* class pngread         *
*                       *
\***********************/

pngread::pngread(singlefile *mfile): pcxread(mfile), png(NULL), info(NULL), line_buffer(NULL), whole_image(NULL)
{
}

pngread::~pngread()
{
	if (png)
		png_destroy_read_struct(&png, &info, NULL);

	delete[] line_buffer;
	if (whole_image != NULL) {
		for (int i = 0; i < sy; ++i) {
			delete[] whole_image[i];
		}
		delete[] whole_image;
	}
}

void pngread::setline(CommonPixel *band)
{
	U8 *row;
	if (whole_image != NULL) {
		// Already read the whole image
		row = whole_image[read_row++];
	} else {
		// Read the next row of the png file
		png_read_row(png, line_buffer, NULL);
		row = line_buffer;
	}

	for (int i = 0; i < sx; i++) {
		if (paletted) {
			band[i].m = row[i];
		} else {
			band[i].r = row[i * 4];
			band[i].g = row[i * 4 + 1];
			band[i].b = row[i * 4 + 2];
			band[i].a = row[i * 4 + 3];
		}
	}
}

extern bool _force;
extern int _quiet;

void pngread::filestart(bool paletted)
{
	this->paletted=paletted;
	if (png) {
		// Technically this should never be reached but as a safety net
		png_read_end(png, info);
		png_destroy_read_struct(&png, &info, NULL);
	}

	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info = png_create_info_struct(png);
	if (!png || !info) {
		printf("%s: Agh! Unable to initialize libpng!\n", this->filename());
		exit (254);
	}

	// Set the error out point
	if (setjmp(png_jmpbuf(png))) {
		exit (252);
	}

	// Reset the file back to the start
	fseek(curfile, 0, SEEK_SET);

	// Do a quick header check
	U8 header[8];

	cfread("reading png header", header, 1, 8, curfile);
	if (png_sig_cmp((png_byte *)header, 0, 8)) {
		fprintf(stderr, "%s: Unrecognized file signature!\n", this->filename());
		exit(2);
	}

	// Read in the information
	png_init_io(png, curfile);
	png_set_sig_bytes(png, 8);

	png_read_info(png, info);

	if (paletted) {
		// Colour depth / format
		if (png_get_channels(png, info) >= 3) {
			fprintf(stderr, "%s: Cannot read true colour PNG files!\n", this->filename());
			exit(2);
		}
		if (png_get_channels(png, info) != 1 || png_get_bit_depth(png, info) != 8 || png_get_color_type(png, info) != PNG_COLOR_TYPE_PALETTE) {
			fprintf(stderr, "%s: Cannot read non-paletted PNG files!\n", this->filename());
			exit(2);
		}

		// Gather the png's palette information
		int entries;
		U8 *palette; // Compatible format RGB

		png_get_PLTE(png, info, (png_color**)&palette, &entries);
		if (entries != 256) {
			fprintf(stderr, "%s: PNG file is not a 256 colour file!\n", this->filename());
			exit(2);
		}

		// Look for a matching palette
		int i=0;
		for( ; i<NUM_PALS; i++)
			if(!memcmp(palette, defaultpalettes[i], 768)) break;

		if (i == NUM_PALS) {
			if ( _force ) {
				if (!_quiet) fprintf(stderr, "%s: Warning: Encoding despite unrecognized palette.\n", this->filename());
			} else {
				fprintf(stderr, "%s: Error: Unrecognized palette, aborting.\n"
					"Specify -f on the command line to override this check.\n", this->filename());
				exit(2);
			}
		}
	} else {
		if (png_get_channels(png, info) != 4) {
			fprintf(stderr, "%s: Cannot read 32bpp PNG files without alpha layer!\n", this->filename());
			exit(2);
		}
	}

	// Store the image dimentions
	sx = png_get_image_width(png, info);
	sy = png_get_image_height(png, info);
	totaly = 0;
	thisbandy = 0;

	if (png_get_interlace_type(png, info) == PNG_INTERLACE_NONE) {
		/* Read image row by row */
		line_buffer = new U8[sx * (paletted?1:4)];
	} else {
		/* Interlaced images can only be read as a whole */
		read_row = 0;
		whole_image = new U8*[sy];
		for (int i = 0; i < sy; ++i) {
			whole_image[i] = new U8[sx * (paletted?1:4)];
		}
		png_read_image(png, whole_image);
	}
}

#endif /* WITH_PNG */
