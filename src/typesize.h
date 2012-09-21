#ifndef _TYPESIZE_H
#define _TYPESIZE_H

#include "endian.h"

/*****************************************\
*                                         *
* TYPESIZE.H - Defines variable types     *
*         with a known memory size        *
*         Also defines macros for dealing *
*         with C/C++ exports              *
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

#define maketype(type,size) \
	typedef   signed type S ## size; \
	typedef unsigned type U ## size;

#ifndef _MSC_VER
#define strnicmp strncasecmp
#define stricmp strcasecmp
#endif

#ifdef __BORLANDC__

#	define HAVE_BYTES
#	define HAVE_SHORTS
#	define HAVE_LONGS

	maketype(char,8)
	maketype(short,16)
	maketype(long,32)

// disable warnings for "condition is always false" and "unreachable code"
#pragma warn -ccc
#pragma warn -rch

#elif WIN32

#	define HAVE_BYTES
#	define HAVE_SHORTS
#	define HAVE_LONGS
#	define HAVE_LONGLONGS

	maketype(char,8)
	maketype(short int,16)
	maketype(long int,32)
	maketype(long long,64)

#elif GCC32

#   define HAVE_BYTES
#   define HAVE_SHORTS
#   define HAVE_LONGS
#   define HAVE_LONGLONGS

    maketype(char,8)
    maketype(short int,16)
    maketype(int,32)
    maketype(long long,64)

#elif GCC64

#   define HAVE_BYTES
#   define HAVE_SHORTS
#   define HAVE_LONGS
#   define HAVE_LONGLONGS

    maketype(char,8)
    maketype(short int,16)
    maketype(int,32)
    maketype(long int,64)

#else
#	error Unknown variables sizes, please define.
#endif

#ifdef _MSC_VER
// disable warnings: conversion from $LARGE_SIZE to $SMALL_SIZE
#pragma warning(disable:4267 4244)
// ... conditional expression is constant
#pragma warning(disable:4127)
#endif

#undef maketype

#ifdef DOCHECK
void checksizes()
{
  if (
#ifdef HAVE_BYTES
	(sizeof( S8) != 1) ||
	(sizeof( U8) != 1) ||
#endif
#ifdef HAVE_SHORTS
	(sizeof(S16) != 2) ||
	(sizeof(U16) != 2) ||
#endif
#ifdef HAVE_LONGS
	(sizeof(S32) != 4) ||
	(sizeof(U32) != 4) ||
#endif
#ifdef HAVE_LONGLONGS
	(sizeof(S64) != 8) ||
	(sizeof(U64) != 8) ||
#endif
	(0)	// the ||(0) is so that all previous lines can end in ||
     )
	{
		printf("Fatal: Incorrectly sized variables.\n");
#define PRINT_SZ(t,s) printf("%s size = %d, expected = %d\n", #t, (int)sizeof(t), s)
#ifdef HAVE_BYTES
			PRINT_SZ(S8,1);
			PRINT_SZ(U8,1);
#endif
#ifdef HAVE_SHORTS
			PRINT_SZ(S16,2);
			PRINT_SZ(U16,2);
#endif
#ifdef HAVE_LONGS
			PRINT_SZ(S32,4);
			PRINT_SZ(U32,4);
#endif
#ifdef HAVE_LONGLONGS
			PRINT_SZ(S64,8);
			PRINT_SZ(U64,8);
#endif
#undef PRINT_SZ
		exit(254);
	}

}
#endif /* DOCHECK */

union multitype {
	U32 u32;
	S32 s32;
	U16 u16[2];
	S16 s16[2];
	U8  u8[4];
	S8  s8[4];
};

#ifdef GRFCODEC_BIG_ENDIAN
#	define BE_SWAP16(b) (*((U8*)(&b))+(*(((U8*)(&b))+1)<<8))
#	define BE_SWAP32(b) (*((U8*)(&b))+(*(((U8*)(&b))+1)<<8)+(*(((U8*)(&b))+2)<<16)+(*(((U8*)(&b))+3)<<24))
#	define BYTE_OFSL 1
#	define BYTE_OFSH 0
#elif defined(GRFCODEC_LITTLE_ENDIAN)
#	define BE_SWAP16(b) (b)
#	define BE_SWAP32(b) (b)
#	define BYTE_OFSL 0
#	define BYTE_OFSH 1
#else
# error "Endianness not defined!"
#endif

#endif /* _TYPESIZE_H */

typedef unsigned int uint;
