/*
 * message_english.h
 * Message text and data in English language.
 *
 * Copyright 2005-2009 by Dale McCoy.
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

// Only the properties for messages in English are relevant.
// Properties defined in files for additional languages are ignored.
// MESSAGE_UNUSED and maintaining the message order is required in
// English, but optional in all other languages.

// The first message is different for a reason.
// It is not run through myvsprintf; a number is appended at runtime.
// All other messages follow the printf format
// (see myvsprintf in messages.cpp for supported specifiers).

// Macro invocations broken across multiple lines must be continued with a
// trailing backslash, or checklang.pl will become confused.

// START_MESSAGES(<language>)
// - <language> Language id as defined in language_list.h
START_MESSAGES(RL_ENGLISH)

ERR_MESSAGE(FATAL_MESSAGE_ERROR,"Fatal error issuing message ",0)
ERR_MESSAGE(INTERNAL_ERROR_TEXT,"%t(%d): Fatal error parsing sprite %d: %t cannot equal %d.\n" \
			"\t(function %t).\n",0)
ERR_MESSAGE(INVALID_DATAFILE,"Data file \"%t\" contains invalid data. (%S)\n",0)
ERR_MESSAGE(NO_INPUT_FILE,"Could not open \"%t\" specified on the command line.\n",0)
ERR_MESSAGE(NO_OUTPUT_FILE,"Could not open output file \"%t\" for input \"%t\".\n",0)
ERR_MESSAGE(REPLACE_FAILED,"Could not replace old file (%t) with new file (%t). (%d)\n",0)
ERR_MESSAGE(DATAFILE_ERROR,"Failed to %s data file: \"%t\". %S\n",0)
ERR_MESSAGE(CREATE_FAILED,"Could not create .nforenum directory in %t. (%d)\n",0)
ERR_MESSAGE(DELETE_FAILED,"Could not remove old file %t. (%d)\n",0)

OUT_MESSAGE(CREATED_DIR,"Created .nforenum directory in %t.\n",0)
OUT_MESSAGE(PROCESSING,"Processing from standard input.\n",0)
OUT_MESSAGE(PROCESSING_FILE,"Processing file \"%t\".\n",0)
OUT_MESSAGE(PROCESSING_COMPLETE,"Processing complete.\n",0)
ERR_MESSAGE(APPARENTLY_NOT_NFO,"Does not appear to be an NFO file.\n",0)
ERR_MESSAGE(UNKNOWN_VERSION,"Unknown NFO file version: %d.  ",0)//No \n. That's intentional.
OUT_MESSAGE(SKIPPING_FILE,"Skipping file.\n",0)
OUT_MESSAGE(PARSING_FILE,"Attempting to parse as version 4.\n",0)
ERR_MESSAGE(PARSE_FAILURE,"The sprite following sprite %d could not be processed.\n",0)
ERR_MESSAGE(PARTIAL_PARSE_FAILURE,"A portion of sprite %d could not be processed.\n",0)
ERR_MESSAGE(NOT_IN_SPRITE,"Found pseudosprite continuation line while looking for sprite %d.\n",0)
ERR_MESSAGE(CONSOLE_LINT_FATAL,"Linter failure on sprite %d.\n",0)
ERR_MESSAGE(CONSOLE_LINT_ERROR,"Error on sprite %d.\n",0)
ERR_MESSAGE(CONSOLE_LINT_WARNING,"Warning on sprite %d (level %d).\n",0)
OUT_MESSAGE(CONSOLE_AUTOCORRECT,"Attempting to autocorrect sprite %d.\n",0)
MESSAGE_EX(UNEXP_EOF_STD2,"Unexpected EOF: Expected additional standard Action 2s.\n",0,ERROR,UNUSED_SET)
MESSAGE_EX(UNEXP_EOF_CARGOID,"Unexpected EOF: Unused CargoIDs detected for feature %x.\n",0,ERROR,UNUSED_ID)
MESSAGE_EX(UNEXP_EOF_TOWNNAMES,"Unexpected EOF: Unused town name IDs detected.\n",0,ERROR,UNUSED_ID)
MESSAGE_EX(UNEXP_EOF_LONGJUMP,"Unexpected EOF: Action 7/9 jumps past EOF.\n",0,ERROR,LONG_JUMPLEAD)
OUT_MESSAGE(STARTUP,"NFORenum " VERSION " - Copyright (C) " YEARS " by Dale McCoy\n",0)

NFO_MESSAGE(BAD_RPN,"Invalid RPN expression while reading character %c.\n",0)
NFO_MESSAGE(BAD_RPN_EOF,"No close parenthesis found.\n",0)
NFO_MESSAGE(UNDEF_VAR,"Undefined variable \"%t\".\n",0)
NFO_MESSAGE(COMMAND_INVALID_ARG,"Missing or invalid argument to %t command.\n",0)
NFO_MESSAGE(COMMAND_UNKNOWN,"Unknown comment command: %t\n",0)
NFO_MESSAGE(COMMAND_REVERT_DEFAULT,"Assuming \"DEFAULT\".\n",0)
NFO_MESSAGE(COMMAND_UNKNOWN_VERSION,"Unknown version number. If this version has been released, please get the latest versions.dat from the NFORenum website.\n",0)

//UNPARSEABLE
NFO_MESSAGE(INVALID_CHARACTER,"Invalid character: \"%c\".\n",0)
NFO_MESSAGE(INVALID_EXTENSION,"Invalid escape sequence.\n",0)
NFO_MESSAGE(UNTERMINATED_STRING,"Unterminated literal string.\n",0)
NFO_MESSAGE(REAL_NO_FILENAME,"Apparent real sprite does not contain a file name.\n",MAKE_COMMENT)
NFO_MESSAGE(REAL_MISSING_DATA,"Could not read %t from apparent real sprite.\n",0)

//FATAL
NFO_MESSAGE(INVALID_LENGTH,"All %S are %S bytes long.\n",USE_PREFIX)
NFO_MESSAGE(BAD_LENGTH,"Length does not match %S of %S. (Expected %d bytes)\n",USE_PREFIX)
NFO_MESSAGE(TOO_SHORT,"Linter requires %d byte(s) which do(es) not exist.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(INVALID_FEATURE,"Invalid feature byte.\n",USE_PREFIX)
NFO_MESSAGE(INVALID_TYPE,"Invalid nument1 or random-/variational-type byte.\n",USE_PREFIX)
NFO_MESSAGE(INVALID_ACTION,"Invalid action byte.\n",USE_PREFIX)
NFO_MESSAGE(INVALID_PROP,"Invalid property %2x.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(UNTERM_ACT6,"Action 6 appears to be missing terminating FF.\n",USE_PREFIX)

//ERROR
NFO_MESSAGE(INSUFFICIENT,"Missing %s. Sprite %d declared %d, but only %d have been seen.\n",USE_PREFIX)
NFO_MESSAGE(EXTRA,"Extra %s. The %d declared by sprite %d have already been seen.\n",USE_PREFIX)
NFO_MESSAGE(UNEXPECTED,"Unexpected %s found.\n",USE_PREFIX)
NFO_MESSAGE(FEATURE_MISMATCH,"Feature byte does not match feature byte of preceding Action %x at sprite %d.\n",USE_PREFIX)
NFO_MESSAGE(RAND_2_NUMSETS,"Rand02s require power-of-two sets to choose from.\n",USE_PREFIX)
NFO_MESSAGE(INDIRECT_VAR_7E,"Variable 7B is not allowed with parameter 7E.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(NEITHER_ID_CALLBACK,"%4x is neither callback nor %s.\n",USE_PREFIX)
NFO_MESSAGE(UNDEFINED_SPRITE_SET,"Sprite set %2x does not appear in the preceding Action 1 (sprite %d).\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(INVALID_CARGO_TYPE,"Invalid cargo type: %2x\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(NO_REQD_SETS,"Action 2 declaring no %s sets.\n",USE_PREFIX)
NFO_MESSAGE(CANNOT_EXTEND,"This type of action 5 cannot use the extended format.\n",USE_PREFIX)
NFO_MESSAGE(ACTION_5,"Expected %t sprites for this type.\n",USE_PREFIX)
NFO_MESSAGE(ACTION_5_LIMIT,"Loading sprites 0x%x..0x%x into an array with only 0x%x entries.\n",USE_PREFIX)
NFO_MESSAGE(UNDEFINED_ID,"ID %2x has not been defined.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(INSUFFICIENT_PROPS,"Expected %d more properties.\n",USE_PREFIX)
NFO_MESSAGE(INSUFFICIENT_DATA,"Expected more data for prop %2x. (%d bytes at %d.)\n",USE_PREFIX)
NFO_MESSAGE(INSUFFICIENT_DATA2,"Expected %d more bytes for prop %2x.\n",USE_PREFIX)
NFO_MESSAGE(MISSING_TERMINATOR,"Property data is missing terminating byte(s).\n",USE_PREFIX)
NFO_MESSAGE(DUPLICATE_ACT,"Action %x already found at sprite %d.\n",USE_PREFIX)
NFO_MESSAGE(MISSING_8,"An action 8 must precede action %x.\n",USE_PREFIX)
NFO_MESSAGE(INVALID_VERSION,"Invalid %s version number.\n",USE_PREFIX)
  MESSAGE_UNUSED(BAD_VARIABLE)
NFO_MESSAGE(BAD_VARSIZE,"Variable size %2x is invalid.\n",USE_PREFIX)
NFO_MESSAGE(BAD_CONDITION,"Condition %2x is invalid.\n",USE_PREFIX)
NFO_MESSAGE(GRFCOND_NEEDS_GRFVAR,"Condition %2x requires variable 88.\n",USE_PREFIX)
NFO_MESSAGE(GRFVAR_NEEDS_GRFCOND,"Variable 88 requires a GRFid condition.\n",USE_PREFIX)
NFO_MESSAGE(BITTEST_VARIABLE,"Variable %2x requires a bit-test condition.\n",USE_PREFIX)
NFO_MESSAGE(VARIABLE_SIZE_MISMATCH,"%d is not a valid <size> for variable %2x.\n",USE_PREFIX)
  MESSAGE_UNUSED(INVALID_LABEL)//,"Labels must have the high 3 bits set (E0-FF).\n",USE_PREFIX)
NFO_MESSAGE(INVALID_LITERAL,"%2x was found where a literal %2x was expected.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(INSUFFICIENT_STRINGS,"Expected %d more string(s) for this action 4.\n",USE_PREFIX)
NFO_MESSAGE(TOO_LARGE,"Value of <%s> must not exceed %2xh.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(SPRITENUM_TOO_HIGH,"TTD only defines sprites up to 4984 (1E 13).\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(INVALID_COLOR_TRANS,"Color translation mode 3 is undefined.\n",USE_PREFIX)
NFO_MESSAGE(INVALID_COLOR_SPRITE,"Sprite %d is not a color translation sprite.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(NO_GROUNDSPRITE,"No %s sprite was specified.\n",USE_PREFIX)
NFO_MESSAGE(NO_STD_3,"This livery override action 3 does not follow a standard action 3.\n",USE_PREFIX)
NFO_MESSAGE(NONEXISTANT_VARIABLE,"Testing nonexistant variable %2x.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(MODIFY_BEYOND_LENGTH,"Action 6 attempts to modify a byte after the end of this sprite.\n",USE_PREFIX)
NFO_MESSAGE(NO_FOLLOWING_SPRITE,"There is no following sprite for this Action 6 to modify.\n",USE_PREFIX)

//WARNING1
NFO_MESSAGE(NO_SETS,"Action %x declaring no sets.\n",USE_PREFIX)
NFO_MESSAGE(NO_SPRITES,"Action %x declaring sets with no sprites.\n",USE_PREFIX)
NFO_MESSAGE(SET_WITH_NO_SPRITES,"Set %d contains no sprites.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(NO_PROPS,"Action 0 setting no properties.\n",USE_PREFIX)
NFO_MESSAGE(NO_IDS,"Action 0 setting properties for no IDs.\n",USE_PREFIX)
NFO_MESSAGE(UNUSED_ID,"Redefining ID %2x not used since previous definition at sprite %d.\n",USE_PREFIX)
MESSAGE_EX(UNUSED2IDLEAD,"For feature %x the following cargoIDs have not been used since their most recent definition:\n",MAKE_COMMENT,NFO,UNUSED_ID)
MESSAGE_EX(UNUSEDIDFINAL,"%2x (last defined at sprite %d)\n",MAKE_COMMENT,NFO,UNUSED_ID)
NFO_MESSAGE(UNUSED_SET,"Set %2x defined by the previous Action 1 (sprite %d) has not been used.\n",USE_PREFIX)
NFO_MESSAGE(UNREACHABLE_VAR,"Variation %d cannot be reached.\n",USE_PREFIX)
NFO_MESSAGE(EXTRA_DATA,"No more data was expected. Found %d bytes, expected %d bytes.\n",USE_PREFIX)
NFO_MESSAGE(REUSED_DEFAULT,"Default ID appears earlier in sprite.\n",USE_PREFIX)
NFO_MESSAGE(RESERVED_GRFID,"GRFIDs with a first byte of FF are reserved.\n",USE_PREFIX)
NFO_MESSAGE(INDIRECT_VAR_NOT_6X,"Variable 7B only useful for 60+x variables.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(SHIFT_TOO_FAR,"Shifting variable %2x past its length.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(NO_MODIFICATIONS,"Action 6 does not make any changes.\n",USE_PREFIX)
NFO_MESSAGE(DOES_NOT_MODIFY,"This section does not make any changes.\n",USE_PREFIX|HAS_OFFSET)

//WARNING2
  MESSAGE_UNUSED(BACKWARDS_GOTO)//,"Action %x jumping backwards to label %2x defined at sprite %d.\n",USE_PREFIX)
NFO_MESSAGE(UNUSED_LABEL_NO_PREV_DEF,"Label %2x has not been goto'ed.\n",USE_PREFIX)
NFO_MESSAGE(UNUSED_LABEL_PREV_DEF,"Label %2x has not been goto'ed since previous definition at sprite %d.\n",USE_PREFIX)
NFO_MESSAGE(REPEATED_PROP,"Property %2x is previously defined at offset %d.\n",USE_PREFIX|HAS_OFFSET)
 MESSAGE_UNUSED(OFF_TILE)

//WARNING3
NFO_MESSAGE(ONLY_ONE_CHOICE,"Random 2 declaring only one choice.\n",USE_PREFIX)
NFO_MESSAGE(NOT_VARIATIONAL,"Variational 2 accesses only constant variables (1A & 1C).\n",USE_PREFIX)
NFO_MESSAGE(NOT_RANDOM,"All random options have the same ID.\n",USE_PREFIX)
  MESSAGE_UNUSED(TOO_MANY_STRINGS)

NFO_MESSAGE(NO_ACT1,"No preceding action 1.\n",USE_PREFIX)
  MESSAGE_UNUSED(INVALID_AGAIN)//,"<again> must be 00 or 01.\n",USE_PREFIX)
NFO_MESSAGE(INVALID_OP,"Invalid operation %2x.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(SET_TOO_LARGE,"Sprite set contains more sprites than given feature can use.\n",USE_PREFIX)
NFO_MESSAGE(STRANGE_SET_SIZE,"Sprite set contains an unusual number of sprites.\n",USE_PREFIX)
MESSAGE_EX(UNUSEDFIDLEAD,"For new town names, the following IDs have not been used since their most recent definition:\n",MAKE_COMMENT,NFO,UNUSED_ID)
NFO_MESSAGE(DUPLICATE_GRFID,"GRF ID previously deactivated at offset %d (%8x).\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(NO_GRFIDS,"Action E is not disabling any GRF files.\n",USE_PREFIX)
NFO_MESSAGE(INVALID_SRC,"Source parameter %d is invalid.\n",USE_PREFIX)
NFO_MESSAGE(INVALID_TARGET,"Target parameter is invalid.\n",USE_PREFIX)
NFO_MESSAGE(ONLY_ONE_DATA,"Only one of src1 and src2 may be 0xFF.\n",USE_PREFIX)
NFO_MESSAGE(DUPLICATE_LANG_NAME,"A name for %L was previously specified at offset %d.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(MISSING_FALLBACK,"No fallback string was specified.\n",USE_PREFIX)
NFO_MESSAGE(NO_PARTS,"Zero parts were specified.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(OUT_OF_RANGE_BITS,"Only %d random bits are available.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(INSUFFICIENT_BITS,"%x bits are required, but only %2x were allocated.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(NO_PROBABILITY,"A probability of 0 was specified for this part.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(PARAM_TOO_LARGE,"Parameter %2x out of valid range for variable %2x.\n",HAS_OFFSET|USE_PREFIX)
NFO_MESSAGE(NO_BUILDING_SPRITE,"Building sprites may not be null.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(OVERRAN_SPRITE,"Unexpected end-of-sprite reading building sprite %d.\n",USE_PREFIX)
  MESSAGE_UNUSED(FIRST_SPRITE_CANNOT_SHARE)
NFO_MESSAGE(INVALID_GRFID,"Action %x references reserved GRFid.\n",USE_PREFIX|HAS_OFFSET)
  MESSAGE_UNUSED(INSUFFICIENT_INCLUDE)
  MESSAGE_UNUSED(EXTRA_INCLUDE)
  MESSAGE_UNUSED(UNEXPECTED_INCLUDE)
NFO_MESSAGE(OOR_COUNT,"Attempting to manage too many IDs.\n",USE_PREFIX)
NFO_MESSAGE(INVALID_ID,"ID %2x out of valid range (%2x..%2x).\n",USE_PREFIX)
NFO_MESSAGE(NO_CARGOTYPES,"<num-cid> must be 0 for this feature.\n",USE_PREFIX)
NFO_MESSAGE(UNUSED_CONTROL,"Using unspecified control character %2x.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(INVALID_CONTROL,"Control character %2x should not be used in this string.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(SMASH_STACK,"Control character %2x does not match %K on top of stack.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(SMASH_STACK_SPAN,"Control character %2x reads data from multiple parameters.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(OVERRAN_STACK,"Insufficient stack data for control character %2x.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(CANNOT_SHUFFLE,"There is insufficient stack data to shuffle the stack.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(NO_NULL_FOUND,"No terminating null found.\n",USE_PREFIX)
NFO_MESSAGE(INVALID_SEVERITY,"Invalid severity byte.\n",USE_PREFIX)
NFO_MESSAGE(UNKNOWN_LANG_BIT,"Unrecognized bit set in language-id byte %2x.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(INVALID_MESSAGEID,"Invalid message id.\n",USE_PREFIX)
NFO_MESSAGE(INVALID_PARAM,"%2x is not a GRF parameter.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(INVALID_OVERRIDE,"Cannot override graphics for this feature.\n",USE_PREFIX)
NFO_MESSAGE(COND_SIZE_MISMATCH,"Condition %2x requires %d byte(s).\n",USE_PREFIX)
NFO_MESSAGE(REAL_VAL_TOO_SMALL,"Metadata invalid. %s must not be smaller than %d.\n",USE_PREFIX)
NFO_MESSAGE(REAL_VAL_TOO_LARGE,"Metadata invalid. %s must not be larger than %d.\n",USE_PREFIX)
  MESSAGE_UNUSED(REAL_MOVES_UP)
NFO_MESSAGE(REAL_BAD_COMP,"Metadata invalid. compression can only have bits 0, 1, 3 and/or 6 set.\n",USE_PREFIX)
NFO_MESSAGE(REAL_SPRITE_TOO_LARGE,"Metadata invalid. Sprite size exceeds 64K.\n",USE_PREFIX)
NFO_MESSAGE(INVALID_TEXTID,"Text ID %4x is not a valid text ID.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(UNDEFINED_TEXTID,"Text ID %x has not been defined.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(CHECK_0C_RANGE,"Checking for var0C in the range [%2x,%2x].\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(INVALID_CALLBACK,"%2x is not a valid callback for this feature.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(INVALID_FONT,"%2x is not a valid font.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(SPANS_BLOCKS,"Set %d declares glyphs across a block-break.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(OVERRAN_F_NAME,"The name for language(s) %2x has no terminating null.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(OVERRAN_F_PART,"The name-part starting at %d has no terminating null.\n",USE_PREFIX)
NFO_MESSAGE(OBSCURED_VARIATION,"Variation %d is partially obscured.\n",USE_PREFIX)
NFO_MESSAGE(UNREACHABLE_DEFAULT,"Default result cannot be reached.\n",USE_PREFIX)
NFO_MESSAGE(DIVIDE_BY_ZERO,"var-adjust performs a divide-by-zero or modulo-zero operation.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(USE_SHIFT_AND,"A shift-and var-adjust would suffice here.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(INVALID_SHIFT,"shift may not have both high bits set.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(UNDEFINED_TRIGGER,"Using an undefined trigger.\n",USE_PREFIX)
NFO_MESSAGE(OVERLENGTH_UTF8,"Invalid UTF-8 sequence: Overlength representation.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(INVALID_UTF8,"Invalid UTF-8 sequence: First byte not followed by sufficient continuation bytes.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(OUTOFRANGE_UTF8,"Invalid UTF-8 sequence: Encodes a character beyond the BMP.\n",USE_PREFIX|HAS_OFFSET)
  MESSAGE_UNUSED(UNEXPECTED_UTF8_CONT)
NFO_MESSAGE(UNKNOWN_LANGUAGE,"Language %2x is not defined.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(INCLUDING_00_ID,"Including TextID %4x, which contains a 00 byte.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(AUTOCORRECTING,"Auto-correcting %s from %2x to %2x.\n",HAS_OFFSET)
NFO_MESSAGE(EMBEDDED_00,"Embedded null byte.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(LONG_JUMPLEAD,"The following Action 7/9s jump past the end of this file:\n",USE_PREFIX|NO_CONSOLE)
MESSAGE_EX(LONG_JUMP,"Action %x at sprite %d.\n",MAKE_COMMENT|NO_CONSOLE,NFO,LONG_JUMPLEAD)
NFO_MESSAGE(NO_TEXTS,"Action 4 declaring no strings.\n",USE_PREFIX)
NFO_MESSAGE(AUTOCORRECT_ADD,"Adding a trailing %2x byte.\n",0)
NFO_MESSAGE(INVALID_EXT_CODE,"Undefined extended format code %2x.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(FEATURE_LINK_MISMATCH,"ID %2x is defined with feature %2x.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(FEATURE_CALL_MISMATCH,"ID %2x is defined with feature %2x.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(AUTOCORRECT_LOOP,"Autocorrector seems to be stuck in an infinite loop.\n",USE_PREFIX)
NFO_MESSAGE(BITS_OVERLAP,"<parts> field %d reuses the following bits already used in this Action F: %t\n",USE_PREFIX|HAS_OFFSET)
MESSAGE_EX(MISSING_LANG_NAME,"No name for %L was specified.\n",MAKE_COMMENT|USE_PREFIX,NFO,MISSING_FALLBACK)
NFO_MESSAGE(OUT_OF_RANGE_TEXTID_13,"Action 13 can only define texts in the C4xx, C5xx, C9xx, D0xx, and DCxx ranges.\n",USE_PREFIX)
NFO_MESSAGE(AND_00,"Anding with 0.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(GENERIC_AND_OVERRIDE,"Action 3 may not be both generic and an override.\n",USE_PREFIX)
NFO_MESSAGE(RECURSIVE_F,"Action F may not chain to itself.\n",USE_PREFIX)
NFO_MESSAGE(EXCESSIVE_ADD,"<add-in-%d> should not be negative.\n",USE_PREFIX|HAS_OFFSET)
  MESSAGE_UNUSED(NOT_A_REGISTER)//,"Attempt to access non-existant register %4x.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(DUPLICATE,"%s %2x previously specified at offset %d.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(ACT3_PRECEDES_PROP08,"Prop 08 has not been set for ID %2x.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(MASKED_BIT_SET,"GRFID has a bit set that is clear in the mask.\n",USE_PREFIX)
NFO_MESSAGE(DISCARD_UNSTORED,"Operation 0F follows a non-store operation.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(NO_PERS_REGS,"No persistent registers exist for this feature.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(DUPLICATE_TRANS_TABLE,"Cargo translation table previously found at sprite %d.\n",USE_PREFIX)
NFO_MESSAGE(BAD_RANDSUBTYPE,"Invalid random 2 subtype.\n",USE_PREFIX)
NFO_MESSAGE(NO_BYTE_IDS,"Cannot set (extended-)byte IDs for feature %2x.\n",USE_PREFIX)
NFO_MESSAGE(COULD_NOT_VERIFY,"Cannot check for usage of undefined IDs: Cannot find feature to check.\n",USE_PREFIX)
NFO_MESSAGE(LENGTH_MISMATCH,"Declared length %d does not match actual length %d.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(EXTENSION_MISMATCH,"Found byte %d of a %d-byte escape while reading byte %d of a %d-byte field.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(BEFORE_8,"Action %x must precede action 8.\n",USE_PREFIX)
NFO_MESSAGE(INVALID_VERSION_ACT14,"Action 14 requires version number 7 or higher, found %x.\n",USE_PREFIX)
NFO_MESSAGE(UNKNOWN_ACT14_TYPE,"Unrecognized type %2x, ignoring rest.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(NEED_VERSION_7,"NFO version 7 or higher is needed, found %x.\n",USE_PREFIX)
NFO_MESSAGE(NESTED_CHOICE_LIST,"Choice lists can't be nested.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(NOT_IN_CHOICE_LIST,"Choice list string code outside of choice list.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(NO_DEFAULT_IN_CHOICE_LIST,"No default choice in choice list.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(CHOICE_LIST_NOT_TERMINATED,"The choice list is not terminated.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(INDIRECT_VAR_START,"The first accessed variable may not be 7B.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(MISSING_PALETTE_INFO,"There was no Action 14 specifying the palette.\n",USE_PREFIX)
NFO_MESSAGE(INVALID_PALETTE_INFO,"Palette information must be 1 byte (\"D\", \"W\" or \"A\").\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(INVALID_ACT2_FLAGS,"Unknown flag set (offset %d) in advanced action2.\n",USE_PREFIX|HAS_OFFSET)
NFO_MESSAGE(NOT_IN_REALSPRITE,"Found realsprite continuation line while looking for sprite %d.\n",0)
NFO_MESSAGE(REAL_UNKNOWN_FLAG,"Unknown or duplicate flag '%t'.\n",0)
NFO_MESSAGE(REAL_DUPLICATE_ZOOM,"Duplicate zoomlevel/depth sprite.\n",0)
NFO_MESSAGE(REAL_8BPP_NORMAL_FIRST,"8bpp normal zoom sprite must appear first.\n",0)
NFO_MESSAGE(REAL_32BPP_BEFORE_MASK,"mask sprites must be preceded by 32bpp sprites.\n",0)
NFO_MESSAGE(UNKNOWN_ACT0_DATA,"Unknown data does not allow processesing past this point.\n",USE_PREFIX|HAS_OFFSET)

/* Insert new NFO_MESSAGEs above this line unless a MESSAGE_UNUSED appears in a logical location. */

#if defined DEBUG || defined _DEBUG
ERR_MESSAGE(BAD_STRING,"Error: String %d does not exist (%d/%d).\n",0)
#else
ERR_MESSAGE(BAD_STRING,"Error: String %d does not exist.\n",0)
#endif
ERR_MESSAGE(DATAFILE_MISMATCH,"%t contains information for fewer features than does feat.dat.\n" \
			"Update it, or delete feat.dat.\n",0)
ERR_MESSAGE(BAD_CL_ARG,"Unrecognized command-line argument: \"-%c %t\"\n",0)
OUT_MESSAGE(DATA_FOUND_AT,"Data files loaded from '%t/.nforenum'.\n",0)

END_MESSAGES()
