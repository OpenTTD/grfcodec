#ifndef _GRFCOMM_H
#define _GRFCOMM_H


/* header file for grfcomm.h */

BEGINC

extern char *lastspritefilename;

extern const char *e_openfile;

char *spritefilename(const char *basefilename, const char *reldirectory, const char *ext, int spriteno, const char *mode, int mustexist);
int doopen(const char *grffile, const char *dir, const char *ext, const char *mode,
	char **filename, FILE **file, int mustexist);

ENDC

void cfread(const char *action, void *ptr, size_t size, size_t n, FILE *stream);
void cfwrite(const char *action, const void *ptr, size_t size, size_t n, FILE *stream);

#endif /* _GRFCOMM_H */
