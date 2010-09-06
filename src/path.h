#ifndef _PATH_H
#define _PATH_H

#include <stdlib.h>
#include <string.h>
#include "typesize.h"

#define WILDCARDS 0x01
#define EXTENSION 0x02
#define FILENAME  0x04
#define DIRECTORY 0x08
#define DRIVE     0x10

#define MAXPATH   1024
#define MAXDRIVE  3
#define MAXDIR    1024
#define MAXFILE   1024
#define MAXEXT    32

void fnmerge(register char *pathP, const char *driveP,
	const char *dirP, const char *nameP, const char *extP);
int fnsplit(const char *pathP, char *driveP, char *dirP,
		char *nameP, char *extP);

#endif /* _PATH_H */
