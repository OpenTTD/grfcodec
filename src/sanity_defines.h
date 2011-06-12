/*
 * sanity_defines.h
 * Internal sprite linting stuff.
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

#ifndef _RENUM_SANITY_DEFS_H_INCLUDED_
#define _RENUM_SANITY_DEFS_H_INCLUDED_

#include <cstdarg>
#include "message_mgr.h"

bool CheckLength(int,int,RenumMessageId,...);
bool CheckTextID(uint,uint,uint);
bool CheckID(uint,uint);

void Init0();
void Init123();
void final123();
void Init7();
void final7();
void InitF();
void finalF();

bool IsLabel(uint);
//int GetBit(const string&);

enum ActBit{ACT0=1,ACT1=2,ACT3=4,ACT4=8,EMPTY1=0x10,OVERRIDE3=0x20,GENERIC3=0x40,ACT3_BEFORE_PROP08=0x80};
enum sanstate{UNKNOWN,FIND_PSEUDO,FIND_REAL,FIND_INCLUDE,FIND_RECOLOR,FIND_REAL_OR_RECOLOR};

bool IsValidFeature(int actbits,uint feat);
bool IsValid2Feature(uint feat);
bool IsProp08Set(uint feat,uint id);
uchar Get2Type(uint feat);
uint MaxFeature();

template<typename _Ty>class auto_array{//can't use auto_ptr because I use new[]/delete[]
public:
	auto_array():_p(NULL){}
	~auto_array(){delete[]_p;}
	operator _Ty*&(){return _p;}
	operator const _Ty*()const{return _p;}
	const _Ty* operator=(_Ty*p){return _p=p;}
protected:
	_Ty*_p;
private:
	void operator=(const auto_array<_Ty>&);
	auto_array(const auto_array&);
};

typedef auto_array<uint> Guintp;

#define AUTO_ARRAY(type)\
	auto_array<type>_p;\
	type&operator[](uint x){return _p[x];}\
	type operator[](uint x)const{return _p[x];}\

class apWrapper{
private:
	va_list _ap;
public:
	~apWrapper(){va_end(_ap);}
	operator va_list&(){return _ap;}
	operator const va_list&()const{return _ap;}
#ifdef __va_copy
	va_list&operator=(va_list&ap){
	    __va_copy(_ap,ap);
		return _ap;
	}
#else
	va_list const&operator=(va_list const&ap){
	    return _ap=ap;
	}
#endif
};
#define WrapAp(v)\
	apWrapper ap;\
	va_start(ap,v);

#endif//_RENUM_SANITY_DEFS_H_INCLUDED_
