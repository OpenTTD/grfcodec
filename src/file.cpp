/*****************************************\
*                                         *
* FILE.CC - Classes for image file		  *
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

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "file.h"
#include "error.h"

/***********************\
*						*
* class singlefile		*
*						*
\***********************/

singlefile::singlefile(const char *filename, const char *mode, const char *dir)
{
	FILE *f = fopen(filename, mode);
	if (!f) {
		fperror("\nCan't read %s", filename);
		exit(1);
	}
	setfile(f);
	thefilename = strdup(filename);
	autoclose = 1;
	if (dir)
		directory = strdup(dir);
	else
		directory = NULL;
};

singlefile::~singlefile()
{
	if (autoclose && thefile)
		fclose(thefile);
	free(thefilename);
	free(directory);
};

void singlefile::setfile(FILE *file)
{
	thefile = file;
	thefilename = NULL;
	autoclose = 0;
}
