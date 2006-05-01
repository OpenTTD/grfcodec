
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
#include <getopt.h>
#include <sys/stat.h>

#ifdef MINGW
	#include <io.h>
	#define mkdir(a,b) mkdir(a)
#elif defined(_MSC_VER)
	#include <io.h>
	#include <direct.h>
	#define mkdir(a,b) mkdir(a)
	#define F_OK 0
#else
	#include <unistd.h>
#endif//_MSC_VER

#define DOCHECK

#include "pcx.h"
#include "sprites.h"
#include "pcxsprit.h"
#include "ttdpal.h"
#include "grfcomm.h"
#include "info.h"
#include "error.h"
#include "version.h"
#include "conv.h"

#ifdef WIN32
#	include "path.h"
#endif

BEGINC

char *usagetext=
	"%sUsage:\n"
	"    GRFCODEC -d [<Options>] <GRF-File> [<Directory>]\n"
	"        Decode all sprites in the GRF file and put them in the directory\n"
	"    GRFCODEC -e [-u] <GRF-File> [<Directory>]\n"
	"        Encode all sprites in the directory and combine them in the GRF file\n"
	"\n"
	"<GRF-File> denotes the .GRF file you want to work on, e.g. TRG1.GRF\n"
	"<Directory> is where the individual sprites should be saved. If omitted, they\n"
	"\twill default to a subdirectory called sprites/.\n"
	"\n"
	"Options for decoding:\n"
	"    -w <num>  Write PCX files with the given width (default 800, minimum 16)\n"
	"    -h <num>  Split PCX files when they reach this height (default no limit,\n"
	"              minimum 16)\n"
	"    -b <num>  Organize sprites in boxes of this size (default 16)\n"
	"    -p <pal>  Use this palette instead of the default.  See -p ? for a list.\n"
	"    -t        Disable decoding of plain text characters as strings\n"
	"\n"
	"Options for encoding:\n"
	"    -u        Save uncompressed data (probably not a good idea)\n"
	"    -m <num>  Apply colour translation (-m ? for a list)\n"
	"\n"
	"GRFCODEC is Copyright (C) 2000-2005 by Josef Drexler <josef@ttdpatch.net>\n"
	"You may copy and redistribute it under the terms of the GNU General Public\n"
	"License, as stated in the file 'COPYING'.\n";

ENDC

void showpalettetext()
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
	"[type:] can be bcp or psp.  (Other formats may become available later.)\n"
	"If the type is omitted, bcp is assumed.\n"
	"\n"
	"bcp	binary coded palette with the same format as the palette in a PCX\n"
	"	file: 256 triples of red, green and blue encoded in bytes.\n"
	"psp	the palette format output by Paintshop Pro\n"
	"\n",

	// note, -p values are the array index plus one (so that 0 is not
	// a valid number, which makes the atoi easier)
	PAL_ttd_norm+1, PAL_ttw_norm+1, PAL_ttd_cand+1, PAL_ttw_cand+1,
	PAL_tt1_norm+1, PAL_tt1_mars+1
  );
}

void showcolourmaps()
{
  printf(
	"Options for the -m parameter:\n"
	"\n"
	"	0  Convert a TTD-DOS file to TTD-Windows\n"
	"	1  Convert a TTD-Windows file to TTD-DOS\n"
	"\n"
  );
}

struct defpal {
	char *grffile;
	int defpalno;
};

defpal defpals[] =
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
    };

int *colourmaps[] = { palmap0, palmap1 };


int movetoreal(char *newfile, char *realfile)
{
  // rename original to bak if bak doesn't exist
  char *bakfile = (char*) malloc(strlen(realfile) + 4);
  char *c;
  FILE *tmp;

  strcpy(bakfile, realfile);
  c = strrchr(bakfile, '.');
  if (!c) c = bakfile + strlen(bakfile);
  strcpy(c, ".bak");

  tmp = fopen(bakfile, "rb");

  if (!tmp && (errno == ENOENT)) {
	// .bak doesn't exist, rename orig to .bak
	// XXX: Shouldn't we rename orig to .bak even if .bak already exists? --pasky
	printf("\nRenaming %s to %s", realfile, bakfile);
	if (rename(realfile, bakfile)) {
		errno = EEXIST;		// go delete it
	} else {
		errno = ENOENT;		// don't try to delete it
	}
  }

  if (tmp) fclose(tmp);

  // delete grf if it exists
  if (access(realfile, F_OK) == 0) {
	printf("\nDeleting %s",realfile);
	if (remove(realfile))
		fperror("\nError deleting %s", realfile);
  }

  // rename tmp to grf

  printf("\nReplacing %s with %s\n", realfile, newfile);

  if (rename(newfile, realfile)) {
	fperror("Error renaming %s to %s", newfile, realfile);
	exit(2);
  }
  return 1;
}

class spritefiles : public multifile {
	public:
	spritefiles() { init(); };
	spritefiles(char *basename, char *directory);
	virtual FILE *curfile()  { return thecurfile; };
	virtual FILE *nextfile();
	virtual const char *filename() { return thecurfilename; };
	virtual const char *getdirectory() { return directory; };
	private:
	void init () {
		thecurfile = NULL;
		basename = thecurfilename = directory = NULL;
		filenum = 0; };
	FILE *thecurfile;
	char *thecurfilename, *directory;
	const char *basename;
	unsigned int filenum;
};

spritefiles::spritefiles(char *basename, char *directory)
{
  init();
  spritefiles::basename = basename;
  spritefiles::directory = directory;
}

FILE *spritefiles::nextfile()
{
  FILE *oldfile = thecurfile;
  char *oldname = thecurfilename;

  thecurfilename = strdup(spritefilename(basename, directory, ".PCX", filenum++, "wb", 1));
  thecurfile = fopen(thecurfilename, "wb");

  if (thecurfile) {	// new open succeeded, close old one
	if (oldfile)
		fclose(oldfile);
	if (oldname)
		free(oldname);
  } else {		// retain old one
	thecurfile = oldfile;
	thecurfilename = oldname;
  }

  return thecurfile;
}



int encode(char *file, char *dir, int compress, int *colourmap)
{
  char *grfnew;
  FILE *grf, *infofile;

  doopen(file, "", ".new", "wb", &grfnew, &grf, 0);
  doopen(file, dir, ".nfo", "rt", NULL, &infofile, 1);

  printf("Encoding in temporary file %s\n", grfnew);

  inforeader info(infofile);

  if (colourmap)
	info.installmap(colourmap);

  long totaluncomp = 1, totalcomp = 1;
  long totaltransp = 1, totaluntransp = 1;
  long totalreg = 1, totalunreg = 1;
  int spriteno = 0;


  while (info.next(spriteno)) {
	//int comp1 = totalcomp / totaluncomp;
	int comp2 = (100L * totalcomp / totaluncomp);// % 100;

	//int comp3 = totaltransp / totaluntransp;
	int comp4 = (100L * totaltransp / totaluntransp);// % 100;

	//int comp5 = totalreg / totalunreg;
	int comp6 = (100L * totalreg / totalunreg);// % 100;

	printf("\rSprite%5d  Done:%3d%%  "
		"Compressed:%3d%% (Transparency:%3d%%, Redundancy:%3d%%)\r",
		spriteno, (int) (ftell(info.f)*100L/info.filesize),
		comp2, comp4, comp6);

	if (info.verbatim) {		// non-sprite data, copy verbatim
		if (info.bininclude) {		// include binary file
			FILE *bin = fopen(info.bininclude, "rb");
			if (!bin) {
				fperror("Cannot read %s", info.bininclude);
				exit(2);
			}

			struct stat stat_buf;
			fstat(fileno(bin), &stat_buf);
			off_t fsize = stat_buf.st_size;

			const char *nameofs = info.bininclude + strlen(info.bininclude);
			while (nameofs > info.bininclude) {
				nameofs--;
				if (nameofs[0] == '\\' || nameofs[0] == '/') {
					nameofs++;
					break;
				}
			}
			int namelen = strlen(nameofs);
			if (namelen > 255) {
				fprintf(stderr, "Error: binary include has too long filename %s\n", nameofs);
				exit(2);
			}

			int spritesize = 3 + namelen + fsize;
			if (spritesize < 5) {
				fprintf(stderr, "Error: binary include %s is empty\n", nameofs);
				exit(2);
			}
			if (spritesize > 65535) {
				fprintf(stderr, "Error: binary include %s is too large\n", nameofs);
				exit(2);
			}

			totalcomp += spritesize;
			totaluncomp += spritesize;
			spriteno++;

			fwrite(&spritesize, 1, 2, grf);
			fputc(0xff, grf);
			fputc(0xff, grf);
			fputc(namelen, grf);
			fwrite(nameofs, namelen+1, 1, grf);

			char *buffer = new char[16384];
			while (fsize > 0) {
				int chunk = 16384;
				if (chunk > fsize) chunk=fsize;
				fread(buffer, chunk, 1, bin);
				fwrite(buffer, chunk, 1, grf);
				fsize -= chunk;
			}
			delete[]buffer;
			fclose(bin);
		} else {
			totalcomp += info.size;
			totaluncomp += info.size;
			spriteno++;

			fwrite(&(info.size), 1, 2, grf);
			fputc(0xff, grf);
			for (int i=0; i<info.size; i++)
				fputc(info.nextverb(), grf);
			if (info.verbatim_str) {
				/* XXX: Trailing quote mark. */
				fgetc(info.f);
				info.verbatim_str = 0;
			}
		}
	} else {				// real sprite, encode it
		U8 *image = (U8*) malloc(info.imgsize);
		if (!image) {
			printf("Error: can't allocate sprite memory (%ld bytes)\n", info.imgsize);
			exit(2);
		}

		info.getsprite(image);

		U16 compsize;
		if (info.inf[0] & 8) {
			compsize = encodetile(grf, image, info.imgsize, 0, info.sx, info.sy, info.inf, compress);
			totaltransp += getlasttilesize();	// how much after transparency removed
			totaluntransp += info.imgsize;		// how much with transparency

			totalreg += compsize;			// how much after transp&redund removed
			totalunreg += getlasttilesize();	// how much with redund
		} else {
			compsize = encoderegular(grf, image, info.imgsize, info.inf, compress);
			totaltransp += info.imgsize;
			totaluntransp += info.imgsize;

			totalreg += compsize;
			totalunreg += info.imgsize;
		}

		totalcomp += compsize;
		totaluncomp += info.imgsize;
		spriteno++;
		free(image);
	}
  }

  U16 endoffile = 0;
  U32 checksum = 0;
  fwrite(&endoffile, 1, 2, grf);
  fwrite(&checksum, 1, 4, grf);

  fclose(grf);

  movetoreal(grfnew, spritefilename(file, "", ".grf", -1, "r+b", 0));

  free(grfnew);

  printf("\nDone!\n");
  return 1;
}

int decode(char *file, char *dir, U8 *palette, int box, int width, int height, int *colourmap, int useplaintext)
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

  pcxwrite *pcx;

  if (height == -1)
	pcx = new pcxwrite(new singlefile(spritefilename(file, dir, ".pcx", -2, "wb", 1),"wb", dir));
  else
	pcx = new pcxwrite(new spritefiles(file, dir));

  if (!pcx) {
	printf("Error opening PCX file\n");
	exit(2);
  }

  pcx->setpalette(palette);

  pcx->setcolours(255, 0, 0);

  infowriter *writer = new infowriter(info, (width + box - 1) / box, useplaintext);

  if (colourmap)
	pcx->installwritemap(colourmap);

  pcx->startimage(width, height, box, box, writer);

  count = 0;

  printf("Decoding:\n");

  do {
	int newpct = 100L*ftell(grf)/fsize;

	if (newpct != lastpct) {
		lastpct = newpct;
		printf("Sprite %d at %lX, %3d%% done\r", count, ftell(grf), lastpct);
	}

	count++;
	result = decodesprite(grf, pcx, writer);
  } while (result);
  count--;

  pcx->endimage();
  delete(pcx);	// closes output file

  writer->done(count);

  fclose(info);

  printf("%s has %d sprites, maxx %d, maxy %d, maxs %d.\n",
	file, count, maxx, maxy, maxs);
  fclose(grf);

  return 0;
}

U8 *readpal(char *filearg)
{
  enum paltype { UNK, BCP, PSP };
  paltype type = UNK;
  static U8 pal[256*3];

  if (!strnicmp(filearg, "bcp:", 4))
	type = BCP;
  else if (!strnicmp(filearg, "psp:", 4))
	type = PSP;

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
			printf("%s is not a BCP file.\n", filearg);
			exit(1);
		}
		break;
	case PSP:
		char fmt[16];
		fgets(fmt, sizeof(fmt), f);
		if (strcmp(fmt, "JASC-PAL\r\n")) {
			printf("%s is not a PSP palette file.\n", filearg);
			exit(1);
		}
		int nument, nument2;
		fscanf(f, "%x\n", &nument);
		fscanf(f, "%d\n", &nument2);
		if ( (nument != nument2) || (nument != 256) ) {
			printf("GRFCodec supports only 256 colour palette files.\n");
			exit(1);
		}
		for (int i=0; i<nument; i++) {
			if (!fscanf(f, "%cd %cd %cd\n", pal + i*3, pal+i*3+1, pal+i*3+2)) {
				printf("Error reading palette.\n");
				exit(1);
			}
		}
		break;
  }
  fclose(f);

  return pal;
}

// find default palette
U8* findpal(char *grffile)
{
  char base[12];
  char *bs;
  unsigned int i;

  bs = strrchr(grffile, '\\');
  if (!bs) bs = strrchr(grffile, '/');
  if (!bs) bs = grffile;

  strncpy(base, grffile, sizeof(base)-1);

  bs = strchr(base, '.');
  if (bs) *bs = 0;

  for (i=0; i<sizeof(defpals)/sizeof(defpals[0]); i++) {
	if (!stricmp(defpals[i].grffile, base))
		return defaultpalettes[defpals[i].defpalno];
  }

  return defaultpalettes[0];
}

//extern "C" void debugint(void);

int main(int argc, char **argv)
{
  char directory[128];
  char *grffile = NULL;
  int action = 0;
  int width = 800, height = -1, box = 16, compress = 1;
  U8 *palette = NULL;
  int *colourmap = NULL;
  int useplaintext = 1;

  puts("GRFCodec version " GRFCODECVER " - Copyright (C) 2000-2005 by Josef Drexler");

  checksizes();

#ifdef WIN32
//  debugint();
#endif

  // parse option arguments
  while (1) {
	char opt = getopt(argc, argv, "dew:h:b:cup:m:t");

	if (opt == (char) EOF)
		break;

	switch (opt) {
		case 'e':
			action = 1;
			break;
		case 'd':
			action = 2;
			break;
		case 'w':
			width = atoi(optarg);
			break;
		case 'h':
			height = atoi(optarg);
			break;
		case 'b':
			box = atoi(optarg);
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
		case 'm':
			unsigned int mapnum;
			mapnum = atoi(optarg);
			if (*optarg == '?') {
				showcolourmaps();
				exit(1);
			} else if ( (mapnum >= 0) &&
			     (mapnum <= sizeof(colourmaps)/sizeof(colourmaps[0])) ) {
				colourmap = colourmaps[mapnum];
			} else
				usage();
			break;
		case 'c':
			printf("Warning: Compression is enabled by default, disable with -u\n");
			break;
		case 'u':
			compress = 0;
			break;
		case 't':
			useplaintext = 0;
			break;
		default:
			usage();
	}
  }

  // non-option arguments: filename and potentially directory
  if (optind < argc)
	grffile = argv[optind++];

  if (optind < argc)
	strcpy(directory, argv[optind++]);
  else
	strcpy(directory, "sprites");

  if (directory[strlen(directory) - 1] != '/')
	strcat(directory, "/");

  if (!action || !grffile || (width < 16) ||
	( (height < 16) && (height != -1) ) ||
	(strlen(directory) < 1) ||
	(strlen(grffile) < 1) ||
	(box < 1))
	usage();

  if (action == 1) {
	return encode(grffile, directory, compress, colourmap);
  } else if (action == 2) {
	if (!palette)
		palette = findpal(grffile);

	return decode(grffile, directory, palette, box, width, height, colourmap, useplaintext);
  }

  return 1;
}
