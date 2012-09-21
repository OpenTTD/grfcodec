/*
 * pseudo.h
 * Declaration of the PsuedoSprite class. Stores and re-writes pseudo-sprites.
 *
 * Copyright 2006-2010 by Dale McCoy.
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

#ifndef _RENUM_PSEUDO_H_INCLUDED_
#define _RENUM_PSEUDO_H_INCLUDED_

#include<string>
#include"ExpandingArray.h"


class PseudoSprite{
public:
	//PseudoSprite();
	PseudoSprite(const string&,int);

	void CheckLinkage(int ofs,int count)const;
	// Use to read a single byte if you don't know/care about its meaning.
	// To read a field that happens to be one byte wide, use ExtractByte.
	uint LinkSafeExtractByte(uint)const;
private:
	void LinkBytes(int,size_t);
	Expanding0Array<int> linkage;
	static bool ignorelinkage;

public:
	// Use to read a single byte if you don't know/care about its meaning.
	// To read a field that happens to be one byte wide, use ExtractByte.
	uint operator[](uint offs)const{return LinkSafeExtractByte(offs);}
	operator const char*()const{return packed.c_str();}
	uint Length()const;
	bool IsValid()const{return Length()!=0;}

	PseudoSprite&SetText(uint);
	PseudoSprite&SetText(uint,uint);
	PseudoSprite&SetUTF8(uint,uint);
	PseudoSprite&SetOpByte(uint,char);
	PseudoSprite&SetPositionalOpByte(uint,char);
	PseudoSprite&SetDate(uint,uint);

	PseudoSprite&SetQEscape(uint);
	PseudoSprite&SetQEscape(uint,uint);
private:
	PseudoSprite&SetEscape(uint,bool,string,uint);

public:
	PseudoSprite&SetHex(uint);
	PseudoSprite&SetHex(uint,uint);
	PseudoSprite&SetAllHex();
	PseudoSprite&SetEot(uint);
	PseudoSprite&SetEol(uint,uint,uint);
	PseudoSprite&SetEol(uint loc,uint minbreaks){
		return SetEol(loc,minbreaks,(minbreaks>1)?1:0);
	}
	PseudoSprite&SetNoEol(uint);
	PseudoSprite&SetGRFID(uint);

	PseudoSprite&SetDec(uint,uint);
	PseudoSprite&SetBE(uint,uint);

	PseudoSprite&SetByteAt(uint,uint);
	PseudoSprite&SetWordAt(uint,uint);
	PseudoSprite&SetDwordAt(uint,uint);
	PseudoSprite&Append(uchar);

	PseudoSprite&ColumnAfter(uint);

	uint ExtractByte(uint offs)const;
	uint ExtractWord(uint offs)const;
	uint ExtractExtended(uint offs)const;
	uint ExtractDword(uint offs)const;
	uint ExtendedLen(uint)const;
	uint ExtractVariable(uint,uint)const;

	uint ExtractUtf8(uint&, bool&); // in utf8.cpp.

	uint ExtractQEscapeByte(uint offs){
		SetQEscape(offs);
		return ExtractByte(offs);
	}
	uint ExtractEscapeWord(uint offs){
		SetBE(offs,2);
		return ExtractWord(offs);
	}
	void AddComment(const string&,uint);
	void TrailComment(const string&,uint);
	void AddBlank(uint);

	void NoBeautify();

	friend ostream&operator<<(ostream&,PseudoSprite&);
private:
	ostream&output(ostream&);

public:
	static bool CanQuote(uint);
	static bool MayBeSprite(const string&);
	enum width {_B_, _BX_, _W_, _D_};
	uint ReadValue(istream&, width);

private:
	bool DoQuote(uint)const;
	bool IsText(uint)const;
	bool IsUTF8(uint)const;
	bool IsEot(uint)const;
	bool IsLinePermitted(uint)const;
	bool NoPrint(uint)const;

	void Invalidate();
	bool UseOrig()const;

	string orig,packed;
	Expanding0Array<uchar>beauty;
	ExpandingArray<string>context,ext_print;
	bool valid,useorig;
	const int oldspritenum;

// Support for sequential access
public:
#define PS_LOC_REF(size)		\
	class size;					\
	PseudoSprite& operator >>(size&); \
	PseudoSprite& Extract(size&, uint);	\
	class size {				\
	public:						\
		size();					\
		size& set(uint);		\
		uint val() const;		\
		uint loc() const;		\
		operator uint() const {return val();} \
		friend PseudoSprite& PseudoSprite::operator >>(size&); \
		friend PseudoSprite& PseudoSprite::Extract(size&, uint); \
	private:					\
		PseudoSprite *p;		\
		uint offs;				\
	};							\

	PS_LOC_REF(Byte)
	PS_LOC_REF(Word)
	PS_LOC_REF(ExtByte)
	PS_LOC_REF(Dword)

#undef PS_LOC_REF

	uint BytesRemaining() const;
	PseudoSprite& seek(uint);

private:
	uint extract_offs;
};

#endif//_RENUM_PSEUDO_H_INCLUDED_
