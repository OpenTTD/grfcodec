/*
 * ExpandingArray.h
 * Subclasses of the vector class that expand to hold the data put in them.
 * Reading not-yet-defined data has a defined result, either a cast from 0,
 * or the result of the default constructor.
 *
 * Copyright 2004-2005 by Dale McCoy.
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

#ifndef _RENUM_EXPANDING_ARRAY_H_INCLUDED_
#define _RENUM_EXPANDING_ARRAY_H_INCLUDED_

#ifdef _MSC_VER
#pragma warning(disable:4702)//unreachable code
#include<vector>
#pragma warning(default:4702)
#else
#include<vector>
#endif
using namespace std;

template<typename _Ty>class ExpandingArray:public vector<_Ty>{
	typedef ExpandingArray<_Ty> _MyT;
	typedef vector<_Ty> _Mybase;
public:
	ExpandingArray(){}
	ExpandingArray(const _Ty&_fill):m_fill(_fill){}
	const _Ty&operator[](unsigned int x)const{
		if(x>=_MyT::size())return m_fill;
		return vector<_Ty>::operator[](x);
	}
	_Ty&operator[](unsigned int x){
		if(x>=_MyT::size())_MyT::resize(x+1);
		return _Mybase::operator[](x);
	}
	void ChangeFill(const _Ty&_fill){m_fill=_fill;}
private:
	_Ty m_fill;
};

template<typename _Ty>class Expanding0Array:public vector<_Ty>{
	typedef Expanding0Array<_Ty> _MyT;
public:
	_Ty operator[](unsigned int x)const{
		if(x>=_MyT::size())return 0;
		return vector<_Ty>::operator[](x);
	}
	_Ty&operator[](unsigned int x){
		if(x>=_MyT::size())_MyT::resize(x+1,0);
		return vector<_Ty>::operator[](x);
	}
};
template<>class Expanding0Array<bool>:public vector<bool>{
public:
	bool operator[](unsigned int x)const{
		if(x>=size())return false;
		return vector<bool>::operator[](x);
	}
	reference operator[](unsigned int x){
		if(x>=size())resize(x+1,false);
		return vector<bool>::operator[](x);
	}
};


#endif//_RENUM_EXPANDING_ARRAY_H_INCLUDED_
