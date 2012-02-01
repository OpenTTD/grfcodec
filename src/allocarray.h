/*
 * AllocArray.h
 * Defines a class for storing a collection of objects all derived from
 * the same base class
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

#ifndef _ALLOC_ARRAY_H
#define _ALLOC_ARRAY_H

#ifdef _MSC_VER
#pragma warning(disable:4702)//Unreachable code
//Yes. I have a C++ library in which <vector> contains unreachable code.
#include<vector>
#pragma warning(default:4702)
#else
#include <vector>
#endif

template<typename _Ty>class AllocArray:private std::vector<_Ty*>{
	typedef AllocArray<_Ty> _Myt;
	typedef std::vector<_Ty*> _Mybase;
	typedef typename _Mybase::size_type size_type;
	typedef typename _Mybase::iterator iterator;
	typedef typename _Mybase::reference reference;
	typedef typename _Mybase::const_reference const_reference;
public:
	AllocArray(){}
	~AllocArray(){
		for(size_type i=_Mybase::size();i;)
			delete operator[](--i);
	}
	template<typename _Cty>void push_back(const _Cty&val){
		_Mybase::push_back(new _Cty(val));
	}
	template<typename _Cty>void push_back(const _Cty*val){
		_Mybase::push_back(new _Cty(*val));
	}
	size_type size()const{
		return _Mybase::size();
	}
	reference last(){
		return operator[](size()-1);
	}
	reference operator[](size_type x){
		return _Mybase::operator [](x);
	}
	const_reference operator[](size_type x)const{
		return _Mybase::operator [](x);
	}
	void clear(){
		for(size_type i=_Mybase::size();i;)
			delete operator[](--i);
		_Mybase::clear();
	}
private:
	AllocArray(const _Myt&right);
	void operator=(const _Myt&right);
};

#endif /* _ALLOC_ARRAY_H */
