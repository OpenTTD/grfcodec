/*
 * IDs.cpp
 * ID checking for actions 0-4.
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
#include<fstream>
#include<errno.h>
#include<cstdlib>

using namespace std;

#include"nforenum.h"
#include"inlines.h"
#include"messages.h"
#include"sanity_defines.h"
#include"data.h"
#include"command.h"
#include"ExpandingArray.h"

class IDs:public Guintp{
public:
	SINGLETON(IDs);
};

IDs::IDs(){
	FILE*pFile=myfopen(IDs);
	_p=new uint[MaxFeature()+1];
	for(uint i=0;i<=MaxFeature();i++)
		_p[i]=GetCheckWord(IDs);
	fclose(pFile);
}

class TextIDs{
public:
	uint idClasses[0x20],numspecials;
	bool CheckID(uint,uint);
	void Define(uint);
	bool IsDefined(uint);
	static void Clear();
	AUTO_ARRAY(uchar);
	SINGLETON(TextIDs);
private:
	static Expanding0Array<bool> _m;
};
Expanding0Array<bool> TextIDs::_m;

TextIDs::TextIDs(){
	FILE*pFile=myfopen(TextIDs);
	uint i=0;
	for(;i<0x20;i++)
		idClasses[i]=GetCheckWord(TextIDs);
	_p=new uchar[numspecials=GetCheckByte(TextIDs)];
	myfread(_p,numspecials,TextIDs);
	fclose(pFile);
}

bool TextIDs::CheckID(uint feature,uint ID){
	if((ID&0xF000)==0xD000){
		if(feature==0x48)return ID<0xD400||(ID>0xDBFF&&ID<0xDD00);
		return ID>0xD3FF&&ID<0xD800;
	}
	if(feature==0x49&&ID==0xC7FF)return true;
	if(idClasses[ID>>11]==0xFFFF){
		for(uint i=0;i<numspecials;i++)if(ID>>8==_p[i])return true;
		return false;
	}
	return(ID&0x7FF)<idClasses[ID>>11];
}

void TextIDs::Define(uint ID){
	if(idClasses[ID>>11]==0xFFFF){
		ID-=0xC000;
		_m[ID]=true;
	}
}
bool TextIDs::IsDefined(uint ID){
	if(idClasses[ID>>11]==0xFFFF)
		return const_cast<const Expanding0Array<bool>& >(_m)[ID-0xC000];
	return (ID&0x7FF)<idClasses[ID>>11];
}

void TextIDs::Clear() {
	_m.clear();
}

void ClearTextIDs() {
	TextIDs::Clear();
}

bool CheckTextID(uint feature,uint ID,uint offset){
	VERIFY(feature==0x48||feature==0x49,feature);
	VERIFY(ID<0x10000,ID);
	if(TextIDs::Instance().CheckID(feature,ID)){
		if (feature==0x48) TextIDs::Instance().Define(ID);
		return true;
	}
	IssueMessage(ERROR,INVALID_TEXTID,offset,ID);
	return false;
}

bool IsTextDefined(uint ID) {
	return TextIDs::Instance().IsDefined(ID);
}

bool CheckID(uint feature,uint ID){
	VERIFY(feature<=MaxFeature(),feature);
	VERIFY(ID<0x10000,ID);
	/* Since OpenTTD 0.7 higher vehicle IDs are allowed; GRF version 8
	 * is the first new GRF version after that, so use that as minimum
	 * threshold for allowing the higher IDs. Even though this is not
	 * actually tied to GRF version 8.*/
	uint maxID=_grfver>=8&&feature<=0x03?0xFFFF:IDs::Instance()[feature];
	if(ID>maxID){
		IssueMessage(ERROR,INVALID_ID,ID,feature==0x0C?0x49:0,maxID);
		return false;
	}
	if(feature==0x0C&&ID<0x49){
		IssueMessage(ERROR,INVALID_ID,ID,0x49,0xFFFF);
		return false;
	}
	return true;
}
