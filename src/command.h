/*
 * command.h
 * Defines structs and declares functions for comment commands.
 *
 * Copyright 2004-2006,2008-2009 by Dale McCoy.
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

#ifndef _RENUM_COMMAND_H_INCLUDED_
#define _RENUM_COMMAND_H_INCLUDED_

#ifndef COMMAND_DATA
/* Be careful when changing these defs; they have to remain compatible
 * with the ones in command.cpp.*/
#define COMMAND_DATA_START(name) enum name{
#define COMMAND_DATA(command) command,
#define COMMAND_DATA_EX(placeholder,command) placeholder,
#define COMMAND_DATA_END() };
#endif//COMMAND_DATA

#ifndef RPNOFF
#define RPNON() ((GetState(REALSPRITES)&1)==0)
#define RPNOFF() ((GetState(REALSPRITES)&1)!=0)
#define COMMENTON() ((GetState(REALSPRITES)&2)==0)
#define COMMENTOFF() ((GetState(REALSPRITES)&2)!=0)
#endif

COMMAND_DATA_START(gen)
COMMAND_DATA(REMOVEMESSAGES)
COMMAND_DATA(PRESERVEMESSAGES)
COMMAND_DATA(LINT)
COMMAND_DATA(DIFF)
COMMAND_DATA(USEID2)
COMMAND_DATA(USESET)
COMMAND_DATA(WARNING)
COMMAND_DATA(VERSIONCHECK)
COMMAND_DATA(LET)
COMMAND_DATA(REALSPRITES)
COMMAND_DATA(BEAUTIFY)
COMMAND_DATA(CLEARACTION2)
COMMAND_DATA(CLEARACTIONF)
COMMAND_DATA(TESTID2)
COMMAND_DATA(DEFINEID2)
COMMAND_DATA(LOCATEID2)
COMMAND_DATA(USEOLDSPRITENUMS)
//COMMAND_DATA(VERBOSE)
/* Add new comment commands here; if they need arguments, create a new
 * COMMENT_DATA_START/.../COMMENT_DATA_END block.
 * Also add code to parse_comment in command.cpp and, if necessary, one or
 * both of GetState(enum gen) or a new GetState.
 * Basically the same procedure applies for adding new arguments to existing
 * commands, except that they must go in the approprate COMMAND_DATA block,
 * and the GetState support, if necessary, must go in the correct GetState*/
COMMAND_DATA_END()

COMMAND_DATA_START(san)
COMMAND_DATA(OFF)
COMMAND_DATA(NONE)
COMMAND_DATA(FATAL)
COMMAND_DATA(ERROR)
COMMAND_DATA(WARNING1)
COMMAND_DATA(WARNING2)
COMMAND_DATA(WARNING3)
COMMAND_DATA(WARNING4)
COMMAND_DATA_END()

COMMAND_DATA_START(preserve)
COMMAND_DATA(NOPRESERVE)
COMMAND_DATA_END()

COMMAND_DATA_START(warn)
COMMAND_DATA(DEFAULT)
COMMAND_DATA(ENABLE)
COMMAND_DATA(DISABLE)
COMMAND_DATA_END()

COMMAND_DATA_START(real)
COMMAND_DATA(RPNOFF)
COMMAND_DATA(RPNON)
COMMAND_DATA(COMMENTOFF)
COMMAND_DATA(COMMENTON)
COMMAND_DATA_END()

COMMAND_DATA_START(beaut)//Be sure to update GetState(enum beaut) if necessary
COMMAND_DATA_EX(__NOUSE,OFF)//OFF is already defined in enum san. Same numeric value is vital for this trick.
COMMAND_DATA(ON)
COMMAND_DATA(MAXLEN)
COMMAND_DATA(QUOTEHIGHASCII)
COMMAND_DATA(QUOTEUTF8)
COMMAND_DATA(HEXGRFID)
COMMAND_DATA(LEADINGSPACE)
COMMAND_DATA(CONVERTONLY)
COMMAND_DATA(LINEBREAKS)
COMMAND_DATA(GETCOOKIE)
COMMAND_DATA(SETCOOKIE)
COMMAND_DATA(USEESCAPES)
COMMAND_DATA_END()

COMMAND_DATA_START(ext)
COMMAND_DATA_EX(__NOUSE2,OFF)
COMMAND_DATA_EX(__NOUSE3,ON)
//COMMAND_DATA(INOUTPUT)
COMMAND_DATA_END()

#ifndef _RENUM_COMMAND_H_FUNCTIONS_INCLUDED_
#define _RENUM_COMMAND_H_FUNCTIONS_INCLUDED_

int GetState(enum gen);
uint GetState(enum beaut,int =0);
bool GetWarn(int,int);
bool is_command(const string&);
bool parse_comment(const string&);
void reset_commands();
int DoCalc(const string&,size_t&);
int DoCalc(istream&,int&);
bool CLCommand(int);

#endif//_RENUM_COMMAND_H_FUNCTIONS_INCLUDED_

#endif//_RENUM_COMMAND_H_INCLUDED_
