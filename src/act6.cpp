/*
 * act6.cpp
 * Contains definitions for checking action 6.
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

#include<string>
#include<cassert>

using namespace std;

#include"nforenum.h"
#include"inlines.h"
#include"sanity.h"
#include"messages.h"
#include"pseudo.h"
#include"command.h"

uint Check6(PseudoSprite&data){
	assert(data.ExtractByte(0)==0x06);
	uint ofs=1,minlen=0;
	bool canCorrect=true;
	try{
		while(data.ExtractByte(ofs++)!=0xFF){
			canCorrect=false;
			int num=data.ExtractByte(ofs++)&0x7F;
			if(num)minlen=std::max(minlen,num+data.ExtractExtended(ofs));
			else IssueMessage(WARNING1,DOES_NOT_MODIFY,ofs-1);
			ofs+=data.ExtendedLen(ofs);
			canCorrect=true;
		}
		if(ofs==2)
			IssueMessage(WARNING1,NO_MODIFICATIONS);
		if(ofs!=data.Length())
			IssueMessage(WARNING2,EXTRA_DATA,data.Length(),ofs);
	}catch(...){
		if(_autocorrect&&canCorrect){
			IssueMessage(0,CONSOLE_AUTOCORRECT,_spritenum);
			IssueMessage(0,AUTOCORRECT_ADD,0xFF);
			data.Append(0xFF);
		}else{
			IssueMessage(FATAL,UNTERM_ACT6);
		}
	}
	return minlen;
}
