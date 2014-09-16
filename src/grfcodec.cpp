
/****************************************\
*                                        *
* GRFCODEC - A program to decode and     *
*            encode Transport Tycoon     *
*            Deluxe .GRF files           *
*                                        *
*                                        *
* Copyright (C) 2000-2005 by             *
* Josef Drexler <jdrexler@uwo.ca>        *
*                                        *
* Permission granted to copy and redist- *
* ribute under the terms of the GNU GPL. *
* For more info please read the file     *
* COPYING which should have come with    *
* this file.                             *
*                                        *
\****************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <getopt.h>

#ifdef MINGW
	#include <io.h>
	#define mkdir(a,b) mkdir(a)
	#define isatty _isatty
#elif defined(_MSC_VER)
	#include <io.h>
	#include <direct.h>
	#define mkdir(a,b) mkdir(a)
	#define F_OK 0
	#define isatty _isatty
#else
	#include <unistd.h>
#endif//_MSC_VER

#define DOCHECK
#define DEFINE_PALS

#include "pcxfile.h"
#include "sprites.h"
#include "pcxsprit.h"
#include "pngsprit.h"
#include "ttdpal.h"
#include "grfcomm.h"
#include "info.h"
#include "error.h"
#include "version.h"
#include "conv.h"
#include "nfosprite.h"
#include "path.h"

static const char *version = "GRFCodec " VERSION " - Copyright (C) 2000-2005 by Josef Drexler";

static void usage(void)
{
	printf(
		"%s\n"
		"Usage:\n"
		"    GRFCODEC -d [<Options>] <GRF-File> [<Directory>]\n"
		"        Decode all sprites in the GRF file and put them in the directory\n"
		"    GRFCODEC -e [<Options>] <GRF-File> [<Directory>]\n"
		"        Encode all sprites in the directory and combine them in the GRF file\n"
		"\n"
		"<GRF-File> denotes the .GRF file you want to work on, e.g. TRG1.GRF\n"
		"<Directory> is where the individual sprites should be saved. If omitted, they\n"
		"\twill default to a subdirectory called sprites/.\n"
		"\n"
		"Options for decoding:\n"
		"    -w <num>  Write spritesheets with the given width (default 800, minimum 16)\n"
		"    -h <num>  Split spritesheets when they reach this height (default no limit,\n"
		"              minimum 16)\n"
		"    -b <num>  Organize sprites in boxes of this size (default 16)\n"
		"    -o <ssf>  Sets the format of generated spritesheets.  See -o ? for a list.\n"
		"    -p <pal>  Use this palette instead of the default.  See -p ? for a list.\n"
		"    -t        Disable decoding of plain text characters as strings.\n"
		"    -x        Disable production of unquoted escape sequences.\n"
		"    -xx       Disable production of both quoted and unquoted escape sequences.\n"
		"              This has the side effect of producing a version 6 .nfo, instead\n"
		"              of a version 32 .nfo.\n"
		"    -X        List sprite numbers in the PCX file in hex.\n"
		"\n"
		"Options for encoding:\n"
		"    -c        Crop extraneous transparent blue from real sprites\n"
		"    -u        Save uncompressed data (probably not a good idea)\n"
		"    -q        Suppress warning messages\n"
		"    -s        Suppress progress output\n"
		"    -g <num>  Version of the encoded container format (default 1, maximum 2)\n"
		"    -n        Try both compression algorithms and choose the most efficient\n"
		"\n"
		"Options for both encoding and decoding:\n"
		"    -m <num>  Apply colour translation to all sprites except character-glyphs.\n"
		"    -M <num>  Apply colour translation to all sprites.\n"
		"        If both of these are specified, only the last is obeyed.\n"
		"        (-m ? or -M ? for a list of colour translations.)\n"
		"\n"
		"GRFCODEC is Copyright (C) 2000-2005 by Josef Drexler\n"
		"You may copy and redistribute it under the terms of the GNU General Public\n"
		"License, as stated in the file 'COPYING'.\n",
		version
		);

	exit(1);
}

static void showpalettetext()
{
	printf(
		"Options for the -p parameter:\n"
		"\n"
		"Built-in palettes: use -p <number>, where <number> is one of the following:\n"
		"	%d  DOS TTD\n"
		"	%d  Windows TTD\n"
		"	%d  DOS TTD, Candyland\n"
		"	%d  Windows TTD, Candyland\n"
		"	%d  TT Original\n"
		"	%d  TT Original, Mars landscape\n"
		"\n"
		"External palette files: use -p [type:]filename.\n"
		"[type:] can be bcp, psp, or gpl.  (Other formats may become available later.)\n"
		"If the type is omitted, bcp is assumed.\n"
		"\n"
		"bcp	binary coded palette with the same format as the palette in a PCX\n"
		"	file: 256 triples of red, green and blue encoded in bytes.\n"
		"psp	the palette format output by Paintshop Pro\n"
		"gpl	the palette format output by The GIMP.\n"
		"\n",

		// note, -p values are the array index plus one (so that 0 is not
		// a valid number, which makes the atoi easier)
		PAL_ttd_norm+1, PAL_ttw_norm+1, PAL_ttd_cand+1, PAL_ttw_cand+1,
		PAL_tt1_norm+1, PAL_tt1_mars+1
		);
}

static void showcolourmaps()
{
	printf(
		"Options for the -m parameter:\n"
		"\n"
		"	0  Convert a TTD-DOS file to TTD-Windows\n"
		"	1  Convert a TTD-Windows file to TTD-DOS\n"
		"\n"
		);
}

static void showimageformats()
{
	printf(
		"Options for the -o parameter:\n"
		"\n"
#ifdef WITH_PNG
		"	pcx\n"
		"	png (default)\n"
#else
		"	pcx (default)\n"
#endif
		"\n"
		);
}

struct defpal {
	const char* grffile;
	int defpalno;
};

static const defpal defpals[] =
{
	// DOS TTD
	{ "TRG1",	PAL_ttd_norm },
	{ "TRGC",	PAL_ttd_norm },
	{ "TRGH",	PAL_ttd_norm },
	{ "TRGI",	PAL_ttd_norm },
	{ "TRGT",	PAL_ttd_cand },

	// Windows TTD
	{ "TRG1R",	PAL_ttw_norm },
	{ "TRGCR",	PAL_ttw_norm },
	{ "TRGHR",	PAL_ttw_norm },
	{ "TRGIR",	PAL_ttw_norm },
	{ "TRGTR",	PAL_ttw_cand },

	// DOS TTO or TT+WB
	{ "TREDIT",	PAL_tt1_norm },
	{ "TREND",	PAL_tt1_norm },
	{ "TRTITLE",	PAL_tt1_norm },
	{ "TRHCOM",	PAL_tt1_norm },
	{ "TRHCOM2",	PAL_tt1_mars },

	// TTDPatch
	{ "TTDPATCH",	PAL_ttd_norm },
	{ "TTDPATCHW",	PAL_ttw_norm },
	{ "TTDPBASE",	PAL_ttd_norm },
	{ "TTDPBASEW",	PAL_ttw_norm },
};

static int *colourmaps[] = { palmap0, palmap1 };

bool _interactive;
bool _best_compression = false;

static int movetoreal(char *newfile, char *realfile)
{
	// rename original to bak if bak doesn't exist
	char *bakfile = getbakfilename(realfile);
	FILE *tmp = fopen(bakfile, "rb");

	if (!tmp && (errno == ENOENT)) {
		// .bak doesn't exist, rename orig to .bak
		// XXX: Shouldn't we rename orig to .bak even if .bak already exists? --pasky
		if (_interactive) printf("\nRenaming %s to %s", realfile, bakfile);
		if (rename(realfile, bakfile)) {
			errno = EEXIST;		// go delete it
		} else {
			errno = ENOENT;		// don't try to delete it
		}
	}

	if (tmp) fclose(tmp);
	free(bakfile);

	// delete grf if it exists
	if (access(realfile, F_OK) == 0) {
		if (_interactive) printf("\nDeleting %s",realfile);
		if (remove(realfile))
			fperror("\nError deleting %s", realfile);
	}

	// rename tmp to grf

	if (_interactive) printf("\nReplacing %s with %s\n", realfile, newfile);

	if (rename(newfile, realfile)) {
		fperror("Error renaming %s to %s", newfile, realfile);
		exit(2);
	}
	return 1;
}

enum SpriteSheetFormat {
	SSF_PCX,
#ifdef WITH_PNG
	SSF_PNG,
#endif
};

#ifdef WITH_PNG
SpriteSheetFormat _outputformat = SSF_PNG;
#else
SpriteSheetFormat _outputformat = SSF_PCX;
#endif

const char * getoutputext(bool rgba)
{
	switch (_outputformat) {
#ifdef WITH_PNG
		case SSF_PNG:
			return rgba ? "32.png" : ".png";
#endif
		case SSF_PCX:
		default:
			return ".pcx";
	}
}

class spritefiles : public multifile {
public:
	spritefiles() { init(); }
	spritefiles(const char *basename, const char *directory, bool rgba);
	virtual FILE *curfile()  { return thecurfile; }
	virtual FILE *nextfile();
	virtual const char *filename() { return thecurfilename; }
	virtual const char *getdirectory() { return directory; }
private:
	void init () {
		thecurfile = NULL;
		basename = thecurfilename = NULL;
		directory = NULL;
		filenum = 0;
		rgba=false;
	}
		FILE *thecurfile;
		char *thecurfilename;
		const char *directory;
		const char *basename;
		unsigned int filenum;
		bool rgba;
};

spritefiles::spritefiles(const char *basename, const char *directory, bool rgba)
{
	init();
	spritefiles::basename = basename;
	spritefiles::directory = directory;
	spritefiles::rgba=rgba;
}

FILE *spritefiles::nextfile()
{
	FILE *oldfile = thecurfile;
	char *oldname = thecurfilename;

	thecurfilename = strdup(spritefilename(basename, directory, getoutputext(rgba), filenum++, "wb", 1));
	thecurfile = fopen(thecurfilename, "wb");

	if (thecurfile) {	// new open succeeded, close old one
		if (oldfile)
			fclose(oldfile);
		free(oldname);
	} else {		// retain old one
		free(thecurfilename);
		thecurfile = oldfile;
		thecurfilename = oldname;
	}

	return thecurfile;
}

int _crop=0;
int _quiet=0;

static const char header[] = {
	'\x00', '\x00',                 // End-of-file marker for old OTTDp versions
	'G',    'R',    'F',    '\x82', // Container version 2
	'\x0D', '\x0A', '\x1A', '\x0A', // Detect garbled transmission
};

static int encode(const char *file, const char *dir, int compress, int *colourmap, int grfcontversion)
{
	char *grfnew, *infofile;
	FILE *grf;

	doopen(file, "", ".new", "wb", &grfnew, &grf, 0);
	doopen(file, dir, ".nfo", "rt", &infofile, NULL, 1);

	if (_interactive) printf("Encoding in temporary file %s\n", grfnew);

	inforeader info(infofile, grfcontversion);

	free(infofile);

	if (colourmap)
		info.installmap(colourmap);

	long totaluncomp = 1, totalcomp = 1;
	long totaltransp = 1, totaluntransp = 1;
	long totalreg = 1, totalunreg = 1;

	if (grfcontversion == 2) {
		cfwrite("writing header", header, sizeof(header), 1, grf);
		int size = 1 + 4; // The size of the zoom + end of data
		for(int i = 0; i < info.size(); i++) {
			switch(info[i].GetType()){
			case Sprite::ST_INCLUDE:
			case Sprite::ST_REAL:
				size += 4 + 1 + 4; // Size + type + ID
				break;

			case Sprite::ST_PSEUDO:
				size += 4 + 1 + ((const Pseudo&)info[i]).size();
				break;
			}
		}
		writedword("writing header", size, grf);
		fputc(0x00, grf); // Compression
	}

	for (int pass = 1; pass <= grfcontversion; pass++) {
		for(int i=0;i<info.size();i++){
			if (_interactive && pass != grfcontversion) {
				int comp2 = (100L * totalcomp / totaluncomp);// % 100;
				int comp4 = (100L * totaltransp / totaluntransp);// % 100;
				int comp6 = (100L * totalreg / totalunreg);// % 100;
				printf("\rSprite%5d  Done:%3d%%  "
					"Compressed:%3d%% (Transparency:%3d%%, Redundancy:%3d%%)\r",
					i, (int) (i*100L/info.size()),
					comp2, comp4, comp6);
			}

			switch(info[i].GetType()){
			case Sprite::ST_INCLUDE:{
				static const char *action = "copying binary blob";
				if (pass != grfcontversion) {
					writespritesize(action, 4, grfcontversion, grf);
					fputc(0xfd, grf);
					writedword(action, i + 1, grf);
					break;
				}

				const char *bininclude=((const Include&)info[i]).GetName();
				FILE *bin = fopen(bininclude, "rb");
				if (!bin) {
					fperror("%s:%i: Error: Cannot include %s: Could not open.\n", file, i, bininclude);
					exit(2);
				}

				struct stat stat_buf;
				if ( fstat(fileno(bin), &stat_buf) ) {
					fperror("%s:%i: Error: Could not stat %s.\n", file, i, bininclude);
					exit(2);
				}
				if ( stat_buf.st_mode & S_IFDIR ) {
					fprintf(stderr, "%s:%i: Error: Cannot include %s: Is a directory.\n", file, i, bininclude);
					exit(2);
				}
				off_t fsize = stat_buf.st_size;

				const char *nameofs = bininclude + strlen(bininclude);
				while (nameofs > bininclude) {
					nameofs--;
					if (nameofs[0] == '\\' || nameofs[0] == '/') {
						nameofs++;
						break;
					}
				}
				int namelen = strlen(nameofs);
				if (namelen > 255) {
					fprintf(stderr, "%s:%i: Error: binary include has too long filename %s\n", file, i, nameofs);
					exit(2);
				}

				int spritesize = 3 + namelen + fsize;
				if (spritesize < 5) {
					fprintf(stderr, "%s:%i: Error: binary include %s is empty\n", file, i, nameofs);
					exit(2);
				}
				if (grfcontversion > 1) spritesize++; // We need the first 0xFF to be accounted for as well.
				/* GRF container version only allowed 64K, the rest prevents underflows. */
				if ((grfcontversion == 1 && spritesize > 65535) || spritesize < fsize || spritesize < 0) {
					fprintf(stderr, "%s:%i: Error: binary include %s is too large\n", file, i, nameofs);
					exit(2);
				}

				totalcomp += spritesize;
				totaluncomp += spritesize;

				if (grfcontversion == 2) writedword(action, i + 1, grf);
				writespritesize(action, spritesize, grfcontversion, grf);
				fputc(0xff, grf);
				fputc(0xff, grf);
				fputc(namelen, grf);
				cfwrite(action, nameofs, namelen+1, 1, grf);

				char *buffer = new char[16384];
				while (fsize > 0) {
					int chunk = 16384;
					if (chunk > fsize) chunk=fsize;
					cfread(action, buffer, chunk, 1, bin);
					cfwrite(action, buffer, chunk, 1, grf);
					fsize -= chunk;
				}
				delete[]buffer;
				fclose(bin);
			}
				break;
			case Sprite::ST_PSEUDO:{
				if (pass != 1) {
					break;
				}

				static const char *action = "writing pseudo sprite";
				const Pseudo&sprite=(const Pseudo&)info[i];
				uint size=sprite.size();
				totalcomp += size;
				totaluncomp += size;

				writespritesize(action, size, grfcontversion, grf);
				fputc(0xff, grf);
				cfwrite(action, sprite.GetData(),1,size,grf);
				if(i == 0 && sprite.size() == 4){
					int reported = *((S32*)sprite.GetData());
					reported = BE_SWAP32(reported) + 1;
					if(reported != info.size() && !_quiet)
						fprintf(stderr, "%s:1: Warning: Found %d %s sprites than sprite 0 reports.\n",
						file,
						abs(info.size() - reported),
						info.size()>reported?"more":"fewer");
				}
			}
				break;
			case Sprite::ST_REAL:{	// real sprite, encode it
				static const char *action = "writing real sprite";

				const Real&sprite=(Real&)info[i];
				for (size_t j = 0; j < (grfcontversion == 1 ? 1 : sprite.infs.size()); j++) {
					if (pass != grfcontversion) {
						writespritesize(action, 4, grfcontversion, grf);
						fputc(0xfd, grf);
						writedword(action, i + 1, grf);
						break;
					}

					info.PrepareReal(sprite.infs[j]);
					CommonPixel *image = (CommonPixel*) calloc(info.imgsize, sizeof(CommonPixel));
					if (!image) {
						fprintf(stderr, "%s:%d: Error: can't allocate sprite memory (%ld bytes)\n", file, i, info.imgsize);
						exit(2);
					}

					bool has_mask=sprite.infs[j].depth==DEPTH_8BPP;
					bool rgba=sprite.infs[j].depth==DEPTH_32BPP;
					info.getsprite(image);
					if(j+1<sprite.infs.size()&&sprite.infs[j+1].depth==DEPTH_MASK){
						j++;
						info.PrepareReal(sprite.infs[j]);
						CommonPixel *mask = (CommonPixel*) calloc(info.imgsize, sizeof(CommonPixel));
						if (!mask) {
							fprintf(stderr, "%s:%d: Error: can't allocate sprite memory (%ld bytes)\n", file, i, info.imgsize);
							exit(2);
						}
						info.getsprite(mask);
						for (int i = 0; i < info.imgsize; i++) image[i].m = mask[i].m;
						free(mask);
						has_mask=true;
					}

					int k=0;
					for (int j=info.imgsize-1; j >= 0; j--)
						if (has_mask && image[j].m == 0xFF) k++;

					if (k && !_quiet)
						fprintf(stderr, "%s:%d: Warning: %d of %ld pixels (%ld%%) are pure white\n",
							file, i, k, info.imgsize, k*100/info.imgsize);

					if(_crop && !DONOTCROP(info.inf.info)){
						int i=0,j=0;
						for(i=info.imgsize-1;i>=0;i--)if(!image[i].IsTransparent(rgba))break; // Find last non-blue pixel
						if(i<0)// We've got an all-blue sprite
							info.sx=info.sy=info.imgsize=1;
						else{
							i=info.imgsize-(i+info.sx-i%info.sx/*begining of next line*/);
							info.sy-=i/info.sx;

							for(i=0;i<info.imgsize;i++){
								if(!image[i].IsTransparent(rgba))
									break; // Find first non-blue pixel
							}
							i-=i%info.sx;// Move to beginning of line

							info.sy-=i/info.sx;
							info.inf.yrel+=i/info.sx;
							if(i)memmove(image,image+i,(info.imgsize-i)*sizeof(CommonPixel));
							for(i=0;i<info.sx;i++){
								for(j=0;j<info.sy;j++){
									if(!image[i+j*info.sx].IsTransparent(rgba))goto foundfirst;
								}
							}
foundfirst:
							if(i){
								for(j=0;j<info.sy;j++)
									memmove(image+j*(info.sx-i),image+j*info.sx+i,(info.sx-i)*sizeof(CommonPixel));
								info.inf.xrel+=i;
								info.sx-=i;
							}

							for(i=info.sx-1;i>=0;i--){
								for(j=0;j<info.sy;j++){
									if(!image[i+j*info.sx].IsTransparent(rgba))goto foundlast;
								}
							}
foundlast:
							i=info.sx-i-1;
							if(i){
								for(j=1;j<info.sy;j++)
									memmove(image+j*(info.sx-i),image+j*info.sx,(info.sx-i)*sizeof(CommonPixel));
								info.sx-=i;
							}

						}
						info.inf.xdim = info.sx;
						info.inf.ydim = info.sy;
						info.imgsize = info.sx * info.sy;
					}

					U8 bytes_per_pixel=(has_mask?1:0)+(rgba?4:0);
					U8 *compressed_chunked = NULL;
					U8 *compressed_regular = NULL;
					long compressed_size_chunked = 1 << 30;
					long compressed_size_regular = 1 << 30;
					long uncompressed_size_chunked = 0;

					bool force_chunk = HASTRANSPARENCY(info.inf.info);
					if (force_chunk || _best_compression) {
						SpriteInfo inf = info.inf;
						inf.info |= 8; // Set chunked status
						compressed_size_chunked = encodetile(&compressed_chunked, &uncompressed_size_chunked, image, info.imgsize*bytes_per_pixel, info.sx, info.sy, inf, compress, i, has_mask, rgba, grfcontversion);
					}
					if (!force_chunk || _best_compression) {
						U8 *imgbuffer = (U8*)malloc(info.imgsize*bytes_per_pixel);
						if (!imgbuffer) {
							fprintf(stderr, "%s:%d: Error: can't allocate sprite memory (%ld bytes)\n", file, i, info.imgsize);
							exit(2);
						}
						for (int j = 0; j < info.imgsize; j++) {
							image[j].Encode(imgbuffer + (j * bytes_per_pixel), has_mask, rgba);
						}
						SpriteInfo inf = info.inf;
						inf.info &= ~8; // Clear chunked status
						compressed_size_regular = encoderegular(&compressed_regular, imgbuffer, info.imgsize*bytes_per_pixel, inf, compress, i, grfcontversion);
						free(imgbuffer);
					}

					/* GRF container version 2 saves 4 extra bytes for the chunked data. */
					bool use_chunk = compressed_size_chunked + (grfcontversion == 2 ? 4 : 0) < compressed_size_regular;
					long uncompressed_size = use_chunk ? uncompressed_size_chunked : info.imgsize*bytes_per_pixel;
					long compressed_size   = use_chunk ? compressed_size_chunked : compressed_size_regular;
					if (use_chunk) {
						info.inf.info |= 8; // Set chunked status
					} else {
						info.inf.info &= ~8; // Clear chunked status
					}
					writesprite(grf, use_chunk ? compressed_chunked : compressed_regular, compressed_size, uncompressed_size, info.inf, i, grfcontversion);

					totaltransp += uncompressed_size;	// how much after transparency removed
					totaluntransp += info.imgsize;		// how much with transparency

					totalreg += compressed_size;			// how much after transp&redund removed
					totalunreg += uncompressed_size;	// how much with redund

					totalcomp += compressed_size;
					totaluncomp += info.imgsize;
					free(compressed_chunked);
					free(compressed_regular);
					free(image);
				}
			}
				break;
			default:
				fprintf(stderr, "%s:%d: Error: What type of sprite is that?", file, i);
				exit(2);
			}
		}

		writespritesize("writing end-of-file", 0, grfcontversion, grf);
		if (grfcontversion == 1) {
			writedword("writing checksum", 0, grf);
		}
	}

	fclose(grf);

	movetoreal(grfnew, spritefilename(file, "", ".grf", -1, "r+b", 0));

	free(grfnew);

	if (_interactive) printf("\nDone!\n");
	return 0;
}

static int decode(const char *file, const char *dir, const U8 *palette, int box, int width, int height, int *colourmap, int useplaintext)
{
	int count, result, lastpct = -1;
	FILE *grf, *info;
	char *realdir;
	struct stat statbuf;

	long fsize;

	grf = fopen(file, "rb");
	if (!grf) {
		fperror(e_openfile, file);
		exit(2);
	}

	// make sure the directory exists, or create it if not
	realdir = spritefilename(file, dir, "", -1, "rb", 0);	// make fake filename
	*strrchr(realdir, '/') = 0;	// cut off filename

	if (stat(realdir, &statbuf)) {
		// error during stat
		if (errno != ENOENT) {
			fperror("Error accessing %s", realdir);
			exit(2);
		}

		if (mkdir(realdir, 0755)) {
			fperror("Error making %s", realdir);
			exit(2);
		}
	}

	doopen(file, dir, ".nfo", "wt", NULL, &info, 0);

	fseek(grf, 0, SEEK_END);
	fsize = ftell(grf);
	fseek(grf, 0, SEEK_SET);

	static const char *action = "reading header";
	U8 buffer[sizeof(header)];
	cfread(action, buffer, 1, sizeof(buffer), grf);
	int grfcontversion = memcmp(buffer, header, sizeof(header)) == 0 ? 2 : 1;

	U32 dataoffset = 0;
	if (grfcontversion == 1) {
		fseek(grf, 0, SEEK_SET);
	} else {
		dataoffset = sizeof(header) + 4 + readdword(action, grf); // GRF data offset
		fgetc(grf); // Compression
	}
	printf("Found grf container version %d\n", grfcontversion);

	// We do the 'file' and 'writer' seperate to make
	//   this a little bit less messy
	multifile *imgname, *imgname32;
	pcxwrite *pcx, *pcx32;

	if (height == -1) {
		imgname   = new singlefile(spritefilename(file, dir, getoutputext(false), -2, "wb", 1),"wb", dir);
		imgname32 = new singlefile(spritefilename(file, dir, getoutputext(true),  -2, "wb", 1),"wb", dir);
	} else {
		imgname   = new spritefiles(file, dir, false);
		imgname32 = new spritefiles(file, dir, true);
	}

	// Select the appropriate writer
	switch (_outputformat) {
#ifdef WITH_PNG
		case SSF_PNG:
			pcx   = new pngwrite(imgname, false);
			pcx32 = new pngwrite(imgname32, true);
			break;
#endif
		case SSF_PCX:
		default:
			pcx   = new pcxwrite(imgname);
			pcx32 = NULL;
			break;
	}

	if (!pcx) {
		fprintf(stderr, "%s: Error opening PCX file\n", file);
		exit(2);
	}

	pcx->setpalette(palette);

	pcx->setcolours(255, 0, 0);
	if (pcx32 != NULL) pcx32->setcolours(255, 0, 0);

	infowriter writer(info, (width + box - 1) / box, useplaintext, pcx->getdirectory());

	if (colourmap)
		pcx->installwritemap(colourmap);

	pcx->startimage(true, width, height, box, box);
	if (pcx32 != NULL) pcx32->startimage(false, width, height, box, box);

	count = 0;

	if (_interactive) printf("Decoding:\n");

	do {
		int newpct = 100L*ftell(grf)/fsize;

		if (_interactive && newpct != lastpct) {
			lastpct = newpct;
			printf("Sprite %d at %lX, %3d%% done\r", count, ftell(grf), lastpct);
		}

		pcx->newsprite();
		if (pcx32 != NULL) pcx32->newsprite();

		result = decodesprite(grf, pcx, pcx32, &writer, count, &dataoffset, grfcontversion);
		writer.flush();
		count++;
	} while (result);
	count--;

	pcx->endimage();
	if (pcx32 != NULL) pcx32->endimage();
	delete(pcx);	// closes output file
	delete(pcx32);
	if (pcx32 == NULL) delete imgname32;

	writer.flush();
	writer.done(count);

	fclose(info);

	if (_interactive) printf("%s has %d sprites, maxx %d, maxy %d, maxs %d.\n",
		file, count, maxx, maxy, maxs);
	fclose(grf);

	return 0;
}

static U8 *readpal(const char *filearg)
{
	enum paltype { UNK, BCP, PSP, GPL };
	paltype type = UNK;
	static U8 pal[256*3];

	if (!strnicmp(filearg, "bcp:", 4))
		type = BCP;
	else if (!strnicmp(filearg, "psp:", 4))
		type = PSP;
	else if (!strnicmp(filearg, "gpl:", 4))
		type = GPL;

	if (type != UNK)
		filearg += 4;	// remove type: from filename

	FILE *f = fopen(filearg, "rb");
	if (!f) {
		fperror(e_openfile, filearg);
		exit(1);
	}

	switch (type) {
	case BCP:
	case UNK:
		if (fread(pal, 1, 256*3, f) != 256*3) {
			fprintf(stderr, "Error: %s is not a BCP file.\n", filearg);
			exit(1);
		}
		break;
	case PSP:
		char fmt[16];
		if (fgets(fmt, sizeof(fmt), f) == NULL || strcmp(fmt, "JASC-PAL\r\n")) {
psp_error:
			fprintf(stderr, "Error: %s is not a PSP palette file.\n", filearg);
			exit(1);
		}
		int nument, nument2;
		if (fscanf(f, "%x\n", &nument)  != 1) goto psp_error;
		if (fscanf(f, "%d\n", &nument2) != 1) goto psp_error;
		if ( (nument != nument2) || (nument != 256) ) {
			fprintf(stderr, "%s: Error: GRFCodec supports only 256 colour palette files.\n", filearg);
			exit(1);
		}
		for (int i=0; i<nument; i++) {
			if (!fscanf(f, "%cd %cd %cd\n", pal + i*3, pal+i*3+1, pal+i*3+2)) {
				fprintf(stderr, "Error reading palette.\n");
				exit(1);
			}
		}
		break;
	case GPL:
		if (fgets(fmt, sizeof(fmt), f) == NULL || strcmp(fmt, "GIMP Palette\r\n")) {
gpl_error:
			fprintf(stderr, "Error: %s is not a GIMP palette file.\n", filearg);
			exit(1);
		}
		while (fgets(fmt, sizeof(fmt), f) != NULL && fmt[strlen(fmt)-1] != '\n' ) {}// Name: ...
		while (fgets(fmt, sizeof(fmt), f) != NULL && fmt[strlen(fmt)-1] != '\n' ) {}// Columns: ...
		if (fgets(fmt, sizeof(fmt), f) == NULL) goto gpl_error; // #
		uint r, g, b;
		for (int i=0; i<256; i++) {
			if (!fscanf(f, "%d %d %d\n", &r, &g, &b) || r > 255 || g > 255 || b > 255) {
				fprintf(stderr, "%s: Error: reading palette.\n", filearg);
				exit(1);
			}
			pal[3*i] = (U8) r;
			pal[3*i+1] = (U8) g;
			pal[3*i+2] = (U8) b;
			while (fgets(fmt, sizeof(fmt), f) != NULL && fmt[strlen(fmt)-1] != '\n' ) {}// color name
		}
		break;
	}
	fclose(f);

	return pal;
}

static SpriteSheetFormat setoutputformat(const char *formatarg)
{
	if (!strnicmp(formatarg, "pcx", 3))
		return SSF_PCX;

#ifdef WITH_PNG
	if (!strnicmp(formatarg, "png", 3))
		return SSF_PNG;
#endif

	return SSF_PCX;
}

// find default palette
static U8* findpal(char *grffile)
{
	char base[MAXFILE];
	char *bs;
	unsigned int i;

	bs = strrchr(grffile, '\\');
	if (!bs) bs = strrchr(grffile, '/');
	if (!bs) bs = grffile;

	safestrncpy(base, grffile, MAXFILE);

	bs = strchr(base, '.');
	if (bs) *bs = 0;

	for (i=0; i<sizeof(defpals)/sizeof(defpals[0]); i++) {
		if (!stricmp(defpals[i].grffile, base))
			return defaultpalettes[defpals[i].defpalno];
	}

	return defaultpalettes[0];
}

//extern "C" void debugint(void);

bool _force=false,_mapAll=false,_hexspritenums=false;
int _useexts=2;

int main(int argc, char **argv)
{
	char directory[MAXDIR];
	char *grffile = NULL;
	int action = 0;
	int width = 800, height = 16000, box = 16, compress = 1;
	U8 *palette = NULL;
	int *colourmap = NULL;
	int useplaintext = 1;
	int grfcontversion = 1;

	_interactive = (isatty(fileno(stdout)) != 0);

	checksizes();

#ifdef WIN32
	//  debugint();
#endif

	// parse option arguments
	while (1) {
		char opt = getopt(argc, argv, "dev?w:h:b:up:m:M:o:tfxqcsXg:n");

		if (opt == (char) EOF)
			break;

		switch (opt) {
		case 'e':
			action = 1;
			break;
		case 'd':
			action = 2;
			break;
		case 'v':
			printf("%s\n", version);
			return 0;
		case 'w':
			width = min(max(atoi(optarg), 16), 65535);
			break;
		case 'h':
			height = min(max(atoi(optarg), 16), 65535);
			break;
		case 'b':
			box = atoi(optarg);
			break;
		case 'n':
			_best_compression = true;
			break;
		case 'p':
			unsigned int palnum;
			palnum = atoi(optarg);
			if ( (palnum > 0) &&
				(palnum <= sizeof(defaultpalettes)/sizeof(defaultpalettes[0])) ) {
					palette = defaultpalettes[palnum-1];
				} else if (*optarg == '?') {
					showpalettetext();
					exit(1);
				} else
					palette = readpal(optarg);
				break;
		case 'c':
			_crop++;
			break;
		case 'g':
			grfcontversion = atoi(optarg);
			if (grfcontversion < 1 || grfcontversion > 2) usage();
			break;
		case 'm':
		case 'M':
			_mapAll= opt=='M';
			unsigned int mapnum;
			mapnum = atoi(optarg);
			if (*optarg == '?') {
				printf("%s\n", version);
				showcolourmaps();
				exit(1);
			} else if (mapnum < sizeof(colourmaps)/sizeof(colourmaps[0]) ) {
					colourmap = colourmaps[mapnum];
				} else {
					usage();
				}
				break;
		case 'u':
			compress = 0;
			break;
		case 't':
			useplaintext = 0;
			break;
		case 'f':
			_force=true;
			break;
		case 'x':
			if(_useexts)_useexts--;
			break;
		case 'q':
			_quiet++;
			break;
		case 's':
			_interactive = false;
		case 'X':
			_hexspritenums=true;
			break;
		case 'o':
			if (*optarg == '?') {
				showimageformats();
				exit(1);
			}
			_outputformat = setoutputformat(optarg);
			break;
		default:
			usage();
		}
	}

	// non-option arguments: filename and potentially directory
	if (optind < argc)
		grffile = argv[optind++];

	safestrncpy(directory, optind < argc ? argv[optind++] : "sprites", MAXDIR);

	int offset = strlen(directory);
	if (directory[offset - 1] != '/' ) {
		safestrncpy(directory + offset, "/", MAXDIR - offset);
	}

	if (!action || !grffile || (width < 16) ||
		( (height < 16) && (height != -1) ) ||
		(strlen(directory) < 1) ||
		(strlen(grffile) < 1) ||
		(box < 1))
		usage();

	if (action == 1) {
		return encode(grffile, directory, compress, colourmap, grfcontversion);
	} else if (action == 2) {
		if (!palette)
			palette = findpal(grffile);

		return decode(grffile, directory, palette, box, width, height, colourmap, useplaintext);
	}

	return 0;
}
