/*
 * strings.cpp
 * Contains definitions for checking strings in actions 4, 8, and B.
 *
 * Copyright 2005-2009 by Dale McCoy.
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

#include<cstdio>
#include<string>
#include<cerrno>
#include<cstdlib>

using namespace std;

#include"nforenum.h"
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
	if(!IsValidFeature(ACT4,feature)&&feature!=0x48){IssueMessage(FATAL,INVALID_FEATURE);return;}
	if(_grfver<7&&lang&0x60&&(lang&0x7F)!=0x7F)IssueMessage(WARNING3,UNKNOWN_LANG_BIT,2,lang);
	if(_grfver>6)CheckLangID(lang&0x7F,2);
	if(lang&0x80){
		i=6;
		if(nument!=0) {
			uint base_id = data.ExtractWord(4);
			uint last_id = base_id + nument - 1;
			for (uint i = base_id; i <= last_id; i++) CheckTextID(0x48, i, 4);
		} else
			IssueMessage(WARNING1,NO_TEXTS);
	}else if(feature==0x48)IssueMessage(ERROR,INVALID_FEATURE);
	else if(feature>3)IssueMessage(ERROR,NO_BYTE_IDS,feature);
	else if(nument!=0){
		CheckID(feature,data.ExtractExtended(4))&&CheckID(feature,data.ExtractExtended(4)+nument-1);
		i+=data.ExtendedLen(4)-1;
	}else
		IssueMessage(WARNING1,NO_TEXTS);
	int perms=feature==0x48?CTRL_ALL|CTRL_NO_STACK_CHECK
		:(check4::Instance().*(lang&0x80?&check4::GetGenericPerms:&check4::GetNamePerms))(feature);
	if(feature==0x0B){
		for(;nument--||_autocorrect;){
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
			IssueMessage(0,AUTOCORRECTING,3,NUMENT,data.ExtractByte(3),data.ExtractByte(3)-nument);
			data.SetByteAt(3,data.ExtractByte(3)-nument);
		}else
			IssueMessage(ERROR,INSUFFICIENT_STRINGS,nument);
	}
	if(i<data.Length())IssueMessage(WARNING2,EXTRA_DATA,data.Length(),i);
}

void Check13(PseudoSprite&data){
	if(data.Length()<9){
		IssueMessage(FATAL,INVALID_LENGTH,ACTION,0x13,AT_LEAST,9);
		return;
	}
	const uint GRFid=data.ExtractDword(1);
	data.SetGRFID(1);
	if((GRFid&0xFF)==0xFF)IssueMessage(WARNING1,RESERVED_GRFID);

	uint offs=8;

	data.SetAllHex();
	int nument=(signed)data.ExtractByte(5);
	if(nument!=0){
		const uint id=data.ExtractWord(6);
		if(id>>10==0xD0>>2||id>>8==0xDC||id>>9==0xC4>>1||id>>9==0xC9)
			CheckTextID(0x48,id,6)&&CheckTextID(0x48,id+nument-1,5);
		else
			IssueMessage(ERROR,OUT_OF_RANGE_TEXTID_13);
	}else
		IssueMessage(WARNING1,NO_TEXTS);
	for(;nument--||_autocorrect;){
		int result=CheckString(data,offs,CTRL_ALL|CTRL_NO_STACK_CHECK,!nument);
		if(result){
			if(result!=-1)nument--;
			break;
		}
		try{
			if(data[offs])data.SetEol(offs-1,1);
			else data.SetNoEol(offs-1);
		}catch(uint){}
	}
	if(++nument){
		if(_autocorrect){
			IssueMessage(0,CONSOLE_AUTOCORRECT,_spritenum);
			IssueMessage(0,AUTOCORRECTING,5,NUMENT,data.ExtractByte(5),data.ExtractByte(5)-nument);
			data.SetByteAt(5,data.ExtractByte(5)-nument);
		}else
			IssueMessage(ERROR,INSUFFICIENT_STRINGS,nument);
	}
	if(offs<data.Length())IssueMessage(WARNING2,EXTRA_DATA,data.Length(),offs);
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
	uint choice_list=0;
	bool utf8=false,valid=true;
	try{
		utf8=data.ExtractWord(offs)==0x9EC3;
		if(utf8){
			data.SetUTF8(offs,2);
			offs+=2;
		}
	}catch(uint){}
	while(0 != (ch = (utf8?data.ExtractUtf8(offs, valid):data.ExtractByte(offs++)))) {
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
						data.SetQEscape(offs);//nonprintable -- control or special.
					else data.SetText(offs);
				}else ch=0x20;//valid UTF-8 encoding of U+0080..U+00FF; bypass control-char checks
			}else if(ch>0xE07A&&ch<0xE100)
				ch&=0xFF;//UTF-8 encoding of U+E07B..U+E0FF; run control-char checks
		}else{//!utf8
			if(ch<0x20||(ch>0x7A&&ch<0xA1)||ch==0xAA||ch==0xAC||ch==0xAD||ch==0xAF||(ch>0xB3&&ch<0xB9))
				data.SetQEscape(offs);//nonprintable -- control or special.
			else data.SetText(offs);
		}
		if(ch<0x20){
			data.SetQEscape(offs);
			if(ch==1){
				if(~perms&CTRL_SPACE)IssueMessage(WARNING1,INVALID_CONTROL,offs,ch);
				if(!data.ExtractByte(++offs)&&!include_00_safe)
					IssueMessage(WARNING1,EMBEDDED_00,offs);
				data.SetQEscape(offs);
			}else if(ch==0x0D){
				if(~perms&CTRL_NEWLINE)IssueMessage(WARNING1,INVALID_CONTROL,offs,ch);
				try{
					if(data[offs+1]!=0x0D)data.SetEol(offs,2);
					else data.SetNoEol(offs);
				}catch(uint){}
			}
			else if(ch==0x0E){
				if(~perms&CTRL_FONT_SMALL)IssueMessage(WARNING1,INVALID_CONTROL,offs,ch);
			}else if(ch==0x0F){
				if(~perms&CTRL_FONT_LARGE)IssueMessage(WARNING1,INVALID_CONTROL,offs,ch);
			}else if(ch==0x1F){
				if(~perms&CTRL_SPACE)IssueMessage(WARNING1,INVALID_CONTROL,offs,ch);
				if(!data.ExtractByte(++offs)&&!include_00_safe)
					IssueMessage(WARNING1,EMBEDDED_00,offs);
				data.SetQEscape(offs,2);
				if(!data.ExtractByte(++offs)&&!include_00_safe)
					IssueMessage(WARNING1,EMBEDDED_00,offs);
			}else IssueMessage(WARNING3,UNUSED_CONTROL,offs,ch);
		}else if(ch<0x7B);
		else if(ch==0x81){
			int id=data.ExtractEscapeWord(++offs);
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
		}else if(ch<0x88||ch==0x9A){
			if(ch==0x9A){
				uint arg;
				ch=data.ExtractQEscapeByte(++offs);
				switch(ch){
				case 0x00:		// print qword currency
					if(!include_00_safe)IssueMessage(WARNING1,EMBEDDED_00,offs);
				case 0x01:		// print qword currency
				case 0x02:		// ignore color code
					break;
				case 0x03:		// push WORD
					stack = string(2,char(STACK_WORD)) + stack;
					arg=data.ExtractEscapeWord(++offs);
					if(!(arg&0xFF)&&!include_00_safe)IssueMessage(WARNING1,EMBEDDED_00,offs);
					++offs;
					if(!(arg>>8)&&!include_00_safe)IssueMessage(WARNING1,EMBEDDED_00,offs);
					break;
				case 0x04:		// Delete BYTE characters
					arg=data.ExtractQEscapeByte(++offs);
					if(!arg&&!include_00_safe)IssueMessage(WARNING1,EMBEDDED_00,offs);
				case 0x06:		// print hex byte
				case 0x07:		// ... word
				case 0x08:		// ... dword
				case 0x0B:		// ... qword
				case 0x0C:		// print name of station
				case 0x0D:		// print word in weight
				case 0x16:		// print dword as long date
				case 0x17:		// print dword as short date
				case 0x18:		// print word as horse power
				case 0x19:		// print word as short volume
				case 0x1A:		// print word as short weight
				case 0x1B:		// print two words as long cargo amount
				case 0x1C:		// print two words as short cargo amount
				case 0x1D:		// print two words as tiny cargo amount
					break;

				case 0x10:		// choice list value
					if (choice_list == 0) IssueMessage(ERROR,NOT_IN_CHOICE_LIST,offs);
					/* FALL THROUGH */
				case 0x0E:		// set gender
				case 0x0F:		// set case
					if (_grfver < 7) IssueMessage(WARNING1, NEED_VERSION_7, _grfver);
					arg = data.ExtractQEscapeByte(++offs);
					if (!arg) IssueMessage(ERROR,EMBEDDED_00, offs);
					break;

				case 0x11:		// choice list default
					if (_grfver < 7) IssueMessage(WARNING1, NEED_VERSION_7, _grfver);
					if (choice_list == 0) IssueMessage(ERROR, NOT_IN_CHOICE_LIST, offs);
					choice_list = 2;
					break;

				case 0x12:		// end choice list
					if (_grfver < 7) IssueMessage(WARNING1, NEED_VERSION_7, _grfver);
					if (choice_list == 0) IssueMessage(ERROR, NOT_IN_CHOICE_LIST, offs);
					if (choice_list == 1) IssueMessage(ERROR, NO_DEFAULT_IN_CHOICE_LIST, offs);
					choice_list = 0;
					break;

				case 0x13:		// begin gender choice list
				case 0x15:		// begin plural choice list
					arg = data.ExtractQEscapeByte(++offs);
					if (!arg) IssueMessage(ERROR, EMBEDDED_00, offs);
					/* FALL THROUGH */
				case 0x14:		// begin case choice list
					if (_grfver < 7) IssueMessage(WARNING1, NEED_VERSION_7, _grfver);
					if (choice_list != 0) IssueMessage(ERROR, NESTED_CHOICE_LIST, offs);
					choice_list = 1;
					break;

				default:
					IssueMessage(ERROR,INVALID_EXT_CODE,offs,ch);
					perms|=CTRL_NO_STACK_CHECK;
				}
			}
			if(~perms&CTRL_NO_STACK_CHECK){
				//for Extended format codes (9A XX), "ch" is the XX
				switch(ch){
				case 0x7D:case 0x06:
					STACK_CHECK(STACK_BYTE,1)
				case 0x82:case 0x83:
					STACK_CHECK(STACK_DATE,2)
				case 0x7C:case 0x7E:case 0x84:case 0x85:case 0x87:case 0x07:case 0x0C:case 0x0D:case 0x18:case 0x19: case 0x1A:
					STACK_CHECK(STACK_WORD,2)
				case 0x80:
					STACK_CHECK(STACK_TEXT,2)
				case 0x7B:case 0x7F:case 0x08:case 0x16:case 0x17:case 0x1B:case 0x1C:case 0x1D:
					STACK_CHECK(STACK_DWORD,4)
				case 0x00:case 0x01:case 0x0B:
					STACK_CHECK(STACK_QWORD,8)
				case 0x02:case 0x03:case 0x04:case 0x0E:case 0x0F:case 0x10:case 0x11:case 0x12:
				case 0x13:case 0x14:case 0x15:
					--ret;	// These do not read from the stack.
					break;
				DEFAULT(ch)
				}
				++ret;
			}
		}else if(ch<0x9A){
			data.SetQEscape(offs);
			if(~perms&CTRL_COLOR)IssueMessage(WARNING1,INVALID_CONTROL,offs,ch);
			if(ch==0x99){
				if(!data.ExtractByte(++offs)&&!include_00_safe)
					IssueMessage(WARNING1,EMBEDDED_00,offs);
				data.SetQEscape(offs);
			}
		}else if(ch<0x9E){
			data.SetQEscape(offs);
			IssueMessage(WARNING3,UNUSED_CONTROL,offs,ch);
		}
		if(++offs>=length)break;
	}
	if (choice_list != 0) IssueMessage(ERROR, CHOICE_LIST_NOT_TERMINATED, offs);
	if(ch) {
		if(_autocorrect) {
			IssueMessage(0,CONSOLE_AUTOCORRECT,_spritenum);
			IssueMessage(0,AUTOCORRECT_ADD,0);
			data.Append(0);
			ch=0;
			offs++;
		} else {
			IssueMessage(WARNING1,NO_NULL_FOUND);
		}
	}
	if(retInfo==RETURN_NULL)return ch;
	if(retInfo==RETURN_STACK)return(perms&CTRL_NO_STACK_CHECK)?(unsigned)-1:ret;
	INTERNAL_ERROR(retInfo,retInfo);
}

static const uchar stackSize[]={0,1,2,2,4,2,8};

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
		if (fgets(buffer,102,pFile) == NULL) {
			IssueMessage(0,DATAFILE_ERROR,LOAD,"langs.dat",EOF_READING_NAME,i);
		}
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
	VERIFY(id<0x80,id);
	if(langNames::Instance().names[id]!="")return langNames::Instance().names[id];
	return _unknownLanguage;
}
