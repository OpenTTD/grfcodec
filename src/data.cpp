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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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

// ---------------------------------------------------------------------------
// ------------------------- Feature-independent data ------------------------
// ---------------------------------------------------------------------------

/*	Action 7/9/D variables
	====================

	Format: Bitmask
	Bit(s)			Meaning
	0,1,2			valid width(s) (B,W,D) (If all are clear, this is a bitmask-variable.)
	5 (20h)			write in D
	6 (40h)			read in D
	7 (80h)			read in 7

*/
static const char _dat79Dv[]="\x20\x05"
// Number of variables:
"\x25"
//       x0              x4              x8              xC
/*8x*/"\x00\xC1\x00\xC1\xC5\x80\x80\x00\x84\x00\x00\xC6\x00\xC1\xE1\xE4"
/*9x*/"\x00\x00\xC1\xE6\xE6\xE6\xE6\xE1\x00\x24\x87\x00\x00\xC4\xE4\x24"
/*Ax*/"\x00\xC4\xC4\xC4\xC4"
;


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
	========

   vv Flags -- 01: recolor, 02: mixed, 04: word, 08: allow 80+x, 80: is flag byte
       vv Info count: TN: N sprite-count options for each of T types follow
           vv... Length(s): repeat N options for each of T types
*/
static const char _dat5[]="\x04\x08"
	"\x13\x30\x70\xF0"			//4
	"\x11\x30"					//5
	"\x12\x4A\x5A"				//6
	"\x31\x5D\x41\x06"			//7..9
"\x85\x11\x00\x01"				//A
	"\x21\x71\x85"				//B..C
	"\x12\x10\x12"				//D
"\x82\x11\x00"					//E
	"\x51\x0C\x0F\x08\x08\x37"	//F..13
"\x88\x31\x24\x9A\x09"				//14..15
"\x00"
;


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

/*	Callbacks
	=========

	Count of CBs, then list of:
	1) feature for callback
	2) one-byte bitmask of features for callback | 80h
	3) 7Fh followed by word-sized bitmask
*/
static const char _datcallbacks[]="\x05\x11"
// Count:
"\x57\x01"
//v 00             x4              x8              xC
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x83\x8F\x04\x04\x8F\x83\x07\x9F\x8F\x07\x07\x07\x00\x07\x07"
//v 20             x4              x8              xC
"\x07\x07\x0A\x8F\x04\x09\x09\x09\x0A\x0A\x07\x09\x09\x8F\x07\x09"
"\x09\x8F\x8F\xCF\x8F\x0A\x8F\x0A\x0A\x0B\x0A\x0A\x09\x0A\x80\x80"
//v 40             x4              x8              xC
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
//v 60             x4              x8              xC
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
//v 80             x4              x8              xC
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
//v A0             x4              x8              xC
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
//v C0             x4              x8              xC
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
//v E0             x4              x8              xC
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
//v 100            x4              x8              xC
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
//v 120            x4              x8              xC
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
//v 140            x4              x8              xC
"\x04\x04\x04\x07\x0C\x0B\x0E\x05\x07\x7F\x10\x80"
                                        "\x0A\x0A\x0A\x07\x07\x07"
"\x11\x80\x11\x11\x11\x0D\x0D"
;


/*	Languages
	=========

	Names of languages by ID (empty if not defined), each terminated by newline
*/
static const char _datlangs[]="\x00\x04"
//		x0/x4/x8/xC		x1/x5/x9/xD		x2/x6/xA/xE		x3/x7/xB/xF
/*0x*/	"American\n"	"English\n"		"German\n"		"French\n"
/*0x*/	"Spanish\n"		"Esperanto\n"	"Ido\n"			"Russian\n"
/*0x*/	"Irish\n"		"Maltese\n"		"Tamil\n"		"Chuvash\n"
/*0x*/	"Chinese (Trad)\n" "Serbian\n" "Norwegian (Nynorsk)\n" "Welsh\n"
//		x0/x4/x8/xC		x1/x5/x9/xD		x2/x6/xA/xE		x3/x7/xB/xF
/*1x*/	"Belarusian\n"	"Marathi\n"		"\n"			"\n"
/*1x*/	"Arabic (Egypt)\n" "Czech\n"	"Slovak\n"		"\n"
/*1x*/	"Bulgarian\n"	"\n"			"\n"			"Afrikaans\n"
/*1x*/	"\n"			"\n"			"Greek\n"		"Dutch\n"
//		x0/x4/x8/xC		x1/x5/x9/xD		x2/x6/xA/xE		x3/x7/xB/xF
/*2x*/	"\n"			"Basque\n"		"Catalan\n"		"Luxembourgish\n"
/*2x*/	"Hungarian\n"	"\n"			"Macedonian\n"	"Italian\n"
/*2x*/	"Romanian\n"	"Icelandic\n"	"Latvian\n"		"Lithuanian\n"
/*2x*/	"Slovenian\n"	"Danish\n"		"Swedish\n"		"Norwegian (Bokmal)\n"
//		x0/x4/x8/xC		x1/x5/x9/xD		x2/x6/xA/xE		x3/x7/xB/xF
/*3x*/	"Polish\n"		"Galician\n"	"Frisian\n"		"Ukrainian\n"
/*3x*/	"Estonian\n"	"Finnish\n"		"Portuguese\n"	"Brazilian Portuguese\n"
/*3x*/	"Croatian\n"	"Japanese\n"	"Korean\n"		"\n"
/*3x*/	"Malay\n"		"\n"			"Turkish\n"		"\n"
//		x0/x4/x8/xC		x1/x5/x9/xD		x2/x6/xA/xE		x3/x7/xB/xF
/*4x*/	"\n"			"\n"			"Thai\n"		"\n"
/*4x*/	"\n"			"\n"			"\n"			"\n"
/*4x*/	"\n"			"\n"			"\n"			"\n"
/*4x*/	"\n"			"\n"			"\n"			"\n"
//		x0/x4/x8/xC		x1/x5/x9/xD		x2/x6/xA/xE		x3/x7/xB/xF
/*5x*/	"\n"			"\n"			"\n"			"\n"
/*5x*/	"Vietnamese\n"	"\n"			"Chinese (Simp)\n" "\n"
/*5x*/	"\n"			"\n"			"Indonesian\n"	"\n"
/*5x*/	"Urdu\n"		"\n"			"\n"			"\n"
//		x0/x4/x8/xC		x1/x5/x9/xD		x2/x6/xA/xE		x3/x7/xB/xF
/*6x*/	"\n"			"Hebrew\n"		"Persian\n"		"\n"
/*6x*/	"\n"			"\n"			"\n"			"\n"
/*6x*/	"\n"			"\n"			"\n"			"\n"
/*6x*/	"\n"			"\n"			"\n"			"\n"
//		x0/x4/x8/xC		x1/x5/x9/xD		x2/x6/xA/xE		x3/x7/xB/xF
/*7x*/	"\n"			"\n"			"\n"			"\n"
/*7x*/	"\n"			"\n"			"\n"			"\n"
/*7x*/	"\n"			"\n"			"\n"			"\n"
/*7x*/	"\n"			"\n"			"\n"			"any\n"
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


/*	Features
	========

	Feature bit-mask:
	OR of the appropriate bits from enum ActBit:
		0x01 = ACT0
		0x02 = ACT1
		0x04 = ACT3
		0x08 = ACT4
		0x10 = EMPTY1,
		0x20 = OVERRIDE3
		0x40 = GENERIC3
		0x80 = ACT3_BEFORE_PROP08

	Std action 2 format:
		0x00 = Vehicle style
		0x01 = Station style	(<num-ent1> may be 0)
		0x02 = Cargo style		(<num-ent1> must be 1, <num-ent2> must be 0)
		0x03 = House style
		0x04 = Ind. prod style
		0xFF = No std 2 for this feature
*/
static const char _datfeat[]="\x12\x03"
// Max. feature:
"\x11"
// Feature bit-masks:
// 00              04              08              0C
"\xFF\xDF\xDF\xFF\x5F\x8F\xD9\x0F\x01\x0F\x0D\x8F\x01\x0F\x50\x0F\x0F\x0F"
// Std action 2 formats:
// 00              04              08              0C
"\x00\x00\x00\x00\x01\x02\x02\x03\xFF\x03\x04\x02\xFF\x02\x00\x03\x02\x03"
;


// ---------------------------------------------------------------------------
// ------------------------- Feature-dependent data --------------------------
// ---------------------------------------------------------------------------


/*	Action 0 properties
	===================

	Property definitions:
	*	Lower nibble = data type
		x1 - BYTE, x2 - WORD, x3 - EXT.BYTE, x4 - DWORD
	*	Third quad = formatting
		0x - default, 1x - quote, 2x - decimal, 3x - B-E hex
	*	Special values
		FE - variable length (details in subdata)
		FF - property does not exist
		00 - list terminator

	See full description in act0.cpp.
*/
static const char _dat0[]="\x0D\x06\x11"
// Feature 00:
// x0              x4              x8              xC
"\x2A\xFF\x01\x21\x21\x01\x01\x21\x01\x22\xFF\x22\xFF\x01\x04\xFF"
"\xFF\xFF\x01\x01\x21\x01\x01\x01\x01\x01\x03\x22\x01\x34\x01\x01"
"\x01\x01\x01\x21\x01\x01\x21\x01\x32\x32\x2C"
"\x00"

// Feature 01:
// x0              x4              x8              xC
"\x2A\xFF\x01\x21\x21\xFF\x01\x21\x21\x01\x04\xFF\xFF\xFF\x01\x21"
"\x01\x01\x01\x21\x21\x21\x34\x01\x01\x01\x01\x21\x01\x32\x32\x2C"
"\x03"
"\x00"

// Feature 02:
// x0              x4              x8              xC
"\x2A\xFF\x01\x21\x21\xFF\x01\x21\x01\x01\x01\x21\x01\x22\xFF\x01"
"\x01\x34\x01\x01\x01\x01\x21\x01\x32\x32\x2C\x03"
"\x00"

// Feature 03:
// x0              x4              x8              xC
"\x2A\xFF\x01\x21\x21\xFF\x01\x21\x01\x01\x01\x01\x21\x01\x01\x22"
"\xFF\x21\x01\x34\x01\x01\x21\x01\x32\x32\x2C\x03"
"\x00"

// Feature 04:
// x0              x4              x8              xC
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x14\xFE\x01\x01\x01\x01\xFE\x01"
"\x22\x01\x34\x01\x01\x01\x32\x21\x02"
"\x00"
// Subdata - prop 09:
"\x03r\xFE\x80\x00"
	"l\\\x00l\\\x00l\\\x00l\\\x00|\x34*\xFE\x01\x80\x00"
		"\x01\x01\x01\x01\x01\x01\x34\x00"
// Subdata - prop 0E:
"*\xFE\x02\\\x00\\\x00\x00"
	"\x01\x01r\x01x\x80\x81\xC0\x00"

// Feature 05:
// x0              x4              x8              xC
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01\x01"
"\x00"

// Feature 06:
// x0              x4              x8              xC
"\x01\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x29\x21\x21\x01\x22\xFE\x01\x24"
"\x3A\x3A\x3A\x22"
"\x00"
// Subdata - prop 0D:
"\x01\x01r\x34x\x81\x20\xC0\x00"

// Feature 07:
// x0              x4              x8              xC
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01\x01\xFE\x01\x01\x01\x01\x01"
"\x22\x01\x3A\x02\x01\x81\x01\x04\x01\x01\x01\x01\x01\x01\x04\x01"
"\xFE\x22\x22"
"\x00"
// Subdata - prop 0A:
"\x29\x29\x00"
// Subdata - prop 20:
"\x01r\x01\x80\x00"

// Feature 08:
// Different format this feature only
// For non-FF properties only, each prop is followed by two bytes:
//		The max value for the action 0's <ID> entity
//		The max ID that can be set
// (0f8.dat was merged into here to prevent it from getting out of sync.)

// 00              04              --- 08 ---              --- 0A ---              --- 0C ---              --- 0E ---
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01\x41\x41\x14\x00\xFC\x3A\x12\x12\x24\x12\x12\x12\x12\x12\x14\x12\x12\x14\x12\x12\x22\x12\x12"
// --- 10 ---
"\xFE\x00\x00\xFE\x00\xFF\x14\x00\xFC"
"\x00"
// Subdata - prop 10:
"r\xFE\x0B\xFE\x00"
	"r\x01\x20\xC0\x00"
	"r\x01\x20\x00"
// Subdata - prop 11:
"\x14\x14\x00"

// Feature 09:
// x0              x4              x8              xC
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01\x81\x02\x02\x02\x01\x01\x02"
"\x01\x01\x01"
"\x00"

// Feature 0A:
// x0              x4              x8              xC
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01\x01\xFE\x01\x3A\x3A\x3A\x01"
"\x02\x04\x01\x01\x01\xFE\xFE\x01\x01\x01\x04\x3A\x04\x04\x04\x3A"
"\x04\x01\x01\x24\x3A"
"\x00"
// Subdata - prop 0A:
"\x01\xFCr\xFE\x80\x00"
	"l\xFE\x01\xC1|*\xFE\x02\\\x00\x80\x00"
		"\x00"
		"\x01\x01\xFE\xC0\x00"
			"l\xFE\x0A|\x01\x00"
				"\x00"
// Subdata - prop 15:
"\x01r\x01\x80\x00"
// Subdata - prop 16:
"r\x01\x03\x00"

// Feature 0B:
// x0              x4              x8              xC
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x21\x3A\x3A\x3A\x3A\x3A\x32\x01"
"\x01\x01\x34\x01\x01\x01\x32\x14\x01\x32\x01"
"\x00"

// Feature 0C:
// x0              x4              x8              xC
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01\x01\x01"
"\x00"

// Feature 0D:
// x0              x4              x8              xC
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01\x04\xFE\xFF\xFE\x01\x01\x01"
"\x02"
"\x00"
// Subdata - prop 0A:
"\x01\xFCr\xFE\x80\x00"
	"\x01l\xFE\x01\xC1|\x01*\xFE\x02\\\x00\x80\x00"
		"\x00"
		"\x01\x01\xFE\xC0\x00"
			"l\xFE\x0A|\x01\x00"
				"\x00"
// Subdata - prop 0C:
"\x02\x02\x00"

// Feature 0E:
// x0              x4              x8              xC
"\x00"

// Feature 0F:
// x0              x4              x8              xC
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x14\x02\x02\x01\x01\x01\x2C\x2C"
"\x32"
"\x00"

// Feature 10:
// x0              x4              x8              xC
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x14\x02\x02\x02\x02\x02\xFE\xFE"
"\x01\x01\x01\x02\x02\x01\x01"
"\x00"
// Subdata - prop 0E:
"\x01r\x14\x80\x00"
// Subdata - prop 0F:
"\x01r\x14\x80\x00"

// Feature 11:
// x0              x4              x8              xC
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01\x81\xFF\xFF\xFF\xFF\x01\x02"
"\x01\x01"
"\x00"
;

/*	Variational action 2
	====================
*/
static const char _dat2v[]="\x0D\x12\x11"
// Maximum operator ID for advanced VA2:
"\x16"
// Global variables:
"\x00\x82"		"\x01\x81"		"\x02\x81"		"\x03\x81"
"\x09\x82"		"\x0A\x82"		"\x0C\x82"		"\x10\x84"
"\x12\x81"		"\x18\x84"		"\x1A\xC4"		"\x1B\x81"
"\x1C\x84"		"\x1D\x81"		"\x20\x81"		"\x23\x84"
"\x24\x84"		"\x7D\x84\xFF"	"\x7E\xC2\xFF"	"\x7F\x84\x7F"
"\xFF\xF0"
// Feature 00:
"\x00"
"\x40\x83"		"\x41\x83"		"\x42\x84"		"\x43\x84"
"\x45\x83"		"\x46\x84"		"\x47\x84"		"\x48\x81"
"\x49\x84"		"\x4A\x84"		"\x5F\x81"		"\x60\x81\x73"
"\xFF\xF0"
// Feature 01:
"\x01"
"\x40\x83"		"\x41\x83"		"\x42\x83"		"\x43\x84"
"\x45\x83"		"\x46\x84"		"\x47\x84"		"\x48\x81"
"\x49\x84"		"\x5F\x81"
"\xFF\xF0"
// Feature 02:
"\x02"
"\x42\x81"		"\x43\x84"		"\x46\x84"		"\x47\x84"
"\x48\x81"		"\x49\x84"		"\x5F\x81"
"\xFF\xF0"
// Feature 03:
"\x03"
"\x40\x83"		"\x42\x81"		"\x43\x84"		"\x46\x84"
"\x48\x81"		"\x44\x82"		"\x47\x84"		"\x49\x84"
"\x5F\x81"
"\xFF\xF0"
// Feature 04:
"\x08"
"\x11\x81"		"\x40\x84"		"\x41\x84"		"\x42\x82"
"\x43\x83"		"\x44\x81"		"\x45\x82"		"\x46\x84"
"\x47\x84"		"\x48\x85"		"\x49\x84"		"\x4A\x81"
"\x5F\x83"		"\x60\x82\xFF"	"\x61\x81\xFF"	"\x62\x84\xFF"
"\x63\x81\xFF"	"\x64\x82\xFF"	"\x65\x81\xFF"	"\x66\x84\xFF"
"\x67\x82\xFF"	"\x68\x82\xFF"	"\x69\x81\xFF"
"\xFF\xF0"
// Feature 05:
"\x05"
"\x84\x80"
// Feature 06:
"\x00"
"\x40\x83"		"\x41\x83"		"\x42\x84"		"\x43\x84"
"\x45\x83"		"\x46\x84"		"\x47\x84"		"\x48\x81"
"\x5F\x81"		"\x60\x81\x73"
"\xFF\xF0"
// Feature 07:
"\x08"
"\x40\x81"		"\x41\x81"		"\x42\x81"		"\x43\x81"
"\x44\x84"		"\x45\x81"		"\x46\x81"		"\x47\x82"
"\x5F\x81"
"\x60\x82\x6B"	"\x61\x82\xFF"	"\x62\x84\xFF"	"\x63\x81\xFF"
"\x64\x81\xFF"	"\x65\x81\xFF"  "\x66\x84\xFF"  "\x67\x84\xFF"
"\x80\x80"
// Town variables ("Feature 08" *cough*cough*)
"\x08"
"\x40\x81"		"\x41\x81"
"\xDE\x80"
// Feature 09:
"\x0A"
"\x40\x81"		"\x41\x81"		"\x42\x81"		"\x43\x83"
"\x44\x81"		"\x5F\x81"		"\x60\x84\xFF"	"\x61\x84\xFF"
"\x62\x82\xFF"
"\x80\x80"
// Feature 0A:
"\x08"
"\x40\x82"		"\x41\x82"		"\x42\x82"		"\x43\x82"
"\x44\x81"		"\x45\x84"		"\x46\x84"		"\x60\x82\xFF"
"\x61\x81\xFF"	"\x62\x84\xFF"	"\x63\x84\xFF"	"\x64\x84\xFF"
"\x65\x83\xFF"	"\x66\x83\xFF"	"\x67\x84\xFF"	"\x68\x84\xFF"
"\x7C\x84\x0F"
"\xB6\x80"
// Feature 0B:
"\x0B"
"\x80\x80"
// Feature 0C:
"\x0C"
"\x80\x80"
// Feature 0D:
"\x0D"
"\x80\x80"
// Feature 0E:
"\x0E"
"\x60\x84\xFF"
"\x80\x80"
// Feature 0F:
"\x0F"
"\x40\x83"		"\x41\x82"		"\x42\x82"		"\x43\x82"
"\x80\x80"
// Feature 10:
"\x10"
"\x40\x81"		"\x41\x81"		"\x42\x81"		"\x43\x84"
"\x80\x80"
// Feature 11:
"\x0D"
"\x41\x81"		"\x42\x81"		"\x43\x83"		"\x44\x81"
"\x60\x84\xFF"	"\x61\x84\xFF"	"\x62\x82\xFF"
"\x80\x80"
;

static const char _datD[]="\x14\x04\x11"
"\x11\x0C"
// GRM count:
//--00--          --02--          --04--           --06--
"\x74\x00\x58\x00\x0B\x00\x29\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//--08--          --0A--          --0C--           --0E--
"\x1E\x13\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//--10--          --12--          --14--           --16--
"\x00\x00\x00\x00"
;

static const char _datIDs[]="\x77\x04\x11"
//--00--          --02--          --04--           --06--
"\x73\x00\x57\x00\x0A\x00\x28\x00\xFF\x00\x07\x00\x0C\x00\xFF\x00"
//--08--          --0A--          --0C--           --0E--
"\x00\x00\xFF\x00\x24\x00\x1F\x00\xFF\xFF\x00\x00\x00\x00\xFF\x00"
//--10--          --12--          --14--           --16--
"\xFF\x00\xFF\x00"
;

/*	Action 4 strings
	================

	Flags:
		0x01 = CTRL_FONT_LARGE
		0x02 = CTRL_FONT_SMALL
		0x04 = CTRL_SPACE
		0x08 = CTRL_NEWLINE
		0x10 = CTRL_COLOR
		0x20 = CTRL_NO_STACK_CHECK
*/
static const char _dat4[]="\x03\x05\x11"
// Rules for one-byte IDs:
// 00              04              08              0C
"\x04\x04\x04\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10"
// Rules for two-byte IDs:
// 00              04              08              0C
"\x1C\x1C\x1C\x1C\x04\x04\x00\x04\x00\x1C\x3D\x06\x00\x00\x00\x00\x04\x1C"
;


/*	Randomized action 2
	===================

	Byte triples: num 80 bits/num 83 bits/num triggers
*/
static const char _dat2r[]="\x02\x06\x11"
"\x08\x08\x05"	"\x08\x08\x05"	"\x08\x08\x05"	"\x08\x08\x05"
"\x14\x00\x06"	"\x08\x00\x00"	"\x00\x00\x00"	"\x08\x00\x02"
"\x00\x00\x00"	"\x08\x10\x03"	"\x10\x00\x00"	"\x00\x00\x00"
"\x00\x00\x00"	"\x00\x00\x00"	"\x00\x00\x00"	"\x08\x00\x00"
"\x02\x00\x00"	"\x04\x10\x00"
;


// ---------------------------------------------------------------------------


struct dat{
	const char*data,*name;
	uint len;
};

#undef _RENUM_DATA_H_INCLUDED_
#undef DATA
#undef DATA_FILE
#define DATA() static const dat data[]={
#define DATA_FILE(name)\
	{(char*)_dat##name,"/.nforenum/" #name ".dat",sizeof(_dat##name)-1},\

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
		dir[dir.length()-1]='\0';
	if(stat((dir+"/.nforenum").c_str(),&Stat))return false;
	else if(Stat.st_mode&S_IFREG)return false;
	return true;
}

string getdir(){
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
		if(finddir(cwd))pret=&cwd;
		else if(finddir(home))pret=&home;
		else if(finddir(homedrpath)||makedir(homedrpath))pret=&homedrpath;
		else if(makedir(home))pret=&home;
		else{
			verify(makedir(cwd,true));
			pret=&cwd;
		}
	}
	if(!dosleep)IssueMessage(0,DATA_FOUND_AT,pret->c_str());
	return *pret;
}

FILE*tryopen(const char*name,const char*mode,bool allownull=false){
	static string dir=getdir();
	FILE*pFile=fopen((dir+name).c_str(),mode);
	if(pFile||allownull)return pFile;
	IssueMessage(0,DATAFILE_ERROR,OPEN,name+1,ERRNO,errno);
	perror(NULL);
	assert(false);
	exit(EDATA);
}

FILE*_myfopen(files file){
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
	pFile=tryopen(data[file].name,"wb");
	if(fwrite(data[file].data,1,data[file].len,pFile)!=data[file].len){
		IssueMessage(0,DATAFILE_ERROR,WRITE,data[file].name+1,-1);
		assert(false);
		exit(EDATA);
	}
	fclose(pFile);
	pFile=tryopen(data[file].name,"rb");
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
