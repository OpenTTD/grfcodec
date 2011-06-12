/*
 * strings.h
 * Contains declarations for checking strings in actions 4, 8, and B.
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
 * along with this program; if not, write to the Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _RENUM_STRINGS_H_INCLUDED_
#define _RENUM_STRINGS_H_INCLUDED_

#define CTRL_FONT_LARGE 1
#define CTRL_FONT_SMALL 2
#define CTRL_FONT (CTRL_FONT_LARGE|CTRL_FONT_SMALL)
#define CTRL_SPACE 4
#define CTRL_NEWLINE 8
#define CTRL_COLOR 0x10
#define CTRL_ALL (CTRL_FONT|CTRL_SPACE|CTRL_NEWLINE|CTRL_COLOR)
#define CTRL_NO_STACK_CHECK 0x20

enum{STACK_BYTE=1,STACK_WORD,STACK_TEXT,STACK_DWORD,STACK_DATE,STACK_QWORD,STACK_INVALID};

enum{RETURN_NULL,RETURN_STACK};

class PseudoSprite;

void Check4(PseudoSprite&);
int CheckString(PseudoSprite&,uint&,int,bool =false,string="",int =0);
string MakeStack(int,...);
string GetLangName(uint);
void CheckLangID(uint,uint);

#endif//_RENUM_STRINGS_H_INCLUDED_
