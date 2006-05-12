/*
 * strings.cpp
 * Contains definitions for checking strings in actions 4, 8, and B.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include<cstdio>
#include<string>
#include<cerrno>

using namespace std;

#include"renum.h"
#include"inlines.h"
#include"sanity_defines.h"
#include"pseudo.h"
#include"strings.h"
#include"data.h"
#include"utf8.h"
#include"messages.h"
#include"command.h"

class check4:public Guintp{
public:
	int GetGenericPerms(int feature);
	int GetNamePerms(int feature);
	SINGLETON(check4)
};

check4::check4(){
	FILE*pFile=myfopen(4);
	_p=new uint[MaxFeature()+1];
	uint i=0;
	for(;i<=MaxFeature();i++)
		_p[i]=fgetc(pFile);
	for(i=0;i<=MaxFeature();i++)
		_p[i]|=GetCheckByte(4)<<8;
	fclose(pFile);
}

int check4::GetGenericPerms(int feature){return _p[feature]>>8;}
int check4::GetNamePerms(int feature){return _p[feature]&0xFF;}

void Check4(PseudoSprite&data){
	const uint feature=data.ExtractByte(1),lang=data.ExtractByte(2);
	data.SetAllHex();
	uint i=5;
	int nument=(signed)data.ExtractByte(3);
	if(!IsValidFeature(ACT4,feature)&&feature!=0x48)IssueMessage(ERROR,INVALID_FEATURE);
	if(_grfver<7&&lang&0x60&&(lang&0x7F)!=0x7F)IssueMessage(WARNING3,UNKNOWN_LANG_BIT,2,lang);
	if(_grfver>6)CheckLangID(lang,2);
	if(lang&0x80){
		i=6;
		CheckTextID(0x48,data.ExtractWord(4),4)&&CheckTextID(0x48,data.ExtractWord(4)+nument-1,3);
	}else if(feature==0x48)IssueMessage(ERROR,INVALID_FEATURE);
	else{
		CheckID(feature,data.ExtractByte(4))&&CheckID(feature,data.ExtractByte(4)+nument-1);
	}
	int perms=feature==0x48?CTRL_ALL|CTRL_NO_STACK_CHECK
		:(check4::Instance().*(lang&0x80?&check4::GetGenericPerms:&check4::GetNamePerms))(feature);
	if(feature==0x0B){
		for(;nument--;){
			int result=CheckString(data,i,perms,!nument,MakeStack(1,STACK_WORD));
			if(result){
				if(result!=-1)nument--;
				break;
			}
			try{
				if(data[i])data.SetEol(i-1,1);
			}catch(uint){}
		}
	}else
		for(;nument--||_autocorrect;){
			int result=CheckString(data,i,perms,!nument);
			if(result){
				if(result!=-1)nument--;
				break;
			}
			try{
				if(data[i])data.SetEol(i-1,1);
				else data.SetNoEol(i-1);
			}catch(uint){}
		}
	if(++nument){
		if(_autocorrect){
			IssueMessage(0,CONSOLE_AUTOCORRECT,_spritenum);
			IssueMessage(0,AUTOCORRECTING,3,"num-ent",data.ExtractByte(3),data.ExtractByte(3)-nument);
			data.SetByteAt(3,uchar(data.ExtractByte(3)-nument));
		}else
			IssueMessage(ERROR,INSUFFICIENT_STRINGS,nument);
	}
	if(i<data.Length())IssueMessage(WARNING2,EXTRA_DATA,i);
}

#define STACK_CHECK(type,width)\
	if(stackoffs+width>stack.length()){\
		IssueMessage(WARNING1,OVERRAN_STACK,offs,ch);\
		perms|=CTRL_NO_STACK_CHECK;\
	}else if((stack[stackoffs]&0x0F)!=type){\
		IssueMessage(WARNING1,SMASH_STACK,offs,ch,stack[stackoffs]);\
		perms|=CTRL_NO_STACK_CHECK;\
	}else if((stack[stackoffs]&0xF0)!=(stack[stackoffs+width-1]&0xF0)){\
		IssueMessage(WARNING1,SMASH_STACK_SPAN,offs,ch);\
		perms|=CTRL_NO_STACK_CHECK;\
	}\
	stackoffs+=width;\
	break;

/* Checks the string starting at at offs using perms (bitmask of CTRL_* in
 * string.h) and stack (returned from MakeStack) to control messages.
 * Returns with offs set to the ->next<- byte to process.
 * Return depends on retInfo: either the last character processed
 * (RETURN_NULL), or the number of stack-accessing control characters
 * encountered or -1 if the stack was smashed (RETURN_STACK)
 */
int CheckString(PseudoSprite&data,uint&offs,int perms,bool include_00_safe,string stack,const int retInfo){
	const uint length=data.Length();
	if(offs>=length)return -1;
	uint stackoffs=0,ret=0,ch;
	bool utf8=false,valid;
	try{
		utf8=data.ExtractWord(offs)==0x9EC3;
		if(utf8){
			data.SetUTF8(offs,2);
			offs+=2;
		}
	}catch(uint){}
	while(0!=(ch=GetUtf8Char(data,offs,utf8,valid))){
		if(ch>(uint)-10){
			IssueMessage(ERROR,OUTOFRANGE_UTF8,offs+(int)ch);
			continue;
		}
		offs--;//back up to last-parsed; move back to next-to-parse at end of loop (line ~212)
		if(utf8){
			if(ch<128){
				data.SetText(offs);
				if(ch>0x7A)ch=0x20;//bypass the control-char checks for 7B..7F
			}else if(ch<256){
				if(!valid){//invalid UTF-8, parse as bytes
					if((/*ch>0x7A&&*/ch<0xA1)||ch==0xAA||ch==0xAC||ch==0xAD||ch==0xAF||(ch>0xB3&&ch<0xB9))
						data.SetHex(offs);//nonprintable -- control or special.
					else data.SetText(offs);
				}else ch=0x20;//valid UTF-8 encoding of U+0080..U+00FF; bypass control-char checks
			}else if(ch>0xE07A&&ch<0xE100)ch&=0xFF;//UTF-8 encoding of U+E07B..U+E0FF; run control-char checks
		}else{//!utf8
			if((ch>0x7A&&ch<0xA1)||ch==0xAA||ch==0xAC||ch==0xAD||ch==0xAF||(ch>0xB3&&ch<0xB9))
				data.SetHex(offs);//nonprintable -- control or special.
			else data.SetText(offs);
		}
		if(ch==1){
			if(~perms&CTRL_SPACE)IssueMessage(WARNING1,INVALID_CONTROL,offs,ch);
			if(!data.ExtractByte(++offs)&&!include_00_safe)
				IssueMessage(WARNING1,EMBEDDED_00,offs);
		}else if(ch==0x0D){
			if(~perms&CTRL_NEWLINE)IssueMessage(WARNING1,INVALID_CONTROL,offs,ch);
			try{
				if(data[offs+1]!=0x0D)data.SetEol(offs,2);
				else data.SetNoEol(offs);
			}catch(uint){}
		}
		else if(ch==0x0E){if(~perms&CTRL_FONT_SMALL)IssueMessage(WARNING1,INVALID_CONTROL,offs,ch);}
		else if(ch==0x0F){if(~perms&CTRL_FONT_LARGE)IssueMessage(WARNING1,INVALID_CONTROL,offs,ch);}
		else if(ch==0x1F){
			if(~perms&CTRL_SPACE)IssueMessage(WARNING1,INVALID_CONTROL,offs,ch);
			if(!data.ExtractByte(++offs)&&!include_00_safe)
				IssueMessage(WARNING1,EMBEDDED_00,offs);
			if(!data.ExtractByte(++offs)&&!include_00_safe)
				IssueMessage(WARNING1,EMBEDDED_00,offs);
		}else if(ch<0x20)IssueMessage(WARNING3,UNUSED_CONTROL,offs,ch);
		else if(ch<0x7B);
		else if(ch==0x81){
			int id=data.ExtractWord(++offs);
			CheckTextID(0x49,id,offs);
			if((!(id&0xFF)||!(id&0xFF00))&&!include_00_safe)
				IssueMessage(WARNING1,INCLUDING_00_ID,offs,id);
			++offs;
		}else if(ch==0x86){
			if(~perms&CTRL_NO_STACK_CHECK){
				if(stack.length()<8){
					IssueMessage(WARNING1,CANNOT_SHUFFLE,offs);
					perms|=CTRL_NO_STACK_CHECK;
				}else{
					swap(stack[6],stack[0]);
					swap(stack[7],stack[1]);
					swap(stack[4],stack[6]);
					swap(stack[5],stack[7]);
					swap(stack[2],stack[4]);
					swap(stack[3],stack[5]);
				}
			}
		}else if(ch<0x88){
			if(~perms&CTRL_NO_STACK_CHECK){
				switch(ch){
				case 0x7D:
					STACK_CHECK(STACK_BYTE,1)
				case 0x82:case 0x83:
					STACK_CHECK(STACK_DATE,2)
				case 0x7C:case 0x7E:case 0x84:case 0x85:case 0x87:
					STACK_CHECK(STACK_WORD,2)
				case 0x80:
					STACK_CHECK(STACK_TEXT,2)
				case 0x7B:case 0x7F:
					STACK_CHECK(STACK_DWORD,4)
				DEFAULT(ch)
				}
				++ret;
			}
		}else if(ch<0x99){
			if(~perms&CTRL_COLOR)IssueMessage(WARNING1,INVALID_CONTROL,offs,ch);
		}else if(ch==0x99){
			if(~perms&CTRL_COLOR)IssueMessage(WARNING1,INVALID_CONTROL,offs,ch);
			offs++;
		}else if(ch<0x9E)IssueMessage(WARNING3,UNUSED_CONTROL,offs,ch);
		if(++offs>=length)break;
	}
	if(ch)IssueMessage(WARNING1,NO_NULL_FOUND);
	if(retInfo==RETURN_NULL)return ch;
	if(retInfo==RETURN_STACK)return(perms&CTRL_NO_STACK_CHECK)?(unsigned)-1:ret;
	INTERNAL_ERROR(retInfo,retInfo);
}

static const uchar stackSize[]={0,2,2,2,4,2};

string MakeStack(int items,...){
	string ret;
	WrapAp(items);
	uint item;
	for(int i=0;i<items;i++){
		item=va_arg(ap.operator va_list&(),uint);
		//             ^^^^^^^^^^^^^^^^^^^
		// gcc complains without that call.
		VERIFY(item&&item<STACK_INVALID,item);
		ret+=string(stackSize[item],char(item|i<<4));
	}
	return ret;
}

/********************************************************
 * Lang ID code
 *******************************************************/

struct langNames{
	string names[0x80];
	C_SINGLETON(langNames)
};

langNames::langNames(){
	FILE*pFile=myfopen(langs);
	char buffer[102];
	for(uint i=0;i<0x80;i++){
		fgets(buffer,102,pFile);
		if(buffer[strlen(buffer)-1]!='\n'){
			if(strlen(buffer)==101)
				IssueMessage(0,DATAFILE_ERROR,LOAD,"langs.dat",OVERLENGTH_NAME,i);
			else
				IssueMessage(0,DATAFILE_ERROR,LOAD,"langs.dat",EOF_READING_NAME,i);
			assert(false);
			exit(EDATA);
		}
		buffer[strlen(buffer)-1]='\0';
		names[i]=buffer;
	}
}

const char*_unknownLanguage="Unknown Language (%2x)";

void CheckLangID(uint id,uint offs){
	VERIFY(_grfver>6,_grfver);
	if(GetLangName(id)==_unknownLanguage)IssueMessage(WARNING2,UNKNOWN_LANGUAGE,offs,id);
}

string GetLangName(uint id){
	VERIFY(_grfver>6,_grfver);
	VERIFY(id<0x80,id);
	if(langNames::Instance().names[id]!="")return langNames::Instance().names[id];
	return _unknownLanguage;
}
