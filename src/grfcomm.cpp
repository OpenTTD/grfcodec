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

#if defined(WIN32) || defined(GCC32) || defined(GCC64)
#	include "path.h"
#endif


#include "grfcomm.h"

char *lastspritefilename;

const char *e_openfile = "Error opening %.228s";



static int getspritefilename(char *filename, const char *basefilename, char *subdirectory, const char *ext, long spriteno)
{
	char bdrive[MAXDRIVE], bdirectory[MAXDIR], bname[MAXFILE], bext[MAXEXT];
	char sdrive[MAXDRIVE], sdirectory[MAXDIR];

	fnsplit(subdirectory, sdrive, sdirectory, NULL, NULL);
	fnsplit(basefilename, bdrive, bdirectory, bname, bext);

	if (strlen(sdrive)) {		// drive given, go relative
		char sfullpath[MAXPATH];
		safestrncpy(sfullpath, subdirectory, MAXPATH);
		fnsplit(sfullpath, sdrive, sdirectory, NULL, NULL);

		safestrncpy(bdrive, sdrive, MAXDRIVE);
	}

	if (strlen(sdirectory)) {
		int offset = (sdirectory[0] == '\\' || sdirectory[0] == '/') ? 0 : strlen(bdirectory);
		safestrncpy(bdirectory + offset, sdirectory, MAXDIR - offset);
	}

	if (spriteno >= 0) {
		int offset = strlen(bname);
		snprintf(bname + offset, MAXFILE - offset, "%02ld", spriteno);
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

	safestrncpy(bext, ext, MAXEXT);

	fnmerge(filename, bdrive, bdirectory, bname, bext);
	fnmerge(subdirectory, bdrive, bdirectory, NULL, NULL);

	return 0;
}

char *spritefilename(const char *basefilename, const char *reldirectory, const char *ext, int spriteno, const char *mode, int mustexist)
{
	static char filename[MAXFILE];
	char directory[MAXDIR];
	FILE *sprite = NULL;

	safestrncpy(directory, reldirectory, MAXDIR);
	getspritefilename(filename, basefilename, directory, ext, spriteno);

	size_t dir_len = strlen(directory);
	if (dir_len != 0 && (directory[dir_len-1] == '\\' || directory[dir_len-1] == '/'))
		directory[dir_len-1] = 0;	// cut off trailing backslash

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

void cfread(const char *action, void *ptr, size_t size, size_t n, FILE *stream)
{
	size_t read = fread(ptr, 1, size * n, stream);

	if (read != size * n) {
		fperror("\nError while %s, got %d, wanted %d, at %ld", action, read, size * n,
			ftell(stream));
		exit(2);
	}
}

void cfwrite(const char *action, const void *ptr, size_t size, size_t n, FILE *stream)
{
	size_t written = fwrite(ptr, 1, size * n, stream);

	if (written != size * n) {
		fperror("\nError while %s, got %d, wanted %d", action, written, size * n);
		exit(2);
	}
}

/**
 * Get the "backup" version of this filename,
 * i.e. with .bak at the end.
 * @param filename The filename to get the backup version of.
 * @return An allocated string. You have to free it!
 */
char *getbakfilename(const char *filename)
{
	/* Length of filename + length of ".bak" + '\0' */
	int len = strlen(filename) + 4 + 1;
	char *bakfile = (char*) malloc(len);

	strcpy(bakfile, filename); // Safe use due to already checked buffer size
	char *c = strrchr(bakfile, '.');
	if (!c) c = bakfile + strlen(bakfile);
	strcpy(c, ".bak"); // Safe use due to already checked buffer size

	return bakfile;
}

char *safestrncpy(char *dest, const char *src, size_t n)
{
	if (dest == NULL) return dest;

	strncpy(dest, src, n);
	if (n > 0) dest[n - 1] = 0;
	return dest;
}
