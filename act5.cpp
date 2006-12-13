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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include<string>
#include<fstream>
#include<cassert>
#include<errno.h>

using namespace std;

#include"renum.h"
#include"sanity.h"
#include"inlines.h"
#include"ExpandingArray.h"
#include"sanity_defines.h"
#include"data.h"
#include"pseudo.h"
#include"messages.h"
#include"command.h"

class c5:public Guintp{
public:
	int maxFeature;
	SINGLETON(c5)
};

c5::c5(){
	FILE*pFile=myfopen(5);
	_p=new uint[(maxFeature=GetCheckByte(5))+1];
	for(int i=5;i<=maxFeature;i++)
		_p[i]=GetCheckWord(5);
	fclose(pFile);
}


int Check5(PseudoSprite&data,sanstate&state){
	int length=(int)data.Length();
	if(CheckLength(length,(data.ExtractByte(2)==0xFF)?5:3,INVALID_LENGTH,ACTION,5,ONE_OF,3,5))return-1;
	int feature=data.ExtractByte(1);
	uint sprites=data.ExtractExtended(2);
	state=FIND_REAL;
	if(feature<4||feature>c5::Instance().maxFeature){
		IssueMessage(FATAL,INVALID_FEATURE);
		state=FIND_REAL_OR_RECOLOR;
		return sprites;
	}
	if(feature==4){
		if(sprites!=48&&sprites!=112&&sprites!=240)IssueMessage(ERROR,SIGNALS);
	}else{
		uint expSprites=c5::Instance()[feature];
		if(sprites!=expSprites)IssueMessage(ERROR,ACTION_5,expSprites,expSprites);
	}
	if(feature==0x0A)state=FIND_RECOLOR;
	return sprites;
}
