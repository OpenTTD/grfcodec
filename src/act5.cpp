/*
 * act5.cpp
 * Contains definitions for checking action 5.
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

#include<string>
#include<fstream>
#include<cassert>
#include<errno.h>

using namespace std;

#include"nforenum.h"
#include"sanity.h"
#include"inlines.h"
#include"ExpandingArray.h"
#include"sanity_defines.h"
#include"data.h"
#include"pseudo.h"
#include"messages.h"
#include"command.h"

extern bool _base_grf;

class c5{
public:
	int maxFeature(){return (int)sizes.size()+3;}
	const vector<int>&operator[](int x)const {return sizes[x-4];}
	SINGLETON(c5)
private:
	vector<vector<int> >sizes;
};

c5::c5(){
	FILE*pFile=myfopen(5);
	vector<int> temp;
	int ch, count, opts, flags;
	while((ch=GetCheckByte(5))!=0){
		flags = 0;
		if(ch&0x80){
			flags = ch&0x7F;
			ch=GetCheckByte(5);
		}
		count = ch>>4;
		opts = ch&0xF;
		for(int i=count;i;i--){
			temp.clear();
			temp.push_back(flags);
			for(int j=opts;j;j--)
				if(flags&4) temp.push_back(GetCheckWord(5));
				else temp.push_back(GetCheckByte(5));
			sizes.push_back(temp);
		}
	}
	fclose(pFile);
}

//	I finally want to do runtime-generated varargs calls.
// But I can't, so I have to manually generate the string instead.
void Act5CountWarn(const vector<int>&sizes){
	string str = mysprintf("%S",ACT5_SIZE, sizes[1], sizes[1]);
	int count=(int)sizes.size()-1;
	switch(count){
	case 1:
		break;
	default:
		for(int i=2;i<count;i++)
			str += ", "+mysprintf("%S",ACT5_SIZE, sizes[i], sizes[i]);
		str+=",";
		//fallthrough
	case 2:
		str += mysprintf("%S",ACT5_ORSIZE, sizes[count], sizes[count]);
	}
	IssueMessage(WARNING1, ACTION_5, str.c_str());
}

int Check5(PseudoSprite&data,sanstate&state){
	int feature=data.ExtractByte(1);
	bool hasoffset = (feature&0x80)!=0;
	feature&=0x7F;
	int sprites=data.ExtractExtended(2);
	uint off=2+data.ExtendedLen(2);
	state=FIND_REAL_OR_RECOLOR;
	if(feature<4||feature>c5::Instance().maxFeature()){
		IssueMessage(FATAL,INVALID_FEATURE);
		return sprites;
	}
	const vector<int>&expSprites=c5::Instance()[feature];
	if(!hasoffset){
		for(int i=(int)expSprites.size();--i;){		// Test [1] ... [.size()-1]
			if(expSprites[i] == 0)goto countok;
			if(expSprites[i] == sprites)goto countok;
		}
		/* A base GRF may provide 10 sprites for shores. */
		if(_base_grf && feature == 0x0D && sprites == 0x0A)goto countok;
		/* Having more sprites is generally okay for base GRFs as they are then more up-to-date than NFORenum. */
		if(_base_grf && feature == 0x15 && sprites > expSprites[0])goto countok;
		Act5CountWarn(expSprites);
	}else{
		if(!(expSprites[0]&8))
			IssueMessage(ERROR,CANNOT_EXTEND);
		int pastend=sprites + data.ExtractExtended(off);
		off+=data.ExtendedLen(off);
		if(expSprites[1] != 0 && expSprites[1] < pastend)
			IssueMessage(WARNING1, ACTION_5_LIMIT, sprites, pastend-1, expSprites[1]);
	}
countok:
	if(off<data.Length())IssueMessage(WARNING2,EXTRA_DATA,data.Length(),off);
	if(expSprites[0]&1)state=FIND_RECOLOR;
	else if(!(expSprites[0]&2))state=FIND_REAL;
	return sprites;
}
