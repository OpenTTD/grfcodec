/*
 * renum.h
 *
 * Contains generic NFORenum stuff
 *
 * Copyright 2004-2006 by Dale McCoy.
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
 * along with this program; if not, write to the Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _RENUM_RENUM_H_INCLUDED_
#define _RENUM_RENUM_H_INCLUDED_

/* If your compiler errors on the following line, boost is not
 * properly installed.
 * Get boost from http://www.boost.org */
#include <boost/current_function.hpp>

/* file handling defs:
 * dirname contains the name of the sprites directory
 * foo_ext contains the extention to use for foo
 * These used to be all caps on _WIN32 systems, but I decided that
 * consistancy was preferable, and Win32 systems are case-insensitive.
 */

#define dirname "sprites/"

#define nfo_ext ".nfo"
#define new_ext ".new"
#define bak_ext ".bak"

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;

#if defined DEBUG || defined _DEBUG
inline int _FORCE_INT_(int x){return x;}
#define verify assert
#else
#define verify(x) (void(x))
#define _FORCE_INT_(x) ((int)x)
#endif

enum{EOK,EWARN=3,EERROR,EPARSE,EFILE,EDATA,EFATAL};
void SetCode(int);

#define INTERNAL_ERROR(var,val)\
	if(true){\
		IssueMessage(0,INTERNAL_ERROR_TEXT,__FILE__,__LINE__,_spritenum,#var,_FORCE_INT_(val),BOOST_CURRENT_FUNCTION);\
		assert(false);\
		exit(EFATAL);\
	}else\
		((void)0)

#define DEFAULT(var)\
	default:\
		INTERNAL_ERROR(var,var);

#define VERIFY(cond,var)\
	if(!(cond))INTERNAL_ERROR(var,var);\
	else\
		((void)0)

#define EXPECTED_BYTES(off) (off>>24)
#define EXPECTED_LOC(off) (off&0xFFFFFF)

#define spritenum() (GetState(DIFF)?-1:\
	GetState(USEOLDSPRITENUMS)?oldspritenum:(int)_spritenum)

#endif//_RENUM_RENUM_H_INCLUDED_
