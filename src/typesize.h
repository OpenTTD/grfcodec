#ifndef _TYPESIZE_H
#define _TYPESIZE_H

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

#include <cstdint>
#include <bit>

#define HAVE_BYTES
#define HAVE_SHORTS
#define HAVE_LONGS
#define HAVE_LONGLONGS

typedef   int8_t S8;
typedef  uint8_t U8;
typedef  int16_t S16;
typedef uint16_t U16;
typedef  int32_t S32;
typedef uint32_t U32;
typedef  int64_t S64;
typedef uint64_t U64;

#ifndef _MSC_VER
#define strnicmp strncasecmp
#define stricmp strcasecmp
#endif

#ifdef __BORLANDC__

// disable warnings for "condition is always false" and "unreachable code"
#pragma warn -ccc
#pragma warn -rch

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

static constexpr U16 BE_SWAP16(U16 value)
{
	if constexpr (std::endian::native == std::endian::big) {
		return (value >> 8) | (value << 8);
	}
	return value;
}

static constexpr U32 BE_SWAP32(U32 value)
{
	if constexpr (std::endian::native == std::endian::big) {
		return ((value >> 24) & 0xFF) | ((value >> 8) & 0xFF00) | ((value << 8) & 0xFF0000) | ((value << 24) & 0xFF000000);
	}
	return value;
}

#endif /* _TYPESIZE_H */

typedef unsigned int uint;
