/*
 * act0.cpp
 * Contains definitions for checking action 0s.
 *
 * Copyright 2005-2010 by Dale McCoy.
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
#ifdef _MSC_VER
#pragma warning(disable:4702)//unreachable code
#include<vector>
#pragma warning(default:4702)
#else
#include<vector>
#endif
#include<cassert>
#include<errno.h>
#include<cstdlib>

using namespace std;

#include"nforenum.h"
#include"inlines.h"
#include"messages.h"
#include"ExpandingArray.h"
#include"sanity_defines.h"
#include"data.h"
#include"pseudo.h"
#include"command.h"
#include"rangedint.h"
#include"act123.h"

/*Data Format:
bits   indicate width:
0-2		width
3		special formatting or checks
5-4		Basic formatting
7-6		(FE strings only) linebreak formatting
7		(Non-FE strings only) permit property multiple times

Width:
1-4: byte, word, extended byte, double; resp.

Special formatting/checks:
If set:
- Print decimal values as dates
- Check BE words against defined TTD and DCxx IDs
- Check BE doubles against remaining size of property (e.g. indu prop 0A)
- Check default words against defined IDs for feature that relates to this one
	(i.e. Check that tiles used in indu prop 0A are defined.)

Basic formatting:
0:default 1:quote 2:decimal 3:B-E hex

linebreak formatting:
0x:default  Cx:linebreak;long lead

Special cases:
char FE means variable length, see corresponding LengthData struct in subData
char FF means no such property


for variable length properties only:
As above, with special cases as follows:
char r means the next char is repeated some number of times, set by the char
  after it, for which chars 80-FF take on the value of the data indicated by
  char num&0x7F of this string (well, sorta--FEs aren't counted, for one)
char x means multiply the next two chars together
char * means the next char is repeated an arbitrary number of times. This is
  followed by the length of the terminator (1, 2, or 4) and then the
  terminator
char l means the next character appears in the NFO
char | means what is on either side is valid -- right side will be ignored if
left side does not have at least one literal (ex: \x03 == l\xFF\x02|\x01, but
\x01|l\xFF\x02 will always read exactly one byte.)
char FD takes the same type of parameter as r, but adds that value to the
  list of 80+x values for all following FE substrings.
char FE as above
char C0 means to insert a linebreak (as if encountering C1..C4) but without
  reading any bytes
A backslash is used to escape nulls that are not end-of-string characters
char FF is undef
*/

typedef basic_string<uchar> ustring;
typedef vector<int> int_str;

class PropData:public auto_array<PropData>{
public:
	PropData(){}
	void Init(FILE*,bool);
	uchar GetData(uint)const;
	uint GetLength()const{return(uint)length.length();}
	const PropData*GetVarLength(int)const;
	uint GetValue(uint&,const int_str&)const;
	uint maxfirst(int prop){return idRange[prop]&0xFF;}
	uint maxlast(int prop){return (idRange[prop]>>8)&0xFF;}
private:
	ustring length;
	vector<uint> idRange;
	void readString(FILE*,bool);
	int CountFE();
	void operator=(const PropData&);
	PropData(const PropData&);
};

class Check0:public auto_array<PropData>{
public:
	void Check(PseudoSprite&);
	static Check0&Instance(){static Check0 obj;return obj;}
	uint GetFeat8(){return _p[8].GetLength();}
private:
	Check0();
	bool CheckVar(uint&,PseudoSprite&,const PropData&,bool addblank,bool,int_str =int_str())const;
	void operator=(const Check0&);
	Check0(const Check0&);
};

void Check0(PseudoSprite&str){
	Check0::Instance().Check(str);
}

uchar PropData::GetData(unsigned int prop)const{
	return prop<length.length()?length[prop]:(uchar)0xFF;
}

const PropData*PropData::GetVarLength(int prop)const{
	PropData*ret=_p;
	for(int i=0;i<prop;i++)
		if(length[i]==0xFE)ret++;
	return ret;
}

uint PropData::GetValue(uint&len_off,const int_str&decoded)const{
	uint value=GetData(len_off);
	if(value=='x'){
		value=GetValue(++len_off,decoded);
		return value*GetValue(++len_off,decoded);
	}
	if(value=='&'){
		value=GetValue(++len_off,decoded);
		return value&GetValue(++len_off,decoded);
	}
	if(value=='l')
		return GetData(++len_off);
	if(value=='m'){
		uint len = GetData(++len_off);
		value=0;
		for(uint k=0;k<len;k++)
			value|=GetData(++len_off)<<(8*k);
		return value;
	}
	if(value&0x80)
		return decoded[value&0x7F];
	return value;
}

void PropData::Init(FILE*pFile, bool withIDs){
	readString(pFile, withIDs);
	int j=CountFE();
	if(j){
		_p=new PropData[j];
		for(int k=0;k<j;k++)
			_p[k].Init(pFile, false);
	}
}

void PropData::readString(FILE*pFile, bool withIDs){
	int ch;
	bool escape=false;
	do{
		if((ch=fgetc(pFile))==EOF)
			throw 0;
		if(!escape){
			if(!ch)break;
			if(ch=='\\'){
				escape=true;
				continue;
			}
		}
		escape=false;
		if(withIDs){
			if(ch!=0xFF)
				idRange.push_back(GetCheckWord(0));
			else
				idRange.push_back(0);
		}
		length.append(1,(char)ch);
	}while(true);
}

int PropData::CountFE(){
	int ret=0;
	size_t start=0;
	while((start=length.find_first_of((unsigned char)0xFE,start)+1)!=0)ret++;
	return ret;
}

class Prop08Tracking{
	STATIC(Prop08Tracking)
public:
	static void Set(uint feat,uint id){
		_m[feat][id]=true;
	}
	static void Reset(){
		_m.clear();
	}
	static bool Check(uint feat,uint id){
		return ((const ExpandingArray<Expanding0Array<bool> >)_m)[feat][id];
	}
private:
	static ExpandingArray<Expanding0Array<bool> > _m;
};

ExpandingArray<Expanding0Array<bool> > Prop08Tracking::_m;

bool IsProp08Set(uint feature,uint id){
	return Prop08Tracking::Check(feature,id);
}

uint CargoTransTable(int newlimit=0){
	static int limit=0;
	static uint prevtable;
	if(newlimit<0)
		limit=0;
	else if(newlimit){
		if(limit)
			IssueMessage(WARNING1,DUPLICATE_TRANS_TABLE,prevtable);
		limit = newlimit;
		prevtable=_spritenum;
	}
	if(limit) return (uint)limit;
	return 0x1B;
}

void Init0(){
	Prop08Tracking::Reset();
	CargoTransTable(-1);
}

#define GetWidth(x)		((x)&7)
#define IsSpecial(x)	((x)&8)
#define GetFormat(x)	((x>>4)&3)
#define GetLinebreak(x)	((x>>6)&3)

bool IsTextDefined(uint);

static vector<uint> lengthlist;

static void FormatSprite(PseudoSprite&str, uint&ofs, const uint format, const uint IDs = 1) {
	uint feature = str.ExtractByte(1);
	for(uint j=0;j<IDs;j++){
		switch(GetFormat(format)){
		case 0:		// default
			if (GetWidth(format)==2 && IsSpecial(format)) {	// Check word against defined IDs.
				uint k=UINT_MAX;
				for (uint i=0;i<=MaxFeature();i++)
					if (Check2v::GetEffFeature(i,0x82)==feature && (k==UINT_MAX||k==feature)) k=i;

				static Expanding0Array<bool> warned;
				if (k>MaxFeature()) {
					if (!warned[feature]) IssueMessage(WARNING1,COULD_NOT_VERIFY);
					warned[feature]=true;
					break;
				}

				if (!IsProp08Set(k,str.ExtractWord(ofs)))
					IssueMessage(ERROR,ACT3_PRECEDES_PROP08,ofs,str.ExtractWord(ofs));
			}
			break;
		case 1:
			if(IsSpecial(format)) {
				str.SetText(ofs);
			} else {
				str.SetText(ofs,GetWidth(format));
			}
			break;
		case 2:		// Decimal (or date)
			if (IsSpecial(format))
				str.SetDate(ofs,GetWidth(format));
			else
				str.SetDec(ofs,GetWidth(format));
			break;
		case 3:		// BE hex
			if (IsSpecial(format)) {
				if (GetWidth(format)==2) {	// Check word against defined TextIDs.
					if(!IsTextDefined(str.ExtractWord(ofs)))
						IssueMessage(ERROR,UNDEFINED_TEXTID,ofs,str.ExtractWord(ofs));
				} else if (GetWidth(format)==4)		// Check dword against prop length
					lengthlist.push_back(ofs);
			}
			str.SetBE(ofs,GetWidth(format));
			break;
		}
		if(GetWidth(format)==3)
			ofs+=str.ExtendedLen(ofs);
		else
			ofs+=GetWidth(format);
	}
}

void Check0::Check(PseudoSprite&str){
	assert(str.ExtractByte(0)==0);
	int feature=str.ExtractByte(1);
	//IssueMessage(-2,V_ACTION_FOR,1,0,ACTION_STRINGS,0,FEATURE_STRINGS,feature);
	if(!IsValidFeature(ACT0,feature)){
		IssueMessage(FATAL,INVALID_FEATURE);
		return;
	}
	int propsRemain=str.ExtractByte(2),prop=-1,len;
	//IssueMessage(-1,V_PROPS,2,
	unsigned int i=4+str.ExtendedLen(4),firstID=str.ExtractExtended(4),IDs=str.ExtractByte(3),j;
	//IssueMessage(-1,V_IDS,3,firstID,firstID+IDs-1,IDs);
	uint maxID=firstID+IDs-1;
	if(propsRemain==0)
		IssueMessage(WARNING1,NO_PROPS);
	if(IDs==0&&(propsRemain!=1||feature||str.ExtractByte(i)!=0x1A)){
		IssueMessage(WARNING1,NO_IDS);
		return;
	}
	feature!=8&&IDs&&CheckID(feature,firstID)&&CheckID(feature,maxID);
	Expanding0Array<uint>propLoc, idWidth;
	try{
		while(propsRemain||_autocorrect){
			try{
				prop=str.ExtractByte(i);
			}catch(unsigned int){
				if(propsRemain>0)IssueMessage(ERROR,INSUFFICIENT_PROPS,propsRemain);
				if(_autocorrect>=2||(_autocorrect&&propsRemain<3&&str.ExtractByte(2)-propsRemain>1)){
					if(i>str.Length()){
						i=propLoc[prop];
						propsRemain++;
					}
					if(propsRemain){
						IssueMessage(0,CONSOLE_AUTOCORRECT,_spritenum);
						IssueMessage(0,AUTOCORRECTING,2,NUMINFO,str.ExtractByte(2),str.ExtractByte(2)-propsRemain);
						str.SetByteAt(2,str.ExtractByte(2)-propsRemain);
					}
					break;
				}
				return;
			}
			if(feature==8 && prop==9)
				CargoTransTable(IDs-1);
			len=_p[feature].GetData(prop);
			if(prop==8)// Mark prop 08 as set, if necessary.
				for(uint i=firstID;i<=maxID;i++)
					Prop08Tracking::Set(feature,i);

			if(len==0xFF){
				IssueMessage(FATAL,INVALID_PROP,i,prop);
				if(_autocorrect){
					if(propsRemain){
						IssueMessage(0,CONSOLE_AUTOCORRECT,_spritenum);
						IssueMessage(0,AUTOCORRECTING,2,NUMINFO,str.ExtractByte(2),str.ExtractByte(2)-propsRemain);
						str.SetByteAt(2,str.ExtractByte(2)-propsRemain);
					}
					break;
				}
				return;
			}
			if(feature==8){
				if(firstID>_p[feature].maxfirst(prop))
					IssueMessage(ERROR,INVALID_ID,firstID,0,_p[feature].maxfirst(prop));
				if(IDs&&maxID>_p[feature].maxlast(prop))
					IssueMessage(ERROR,INVALID_ID,maxID,0,_p[feature].maxlast(prop));
			}
			if(propLoc[prop]&&!(len&0x80))
				IssueMessage(WARNING2,REPEATED_PROP,i,prop,propLoc[prop]);
			propLoc[prop]=i++;
			if(len==0xFE){
				const PropData*data=_p[feature].GetVarLength(prop);
				for(j=0;j<IDs;j++){
					if(!CheckVar(i,str,*data,j+1<IDs,true))return;
					while (lengthlist.size()) {
						const uint loc = lengthlist.back(),
							val = str.ExtractDword(loc),
							dist = i-(loc+4);
						if (val != dist) {
							IssueMessage(ERROR,LENGTH_MISMATCH,loc,val,dist);
							if (_autocorrect) {
								IssueMessage(0,CONSOLE_AUTOCORRECT,_spritenum);
								IssueMessage(0,AUTOCORRECTING,loc,PROPLENGTH,val,dist);
								str.SetDwordAt(loc,dist);
							}
						}
						lengthlist.pop_back();
					}
				}
			}else
				FormatSprite(str,i,len,IDs);
			propsRemain--;
		}
		if(i>str.Length())
			IssueMessage(ERROR,INSUFFICIENT_DATA2,i-str.Length(),prop);
		else{
			if(i<str.Length()){
				len=_p[feature].GetData(str.ExtractByte(4+str.ExtendedLen(4)));
				if(_autocorrect&&str.ExtractByte(2)==1&&GetWidth(len)<5){
					// If setting one property, assume setting same prop for more IDs
					while(i+(GetWidth(len)==3?str.ExtendedLen(i):GetWidth(len))<=str.Length()&&IDs<0xFF){
						FormatSprite(str,i,len);
						IDs++;
					}
					if(IDs!=str.ExtractByte(3)){
						IssueMessage(0,CONSOLE_AUTOCORRECT,_spritenum);
						IssueMessage(0,AUTOCORRECTING,3,NUMIDS,str.ExtractByte(3),IDs);
						str.SetByteAt(3,IDs);
					}
				}
				if(i<str.Length())
					IssueMessage(WARNING2,EXTRA_DATA,str.Length(),i);
			}
			if(!GetState(LINEBREAKS))return;
			bool linebreaks=(IDs>1||GetState(LINEBREAKS)==3)&&str.ExtractByte(2)>1;
			uint data;
			for(i=0;i<propLoc.size();i++)
				linebreaks |= propLoc[i] && _p[feature].GetData(i)==0xFE;
			if(!linebreaks)return;
			for(i=0;i<propLoc.size();i++){
				if(!propLoc[i])continue;
				str.SetEol(propLoc[i]-1,1);
				if((data=_p[feature].GetData(i))==0xFE)continue;
				for(uint j=IDs;j;j--)
					str.ColumnAfter(propLoc[i]+GetWidth(data)*j);
			}
		}
	}catch(uint off){
		IssueMessage(FATAL,INSUFFICIENT_DATA,prop,EXPECTED_BYTES(off),EXPECTED_LOC(off));
	}
}

bool Check0::CheckVar(uint&str_loc,PseudoSprite&str,const PropData&vdata,bool canaddblank,bool appendeol,int_str decoded)const{
	bool findPipe=false;
	uchar ch;
	int_str pass;
	uint orig_loc=str_loc,vdatalen=vdata.GetLength();
	bool addblank=false;
	for(uint i=0;i<vdatalen;i++){
		ch=vdata.GetData(i);
		if(findPipe&&ch!='|')continue;
		else if(findPipe){
			findPipe=false;
			continue;
		}
		switch(ch){
		case'|':return true;
		case'l':
		case'm':{
			uint len=(ch=='m'?vdata.GetData(++i):1);
			for(uint k=0;k<len;k++){
				/* Check for a specific raw data value.
				 * As this is used to check for certain formats, we do not want stuff like warning 209,
				 * so use LinkSafeExtractByte().
				 * Though the correct way of doing this would be to change the behaviour of '|' to dismiss
				 * all warnings and errors of the branch that was declined, this still works reasonable well. */
				if(vdata.GetData(++i)!=str.LinkSafeExtractByte(str_loc++)){
					str_loc=orig_loc;
					findPipe=true;
					break;
				}
			}
			break;
		}
		case'e':
		case'n':{
			uint lhs=vdata.GetValue(++i,decoded);
			uint rhs=vdata.GetValue(++i,decoded);
			const PropData*vdata2=vdata.GetVarLength(i);
			uchar data=vdata.GetData(++i);
			if ((lhs == rhs) == (ch == 'n')) break;
			if(data=='a'){
				IssueMessage(ERROR,UNKNOWN_ACT0_DATA,str_loc);
				return false;
			}else if(data=='s'){
				str_loc=orig_loc;
				findPipe=true;
			}else if(data==0xFE){
				if(!CheckVar(str_loc,str,*vdata2,false,false,pass))return false;
			}else if(GetWidth(data)<5){
				FormatSprite(str,str_loc,data,1);
			}else{
				IssueMessage(0,INVALID_DATAFILE,"0.dat",DAT2,ch,data);
				exit(EDATA);
			}
			break;
		}
		case'r':{
			const PropData*vdata2=vdata.GetVarLength(i);
			uchar repeat_data=vdata.GetData(++i);
			uint times=vdata.GetValue(++i,decoded);
			if(repeat_data==0xFE){
				for(uint j=0;j<times;j++)
					if(!CheckVar(str_loc,str,*vdata2,false,false,pass))return false;
			}else if(GetWidth(repeat_data)<5){
				FormatSprite(str,str_loc,repeat_data,times);
			}else{
				IssueMessage(0,INVALID_DATAFILE,"0.dat",DAT2,'r',repeat_data);
				exit(EDATA);
			}
			break;
		}case'*':{
			const PropData*vdata2=vdata.GetVarLength(i);
			uchar repeat_data=vdata.GetData(++i);
			int term_len=vdata.GetData(++i);
			uint term;
			uint(PseudoSprite::*ExtractTerm)(uint)const;
			switch(term_len){
			case 1:
				term=vdata.GetData(++i);
				ExtractTerm=&PseudoSprite::ExtractByte;
				break;
			case 2:
				term=vdata.GetData(++i);
				term|=vdata.GetData(++i)<<8;
				ExtractTerm=&PseudoSprite::ExtractWord;
				break;
			case 4:
				term=vdata.GetData(++i);
				term|=vdata.GetData(++i)<<8;
				term|=vdata.GetData(++i)<<16;
				term|=vdata.GetData(++i)<<24;
				ExtractTerm=&PseudoSprite::ExtractDword;
				break;
			default:
				IssueMessage(0,INVALID_DATAFILE,"0.dat",DAT3,'*',repeat_data,term_len);
				exit(EDATA);
			}
			try{
				if(repeat_data==0xFE){
					while((str.*ExtractTerm)(str_loc)!=term)
						if(!CheckVar(str_loc,str,*vdata2,false,false,pass))return false;
				}else if(GetWidth(repeat_data)<5){
					while((str.*ExtractTerm)(str_loc)!=term)
						FormatSprite(str,str_loc,repeat_data);
				}else{
					IssueMessage(0,INVALID_DATAFILE,"0.dat",DAT2,'*',repeat_data);
					exit(EDATA);
				}
				str_loc+=term_len;
				break;
			}catch(uint){
				IssueMessage(ERROR,MISSING_TERMINATOR);
				return false;
			}
			break;
		}case 0xFD:
			pass.push_back(vdata.GetValue(++i,decoded));
			break;
		case 0xFE:
			CheckVar(str_loc,str,*vdata.GetVarLength(i),false,false,pass);
			break;
		default:
			switch(GetWidth(ch)){
			case 1:
			case 2:
			case 3:
			case 4:
				decoded.push_back(str.ExtractVariable(str_loc,GetWidth(ch)));
				FormatSprite(str,str_loc,ch);
			case 0:
				break;
			DEFAULT(ch)
			}
			if(GetLinebreak(ch)==3){
				str.SetEol(str_loc-1,2);
				addblank|=canaddblank;
			}
		}
	}
	if(findPipe){
		IssueMessage(ERROR,MISSING_TERMINATOR);
		return false;
	}
	if(appendeol)
		str.SetEol(str_loc-1,1,canaddblank?1:0);
	if(addblank && GetState(LINEBREAKS)>1)
		str.AddBlank(str_loc-1);
	return true;
}

Check0::Check0(){
	FILE*pFile=myfopen(0);
	_p=new PropData[MaxFeature()+1];
	for(uint i=0;i<=MaxFeature();i++)
		_p[i].Init(pFile,i==8);
	fclose(pFile);
}
