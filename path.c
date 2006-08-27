
#include "path.h"

static void CopyIt(char *dst, const char *src, unsigned maxlen)
{
	if (dst) {
		if(strlen(src) >= maxlen)
		{
			strncpy(dst, src, maxlen);
			dst[maxlen] = 0;
		}
		else
			strcpy(dst, src);
	}
}

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

static char *stpcopy(char *dest, const char *src)
{
  return strcpy(dest, src) + strlen(src);
}

void fnmerge(register char *pathP, const char *driveP,
	const char *dirP, const char *nameP, const char *extP)
{
	if (driveP && *driveP)
		{
		*pathP++ = *driveP++;
		*pathP++ = ':';
		}
	if (dirP && *dirP)
		{
		pathP = stpcopy(pathP,dirP);
		if (*(pathP-1) != '\\' && *(pathP-1) != '/') *pathP++ = '/';
		}
	if (nameP)
	pathP = stpcopy(pathP,nameP);
	if (extP && *extP)
		{
		if (*extP != '.') *pathP++ = '.';
		pathP = stpcopy(pathP,extP);
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
		Wrk = MAXPATH;
	*pB++ = 0;
	strncpy(pB, pathP, Wrk);
	*(pB += Wrk) = 0;

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
				CopyIt(extP, pB, MAXEXT - 1);
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
				CopyIt(dirP, pB, MAXDIR - 1);
				*pB-- = 0;
				break;
			}
		case '/'  :
		case '\\' :
			if (!Wrk) {
				Wrk++;
				if (*++pB)
					Ret |= FILENAME;
					CopyIt(nameP, pB, MAXFILE - 1);
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
		CopyIt(driveP, &buf[1], MAXDRIVE - 1);
	}

	return (Ret);
}
