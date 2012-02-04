/*
 * inlines.h
 * Contains inlinable fuctions for renum
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


#ifndef _RENUM_INLINES_H_INCLUDED_
#define _RENUM_INLINES_H_INCLUDED_

#include<iostream>
#include<cstdarg>
#include<cassert>
#include<cstring>
#include<climits>

#include"globals.h"
#include"nforenum.h"

inline string safetostring(const char*ptr){return ptr?ptr:"";}

inline istream&eat_white(istream&in){
	while(isspace(in.peek()))in.ignore();
	return in;
}

inline void strip_trailing_white(string&str){
	string::iterator last = str.end();
	for (string::iterator i = str.begin(); i != str.end(); i++)
		if (!isspace(*i)) last = i;
	if (last != str.end()) str.erase(last+1, str.end());
}

inline int ctoi(char ch){
	if(ch>='0'&&ch<='9')return ch-'0';
	if(ch>='A'&&ch<='F')return ch-'A'+10;
	if(ch>='a'&&ch<='f')return ch-'a'+10;
	return 0;
}

inline bool is_comment(istream&in){
	if(strchr("#;",in.peek()))return true;
	if(in.peek()!='/')return false;
	in.ignore();
	if(in.peek()=='/'){
		in.unget();
		return true;
	}
	in.unget();
	return false;
}

inline bool is_comment(const string&str,size_t off){
	if(strchr("#;",str[off]))return true;
	if(str[off]!='/'||str[off+1]!='/') return false;
	return true;
}

inline bool is_comment(const string&line){
	return line==""||is_comment(line,line.find_first_not_of(WHITESPACE));
}

inline string UCase(string str){
	size_t len=str.length();
	for(size_t i=0;i<len;i++)
		str[i]=(char)toupper(str[i]);
	return str;
}

inline string itoa(uint x,uint radix=10,uint minlen=1){
	string ret;
	if(radix==256){
		while(x||minlen--){
			ret+=char(x&0xFF);
			x>>=8;
		}
		return ret;
	}
	if(!x)ret="0";
	assert(radix<=16);
	while(x){
		ret="0123456789ABCDEF"[x%radix]+ret;
		x/=radix;
	}
	while(ret.length()<minlen)
		ret=((radix==10)?' ':'0')+ret;
	return ret;
}

inline string itoa(int x,uint radix=10,uint minlen=1){
	if(x>=0)return itoa((uint)x,radix,minlen);
	if(x==INT_MIN)return '-'+itoa(uint(INT_MAX)+1,radix,minlen);
	else return '-'+itoa((uint)-x,radix,minlen);
}

inline uint ReadHex(istream&in,uint digits){
	uint ret;
	while(isspace(in.peek()))in.ignore();
	char ch;
	in.get(ch);
	if((ret=ctoi(ch))==0&&ch!='0'){
		in.unget().clear(ios::badbit);
		return ret;
	}
	for(;--digits;){
		in.get(ch);
		if(ctoi(ch)==0&&ch!='0'){
			in.unget();
			return ret;
		}
		ret<<=4;
		ret|=ctoi(ch);
	}
	return ret;
}

#endif//_RENUM_INLINES_H_INCLUDED_
