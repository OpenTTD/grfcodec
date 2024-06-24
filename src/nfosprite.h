/*
 * nfosprite.h
 * A collection of classes for parsing and sprites from an .nfo file.
 *
 * Copyright 2006 by Dale McCoy.
 * dalestan@gmail.com
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

#ifndef _NFOSPRITE_H
#define _NFOSPRITE_H

#include <istream>
#include <vector>

#include"typesize.h"
#include"sprites.h"

typedef unsigned short ushort;

class Sprite{
protected:
	Sprite(){}
public:
	virtual ~Sprite(){}
	enum SpriteType{ST_REAL,ST_PSEUDO,ST_INCLUDE};
	virtual SpriteType GetType()const =0;
	class unparseable{//thrown by the constructors
	public:
		unparseable(std::string reason,size_t sprite);
		std::string reason;
	};
};

class Real:public Sprite{
public:
	void AddSprite(size_t,int,const std::string&);
	Sprite::SpriteType GetType()const{return ST_REAL;}
	std::vector<SpriteInfo> infs;
private:
	std::ostream&output(std::ostream&)const;
	static std::string prevname;
	static int prevy;
};

class Pseudo:public Sprite{
public:
	Pseudo(size_t,int,int,const std::string&,int);

	U8 operator[](int offs)const{return packed[offs];}
	uint size()const;

	Sprite::SpriteType GetType()const{return ST_PSEUDO;}
	const char*GetData()const{return packed.c_str();}

	//static bool CanQuote(uint);
	static bool MayBeSprite(const std::string&);
	enum width {_B_, _BX_, _W_, _D_};

	static uint ReadValue(std::istream&, width);

private:
	std::string packed;
};

class Include:public Sprite{
public:
	Include(const std::string&);
	Sprite::SpriteType GetType()const{return ST_INCLUDE;}
	const char *GetName()const{return name.c_str();}
private:
	std::string name;
};

#endif /* _NFOSPRITE_H */
