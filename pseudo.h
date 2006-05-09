/*
 * pseudo.h
 * Declaration of the PsuedoSprite class. Stores and re-writes pseudo-sprites.
 *
 * Copyright 2006 by Dale McCoy.
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

#ifndef _RENUM_PSEUDO_H_INCLUDED_
#define _RENUM_PSEUDO_H_INCLUDED_

#include<string>
#include"ExpandingArray.h"


class PseudoSprite{
public:
	//PseudoSprite();
	PseudoSprite(const string&,int);

	uint operator[](uint offs)const{return ExtractByte(offs);}
	uint Length()const;
	bool IsValid()const{return Length()!=0;}

	PseudoSprite&SetText(uint);
	PseudoSprite&SetText(uint,uint);
	PseudoSprite&SetUTF8(uint,uint);
	PseudoSprite&SetHex(uint);
	PseudoSprite&SetAllHex();
	PseudoSprite&SetEot(uint);
	PseudoSprite&SetEol(uint,uint);
	PseudoSprite&SetNoEol(uint);
	PseudoSprite&SetGRFID(uint);

	PseudoSprite&SetDec(uint,uint){return*this;}
	PseudoSprite&SetBE(uint,uint){return*this;}

	PseudoSprite&SetByteAt(uint,uint);

	PseudoSprite&PadAfter(uint,uint);

	uint ExtractByte(uint offs)const;
	uint ExtractWord(uint offs)const;
	uint ExtractExtended(uint offs)const;
	uint ExtractDword(uint offs)const;
	uint ExtendedLen(uint)const;
	uint ExtractVariable(uint,uint)const;

	void AddComment(const string&,uint);
	void TrailComment(const string&,uint);
	void AddBlank(uint);

	void NoBeautify();

	friend ostream&operator<<(ostream&,const PseudoSprite&);
private:
	ostream&output(ostream&)const;

public:
	static bool CanQuote(uint);
	static bool MayBeSprite(const string&);

private:
	bool IsText(uint)const;
	bool IsUTF8(uint)const;
	bool IsEot(uint)const;
	bool IsLinePermitted(uint)const;

	void Invalidate();
	bool UseOrig()const;

	string orig,packed;
	Expanding0Array<char>beauty;
	ExpandingArray<string>context;
	bool valid,useorig;
	const int spriteno;
};

#endif//_RENUM_PSEUDO_H_INCLUDED_
