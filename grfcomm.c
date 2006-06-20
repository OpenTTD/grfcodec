/*
	Functions and classes common to all GRF programs
	Copyright (C) 2000-2003 by Josef Drexler
	Distributed under the GNU General Public License
*/

#include <stdlib.h>
#include <stdio.h>
//#include <dir.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#ifdef MINGW
#	include <io.h>
#	define mkdir(a,b) mkdir(a)
#elif defined(_MSC_VER)
#	include <direct.h>
#	define mkdir(a,b) mkdir(a)
#endif


#include "error.h"
//#include "sprites.h"

#ifdef WIN32
#	include "path.h"
#endif

#include "grfcomm.h"

extern char* usagetext;
char *lastspritefilename;

const char *e_openfile = "Error opening %.228s";


void usage(char *extratext)
{
	printf(usagetext, extratext);

	exit(1);
}


int getspritefilename(char *filename, const char *basefilename, char *subdirectory, const char *ext, long spriteno)
{
	char *fullpath = strdup(basefilename);//_fullpath(NULL, basefilename, 0);

	char bdrive[5], bdirectory[128], bname[20], bext[6];
	char sdrive[5], sdirectory[128];

	fnsplit(subdirectory, sdrive, sdirectory, NULL, NULL);
	fnsplit(fullpath,     bdrive, bdirectory, bname, bext);

	if (strlen(sdrive)) {		// drive given, go relative
		char *sfullpath = strdup(subdirectory);//_fullpath(NULL, subdirectory, 0);
		fnsplit(sfullpath, sdrive, sdirectory, NULL, NULL);
		free(sfullpath);

		strcpy(bdrive, sdrive);
	}

	if (strlen(sdirectory)) {
		if (sdirectory[0] == '\\' || sdirectory[0] == '/')	// absolute path
			strcpy(bdirectory, sdirectory);
		else
			strcat(bdirectory, sdirectory);
	}

	free(fullpath);

	if (spriteno >= 0) {
		sprintf(bname + strlen(bname), "%02ld", spriteno);
		/*	int baselen = 8 - strlen(bname);
		char *numpart = bname + (8 - baselen);

		long max = 1;
		int i;
		for (i=0; i<baselen; i++) max*=10;

		if (spriteno < max) {		// can be expressed with numbers only
		sprintf(numpart, "%0*d", baselen, spriteno);

		} else {
		spriteno -= max;
		max = 1;
		for (i=1; i<baselen; i++) max*=36;

		spriteno += 10*max;

		if (spriteno >= max*36) {
		printf("Cannot find a sprite filename!\n");
		exit(2);
		}

		for (i=0; i<baselen; i++) {
		long digit = spriteno / max;
		if (digit < 10)
		*numpart = '0' + digit;
		else
		*numpart = 'A' + digit - 10;

		spriteno -= max * digit;
		max /= 36;
		numpart++;
		}
		}
		*/
	}

	strcpy(bext, ext);

	fnmerge(filename, bdrive, bdirectory, bname, bext);
	fnmerge(subdirectory, bdrive, bdirectory, NULL, NULL);

	return 0;
}

char *spritefilename(const char *basefilename, const char *reldirectory, const char *ext, int spriteno, const char *mode, int mustexist)
{
	static char filename[128];
	char directory[128];
	FILE *sprite = NULL;

	strcpy(directory, reldirectory);
	getspritefilename(filename, basefilename, directory, ext, spriteno);

	if (directory[strlen(directory)-1] == '\\' || directory[strlen(directory)-1] == '/')
		directory[strlen(directory)-1] = 0;	// cut off trailing backslash

	while (mustexist) {	// actuall mustexist doesn't change, loop is terminated by explicit break
		sprite = fopen(filename, mode);
		if (!sprite) {
			if (errno == ENOENT) {		// directory doesn't exist
				if (strchr(mode, 'w')) {	// but we need to write to a file there
					if (mkdir(directory, 0755)) {	// so try creating it
						fperror("Creating %.228s", directory);
						exit(2);
					}
				} else {			// not writing, so report file missing
					fperror("Cannot read %s", filename);
					exit(2);
				}
			} else {
				fperror(e_openfile, filename);
				exit(2);
			}
		} else
			break;
	}

	if (sprite)
		fclose(sprite);

	lastspritefilename = filename;

	return filename;
}

int doopen(const char *grffile, const char *dir, const char *ext, const char *mode,
		   char **filename, FILE **file, int mustexist)
{
	char *fn;
	FILE *f;

	fn = spritefilename(grffile, dir, ext, -1, mode, mustexist);
	if (filename) *filename=strdup(fn);

	f = fopen(fn, mode);

	if (!f) {
		fperror(e_openfile, lastspritefilename);
		exit(2);
	}

	if (file) *file = f;
	else fclose(f);

	return 1;
}

