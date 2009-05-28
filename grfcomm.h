#ifndef _GRFCOMM_H
#define _GRFCOMM_H


/* header file for grfcomm.h */

BEGINC

extern char *lastspritefilename;

extern const char *e_openfile;

void usage(const char *extratext
#ifdef __cplusplus
	 = ""
#endif
);

char *spritefilename(const char *basefilename, const char *reldirectory, const char *ext, int spriteno, const char *mode, int mustexist);
int doopen(const char *grffile, const char *dir, const char *ext, const char *mode,
	char **filename, FILE **file, int mustexist);

ENDC

#endif /* _GRFCOMM_H */
