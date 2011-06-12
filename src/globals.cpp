/*
 * globals.cpp
 * Defines global variables and constants for renum.
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

#include<iostream>
#include<string>

using namespace std;

#include "globals.h"

ostream*pNfo=&cout,*pOut,*pErr=&cerr;

const char*const VALID_PSEUDO="0123456789ABCDEFabcdef \t\v\r\n",*const WHITESPACE=" \t\v\r\n";
const string COMMENT="/;#";
string datadir;
const char*COMMENT_PREFIX="//";
bool dosleep=true;
unsigned int _spritenum,_grfver,_autocorrect=0,_act14_pal;
