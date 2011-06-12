/*
 * actB.cpp
 * Contains definitions for checking action Bs.
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

#include<fstream>
#include<string>

using namespace std;

#include"nforenum.h"
#include"inlines.h"
#include"sanity_defines.h"
#include"messages.h"
#include"data.h"
#include"strings.h"
#include"pseudo.h"
#include"command.h"

class B{
public:
	uint maxSeverity,numMessages;
	AUTO_ARRAY(uchar);
	SINGLETON(B);
};

B::B(){
	FILE*pFile=myfopen(B);
	maxSeverity=GetCheckByte(B);
	_p=new uchar[numMessages=GetCheckByte(B)];
	myfread(_p,numMessages,B);
	fclose(pFile);
}

void CheckB(PseudoSprite&data){
	uint sev=data.ExtractByte(1),lang=data.ExtractByte(2),messageid=data.ExtractByte(3);
	uint offset=4;
	int specials;
	if((sev&0x7F)>B::Instance().maxSeverity)IssueMessage(ERROR,INVALID_SEVERITY);
	if(_grfver){
		if(lang&0x80||(_grfver<7&&lang&0x60&&(lang&0x7F)!=0x7F))IssueMessage(WARNING2,UNKNOWN_LANG_BIT,2,lang);
		lang&=0x7F;
		if(_grfver>6)CheckLangID(lang,2);
	}else if((lang&=0x7F)>0x1F){
		_grfver=7;
		CheckLangID(lang,2);
		_grfver=0;
	}
	if(messageid!=0xFF&&messageid>=B::Instance().numMessages){
		IssueMessage(FATAL,INVALID_MESSAGEID);
		return;
	}
	if(messageid==0xFF)
		specials=CheckString(data,offset,CTRL_NEWLINE,false,MakeStack(4,STACK_TEXT,STACK_TEXT,STACK_DWORD,STACK_DWORD),RETURN_STACK);
	else
		specials=B::Instance()[messageid];
	if(specials==-1)return;
	if(specials>1){
		try{
			if(messageid==0xFF&&data.ExtractByte(offset))data.SetEol(offset-1,1);
			else data.SetNoEol(offset-1);
		}catch(uint){}
		CheckString(data,offset,CTRL_NEWLINE);
	}
	if(specials>2){
		if(data.ExtractByte(offset)>0x7F)
			IssueMessage(ERROR,INVALID_PARAM,offset,data.ExtractByte(offset));
		offset++;
	}
	if(specials>3){
		if(data.ExtractByte(offset)>0x7F)
			IssueMessage(ERROR,INVALID_PARAM,offset,data.ExtractByte(offset));
		offset++;
	}
	if(offset!=data.Length())IssueMessage(WARNING2,EXTRA_DATA,data.Length(),offset);
}
