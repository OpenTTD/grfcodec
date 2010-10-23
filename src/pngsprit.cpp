#ifdef WITH_PNG

#include "pngsprit.h"
#include "ttdpal.h"
#include "grfcomm.h"

/***********************\
*                       *
* class pngwrite        *
*                       *
\***********************/

pngwrite::pngwrite(multifile *mfile): pcxwrite(mfile), png(NULL), info(NULL)
{
	// Hopefully 8MiB should suffice
	cache.reserve(8*1024*1024);
}
pngwrite::~pngwrite()
{
	// Make sure we clean up if grfcodec terminates prematurely 
	if (png)
		png_destroy_write_struct(&png, &info);
}

void pngwrite::filestart()
{
	// Create the libpng structs
	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info = png_create_info_struct(png);
	if (!png || !info) {
		printf("%s: Agh! Unable to initalize libpng!\n", this->filename());
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
		// Store the final image's size
		png_set_IHDR(png, info, sx, totaly, 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

		// Set the palette data
		png_set_PLTE(png, info, (png_color*)pcxwrite::palette, 256);

		// Initial libpng io
		png_init_io(png, curfile);

		// Write the the png header
		png_write_info(png, info);

		// Flush the header and every so many lines
		png_write_flush(png);
		png_set_flush(png, 64);

		// Write the image data
		for (unsigned int i = 0, j = cache.size(); i < j; i += sx)
			png_write_row(png, (png_byte*)&cache[i]);

		// Cleanup incase we are writing multiple files
		png_destroy_write_struct(&png, &info);
		cache.clear();
		png = NULL;
		info = NULL;
	}
}

// Hooks the writing of lines out to file,
//   which we will be buffering internally
void pngwrite::encodebytes(U8 byte, int num)
{
	for (int i = 0; i < num; i++)
		cache.push_back(byte);
}

void pngwrite::encodebytes(U8 buffer[], int num)
{
	for (int i = 0; i < num; i++)
		cache.push_back(buffer[i]);
}

/***********************\
*                       *
* class pngread         *
*                       *
\***********************/

pngread::pngread(singlefile *mfile): pcxread(mfile), png(NULL), info(NULL)
{
}

pngread::~pngread()
{
	if (png)
		png_destroy_read_struct(&png, &info, NULL);
}

void pngread::setline(U8 *band)
{
	// Read the next row of the png file
	png_read_row(png, band, NULL);
}

extern bool _force;
extern int _quiet;

void pngread::filestart()
{
	if (png) {
		// Technically this should never be reached but as a safety net
		png_read_end(png, info);
		png_destroy_read_struct(&png, &info, NULL);
	}

	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info = png_create_info_struct(png);
	if (!png || !info) {
		printf("%s: Agh! Unable to initalize libpng!\n", this->filename());
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

	// Store the image dimentions
	sx = png_get_image_width(png, info);
	sy = png_get_image_height(png, info);
	totaly = 0;
	thisbandy = 0;
}

#endif /* WITH_PNG */
