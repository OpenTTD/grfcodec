
#include <stdio.h>
#include "path.h"
#include "grfcomm.h"

static int DotFound(const char *pB)
{
	if (*(pB-1) == '.')
		pB--;
	switch (*--pB) {
	case ':'  :
		if (*(pB-2) != '\0')
			break;
	case '/'  :
	case '\\' :
	case '\0' :
		return 1;
	}
	return 0;
}

static char *stpcopy(char *dest, const char *src, unsigned maxlen)
{
	safestrncpy(dest, src, maxlen);
	return dest + strlen(dest);
}

void fnmerge(register char *pathP, const char *driveP,
	const char *dirP, const char *nameP, const char *extP)
{
	char *origpathP = pathP;
	if (driveP && *driveP) {
		*pathP++ = *driveP++;
		*pathP++ = ':';
	}

	if (dirP && *dirP) {
		pathP = stpcopy(pathP, dirP, MAXPATH - (pathP - origpathP));
		if (*(pathP-1) != '\\' && *(pathP-1) != '/') *pathP++ = '/';
	}

	if (nameP) {
		pathP = stpcopy(pathP, nameP, MAXPATH - (pathP - origpathP));
	}

	if (extP && *extP) {
		if (*extP != '.') *pathP++ = '.';
		pathP = stpcopy(pathP, extP, MAXPATH - (pathP - origpathP));
	}
	*pathP = 0;
}

int fnsplit(const char *pathP, char *driveP, char *dirP,
	char *nameP, char *extP)
{
	register char	*pB;
	register int	Wrk;
	int		Ret;

	char buf[ MAXPATH+2 ];

	/*
	  Set all string to default value zero
	*/
	Ret = 0;
	if (driveP)
		*driveP = 0;
	if (dirP)
		*dirP = 0;
	if (nameP)
		*nameP = 0;
	if (extP)
		*extP = 0;

	/*
		Copy filename into template up to MAXPATH characters
	*/
	pB = buf;
	while (*pathP == ' ')
		pathP++;
	if ((Wrk = strlen(pathP)) > MAXPATH)
		Wrk = MAXPATH - 1;
	*pB++ = 0;
	safestrncpy(pB, pathP, Wrk + 1);
	pB += Wrk;

	/*
	  Split the filename and fill corresponding nonzero pointers
	*/
	Wrk = 0;
	for (; ; ) {
		switch (*--pB) {
		case '.'  :
			if (!Wrk && (*(pB+1) == '\0'))
				Wrk = DotFound(pB);
			if ((!Wrk) && ((Ret & EXTENSION) == 0)) {
				Ret |= EXTENSION;
				safestrncpy(extP, pB, MAXEXT);
				*pB = 0;
			}
			continue;
		case ':'  :
			if (pB != &buf[2])
				continue;
		case '\0' :
			if (Wrk) {
				if (*++pB)
					Ret |= DIRECTORY;
				safestrncpy(dirP, pB, MAXDIR);
				*pB-- = 0;
				break;
			}
		case '/'  :
		case '\\' :
			if (!Wrk) {
				Wrk++;
				if (*++pB)
					Ret |= FILENAME;
				safestrncpy(nameP, pB, MAXFILE);
				*pB-- = 0;
				if (*pB == 0 || (*pB == ':' && pB == &buf[2]))
					break;
			}
			continue;
		case '*'  :
		case '?'  :
			if (!Wrk)
				Ret |= WILDCARDS;
                default :
			continue;
		}
		break;
	}
	if (*pB == ':') {
		if (buf[1])
			Ret |= DRIVE;
		safestrncpy(driveP, &buf[1], MAXDRIVE);
	}

	return (Ret);
}
