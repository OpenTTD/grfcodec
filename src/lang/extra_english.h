/*
 * extra_english.h
 * Extra program text in English language.
 *
 * Copyright 2005-2006,2009 by Dale McCoy.
 * Copyright 2006 by Dan Masek.
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

// Names of extra strings, below, are passed to correspond
// with a %s or %S argument in the format.
// A %S string consumes any arguments that immediately follow it,
// according to its format, while %s strings do not.
// If a literal char* must be passed, (eg __FILE__), use %t instead.

// START_EXTRA_STRINGS(<language>)
// - <language> Language id as defined in language_list.h
START_EXTRA_STRINGS(RL_ENGLISH)

EXTRA(PREFIX_UNPARSEABLE,"Processing Failure: ")
EXTRA(PREFIX_LINT_FATAL,"Fatal Error (%d): ")
EXTRA(PREFIX_LINT_ERROR,"Error (%d): ")
EXTRA(PREFIX_LINT_WARNING,"Warning (%d): ")
EXTRA(OFFSET,"Offset %d: ")

EXTRA(LOADING,"loading")
EXTRA(LOADED,"loaded")
EXTRA(LOTS,"lots-of-cargo")
EXTRA(HOUSE_INSTYTILE,"houses and industry tiles")
EXTRA(INDUSTRIES,"industries")
EXTRA(BASICSTD2,"basic standard 02s")
EXTRA(PROD2S,"Production callbacks")
EXTRA(PROD2,"production callback")
EXTRA(GROUND,"ground")
EXTRA(NONTRANS,"non-transparent")

EXTRA(GRF,"GRF")

EXTRA(ACTION,"action %xs")
EXTRA(IMPORTS,"sound import sprites")
EXTRA(TYPE,"%s for %s")
EXTRA(AT_LEAST,"at least %d")
EXTRA(EXACTLY,"%d")
EXTRA(ONE_OF,"%d or %d")
EXTRA(VARS,"%s and %s")
EXTRA(VAL,"%2x")
EXTRA(VALS,"%2x and %2x")
EXTRA(REAL_S,"real sprite")
EXTRA(BIN_INCLUDE,"binary include")
EXTRA(RECOLOR_TABLE,"recolor table")
EXTRA(SET,"action 1 set")
EXTRA(CID,"cargo ID")
EXTRA(PROPLENGTH,"declared property length")

EXTRA(ID,"ID")
EXTRA(CARGO,"Cargo")

EXTRA(ACT5_SIZE,"%d (0x%x)")
EXTRA(ACT5_ORSIZE," or %d (0x%x)")

EXTRA(EOF_READING_NAME,"EOF while reading name %2x.")
EXTRA(OVERLENGTH_NAME,"Name %2x exceeds 100 characters long.")

EXTRA(OPEN,"open")
EXTRA(LOAD,"load")
EXTRA(WRITE,"write")

EXTRA(BYTE1,"byte 1")
EXTRA(FEATURE,"feature")

EXTRA(NUMINFO,"numinfo")
EXTRA(NUMIDS,"numids")

EXTRA(NUMENT1,"nument1")
EXTRA(NUMENT2,"nument2")
EXTRA(NVAR,"nvar")
EXTRA(NRAND,"nrand")
EXTRA(NID,"n-id")
EXTRA(NUMCID,"num-cid")
EXTRA(NUM,"num")
EXTRA(NUMENT,"num-ent")
EXTRA(NUMSPRITES,"num-sprites")
EXTRA(NUMSETS,"num-sets")
EXTRA(NUMDEFS,"num-defs")

EXTRA(VARSIZE,"var-size")
EXTRA(COND,"cond")

EXTRA(XPOS,"xpos")
EXTRA(YPOS,"ypos")
EXTRA(XSIZE,"xsize")
EXTRA(YSIZE,"ysize")
EXTRA(XREL,"xrel")
EXTRA(YREL,"yrel")

EXTRA(XOFF,"xoff")
EXTRA(XOFF_EXT,"xoff+xextent")
EXTRA(YOFF,"yoff")
EXTRA(YOFF_EXT,"yoff+yextent")

EXTRA(FILELINE,"(%t:%d)")
EXTRA(ERRNO,"(%d)")
EXTRA(DAT2,"%2x %2x")
EXTRA(DAT3,"%2x %2x %2x")

END_EXTRA_STRINGS()
