#ifndef _FILE_H
#define _FILE_H

/*****************************************\
*                                         *
* FILE.H - Classes for image file		  *
*		  processing					  *
*                                         *
* Copyright (C) 2000 by Josef Drexler     *
*               <jdrexler@julian.uwo.ca>  *
*                                         *
* Permission granted to copy and redist-  *
* ribute under the terms of the GNU GPL.  *
* For more info please read the file      *
* COPYING which should have come with     *
* this file.                              *
*                                         *
\*****************************************/


class multifile {
	public:
	multifile() {};
	virtual ~multifile() {};
	virtual FILE *curfile()  { return NULL; };
	virtual FILE *nextfile() { return NULL; };
	virtual const char *filename() { return NULL; };
	virtual const char *getdirectory() { return NULL; };
};

class singlefile : public multifile {
	public:
	singlefile(const char *filename, const char *mode, const char *dir);
	virtual ~singlefile();

	void setfile(FILE *file);

	virtual FILE *curfile()  { return thefile; };
	virtual const char *filename() { return thefilename; };
	virtual const char *getdirectory() { return directory; };

	private:
	char *thefilename, *directory;
	FILE *thefile;
	int autoclose;
};

#endif /* _FILE_H */
