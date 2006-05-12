/*
 * act0.cpp
 * Contains definitions for checking action 0s.
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

using namespace std;

#include"renum.h"
#include"inlines.h"
#include"messages.h"
#include"ExpandingArray.h"
#include"sanity_defines.h"
#include"data.h"
#include"pseudo.h"
#include"command.h"

/*Data Format:
chars 01,02,04 indicate bytes,words,doubles
char 03 extended byte
For 01..04, the high nibble indicates how to beautify:
Lower chawmp: 0x:default 1x:quote 2x:decimal 3x:B-E hex (2x and 3x are currently unsupported)
The upper chawmp, which only applies in FE strings:
0x:default  Cx:linebreak;long lead
char FE means variable length, see corresponding LengthData struct in subData
char FF means no such property

for variable length properties only:
01..04 as above 
char r means the next char is repeated some number of times, set by the char
  after it, for which chars 80-FF take on the value of the data indicated by
  char num&0x7F of this string (well, sorta--FEs aren't counted, for one)
char x means multiply the next two chars together
char * means the next char is repeated an arbitrary number of times. This is
  followed by the length of the terminator (1, 2, or 4) and then the
  terminator
char l means the next character appears in the NFO
char | means what is on either side is valid -- the left side MUST have at
least one literal. (ex: \x03 == l\xFF\x02|\x01)
char FE means variable, see corresponding LengthData struct in subData
char x0 means apply top nibble to previous byte
A backslash is used to escape nulls that are not end-of-string characters
all other characters are undef
*/

typedef basic_string<uchar> ustring;
typedef vector<int> int_str;

class PropData:public auto_array<PropData>{
public:
	PropData(){}
	void Init(FILE*);
	uchar GetData(uint)const;
	uint GetLength()const{return(uint)length.length();}
	const PropData*GetVarLength(int)const;
	uint GetRepeat(uint&,const int_str&)const;
private:
	ustring length;
	static ustring readString(FILE*);
	static int CountFE(const ustring&);
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
	bool CheckVar(uint&,PseudoSprite&,const PropData&,bool addblank)const;
	void operator=(const Check0&);
	Check0(const Check0&);
};

class Feat8{
	SINGLETON(Feat8)
public:
	struct Feat8sub{
		uint maxfirst;
		uint maxlast;
	};
	const Feat8sub&operator[](uint i)const{return _p[i];}
	const Feat8&operator=(Feat8sub*ptr){_p=ptr;return*this;}
private:
	auto_array<Feat8::Feat8sub>_p;
};

Feat8::Feat8(){
	uint numprops=Check0::Instance().GetFeat8();
	_p=new Feat8sub[numprops];
	FILE*pFile=myfopen(0f8);
	for(uint i=8;i<numprops;i++){
		_p[i].maxfirst=GetCheckByte(0f8);
		_p[i].maxlast=GetCheckByte(0f8);
	}
	fclose(pFile);
}

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

uint PropData::GetRepeat(uint&len_off,const int_str&decoded)const{
	uint repeat=GetData(len_off);
	if(repeat=='x'){
		repeat=GetRepeat(++len_off,decoded);
		return repeat*GetRepeat(++len_off,decoded);
	}
	if(repeat=='l')
		return GetData(++len_off);
	if(repeat&0x80)
		return decoded[repeat&0x7F];
	return repeat;
}

void PropData::Init(FILE*pFile){
	int j=CountFE(length=readString(pFile));
	if(j){
		_p=new PropData[j];
		for(int k=0;k<j;k++)
			_p[k].Init(pFile);
	}
}

ustring PropData::readString(FILE*pFile){
	ustring ret;
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
		ret.append(1,(char)ch);
	}while(true);
	return ret;
}

int PropData::CountFE(const ustring&str){
	int ret=0;
	size_t start=0;
	while((start=str.find_first_of((unsigned char)0xFE,start)+1)!=0)ret++;
	return ret;
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
	if(IDs==0&&(propsRemain!=1||feature||str.ExtractByte(i)!=0x1A))IssueMessage(WARNING1,NO_IDS);
	feature!=8&&IDs&&CheckID(feature,firstID)&&CheckID(feature,maxID);
	Expanding0Array<uint>propLoc;
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
						IssueMessage(0,AUTOCORRECTING,2,"num-info",str.ExtractByte(2),str.ExtractByte(2)-propsRemain);
						str.SetByteAt(2,uchar(str.ExtractByte(2)-propsRemain));
					}
					break;
				}
				return;
			}
			len=_p[feature].GetData(prop);
			if(len==0xFF){
				IssueMessage(FATAL,INVALID_PROP,i,prop);
				if(_autocorrect){
					if(propsRemain){
						IssueMessage(0,CONSOLE_AUTOCORRECT,_spritenum);
						IssueMessage(0,AUTOCORRECTING,2,"num-info",str.ExtractByte(2),str.ExtractByte(2)-propsRemain);
						str.SetByteAt(2,uchar(str.ExtractByte(2)-propsRemain));
					}
					break;
				}
				return;
			}
			if(feature==8){
				if(firstID>Feat8::Instance()[prop].maxfirst)
					IssueMessage(ERROR,INVALID_ID,firstID,0,Feat8::Instance()[prop].maxfirst);
				if(IDs&&maxID>Feat8::Instance()[prop].maxlast)
					IssueMessage(ERROR,INVALID_ID,maxID,0,Feat8::Instance()[prop].maxlast);
			}
			if(propLoc[prop])
				IssueMessage(WARNING2,REPEATED_PROP,i,prop,propLoc[prop]);
			propLoc[prop]=i;
			if(len==0xFE){
				i++;
				const PropData*data=_p[feature].GetVarLength(prop);
				for(j=0;j<IDs;j++)
					if(!CheckVar(i,str,*data,j+1<IDs))return;
			}else if((len&7)==3){
				i++;
				for(j=0;j<IDs;j++)
					i+=str.ExtendedLen(i);
			}else{
				i++;
				for(uint j=0;j<IDs;j++){
					switch((len>>4)&3){
					case 1:str.SetText(i,len&7);break;
					case 2:str.SetDec(i,len&7);break;
					case 3:str.SetBE(i,len&7);break;
					}
					i+=(len&7);
				}
			}
			propsRemain--;
		}
		if(i>str.Length())
			IssueMessage(ERROR,INSUFFICIENT_DATA2,i-str.Length(),prop);
		else{
			if(i<str.Length()){
				len=_p[feature].GetData(str.ExtractByte(4+str.ExtendedLen(4)));
				if(_autocorrect&&str.ExtractByte(2)==1&&(len&7)<5){
					while(i+((len&7)==3?str.ExtendedLen(i):(len&7))<=str.Length()&&IDs<0xFF){
						switch((len>>4)&3){
						case 1:str.SetText(i,len&7);break;
						case 2:str.SetDec(i,len&7);break;
						case 3:str.SetBE(i,len&7);break;
						}
						IDs++;
						if(len==3)i+=str.ExtendedLen(i);
						else i+=len&7;
					}
					if(IDs!=str.ExtractByte(3)){
						IssueMessage(0,CONSOLE_AUTOCORRECT,_spritenum);
						IssueMessage(0,AUTOCORRECTING,3,"num-IDs",str.ExtractByte(3),IDs);
						str.SetByteAt(3,(uchar)IDs);
					}
				}
				if(i<str.Length())
					IssueMessage(WARNING2,EXTRA_DATA,i);
			}
			bool linebreaks=(IDs>1||GetState(LINEBREAKS)==3)&&str.ExtractByte(2)>1;
			uint maxwidth=2,data,width;
			for(i=0;i<propLoc.size();i++)
				if(propLoc[i])
					if((data=_p[feature].GetData(i))==0xFE)linebreaks=true;
					else if(data==0x14)maxwidth=max<uint>(maxwidth,6);
					else maxwidth=max(maxwidth,uint(data&7)*3-1);
			if(!linebreaks)return;
			for(i=0;i<propLoc.size();i++){
				if(!propLoc[i])continue;
				str.SetEol(propLoc[i]-1,1);
				if((data=_p[feature].GetData(i))==0xFE)continue;
				if((width=(data==0x14)?6:(data&7)*3-1)<maxwidth){
					uint j=IDs;
					for(;j;j--)
						str.PadAfter(propLoc[i]+(data&7)*j,maxwidth-width);
				}
			}
		}
	}catch(uint off){
		IssueMessage(FATAL,INSUFFICIENT_DATA,prop,EXPECTED_BYTES(off),EXPECTED_LOC(off));
	}
}

bool Check0::CheckVar(uint&str_loc,PseudoSprite&str,const PropData&vdata,bool canaddblank)const{
	bool findPipe=false;
	uchar ch;
	uint orig_loc=str_loc,vdatalen=vdata.GetLength();
	int_str decoded;
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
			if(vdata.GetData(++i)!=str.ExtractByte(str_loc++)){
				str_loc=orig_loc;
				findPipe=true;
			}
			break;
		case'r':{
			const PropData*vdata2=vdata.GetVarLength(i);
			uchar repeat_data=vdata.GetData(++i);
			uint times=vdata.GetRepeat(++i,decoded);
			if(repeat_data==0xFE){
				for(uint j=0;j<times;j++)
					if(!CheckVar(str_loc,str,*vdata2,false))return false;
			}else if((repeat_data&7)<5){
				switch((repeat_data>>4)&3){
				case 1:str.SetText(str_loc-1-repeat_data&7,repeat_data&7);
				case 2:str.SetDec(str_loc-1-repeat_data&7,repeat_data&7);
				case 3:str.SetBE(str_loc-1-repeat_data&7,repeat_data&7);
				}
				str_loc+=(repeat_data&7)*times;
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
				if((repeat_data&7)<5){
					while((str.*ExtractTerm)(str_loc)!=term)
						str_loc+=repeat_data;
				}else if(repeat_data==0xFE){
					while((str.*ExtractTerm)(str_loc)!=term)
						if(!CheckVar(str_loc,str,*vdata2,false))return false;
				}else{
					IssueMessage(0,INVALID_DATAFILE,DAT2,"0.dat",'*',repeat_data);
					exit(EDATA);
				}
				str_loc+=term_len;
				break;
			}catch(uint){
				IssueMessage(ERROR,MISSING_TERMINATOR);
				return false;
			}
			break;
		}case 0xFE:
			CheckVar(str_loc,str,*vdata.GetVarLength(i),false);
			break;
		default:
			switch(ch&7){
			case 3:
				decoded.push_back(str.ExtractExtended(str_loc));
				str_loc+=str.ExtendedLen(str_loc);
				break;
			case 1:
			case 2:
			case 4:
				decoded.push_back(str.ExtractVariable(str_loc,ch&7));
				str_loc+=(ch&7);
			case 0:
				break;
			DEFAULT(ch)
			}
			switch((ch>>4)&3){
			case 1:str.SetText(str_loc-1-ch&7,ch&7);
			case 2:str.SetDec(str_loc-1-ch&7,ch&7);
			case 3:str.SetBE(str_loc-1-ch&7,ch&7);
			}
			if(ch>>6==3){
				str.SetEol(str_loc-1,2);
				addblank|=canaddblank;
			}
		}
	}
	if(findPipe){
		IssueMessage(ERROR,MISSING_TERMINATOR);
		return false;
	}
	if(addblank)str.AddBlank(str_loc-1);
	return true;
}

Check0::Check0(){
	FILE*pFile=myfopen(0);
	_p=new PropData[MaxFeature()+1];
	for(uint i=0;i<=MaxFeature();i++)
		_p[i].Init(pFile);
	fclose(pFile);
}
