/*
 * data.h
 * Declarations for the data files helper functions.
 *
 * Copyright 2005-2006 by Dale McCoy.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _RENUM_DATA_H_INCLUDED_
#define _RENUM_DATA_H_INCLUDED_

#include <cstdio>

#ifndef DATA
#define DATA() enum files{
#endif
#ifndef DATA_FILE
#define DATA_FILE(name) dat##name,
#endif
#ifndef END_DATA
#define END_DATA() };
#endif

DATA()
DATA_FILE(feat)
DATA_FILE(0)
DATA_FILE(2v)
DATA_FILE(79Dv)
DATA_FILE(D)
DATA_FILE(IDs)
DATA_FILE(4)
DATA_FILE(B)
DATA_FILE(5)
DATA_FILE(TextIDs)
DATA_FILE(0f8)
DATA_FILE(callbacks)
DATA_FILE(2r)
DATA_FILE(langs)

/*
 * To add a new data files, pick a name (matching the regex [0-9a-zA-Z_]*),
 * add a DATA_FILE(name) macro here, and add a static const char _datname[]
 * to data.cpp. (Preferably after all the others, about a two-thirds of the
 * way down. More instructions there.)
 *
 * Open the file with FILE*pFile=myfopen(name);. myfopen (re)writes, if
 * necessary, name.dat and returns a non-null input FILE*.
 * Read from the file using standard C file IO functions, or the three
 * #defines below. CheckEOF errors if ch==EOF and returns ch. GetCheck*
 * returns a byte/word read from pFile, and will error if insufficient data
 * is present. GetCheckWord (and ...Dword, if/when implemented) read
 * words/dwords little-endian.
 * name is used to generate proper error messages.
 *
 * Close the file with fclose(), as normal.
 * Unless you have a very good reason, do not call the FILE* anything other
 * than pFile. The GetCheck*s all assume that the FILE* is pFile.
 */

END_DATA()

#define myfopen(file) _myfopen(dat##file)
FILE*_myfopen(files);
int _CheckEOF(int,files,const char*,int);
int _GetCheckWord(FILE*,files,const char*,int);
#define CheckEOF(ch,name) _CheckEOF(ch,dat##name,__FILE__,__LINE__)
#define GetCheckByte(name) CheckEOF(fgetc(pFile),name)
#define GetCheckWord(name) _GetCheckWord(pFile,dat##name,__FILE__,__LINE__)

#endif//_RENUM_DATA_H_INCLUDED_
