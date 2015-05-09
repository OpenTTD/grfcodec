/*
 * data.cpp
 * Data file contents and implementation of helper functions.
 *
 * Copyright 2005-2009 by Dale McCoy.
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

#ifdef _WIN32
#include"win32.h"
#endif

#include<cstdio>
#include<cstdlib>
#include<cassert>
#include<string>
#include<sys/stat.h>
#include<cstdarg>
#include<errno.h>
#ifndef _WIN32
#   include<unistd.h>
#endif

using namespace std;

#include"nforenum.h"
#include"inlines.h"
#include"globals.h"
#include"data.h"
#include"messages.h"
#include"win32.h"
#include"sanity_defines.h"
#include"strings.h"

//Let's dump the data files into the source code. That sounds like fun, right?

/*  Data file format
	================

	Files that appear before the DATA_FILE(feat) line in data.h have two leading
	bytes that are checked and then eaten by myfopen.

	The first byte is a format byte; start it at \x00. Each time the file format
	gets updated (ie the file reader changes), increment it and reset
	the version byte to \x00.

	The second byte is a version byte; increment it every time the file
	gets updated.


	Files that depend on feat.dat (appear below it in data.h), have three
	leading bytes that are eaten by myfopen. The first two are as described
	above.

	The third byte is the maximum feature supported by this file.
	This should usually match the third byte of _datfeat, but under some
	circumstances, it is appropriate to increment the this byte without
	changing _datfeat.
*/


#define NDF_HEADER(format, version) format, version
#define NDF_END 0


// ---------------------------------------------------------------------------
// ------------------------- Feature-independent data ------------------------
// ---------------------------------------------------------------------------

/*	Action 7/9/D variables */
#define NOTHING 0x00   /* Only VarAction2 access */
#define BITMASK 0x00   /* Bitmask variable, use instead of B, W or D */
#define B 0x01         /* Byte access allowed */
#define W 0x02         /* Word access allowed */
#define D 0x04         /* DWord access allowed */
#define WD 0x20        /* Action D Write access allowed */
#define RD 0x40        /* Action D Read access allowed */
#define R7 0x80        /* Action 7 Read access allowed */
static const unsigned char _dat79Dv[]={
NDF_HEADER(0x20, 5),
/* Number of variables: */ 0x25,

/*80*/                           NOTHING,
/*81*/ B |              RD | R7,
/*82*/                           NOTHING,
/*83*/ B |              RD | R7,
/*84*/ B |     D |      RD | R7,
/*85*/  BITMASK  |           R7,
/*86*/  BITMASK  |           R7,
/*87*/                           NOTHING,
/*88*/         D |           R7,
/*89*/                           NOTHING,
/*8A*/                           NOTHING,
/*8B*/     W | D |      RD | R7,
/*8C*/                           NOTHING,
/*8D*/ B |              RD | R7,
/*8E*/ B |         WD | RD | R7,
/*8F*/         D | WD | RD | R7,
/*90*/                           NOTHING,
/*91*/                           NOTHING,
/*92*/ B |              RD | R7,
/*93*/     W | D | WD | RD | R7,
/*94*/     W | D | WD | RD | R7,
/*95*/     W | D | WD | RD | R7,
/*96*/     W | D | WD | RD | R7,
/*97*/ B |         WD | RD | R7,
/*98*/                           NOTHING,
/*99*/         D | WD,
/*9A*/ B | W | D |           R7,
/*9B*/                           NOTHING,
/*9C*/                           NOTHING,
/*9D*/         D |      RD | R7,
/*9E*/         D | WD | RD | R7,
/*9F*/         D | WD,
/*A0*/                           NOTHING,
/*A1*/         D |      RD | R7,
/*A2*/         D |      RD | R7,
/*A3*/         D |      RD | R7,
/*A4*/         D |      RD | R7,
NDF_END
};
#undef B
#undef W
#undef D
#undef WD
#undef RD
#undef R7

/*	Action B
	========
*/
static const char _datB[]="\x01\x02"
// Maximum severity:
"\x03"
// Number of message types:
"\x07"
// Parameter counts for message types:
"\x02\x02\x02\x03\x02\x02\x02"
;


/*	Action 5
 * Historical remark:
 *   The higher nibble in the OPTIONS byte can be used to define the type and alternatives for multiple entries.
 *   We do not use that anymore, as we want one line per entry.
 */
#define OPTIONS(num) 0x10 | num      /* Number of alternatives wrt. total sprite count */
#define RECOLOUR 0x81                /* Only allow recolour sprites */
#define MIXED 0x82                   /* Allow both recolour and real sprites */
#define WORD 0x84                    /* Spritecount is larger than a byte and is defined using W() */
#define OFFSET 0x88                  /* Allow replacing sprite ranges with offsets */
#define W(cnt) cnt & 0xFF, cnt >> 8  /* Construct word count */
static const unsigned char _dat5[]={
NDF_HEADER(0x04, 17),
/*04*/ OFFSET,          OPTIONS(3), 0x30, 0x70, 0xF0,
/*05*/ OFFSET,          OPTIONS(1), 0x30,
/*06*/ OFFSET,          OPTIONS(2), 0x4A, 0x5A,
/*07*/                  OPTIONS(1), 0x5D,
/*08*/ OFFSET,          OPTIONS(1), 0x41,
/*09*/ OFFSET,          OPTIONS(1), 0x06,
/*0A*/ OFFSET | RECOLOUR | WORD, OPTIONS(1), W(0x100),
/*0B*/ OFFSET,          OPTIONS(1), 0x71,
/*0C*/                  OPTIONS(1), 0x85,
/*0D*/                  OPTIONS(2), 0x10, 0x12,
/*0E*/ MIXED,           OPTIONS(1), 0x00,
/*0F*/ OFFSET,          OPTIONS(1), 0x0C,
/*10*/ OFFSET,          OPTIONS(1), 0x0F,
/*11*/ OFFSET,          OPTIONS(1), 0x08,
/*12*/ OFFSET,          OPTIONS(1), 0x08,
/*13*/ OFFSET,          OPTIONS(1), 0x37,
/*14*/ OFFSET,          OPTIONS(1), 0x24,
/*15*/ OFFSET,          OPTIONS(1), 0xAF,
/*16*/ OFFSET,          OPTIONS(1), 0x09,
/*17*/ OFFSET,          OPTIONS(1), 0x10,
/*18*/ RECOLOUR,        OPTIONS(1), 0x01,
00,
NDF_END
};
#undef OPTIONS
#undef RECOLOUR
#undef MIXED
#undef WORD
#undef OFFSET
#undef W

/*	Text IDs
	========
*/
static const char _datTextIDs[]="\x04\x09"
//-0000-          -1000-          -2000-          -3000-
"\x35\x03\x11\x00\x25\x00\x19\x00\x5D\x00\x11\x00\x6D\x00\x08\x00"
//-4000-          -5000-          -6000-          -7000-
"\x11\x00\x3C\x00\x2A\x00\x08\x00\x19\x00\x39\x00\x80\x00\x00\x00"
//-8000-          -9000-          -A000-          -B000-
"\x08\x01\x6C\x00\x38\x00\x43\x00\x44\x00\x00\x00\x06\x00\x00\x00"
//-C000-          -D000-          -E000-          -F000-
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x4D\x00\x00\x00\x00\x00\x9B\x01"
// Number of special string ID ranges:
"\x03"
// High bytes of special string IDs:
"\xC4\xC5\xC9"
;

/*	Callbacks */
#define W(cnt) cnt & 0xFF, cnt >> 8  /* Construct word count */
#define TRAIN 0x0
#define ROADVEH 0x1
#define SHIP 0x2
#define AIRCRAFT 0x3
#define STATION 0x4
#define CANAL 0x5
#define BRIDGE 0x6
#define HOUSE 0x7
#define GLOBAL 0x8
#define INDTILE 0x9
#define INDUSTRY 0xA
#define CARGO 0xB
#define SOUND 0xC
#define AIRPORT 0xD
#define SIGNAL 0xE
#define OBJECT 0xF
#define RAILTYPE 0x10
#define AIRTILE 0x11
#define MASK(f) 0x80 | (1 << f)
#define NONE 0x80
#define GROUNDVEHICLE MASK(TRAIN) | MASK(ROADVEH)
#define VEHICLE MASK(TRAIN) | MASK(ROADVEH) | MASK(SHIP) | MASK(AIRCRAFT)
static const unsigned char _datcallbacks[]={
NDF_HEADER(0x05, 24),
/* Count: */ W(0x161),
/* 00*/ NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
/* 10*/ GROUNDVEHICLE | MASK(SHIP), GROUNDVEHICLE, VEHICLE, STATION, STATION, VEHICLE, GROUNDVEHICLE, HOUSE,
/* 18*/ MASK(STATION) | VEHICLE, VEHICLE, HOUSE, HOUSE, HOUSE, TRAIN, HOUSE, HOUSE,
/* 20*/ HOUSE, HOUSE, INDUSTRY, VEHICLE, STATION, INDTILE, INDTILE, INDTILE,
/* 28*/ INDUSTRY, INDUSTRY, HOUSE, INDTILE, INDTILE, VEHICLE, HOUSE, INDTILE,
/* 30*/ INDTILE, VEHICLE, VEHICLE, MASK(BRIDGE) | VEHICLE, VEHICLE, INDUSTRY, VEHICLE, INDUSTRY,
/* 38*/ INDUSTRY, CARGO, INDUSTRY, INDUSTRY, INDTILE, INDUSTRY, NONE,  NONE,
/* 40*/ NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,
/* 50*/ NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,
/* 60*/ NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,
/* 70*/ NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,
/* 80*/ NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,
/* 90*/ NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,
/* A0*/ NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,
/* B0*/ NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,
/* C0*/ NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,
/* D0*/ NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,
/* E0*/ NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,
/* F0*/ NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,
/*100*/ NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,
/*110*/ NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,
/*120*/ NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,
/*130*/ NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,  NONE,
/*140*/ STATION, STATION, STATION, HOUSE, SOUND, CARGO, SIGNAL, CANAL,
/*148*/ HOUSE, STATION, INDUSTRY, INDUSTRY, INDUSTRY, HOUSE, HOUSE, HOUSE,
/*150*/ AIRTILE, NONE, AIRTILE, AIRTILE, AIRTILE, AIRPORT, AIRPORT, OBJECT,
/*158*/ OBJECT, OBJECT, OBJECT, OBJECT, OBJECT, OBJECT, VEHICLE, INDUSTRY,
/*160*/ VEHICLE,
NDF_END
};
#undef W
#undef TRAIN
#undef ROADVEH
#undef SHIP
#undef AIRCRAFT
#undef STATION
#undef CANAL
#undef BRIDGE
#undef HOUSE
#undef GLOBAL
#undef INDTILE
#undef INDUSTRY
#undef CARGO
#undef SOUND
#undef AIRPORT
#undef SIGNAL
#undef OBJECT
#undef RAILTYPE
#undef AIRTILE
#undef MASK
#undef NONE
#undef GROUNDVEHICLE
#undef VEHICLE

/* Languages
 * ========
 *
 * Names of languages by ID (empty if not defined), each terminated by newline
 */
static const char _datlangs[]="\x00\x08"
//      x0/x4/x8/xC               x1/x5/x9/xD               x2/x6/xA/xE               x3/x7/xB/xF
/*0x*/  "American\n"              "English\n"               "German\n"                "French\n"
/*0x*/  "Spanish\n"               "Esperanto\n"             "Ido\n"                   "Russian\n"
/*0x*/  "Irish\n"                 "Maltese\n"               "Tamil\n"                 "Chuvash\n"
/*0x*/  "Chinese (Traditional)\n" "Serbian\n"               "Norwegian (Nynorsk)\n"   "Welsh\n"
//      x0/x4/x8/xC               x1/x5/x9/xD               x2/x6/xA/xE               x3/x7/xB/xF
/*1x*/  "Belarusian\n"            "Marathi\n"               "Faroese\n"               "Scottish Gaelic\n"
/*1x*/  "Arabic (Egypt)\n"        "Czech\n"                 "Slovak\n"                "\n"
/*1x*/  "Bulgarian\n"             "\n"                      "\n"                      "Afrikaans\n"
/*1x*/  "\n"                      "\n"                      "Greek\n"                 "Dutch\n"
//      x0/x4/x8/xC               x1/x5/x9/xD               x2/x6/xA/xE               x3/x7/xB/xF
/*2x*/  "\n"                      "Basque\n"                "Catalan\n"               "Luxembourgish\n"
/*2x*/  "Hungarian\n"             "\n"                      "Macedonian\n"            "Italian\n"
/*2x*/  "Romanian\n"              "Icelandic\n"             "Latvian\n"               "Lithuanian\n"
/*2x*/  "Slovenian\n"             "Danish\n"                "Swedish\n"               "Norwegian (Bokmal)\n"
//      x0/x4/x8/xC               x1/x5/x9/xD               x2/x6/xA/xE               x3/x7/xB/xF
/*3x*/  "Polish\n"                "Galician\n"              "Frisian\n"               "Ukrainian\n"
/*3x*/  "Estonian\n"              "Finnish\n"               "Portuguese\n"            "Brazilian Portuguese\n"
/*3x*/  "Croatian\n"              "Japanese\n"              "Korean\n"                "\n"
/*3x*/  "Malay\n"                 "Australian\n"            "Turkish\n"               "\n"
//      x0/x4/x8/xC               x1/x5/x9/xD               x2/x6/xA/xE               x3/x7/xB/xF
/*4x*/  "\n"                      "\n"                      "Thai\n"                  "\n"
/*4x*/  "\n"                      "\n"                      "\n"                      "\n"
/*4x*/  "\n"                      "\n"                      "\n"                      "\n"
/*4x*/  "\n"                      "\n"                      "\n"                      "\n"
//      x0/x4/x8/xC               x1/x5/x9/xD               x2/x6/xA/xE               x3/x7/xB/xF
/*5x*/  "\n"                      "\n"                      "\n"                      "\n"
/*5x*/  "Vietnamese\n"            "\n"                      "Chinese (Simplified)\n"  "\n"
/*5x*/  "\n"                      "\n"                      "Indonesian\n"            "\n"
/*5x*/  "Urdu\n"                  "\n"                      "\n"                      "\n"
//      x0/x4/x8/xC               x1/x5/x9/xD               x2/x6/xA/xE               x3/x7/xB/xF
/*6x*/  "\n"                      "Hebrew\n"                "Persian\n"               "\n"
/*6x*/  "\n"                      "\n"                      "Latin\n"                 "\n"
/*6x*/  "\n"                      "\n"                      "\n"                      "\n"
/*6x*/  "\n"                      "\n"                      "\n"                      "\n"
//      x0/x4/x8/xC               x1/x5/x9/xD               x2/x6/xA/xE               x3/x7/xB/xF
/*7x*/  "\n"                      "\n"                      "\n"                      "\n"
/*7x*/  "\n"                      "\n"                      "\n"                      "\n"
/*7x*/  "\n"                      "\n"                      "\n"                      "\n"
/*7x*/  "\n"                      "\n"                      "\n"                      "any\n"
;

/*	Version check data
	==================
*/
static const char _datversions[]="\x00\x04"
// Maximum version (beta):
"\x09"
// Revisions (starting from beta 6)
"\xD8\x01"	"\x31\x06"	"\x81\x07"	"\x70\x11"
;


/*	Features */
#define ACT0 0x01
#define ACT1 0x02
#define ACT3 0x04
#define ACT4 0x08
#define EMPTY1 0x10
#define OVERRIDE3 0x20
#define GENERIC3 0x40
#define ACT3_BEFORE_PROP08 0x80

#define ACT2_VEHICLE 0x00
#define ACT2_STATION 0x01
#define ACT2_CARGO 0x02
#define ACT2_HOUSE 0x03
#define ACT2_INDPROD 0x04
#define ACT2_NONE 0xFF
static const unsigned char _datfeat[]={
NDF_HEADER(0x12, 3),
/* Max. feature: */ 0x11,

/*00*/ ACT0 | ACT1 | ACT3 | ACT4 | EMPTY1 | OVERRIDE3 | GENERIC3 | ACT3_BEFORE_PROP08,
/*01*/ ACT0 | ACT1 | ACT3 | ACT4 | EMPTY1 |             GENERIC3 | ACT3_BEFORE_PROP08,
/*02*/ ACT0 | ACT1 | ACT3 | ACT4 | EMPTY1 |             GENERIC3 | ACT3_BEFORE_PROP08,
/*03*/ ACT0 | ACT1 | ACT3 | ACT4 | EMPTY1 | OVERRIDE3 | GENERIC3 | ACT3_BEFORE_PROP08,
/*04*/ ACT0 | ACT1 | ACT3 | ACT4 | EMPTY1 |             GENERIC3,
/*05*/ ACT0 | ACT1 | ACT3 | ACT4 |                                 ACT3_BEFORE_PROP08,
/*06*/ ACT0 |               ACT4 | EMPTY1 |             GENERIC3 | ACT3_BEFORE_PROP08,
/*07*/ ACT0 | ACT1 | ACT3 | ACT4,
/*08*/ ACT0,
/*09*/ ACT0 | ACT1 | ACT3 | ACT4,
/*0A*/ ACT0 |        ACT3 | ACT4,
/*0B*/ ACT0 | ACT1 | ACT3 | ACT4 |                                 ACT3_BEFORE_PROP08,
/*0C*/ ACT0,
/*0D*/ ACT0 | ACT1 | ACT3 | ACT4,
/*0E*/                             EMPTY1 |             GENERIC3,
/*0F*/ ACT0 | ACT1 | ACT3 | ACT4,
/*10*/ ACT0 | ACT1 | ACT3 | ACT4,
/*11*/ ACT0 | ACT1 | ACT3 | ACT4,

/*00*/ ACT2_VEHICLE,
/*01*/ ACT2_VEHICLE,
/*02*/ ACT2_VEHICLE,
/*03*/ ACT2_VEHICLE,
/*04*/ ACT2_STATION,
/*05*/ ACT2_CARGO,
/*06*/ ACT2_CARGO,
/*07*/ ACT2_HOUSE,
/*08*/ ACT2_NONE,
/*09*/ ACT2_HOUSE,
/*0A*/ ACT2_INDPROD,
/*0B*/ ACT2_CARGO,
/*0C*/ ACT2_NONE,
/*0D*/ ACT2_CARGO,
/*0E*/ ACT2_VEHICLE,
/*0F*/ ACT2_HOUSE,
/*10*/ ACT2_CARGO,
/*11*/ ACT2_HOUSE,

NDF_END
};
#undef ACT0
#undef ACT1
#undef ACT3
#undef ACT4
#undef EMPTY1
#undef OVERRIDE3
#undef GENERIC3
#undef ACT3_BEFORE_PROP08

#undef ACT2_VEHICLE
#undef ACT2_STATION
#undef ACT2_CARGO
#undef ACT2_HOUSE
#undef ACT2_INDPROD
#undef ACT2_NONE


// ---------------------------------------------------------------------------
// ------------------------- Feature-dependent data --------------------------
// ---------------------------------------------------------------------------


/*	Action 0 properties */

/* Invalid property */
#define INVALID 0xFF

/* End of current block */
#define END 00

/* Zeros need escaping to distinguish from END */
#define ZERO '\\', 0

/* Data size */
#define BYTE 0x01
#define WORD 0x02
#define EXTBYTE 0x03
#define DWORD 0x04

/* Formatting */
#define QUOTED 0x10   /* Quoted string */
#define DECIMAL 0x20  /* Escaped decimal */
#define HEX 0x30      /* Escaped hex */

#define MAGIC 0x08                 /* Various meanings */
#define DATE DECIMAL | MAGIC       /* Date, that is days since 1920 if WORD, or since 0 if DWORD. For BYTE it means years since 1920. */
#define TEXTID WORD | HEX | MAGIC  /* Check for valid Text/String ID */
#define TILEID WORD | MAGIC        /* Check for valid tile ID, e.g. industrytile in industrylayout */
#define REMAININGSIZE DWORD | HEX | MAGIC /* Value is number of bytes for the property after this length information */

/* Allow multiple assignments to a property. (Only valid outside of SUBDATA) */
#define REASSIGNABLE 0x80

/* Insert linebreak. (Only valid within SUBDATA)
 * May be specified alone, or combined with any of the BYTE/EXTBYTE/WORD/DWORD datas */
#define LINEBREAK 0xC0

/* Is replaced by the data following after the next END. Mulitple SUBDATA are resolved in a depth-first style. */
#define SUBDATA 0xFE

/* Raw byte to appear in the nfo */
#define RAW(b) 'l', b
#define RAWBYTES(len, ...) 'm', len, __VA_ARGS__

/* Test for a specific byte value
 * 'offset' is the position of the byte to test within the current SUBDATA.
 * 'mask' is a AND bitmask to apply before testing.
 * 'value' is the value to test for.
 * 'data' is the stuff to process when the test "(byte[offset] & mask) == value" succeeds. */
#define IF(lhs, rhs, data) 'e', lhs, rhs, data
#define IFNOT(lhs, rhs, data) 'n', lhs, rhs, data
#define ASSERT 'a'
#define SKIPNEXT 's'

/* Bitwise AND */
#define AND(lhs, rhs) '&', lhs, rhs

/* Repeat 'data' 'times' times, where 'times' is known before reading the data.
 * 'times' may be a fixed number, or a reference to an earlier BYTE/EXTBYTE/WORD/DWORD within the current SUBDATA.
 * You can also multiply multiple values using MUL() */
#define REPEAT_N(data, times) 'r', data, times
#define MUL(a, b) 'x', a, b
#define DATAFROM(offset) offset | 0x80           /* Uses the value read with the token at position 'offset' within the current SUBDATA */

/* Repeat 'data' until a certain terminator with length 'length' is encountered */
#define REPEAT_UNTIL(data, length, ...) '*', data, length, __VA_ARGS__

static const unsigned char _dat0[]={
NDF_HEADER(0x0E, 1),
/*Maximum feature:*/ 0x11,
// Feature 00:
/*00*/ WORD | DATE, INVALID, BYTE, BYTE | DECIMAL, BYTE | DECIMAL, BYTE, BYTE, BYTE | DECIMAL,
/*08*/ BYTE, WORD | DECIMAL, INVALID, WORD | DECIMAL, INVALID, BYTE, DWORD, INVALID,
/*10*/ INVALID, INVALID, BYTE, BYTE, BYTE | DECIMAL, BYTE, BYTE, BYTE,
/*18*/ BYTE, BYTE, EXTBYTE, WORD | DECIMAL, BYTE, DWORD | HEX, BYTE, BYTE,
/*20*/ BYTE, BYTE, BYTE, BYTE | DECIMAL, BYTE, BYTE, BYTE | DECIMAL, BYTE,
/*28*/ WORD | HEX, WORD | HEX, DWORD | DATE, WORD, SUBDATA, SUBDATA,
END,
// Subdata - prop 2C:
BYTE, REPEAT_N(BYTE, DATAFROM(0)), END,
// Subdata - prop 2D:
BYTE, REPEAT_N(BYTE, DATAFROM(0)), END,

// Feature 01:
/*00*/ WORD | DATE, INVALID, BYTE, BYTE | DECIMAL, BYTE | DECIMAL, INVALID, BYTE, BYTE | DECIMAL,
/*08*/ BYTE | DECIMAL, BYTE, DWORD, INVALID, INVALID, INVALID, BYTE, BYTE | DECIMAL,
/*10*/ BYTE, BYTE, BYTE, BYTE | DECIMAL, BYTE | DECIMAL, BYTE | DECIMAL, DWORD | HEX, BYTE,
/*18*/ BYTE, BYTE, BYTE, BYTE | DECIMAL, BYTE, WORD | HEX, WORD | HEX, DWORD | DATE,
/*20*/ EXTBYTE, BYTE, WORD, BYTE, SUBDATA, SUBDATA,
END,
// Subdata - prop 24:
BYTE, REPEAT_N(BYTE, DATAFROM(0)), END,
// Subdata - prop 25:
BYTE, REPEAT_N(BYTE, DATAFROM(0)), END,

// Feature 02:
/*00*/ WORD | DATE, INVALID, BYTE, BYTE | DECIMAL, BYTE | DECIMAL, INVALID, BYTE, BYTE | DECIMAL,
/*08*/ BYTE, BYTE, BYTE, BYTE | DECIMAL, BYTE, WORD | DECIMAL, INVALID, BYTE,
/*10*/ BYTE, DWORD | HEX, BYTE, BYTE, BYTE, BYTE, BYTE | DECIMAL, BYTE,
/*18*/ WORD | HEX, WORD | HEX, DWORD | DATE, EXTBYTE, BYTE, WORD, SUBDATA, SUBDATA,
END,
// Subdata - prop 1E:
BYTE, REPEAT_N(BYTE, DATAFROM(0)), END,
// Subdata - prop 1F:
BYTE, REPEAT_N(BYTE, DATAFROM(0)), END,

// Feature 03:
/*00*/ WORD | DATE, INVALID, BYTE, BYTE | DECIMAL, BYTE | DECIMAL, INVALID, BYTE, BYTE | DECIMAL,
/*08*/ BYTE, BYTE, BYTE, BYTE, BYTE | DECIMAL, BYTE, BYTE, WORD | DECIMAL,
/*10*/ INVALID, BYTE | DECIMAL, BYTE, DWORD | HEX, BYTE, BYTE, BYTE | DECIMAL, BYTE,
/*18*/ WORD | HEX, WORD | HEX, DWORD | DATE, EXTBYTE, WORD, SUBDATA, SUBDATA, WORD,
END,
// Subdata - prop 1D:
BYTE, REPEAT_N(BYTE, DATAFROM(0)), END,
// Subdata - prop 1E:
BYTE, REPEAT_N(BYTE, DATAFROM(0)), END,

// Feature 04:
/*00*/ INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
/*08*/ DWORD | QUOTED, SUBDATA, BYTE, BYTE, BYTE, BYTE, SUBDATA, BYTE,
/*10*/ WORD | DECIMAL, BYTE, DWORD | HEX, BYTE, BYTE, BYTE, WORD | HEX, BYTE | DECIMAL,
/*18*/ WORD, INVALID, SUBDATA,
END,
// Subdata - prop 09:
EXTBYTE, REPEAT_N(SUBDATA, DATAFROM(0)), END,
	LINEBREAK, RAW(ZERO), RAW(ZERO), RAW(ZERO), RAW(ZERO), '|', LINEBREAK, DWORD | HEX, LINEBREAK, REPEAT_UNTIL(SUBDATA, 1, 0x80), END,
		BYTE, BYTE, BYTE, BYTE, BYTE, BYTE, DWORD | HEX, LINEBREAK, END,
// Subdata - prop 0E:
REPEAT_UNTIL(SUBDATA, 2, ZERO, ZERO), END,
	BYTE, BYTE, REPEAT_N(BYTE, MUL(DATAFROM(0), DATAFROM(1))), LINEBREAK, END,
// Subdata - prop 20:
EXTBYTE, REPEAT_N(SUBDATA, DATAFROM(0)), END,
	// spritelayout without flags
	BYTE, IFNOT(AND(DATAFROM(0), RAW(0x40)), ZERO, SKIPNEXT), DWORD, LINEBREAK, REPEAT_N(SUBDATA, AND(DATAFROM(0), RAW(0x3F))),
	// spritelayout with flags
	'|', BYTE, LINEBREAK, SUBDATA, REPEAT_N(SUBDATA, AND(DATAFROM(0), RAW(0x3F))), END,
		// building sprite without flags
		DWORD, BYTE | DECIMAL, BYTE | DECIMAL, RAW(0x80), LINEBREAK,
		'|', DWORD, BYTE | DECIMAL, BYTE | DECIMAL, BYTE | DECIMAL, BYTE | DECIMAL, BYTE | DECIMAL, BYTE | DECIMAL, LINEBREAK, END,
		// groundsprite with flags
		DWORD, WORD | HEX,
		IFNOT(AND(DATAFROM(1), RAWBYTES(2, 0x30, 0xFF)), ZERO, ASSERT), // assert on unknown flags
		IFNOT(AND(DATAFROM(1), RAW(0x01)), ZERO, BYTE), // skip sprite
		IFNOT(AND(DATAFROM(1), RAW(0x02)), ZERO, BYTE), // sprite offset
		IFNOT(AND(DATAFROM(1), RAW(0x04)), ZERO, BYTE), // palette offset
		IFNOT(AND(DATAFROM(1), RAW(0x40)), ZERO, BYTE), // sprite var10
		IFNOT(AND(DATAFROM(1), RAW(0x80)), ZERO, BYTE), // palette var10
		LINEBREAK,
		END,
		// buildingsprite with flags
		DWORD, WORD | HEX, BYTE | DECIMAL, BYTE | DECIMAL, RAW(0x80),
		IFNOT(AND(DATAFROM(1), RAWBYTES(2, ZERO, 0xFF)), ZERO, ASSERT), // assert on unknown flags
		IFNOT(AND(DATAFROM(1), RAW(0x01)), ZERO, BYTE), // skip sprite
		IFNOT(AND(DATAFROM(1), RAW(0x02)), ZERO, BYTE), // sprite offset
		IFNOT(AND(DATAFROM(1), RAW(0x04)), ZERO, BYTE), // palette offset
		IFNOT(AND(DATAFROM(1), RAW(0x10)), ZERO, BYTE), // X offset
		IFNOT(AND(DATAFROM(1), RAW(0x20)), ZERO, BYTE), // Y offset
		IFNOT(AND(DATAFROM(1), RAW(0x40)), ZERO, BYTE), // sprite var10
		IFNOT(AND(DATAFROM(1), RAW(0x80)), ZERO, BYTE), // palette var10
		LINEBREAK,
		'|', DWORD, WORD | HEX, BYTE | DECIMAL, BYTE | DECIMAL, BYTE | DECIMAL, BYTE | DECIMAL, BYTE | DECIMAL, BYTE | DECIMAL,
		IFNOT(AND(DATAFROM(1), RAWBYTES(2, ZERO, 0xFF)), ZERO, ASSERT), // assert on unknown flags
		IFNOT(AND(DATAFROM(1), RAW(0x01)), ZERO, BYTE), // skip sprite
		IFNOT(AND(DATAFROM(1), RAW(0x02)), ZERO, BYTE), // sprite offset
		IFNOT(AND(DATAFROM(1), RAW(0x04)), ZERO, BYTE), // palette offset
		IFNOT(AND(DATAFROM(1), RAW(0x10)), ZERO, SUBDATA), // X/Y offset
		IFNOT(AND(DATAFROM(1), RAW(0x20)), ZERO, BYTE), // Z offset
		IFNOT(AND(DATAFROM(1), RAW(0x40)), ZERO, BYTE), // sprite var10
		IFNOT(AND(DATAFROM(1), RAW(0x80)), ZERO, BYTE), // palette var10
		LINEBREAK,
		END,
			BYTE, BYTE, END, // X/Y offset

// Feature 05:
/*00*/ INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
/*08*/ BYTE, BYTE,
END,

// Feature 06:
/*00*/ BYTE, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
/*08*/ BYTE | DATE, BYTE | DECIMAL, BYTE | DECIMAL, BYTE, WORD | DECIMAL, SUBDATA, BYTE, DWORD | DECIMAL,
/*10*/ TEXTID, TEXTID, TEXTID, WORD | DECIMAL,
END,
// Subdata - prop 0D:
BYTE, BYTE, REPEAT_N(DWORD | HEX, MUL(DATAFROM(1), 32)), LINEBREAK, END,

// Feature 07:
/*00*/ INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
/*08*/ BYTE, BYTE, SUBDATA, BYTE, BYTE, BYTE, BYTE, BYTE,
/*10*/ WORD | DECIMAL, BYTE, TEXTID, WORD, BYTE, BYTE | REASSIGNABLE, BYTE, DWORD,
/*18*/ BYTE, BYTE, BYTE, BYTE, BYTE, BYTE, DWORD, BYTE,
/*20*/ SUBDATA, WORD | DECIMAL, WORD | DECIMAL,
END,
// Subdata - prop 0A:
BYTE | DATE, BYTE | DATE, END,
// Subdata - prop 20:
BYTE, REPEAT_N(BYTE, DATAFROM(0)), END,

// Feature 08:
// Different format this feature only:
// Non-INVALID properties are followed by two bytes:
//		The max value for the action 0's <ID> entity
//		The max ID that can be set
// (0f8.dat was merged into here to prevent it from getting out of sync.)
/*00*/ INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
/*08*/ BYTE,          0x46, 0x46, DWORD | QUOTED, 0x00, 0xFC, TEXTID,         0x12, 0x12, DWORD | DECIMAL, 0x12, 0x12,
/*0C*/ WORD | QUOTED, 0x12, 0x12, DWORD | QUOTED, 0x12, 0x12, DWORD | QUOTED, 0x12, 0x12, WORD  | DECIMAL, 0x12, 0x12,
/*10*/ SUBDATA,       0x00, 0x00, SUBDATA,        0x00, 0xFF, DWORD | QUOTED, 0x00, 0xFC, SUBDATA,         0x7E, 0x7E,
/*14*/ SUBDATA,       0x7E, 0x7E, BYTE,           0x7E, 0x7E,
END,
// Subdata - prop 10:
REPEAT_N(SUBDATA, 11), SUBDATA, END,
	REPEAT_N(BYTE, 32), LINEBREAK, END,
	REPEAT_N(BYTE, 32), END,
// Subdata - prop 11:
DWORD | QUOTED, DWORD | QUOTED, END,
// Subdata - prop 13:
REPEAT_UNTIL(SUBDATA, 1, ZERO), END, BYTE, REPEAT_UNTIL(SUBDATA, 1, ZERO), LINEBREAK, END, BYTE | MAGIC | QUOTED, END,
// Subdata - prop 14:
REPEAT_UNTIL(SUBDATA, 1, ZERO), END, BYTE, REPEAT_UNTIL(SUBDATA, 1, ZERO), LINEBREAK, END, BYTE | MAGIC | QUOTED, END,

// Feature 09:
/*00*/ INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
/*08*/ BYTE, BYTE | REASSIGNABLE, WORD, WORD, WORD, BYTE, BYTE, WORD,
/*10*/ BYTE, BYTE, BYTE,
END,

// Feature 0A:
/*00*/ INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
/*08*/ BYTE, BYTE, SUBDATA, BYTE, TEXTID, TEXTID, TEXTID, BYTE,
/*10*/ WORD, DWORD, BYTE, BYTE, BYTE, SUBDATA, SUBDATA, BYTE,
/*18*/ BYTE, BYTE, DWORD, TEXTID, DWORD, DWORD, DWORD, TEXTID,
/*20*/ DWORD, BYTE, BYTE, DWORD | DECIMAL, TEXTID,
END,
// Subdata - prop 0A:
BYTE, REMAININGSIZE | LINEBREAK, REPEAT_N(SUBDATA, DATAFROM(0)), END,
	RAW(0xFE), BYTE, BYTE | LINEBREAK, '|', REPEAT_UNTIL(SUBDATA, 2, ZERO, 0x80), END,
		END, // bogus end for RAW(0xFE)
		BYTE, BYTE, SUBDATA, LINEBREAK, END,
			RAW(0xFE), TILEID, '|', BYTE, END,
				END, // bogus end for RAW(0xFE)
// Subdata - prop 15:
BYTE, REPEAT_N(BYTE, DATAFROM(0)), END,
// Subdata - prop 16:
REPEAT_N(BYTE, 3), END,

// Feature 0B:
/*00*/ INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
/*08*/ BYTE | DECIMAL, TEXTID, TEXTID, TEXTID, TEXTID, TEXTID, WORD | HEX, BYTE,
/*10*/ BYTE, BYTE, DWORD | HEX, BYTE, BYTE, BYTE, WORD | HEX, DWORD | QUOTED,
/*18*/ BYTE, WORD | HEX, BYTE, TEXTID, TEXTID, WORD,
END,

// Feature 0C:
/*00*/ INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
/*08*/ BYTE, BYTE, BYTE,
END,

// Feature 0D:
/*00*/ INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
/*08*/ BYTE, DWORD, SUBDATA, INVALID, SUBDATA, BYTE, BYTE, BYTE,
/*10*/ WORD, WORD,
END,
// Subdata - prop 0A:
BYTE, REMAININGSIZE | LINEBREAK, REPEAT_N(SUBDATA, DATAFROM(0)), END,
	BYTE, RAW(0xFE), BYTE, BYTE | LINEBREAK, '|', BYTE, REPEAT_UNTIL(SUBDATA, 2, ZERO, 0x80), END,
		END, // bogus end for RAW(0xFE)
		BYTE, BYTE, SUBDATA, LINEBREAK, END,
			RAW(0xFE), TILEID, '|', BYTE, END,
				END, // bogus end for RAW(0xFE)
// Subdata - prop 0C:
WORD, WORD, END,

// Feature 0E:
END,

// Feature 0F:
/*00*/ INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
/*08*/ DWORD | QUOTED, WORD, WORD, BYTE, BYTE, BYTE, DWORD | DATE, DWORD | DATE,
/*10*/ WORD | HEX, WORD | HEX, BYTE | DECIMAL, WORD, BYTE, WORD | HEX, BYTE, BYTE, BYTE,
END,

// Feature 10:
/*00*/ INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
/*08*/ DWORD | QUOTED, WORD, WORD, WORD, WORD, WORD, SUBDATA, SUBDATA,
/*10*/ BYTE, BYTE, BYTE, WORD, WORD, BYTE, BYTE, DWORD | DATE,
/*18*/ SUBDATA, SUBDATA, BYTE, WORD, WORD, SUBDATA,
END,
// Subdata - prop 0E:
BYTE, REPEAT_N(DWORD | QUOTED, DATAFROM(0)), END,
// Subdata - prop 0F:
BYTE, REPEAT_N(DWORD | QUOTED, DATAFROM(0)), END,
// Subdata - prop 18:
BYTE, REPEAT_N(DWORD | QUOTED, DATAFROM(0)), END,
// Subdata - prop 19:
BYTE, REPEAT_N(DWORD | QUOTED, DATAFROM(0)), END,
// Subdata - prop 1D:
BYTE, REPEAT_N(DWORD | QUOTED, DATAFROM(0)), END,

// Feature 11:
/*00*/ INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
/*08*/ BYTE, BYTE | REASSIGNABLE, INVALID, INVALID, INVALID, INVALID, BYTE, WORD,
/*10*/ BYTE, BYTE,
END,

NDF_END
};
#undef INVALID
#undef END
#undef ZERO
#undef BYTE
#undef WORD
#undef EXTBYTE
#undef DWORD
#undef QUOTED
#undef DECIMAL
#undef HEX
#undef MAGIC
#undef DATE
#undef TEXTID
#undef TILEID
#undef REMAININGSIZE
#undef REASSIGNABLE
#undef LINEBREAK
#undef SUBDATA
#undef RAW
#undef RAWBYTES
#undef IF
#undef IFNOT
#undef ASSERT
#undef SKIPNEXT
#undef AND
#undef REPEAT_N
#undef MUL
#undef DATAFROM
#undef REPEAT

/*	Variational action 2 */
#define ALLOW0MASK 0x40 /* Allow masking all bits */
#define VAR(var, size) var, 0x80 | size
#define VAR6x(var, size, max) VAR(var, size), max
#define BYTE(var) VAR(var, 1)
#define WORD(var) VAR(var, 2)
#define DWORD(var) VAR(var, 4)
#define BYTE6x(var, max) VAR6x(var, 1, max)
#define WORD6x(var, max) VAR6x(var, 2, max)
#define DWORD6x(var, max) VAR6x(var, 4, max)
#define LASTVAR80(var) (var == 0xFF ? 0xFF : (unsigned char)(var + 1)), (var == 0xFF ? 0xF0 : 0x80)
#define NOVAR80 LASTVAR80(0x7F)
static const unsigned char _dat2v[]={
NDF_HEADER(0x0D, 30),
/*Maximum feature:*/ 0x11,
/*Maximum operator:*/ 0x16,
// Global variables:
WORD(0x00), BYTE(0x01), BYTE(0x02), BYTE(0x03), BYTE(0x06), WORD(0x09), WORD(0x0A), DWORD(0x0B), WORD(0x0C), BYTE(0x0D), BYTE(0x0E), VAR(0x0F, 3),
DWORD(0x10), /* var 11 in feature 04 */ BYTE(0x12), WORD(0x13), WORD(0x14), WORD(0x15), WORD(0x16), DWORD(0x18), VAR(0x1A, 4 | ALLOW0MASK), BYTE(0x1B), DWORD(0x1C), BYTE(0x1D), DWORD(0x1E),
BYTE(0x20), DWORD(0x21), DWORD(0x22), DWORD(0x23), DWORD(0x24),
DWORD6x(0x7D, 0xFF), VAR6x(0x7E, 2 | ALLOW0MASK, 0xFF), DWORD6x(0x7F, 0x7F),
// Things like 25, 5F and 7C as feature specific, so there are put with features
LASTVAR80(0xFF),
// Feature 00:
/*Related:*/ 0x00,
DWORD(0x25),
VAR(0x40, 3), VAR(0x41, 3), DWORD(0x42), DWORD(0x43), VAR(0x45, 3), DWORD(0x46), DWORD(0x47),
BYTE(0x48), DWORD(0x49), DWORD(0x4A), DWORD(0x4B), DWORD(0x4C), WORD(0x4D),
WORD(0x5F),
BYTE6x(0x60, 0x73), DWORD6x(0x61, 0xFF), DWORD6x(0x62, 0xFF),
LASTVAR80(0xFF),
// Feature 01:
/*Related:*/ 0x01,
VAR(0x40, 3), VAR(0x41, 3), VAR(0x42, 3), DWORD(0x43), VAR(0x45, 3), DWORD(0x46), DWORD(0x47),
BYTE(0x48), DWORD(0x49), DWORD(0x4B), DWORD(0x4C), WORD(0x4D),
WORD(0x5F),
DWORD6x(0x61, 0xFF), DWORD6x(0x62, 0xFF),
LASTVAR80(0xFF),
// Feature 02:
/*Related:*/ 0x02,
BYTE(0x42), DWORD(0x43), DWORD(0x46), DWORD(0x47), BYTE(0x48), DWORD(0x49), DWORD(0x4B), DWORD(0x4C),
WORD(0x5F),
LASTVAR80(0xFF),
// Feature 03:
/*Related:*/ 0x03,
VAR(0x40, 3), BYTE(0x42), DWORD(0x43), WORD(0x44), DWORD(0x46), DWORD(0x47), BYTE(0x48), DWORD(0x49),
DWORD(0x4B), DWORD(0x4C),
WORD(0x5F),
LASTVAR80(0xFF),
// Feature 04:
/*Related:*/ 0x08,
BYTE(0x11),
DWORD(0x40), DWORD(0x41), WORD(0x42), VAR(0x43, 3), BYTE(0x44), WORD(0x45), DWORD(0x46), DWORD(0x47),
DWORD(0x48), DWORD(0x49), BYTE(0x4A),
DWORD(0x5F),
WORD6x(0x60, 0xFF), BYTE6x(0x61, 0xFF), DWORD6x(0x62, 0xFF), BYTE6x(0x63, 0xFF), WORD6x(0x64, 0xFF),
BYTE6x(0x65, 0xFF), DWORD6x(0x66, 0xFF), DWORD6x(0x67, 0xFF), WORD6x(0x68, 0xFF), BYTE6x(0x69, 0xFF),
LASTVAR80(0xFF),
// Feature 05:
/*Related:*/ 0x05,
WORD(0x5F),
LASTVAR80(0x83),
// Feature 06:
/*Related:*/ 0x08,
BYTE(0x40), BYTE(0x41),
NOVAR80,
// Feature 07:
/*Related:*/ 0x08,
BYTE(0x40), BYTE(0x41), BYTE(0x42), BYTE(0x43), DWORD(0x44), BYTE(0x45), BYTE(0x46), WORD(0x47),
WORD(0x5F),
WORD6x(0x60, 0x6B), WORD6x(0x61, 0xFF), DWORD6x(0x62,0xFF), BYTE6x(0x63, 0xFF),
BYTE6x(0x64, 0xFF), BYTE6x(0x65, 0xFF), DWORD6x(0x66, 0xFF), DWORD6x(0x67, 0xFF),
NOVAR80,
// Town variables ("Feature 08" *cough*cough*)
/*Related:*/ 0x08,
BYTE(0x40), BYTE(0x41),
LASTVAR80(0xDD),
// Feature 09:
/*Related:*/ 0x0A,
BYTE(0x40), BYTE(0x41), BYTE(0x42), VAR(0x43, 3), BYTE(0x44),
WORD(0x5F),
DWORD6x(0x60, 0xFF), DWORD6x(0x61, 0xFF), WORD6x(0x62, 0xFF),
NOVAR80,
// Feature 0A:
/*Related:*/ 0x08,
WORD(0x40), WORD(0x41), WORD(0x42), WORD(0x43), BYTE(0x44), DWORD(0x45), DWORD(0x46),
VAR(0x5F, 3),
WORD6x(0x60, 0xFF), BYTE6x(0x61, 0xFF), DWORD6x(0x62, 0xFF), DWORD6x(0x63, 0xFF),
DWORD6x(0x64, 0xFF), VAR6x(0x65, 3, 0xFF), VAR6x(0x66, 3, 0xFF), DWORD6x(0x67, 0xFF),
DWORD6x(0x68, 0xFF),
DWORD6x(0x7C, 0x0F),
LASTVAR80(0xB5),
// Feature 0B:
/*Related:*/ 0x0B,
NOVAR80,
// Feature 0C:
/*Related:*/ 0x0C,
NOVAR80,
// Feature 0D:
/*Related:*/ 0x0D,
NOVAR80,
// Feature 0E:
/*Related:*/ 0x0E,
DWORD6x(0x60, 0xFF),
NOVAR80,
// Feature 0F:
/*Related:*/ 0x08,
VAR(0x40, 3), WORD(0x41), DWORD(0x42), BYTE(0x43), BYTE(0x44), DWORD(0x45), DWORD(0x46), BYTE(0x47), BYTE(0x48),
WORD(0x5F),
DWORD6x(0x60, 0xFF), BYTE6x(0x61, 0xFF), DWORD6x(0x62, 0xFF), WORD6x(0x63, 0xFF),
DWORD6x(0x64, 0xFF),
NOVAR80,
// Feature 10:
/*Related:*/ 0x10,
BYTE(0x40), BYTE(0x41), BYTE(0x42), DWORD(0x43), BYTE(0x44),
WORD(0x5F),
NOVAR80,
// Feature 11:
/*Related:*/ 0x0D,
BYTE(0x41), BYTE(0x42), VAR(0x43, 3), BYTE(0x44),
DWORD(0x5F),
DWORD6x(0x60, 0xFF), DWORD6x(0x61, 0xFF), WORD6x(0x62, 0xFF),
NOVAR80,
NDF_END
};
#undef ALLOW0MASK
#undef VAR
#undef VAR6x
#undef BYTE
#undef WORD
#undef DWORD
#undef BYTE6x
#undef WORD6x
#undef DWORD6x
#undef LASTVAR80
#undef NOVAR80

#define W(cnt) cnt & 0xFF, cnt >> 8  /* Construct word count */
static const unsigned char _datD[]={
NDF_HEADER(0x14, 7),
/*Maximum feature:*/ 0x11,
/*Max patch variable:*/ 0x16,
/*Max operator:*/ 0x0C,
// GRM count:
/*00*/W(0x74),
/*01*/W(0x58),
/*02*/W(0x0B),
/*03*/W(0x29),
/*04*/W(0),
/*05*/W(0),
/*06*/W(0),
/*07*/W(0),
/*08*/W(0x131E),
/*09*/W(0),
/*0A*/W(0),
/*0B*/W(0x40),
/*0C*/W(0),
/*0D*/W(0),
/*0E*/W(0),
/*0F*/W(0),
/*10*/W(0),
/*11*/W(0),
NDF_END
};

static const unsigned char _datIDs[]={
NDF_HEADER(0x77, 4),
/*Maximum feature:*/ 0x11,
/*00*/W(0x73),
/*01*/W(0x57),
/*02*/W(0x0A),
/*03*/W(0x28),
/*04*/W(0xFF),
/*05*/W(0x08),
/*06*/W(0x0C),
/*07*/W(0xFF),
/*08*/W(0x00),
/*09*/W(0xFF),
/*0A*/W(0x24),
/*0B*/W(0x1F),
/*0C*/W(0xFFFF),
/*0D*/W(0x00),
/*0E*/W(0x00),
/*0F*/W(0xFF),
/*10*/W(0xFF),
/*11*/W(0xFF),
NDF_END
};
#undef W

/*	Action 4 strings */
static const unsigned char _dat4[]={
NDF_HEADER(0x03, 5),
/*Maximum feature:*/ 0x11,
// Rules for one-byte IDs:
/*00*/CTRL_SPACE,
/*01*/CTRL_SPACE,
/*02*/CTRL_SPACE,
/*03*/CTRL_SPACE,
/*04*/0,
/*05*/0,
/*06*/0,
/*07*/0,
/*08*/0,
/*09*/0,
/*0A*/0,
/*0B*/0,
/*0C*/0,
/*0D*/0,
/*0E*/0,
/*0F*/0,
/*10*/0,
/*11*/                            CTRL_COLOR,
// Rules for two-byte IDs:
/*00*/CTRL_SPACE | CTRL_NEWLINE | CTRL_COLOR,
/*01*/CTRL_SPACE | CTRL_NEWLINE | CTRL_COLOR,
/*02*/CTRL_SPACE | CTRL_NEWLINE | CTRL_COLOR,
/*03*/CTRL_SPACE | CTRL_NEWLINE | CTRL_COLOR,
/*04*/CTRL_SPACE,
/*05*/CTRL_SPACE,
/*06*/0,
/*07*/CTRL_SPACE,
/*08*/0,
/*09*/CTRL_SPACE | CTRL_NEWLINE | CTRL_COLOR,
/*0A*/CTRL_SPACE | CTRL_NEWLINE | CTRL_COLOR | CTRL_FONT_LARGE |                   CTRL_NO_STACK_CHECK,
/*0B*/CTRL_SPACE |                                               CTRL_FONT_SMALL,
/*0C*/0,
/*0D*/0,
/*0E*/0,
/*0F*/0,
/*10*/CTRL_SPACE,
/*11*/CTRL_SPACE | CTRL_NEWLINE | CTRL_COLOR,
NDF_END
};


/*	Randomized action 2 */
#define ACT2RANDOM(self_bits, related_bits, num_triggers) self_bits, related_bits, num_triggers
static const unsigned char _dat2r[]={
NDF_HEADER(0x02, 6),
/*Maximum feature:*/ 0x11,
/*00*/ACT2RANDOM( 8,  8, 5),
/*01*/ACT2RANDOM( 8,  8, 5),
/*02*/ACT2RANDOM( 8,  8, 5),
/*03*/ACT2RANDOM( 8,  8, 5),
/*04*/ACT2RANDOM(20,  0, 6),
/*05*/ACT2RANDOM( 8,  0, 0),
/*06*/ACT2RANDOM( 0,  0, 0),
/*07*/ACT2RANDOM( 8,  0, 2),
/*08*/ACT2RANDOM( 0,  0, 0),
/*09*/ACT2RANDOM( 8, 16, 3),
/*0A*/ACT2RANDOM(16,  0, 0),
/*0B*/ACT2RANDOM( 0,  0, 0),
/*0C*/ACT2RANDOM( 0,  0, 0),
/*0D*/ACT2RANDOM( 0,  0, 0),
/*0E*/ACT2RANDOM( 0,  0, 0),
/*0F*/ACT2RANDOM( 8,  0, 0),
/*10*/ACT2RANDOM( 2,  0, 0),
/*11*/ACT2RANDOM( 4, 16, 0),
NDF_END
};
#undef ACT2RANDOM

// ---------------------------------------------------------------------------


struct dat{
	const char*data,*name;
	uint len;
};

#undef _RENUM_DATA_H_INCLUDED_
#undef DATA
#undef DATA_FILE
#undef END_DATA
#define DATA() static const dat data[]={
#define DATA_FILE(name)\
	{(char*)_dat##name,"/.nforenum/" #name ".dat",sizeof(_dat##name)-1},\

#define END_DATA() };
#define NFORENUM_DIR_LEN (11)

#include "data.h"

bool makedir(const string&dir,bool dieonfail=false){
	if(dir==""){
		if(dieonfail)exit(EDATA);
		return false;
	}
	if(mkdir((dir+"/.nforenum").c_str(),0755)){
		IssueMessage(0,CREATE_FAILED,dir.c_str(),errno);
		perror(NULL);
		if(dosleep)sleep(5);
		if(dieonfail)exit(EDATA);
		return false;
	}else{
		IssueMessage(0,CREATED_DIR,dir.c_str());
		if(dosleep)sleep(5);
		return true;
	}
}

bool finddir(string&dir){
	if(dir=="")return false;
	struct stat Stat;
	if(dir[dir.length()-1]=='\\'||dir[dir.length()-1]=='/')
		dir.resize(dir.length()-1);
	if(stat((dir+"/.nforenum").c_str(),&Stat))return false;
	else if(Stat.st_mode&S_IFREG)return false;
	return true;
}

static string getdir(bool allow_mkdir){
	string *pret;
	string cwd,home,homedrpath;
	if(datadir!=""){
		verify(finddir(datadir)||makedir(datadir,true));
		pret=&datadir;
	}else{
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
// getcwd is deprecated in MSVS 8.0+
		char*pcwd=_getcwd(NULL,0);
#else
		char*pcwd=getcwd(NULL,0);
#endif
		cwd=pcwd;
		home=safetostring(getenv("HOME"));
		homedrpath=safetostring(getenv("HOMEDRIVE"))+safetostring(getenv("HOMEPATH"));
		free(pcwd);
		if (finddir(cwd)) {
			pret=&cwd;
		} else if (finddir(home) || (!home.empty() && (!allow_mkdir || makedir(home)))) {
			pret=&home;
		} else if (finddir(homedrpath) || (!allow_mkdir || makedir(homedrpath))) {
			pret=&homedrpath;
		} else{
			if (allow_mkdir) verify(makedir(cwd, true));
			pret=&cwd;
		}
	}
	if(!dosleep)IssueMessage(0,DATA_FOUND_AT,pret->c_str());
	return *pret;
}

FILE*tryopen(const char*name,const char*mode,bool allownull=false){
	string dir = getdir(mode[0] == 'w');
	FILE*pFile=fopen((dir+name).c_str(),mode);
	if(pFile||allownull)return pFile;
	IssueMessage(0,DATAFILE_ERROR,OPEN,name+1,ERRNO,errno);
	perror(NULL);
	assert(false);
	exit(EDATA);
}

FILE*_myfopen(files file, bool write){
	FILE*pFile=tryopen(data[file].name,"rb",true);
	if(pFile){
		if(fgetc(pFile)==data[file].data[0]&&fgetc(pFile)>=data[file].data[1]){
			if(file>datfeat && (uint)fgetc(pFile)<MaxFeature()){
				IssueMessage(0,DATAFILE_MISMATCH,data[file].name+NFORENUM_DIR_LEN);
				assert(false);
				exit(EDATA);
			}
			return pFile;
		}
		fclose(pFile);
	}
#if WITH_FMEMOPEN
	if (!write) {
		pFile = fmemopen(const_cast<char *>(data[file].data), data[file].len, "rb");
		if (pFile == NULL) {
			IssueMessage(0, DATAFILE_ERROR, OPEN, data[file].name + 1, ERRNO, errno);
			perror(NULL);
			assert(false);
			exit(EDATA);
		}
	} else
#endif /* WITH_FMEMOPEN */
	{
		pFile = tryopen(data[file].name,"wb");
		if (fwrite(data[file].data, 1, data[file].len, pFile) != data[file].len) {
			IssueMessage(0, DATAFILE_ERROR, WRITE, data[file].name + 1, -1);
			assert(false);
			exit(EDATA);
		}
		fclose(pFile);
		pFile = tryopen(data[file].name,"rb");
	}
	fgetc(pFile);
	fgetc(pFile);
	if(file>datfeat && (uint)fgetc(pFile)<MaxFeature()){
		IssueMessage(0,DATAFILE_MISMATCH,data[file].name+NFORENUM_DIR_LEN);
		assert(false);
		exit(EDATA);
	}
	return pFile;
}

int _CheckEOF(int dat,files file,const char*src,int line){
	if(dat==EOF){
		IssueMessage(0,DATAFILE_ERROR,LOAD,data[file].name+NFORENUM_DIR_LEN,FILELINE,src,line);
		assert(false);
		exit(EDATA);
	}
	return dat;
}

int _GetCheckWord(FILE*pFile,files file,const char*src,int line){
	int ret=fgetc(pFile);
	return ret|(_CheckEOF(fgetc(pFile),file,src,line)<<8);
}

void _myfread(FILE*pFile,uchar*target,uint count,files file,const char*src,int line){
	if(fread(target,1,count,pFile)!=count){
		IssueMessage(0,DATAFILE_ERROR,LOAD,data[file].name+NFORENUM_DIR_LEN,FILELINE,src,line);
		assert(false);
		exit(EDATA);
	}
}
