#ifndef _GRFCOMM_H
#define _GRFCOMM_H


/* header file for grfcomm.h */

BEGINC

extern char *lastspritefilename;

extern const char *e_openfile;

void usage(char *extratext
#ifdef __cplusplus
	 = ""
#endif
);

int getspritefilename(char *filename, char *basefilename, char *subdirectory, char *ext, long spriteno);

char *spritefilename(char *basefilename, char *reldirectory, char *ext, int spriteno, char *mode, int mustexist);
int doopen(char *grffile, char *dir, char *ext, char *mode,
	char **filename, FILE **file, int mustexist);

int movetoreal(char *newfile, char *realfile);

ENDC

#endif /* _GRFCOMM_H */
