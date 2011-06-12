/*
 * inject.cpp
 * Contains functions for injecting new sprites into the NFO.
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

#include<queue>
#include<istream>
#include<string>
#include<cassert>

using namespace std;

#include"globals.h"
#include"inject.h"

static queue<string>_injected;
static const istream*_into;

istream&inj_getline(istream&in,string&str){
	assert(&in==_into);
	if(_injected.size()){
		str=_injected.front();
		_injected.pop();
		return in;
	}
	return getline(in,str);
}

int peek(istream&in){
	assert(&in==_into);
	if(&in==_into&&_injected.size())return _injected.front()[0];
	return in.peek();
}

void inject(const string&str){
	if(_into==NULL)(*pOut)<<str<<endl;
	else _injected.push(str);
}

void inject_into(const istream&into){
	_into=&into;
	while(!_injected.empty())_injected.pop();
}
