
/*****************************************\
*                                         *
* GRFDiff - A program to compare a .grf   *
*           file and the decoded .pcx     *
*           file(s) and produce a file    *
*           of the sprites that were      *
*           modified                      *
*                                         *
*                                         *
* Copyright (C) 2003 by Josef Drexler     *
*                      <jdrexler@uwo.ca>  *
*                                         *
* Permission granted to copy and redist-  *
* ribute under the terms of the GNU GPL.  *
* For more info please read the file      *
* COPYING which should have come with     *
* this file.                              *
*                                         *
\*****************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
#include <getopt.h>

#define DOCHECK

#include "error.h"
#include "sprites.h"
#include "grfcomm.h"
#include "grfmrg.h"
#include "path.h"
#include "version.h"


/*
	Format of a .GRD file:

	Offset	Size	Content
	0	4	Magic code to identify the GRD file
	4	2	GRD file version
	6	2	Number of sprites in this file
	8	1	Length of GRF filename=L (including terminating NULL)
	9	L	GRF filename, without .grf extension or path
	9+L	var.	Sprite data

	Each sprite has this format:

	Offset	Size 	Content
	0	2	Number of the sprite, counting from 0
	2	var.	Sprite content as stored in a GRF file

*/

U32 GRDmagic = 0x67fb49ad;

const char *grffile[2];
char *grdfile = NULL;
FILE *grd = NULL;
long grdstart;
int selfextr = 0, alwaysyes = 0, onlylist = 0;

char *difflist;
int difflistfrom, difflistto;

int differences[32][2], diffruns;

const char *exts[2] = { ".GRD", ".EXE" };


static void usage(void)
{
	printf(
		"Usage:\n"
		"    GRFDiff [options] <Org. GRF-File> <New GRF-File>\n"
		"	Compare the two GRF files and produce a GRD file containing only the\n"
		"	sprites from the second file that are different from the first file.\n"
		"\n"
		"GRFDiff makes a .GRD file with the same basename as the new GRF file.\n"
		"\n"
		"Options:\n"
		"	-h   Show this help\n"
		"	-l <Numbers>\n"
		"	     Save the sprites with these numbers in the .GRD instead of\n"
		"	     finding the modified sprites.  With this option, the first\n"
		"	     GRF file can be omitted.\n"
		"	     Format of the numbers: <from1>[-<to1>][,<from2>[-<to2>]]...\n"
		"		e.g.  1-5,8,20-31 (must be increasing numbers)\n"
		"	-n   Only show a list of modified sprites, don't make a .GRD file\n"
		"	-o <GRD-File>\n"
		"	     Write to this file instead\n"
		"	-x   Make a self-extracting (.EXE) file instead of a .GRD\n"
		"	-y   Answer 'y' to all questions\n"
		"\n"
		"You can specify several sets of GRF files along with their -l options by\n"
		"separating them with a double-dash `--'.  The result will be written to a\n"
		"single .GRD file.  Only -l is valid after the first set.\n"
		"\n"
		"GRFDiff is Copyright (C) 2003 by Josef Drexler\n"
		"You may copy and redistribute it under the terms of the GNU General Public\n"
		"License, as stated in the file 'COPYING'.\n"
		);

	exit(1);
}


void closegrd()
{
	if (ftell(grd) <= grdstart) {
		// zero-length file, remove it
		printf("Removing %s\n", grdfile);
		remove(grdfile);
	} else {
		// write "EOF" marker

		GRDmagic = ~GRDmagic;

		cfwrite("writing end-of-file", &GRDmagic, 4, 1, grd);

		// might have some trailing garbage if the last set of comparisons
		// was empty (but others weren't), but I don't know how to truncate
		// a file on Windows
	}

	fclose(grd);
}

void myexit(int code)
{
	if (grd)
		closegrd();
	exit(code);
}


class infostorer : virtual public spriteinfowriter {
public:
	infostorer(U8 *infptr, U8 **dataptr, U16 *datasizeptr);

	virtual void addsprite(int x, U8 info[8]);
	virtual void adddata(U16 newsize, U8 *newdata);

	U8 *inf;
	U8 **data;
	U16 *datasize;
};

infostorer::infostorer(U8 *infptr, U8 **dataptr, U16 *datasizeptr)
{
	inf = infptr;
	datasize = datasizeptr;
	data = dataptr;
}

void infostorer::addsprite(int x, U8 info[8])
{
	int i;
	for(i=0; i<8; i++) inf[i] = info[i];
}

void infostorer::adddata(U16 newsize, U8 *newdata)
{
	*datasize = newsize;
	*data = newdata;
}

class grfstore : virtual public spritestorage {
public:
	grfstore(FILE *f) { grf = f; sdata = NULL; };

	virtual void newsprite();
	virtual void setsize(int sx, int sy);
	virtual void nextpixel(U8 colour);
	virtual void spritedone();

	void getorgdata(U16 *size, U8 **data);

	int differsfrom(grfstore *other);

	FILE *grf;
	int sizex, sizey;
	long spritesize, datasize, spriteofs;
	long fpos_start, fpos_end;
	U8 *sdata;
};

void grfstore::newsprite()
{
	if (sdata)
		free(sdata);
	sdata = NULL;
	fpos_start = ftell(grf);
}

void grfstore::setsize(int sx, int sy)
{
	sizex = sx;
	sizey = sy;
	spritesize = (long) sx * (long) sy;
	spriteofs = 0;

	if (!spritesize)	// verbatim data?
		return;

	sdata = (U8*) malloc(spritesize);
	if (!sdata) {
		printf("Cannot allocate %ld bytes for sprite\n", spritesize);
		myexit(2);
	}
}

void grfstore::nextpixel(U8 colour)
{
	if (spriteofs > spritesize) {
		printf("Sprite has too many pixels!\n");
		myexit(2);
	}
	sdata[spriteofs++] = colour;
}

void grfstore::spritedone()
{
	if (spriteofs < spritesize) {
		printf("Sprite has too few pixels!\n");
		myexit(2);
	}
	fpos_end = ftell(grf);
}

int grfstore::differsfrom(grfstore *other)
{
	if ( (sizex != other->sizex) || (sizey != other->sizey) )
		return 1;

	return memcmp(sdata, other->sdata, spritesize);
}

void grfstore::getorgdata(U16 *size, U8 **data)
{
	*size = fpos_end - fpos_start;
	long fpos_cur = ftell(grf);

	*data = (U8*) malloc(*size);
	if (!*data) {
		printf("Cannot allocate %d bytes for data\n", *size);
		myexit(2);
	}

	fseek(grf, fpos_start, SEEK_SET);
	cfread("reading GRF", *data, 1, *size, grf);
	fseek(grf, fpos_cur, SEEK_SET);
}

char *grfbasename(const char *filename)
{
	static char bname[32];
	fnsplit(filename, NULL, NULL, bname, NULL);
	return bname;
}

time_t mtime(FILE *f)
{
	struct stat statbuf;

	fstat(fileno(f), &statbuf);

	return statbuf.st_mtime;
}

char *strmtime(time_t mtime)
{
	static char strtime[128];

	strftime(strtime, sizeof(strtime), "%d %b %Y  %H:%M:%S", localtime(&mtime));

	return strtime;
}

void mkselfextr()
{
	static const char *action = "writing exe";
	char *block;
	U8 r = 0, e;
	long newr = 0, chunk = 0, grdofs, blank;

	cfwrite(action, grfmrg, 1, grfmrgsize, grd);

	e = 0;

	while (!r) {
		chunk = 1<<e;

		// round size up to next chunksize
		newr = (long) (grfmrgsize+chunk-1) / chunk;

		if (newr > 255) {
			e++;
		} else {
			r = newr;
		}
	}

	fseek(grd, 0x1c, SEEK_SET);

	cfwrite(action, "JD", 2, 1, grd);
	cfwrite(action, &r, 1, 1, grd);
	cfwrite(action, &e, 1, 1, grd);

	fseek(grd, 0, SEEK_END);
	if (ftell(grd) != (S32) grfmrgsize) {
		printf("Huh???\n");
		myexit(2);
	}

	grdofs = newr * chunk;
	blank = grdofs - grfmrgsize;

	if (blank) {
		block = (char*) malloc(blank);
		if (!block) {
			printf("Out of memory.\n");
			myexit(2);
		}

		memset(block, 0, blank);
		cfwrite(action, block, 1, blank, grd);

		free(block);
	}

}

void opengrd()
{
	if (grdfile) {
		grd = fopen(grdfile, "wb");
		if (!grd) {
			fperror(e_openfile, grdfile);
			myexit(2);
		}
	} else if (!onlylist) {
		doopen(grffile[1], "", exts[selfextr], "wb", &grdfile, &grd, 0);
	} else {
		grd = NULL;
	}

	if (grd) {
		printf("Writing %s\n", grdfile);
		if (selfextr)
			mkselfextr();
		grdstart = ftell(grd);
	}

	return;
}

int mkdiff()
{
	static const char *action = "writing GRD";
	FILE *grf[2];
	long grfsize, thisgrdstart = 0;
	int res[2], i, isdiff, numdiff = 0, numdiffofs = 0, lastpct = -1;
	time_t modtime[2];

	for (i=0; i<2; i++) if (grffile[i]) {
		grf[i] = fopen(grffile[i], "rb");
		if (!grf[i]) {
			fperror(e_openfile, grffile[i]);
			myexit(2);
		}
		modtime[i] = mtime(grf[i]);
	} else
		grf[i] = NULL;

	if (grffile[0] && (modtime[0] > modtime[1])) {
		printf("Warning, %s is newer than %s.\n", grffile[0], grffile[1]);
		printf("%s was last modified on %s\n", grffile[0], strmtime(modtime[0]));
		printf("%s was last modified on %s\n", grffile[1], strmtime(modtime[1]));
		printf("But you've told me that %s contains the new sprites.\n", grffile[1]);
		printf("Are you sure you have the right order on the command line [Y/N] ? ");

		if (alwaysyes)
			printf("Y\n");
		else if (tolower(getc(stdin)) != 'y') {
			printf("\nAborted.\n");
			myexit(2);
		}
		printf("\nContinuing.\n");
	}

	if (grd) {
		char *noext = grfbasename(grffile[1]);
		char *p;

		for (p = noext; *p; p++) { *p = tolower(*p); }

		thisgrdstart = ftell(grd);

		cfwrite(action, &GRDmagic, 4, 1, grd);

		i = 1;	// GRD file version
		cfwrite(action, &i, 2, 1, grd);

		numdiffofs = ftell(grd);
		cfwrite(action, &numdiff, 2, 1, grd);

		i = strlen(noext) + 1;
		cfwrite(action, &i, 1, 1, grd);

		cfwrite(action, noext, 1, i, grd);
	} else
		grdfile = NULL;

	fseek(grf[1], 0, SEEK_END);
	grfsize = ftell(grf[1]);
	fseek(grf[1], 0, SEEK_SET);

	U8 inf[2][8], *verbdata[2], *data;
	U16 verbsize[2], size;

	grfstore *grfdata[2];
	infostorer *grfinf[2];

	for (i=0; i<2; i++) if (grf[i]) {
		grfdata[i] = new grfstore(grf[i]);
		grfinf[i] = new infostorer(inf[i], &verbdata[i], &verbsize[i]);
	}

	for (int spriteno = 0; ; spriteno++) {
		int thispct = ftell(grf[1])*100L/grfsize;
		if (thispct != lastpct) {
			lastpct = thispct;
			printf("\rSprite%5d  Done:%3d%%  \r", spriteno, thispct);
		}

		verbdata[0] = verbdata[1] = data = NULL;
		for (i=0; i<2; i++) if (grf[i]) {
			verbsize[i] = 0;
			res[i] = decodesprite(grf[i], grfdata[i], grfinf[i], spriteno);
		}

		if (grf[0] && (res[0] != res[1])) {
			printf("The GRF files have a different number of sprites!\n");
			myexit(2);
		}

		if (!res[1]) {
			printf("\rSprite%5d  Done:%3d%%  \r", spriteno, 100);
			break;
		}

		if (difflist) {
			while ( (difflistfrom < 0) || (spriteno > difflistto) ) {
				difflistfrom = difflistto = 32767;
				if (difflist[0] == ',')
					difflist++;
				if (!difflist[0])
					break;

				difflistfrom = strtol(difflist, &difflist, 0);
				difflistto = -1;
				switch (difflist[0]) {
				case ',':
					difflist++;
				case 0:
					difflistto = difflistfrom;
					break;
				case '-':
					difflist++;
					difflistto = strtol(difflist, &difflist, 0);
					break;
				default:
					printf("Invalid number format at %s\n", difflist);
					myexit(2);
				}
			}
			isdiff = ( (spriteno >= difflistfrom) && (spriteno <= difflistto) );
		} else {
			isdiff = 0;

			// find out if they're different, and remember original data if so
			if (verbsize[0]) {
				if (verbsize[0] != verbsize[1])
					isdiff = 1;
				else
					isdiff = memcmp(verbdata[0], verbdata[1], verbsize[0]);

			} else if (verbsize[1]) {
				isdiff = 1;
			} else {
				isdiff = (inf[0][0] & ~1) != (inf[1][0] & ~1);
				if (!isdiff)
					isdiff = memcmp(inf[0]+1, inf[1]+1, 7);
				if (!isdiff)
					isdiff = grfdata[0]->differsfrom(grfdata[1]);
			}
		}

		if (isdiff) {
			numdiff++;

			if (!onlylist) {
				// this is a bit redundant for the verbatim data,
				// but the code is a lot clearer this way
				grfdata[1]->getorgdata(&size, &data);

				// store the modified sprite
				cfwrite(action, &spriteno, 2, 1, grd);
				cfwrite(action, data, 1, size, grd);
			}

			if ( (diffruns < 0) || (spriteno != differences[diffruns][1]+1) ) {
				diffruns++;
				int nummax = sizeof(differences)/sizeof(differences[0]) - 1;
				if (diffruns >= nummax) {
					diffruns = nummax;
					differences[diffruns][0] = -1;
				} else
					differences[diffruns][0] = spriteno;
			}
			differences[diffruns][1] = spriteno;
		}

		if (data) free(data);
		if (verbdata[0]) free(verbdata[0]);
		if (verbdata[1]) free(verbdata[1]);
	}

	if (!onlylist) {
		long grdcur = ftell(grd);
		fseek(grd, numdiffofs, SEEK_SET);
		cfwrite(action, &numdiff, 2, 1, grd);
		fseek(grd, grdcur, SEEK_SET);
	}
	if (diffruns < 0) {
		printf("\nNo differences.");
		if (grd)
			fseek(grd, thisgrdstart, SEEK_SET);
	} else {
		printf("\nSprites with differences: ");
		for (i=0; i<=diffruns; i++) {
			if (differences[i][0] >= 0) {
				printf("%d", differences[i][0]);
				if (differences[i][0] < differences[i][1])
					printf("-%d", differences[i][1]);
			} else
				printf("... and others");

			if (i < diffruns)
				printf(", ");
		}
		printf(" (%d total)", numdiff);
	}

	for (i=0; i<2; i++)
		if (grf[i]) fclose(grf[i]);

	printf("\nDone!\n");
	return 1;
}

void onlyfirst(char opt)
{
	printf("Warning: Option `-%c' is only valid in the first set of GRF files\n\tand cannot be set later.\n", opt);
}

int moreargs(int argc, char **argv)
{
	if (optind >= argc)
		return 0;

	return strcmp(argv[optind], "--");
}


int main(int argc, char **argv)
{

	puts("GRFDiff " VERSION " - Copyright (C) 1999-2003 by Josef Drexler");

	// loop over all sets of comparisons
	for (int grfset=0; optind<argc; grfset++) {
		// reinitialize the repeatable options
		difflistfrom = diffruns = -1;
		grffile[0] = grffile[1] = difflist = NULL;

		// parse option arguments
		while (optind < argc) {
			// get options in original order (list of options starts with "-")
			char opt = getopt(argc, argv, "-hvl:no:xy");

			if (opt == (char) EOF)
				break;

			if ( (opt == 1) && ( (*(U16 *) optarg) == '+') )
				break;	// next set of files starting

			switch (opt) {
			case 'v':
				/* The version is already printed. */
				return 0;
			case 'l':
				difflist = optarg;
				break;
			case 'n':
				if (grfset)
					onlyfirst(opt);
				else
					onlylist = 1;
				break;
			case 'o':
				if (grfset)
					onlyfirst(opt);
				else
					grdfile = optarg;
				break;
			case 'x':
				if (grfset)
					onlyfirst(opt);
				else
					selfextr = 1;
				break;
			case 'y':
				if (grfset)
					onlyfirst(opt);
				else
					alwaysyes = 1;
				break;
			case 1:
				if (!grffile[0])
					grffile[0] = optarg;
				else if (!grffile[1])
					grffile[1] = optarg;
				else		// too many non-option arguments
					usage();
				break;

			default:
				usage();
			}
		}

		if (!grffile[0] || (strlen(grffile[0]) < 1)
			|| (!difflist &&
			(!grffile[1] || (strlen(grffile[1]) < 1))
			))
			usage();

		// only one file specified while using -l
		if (difflist && !grffile[1]) {
			grffile[1] = grffile[0];
			grffile[0] = NULL;
		}

		if (!grfset)
			opengrd();

		mkdiff();
	}

	myexit(0);
	return 0;
}
