/*
 * act79D.cpp
 * Contains definitions for checking actions 7, 9, and D.
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

#include<string>
#include<cassert>
#include<fstream>
#include<errno.h>

using namespace std;

#include"nforenum.h"
#include"inlines.h"
#include"messages.h"
#include"sanity_defines.h"
#include"data.h"
#include"pseudo.h"
#include"command.h"

uint numvars;
class Vars{
public:
	bool canRead7(uint v){return(v<0x80||(v<numvars&&_p[v&0x7F]&0x80));}
	bool canReadD(uint v){return(v<0x80||v==0xFF||(v<numvars&&_p[v&0x7F]&0x40));}
	bool canWriteD(uint v){return(v<0x80||(v<numvars&&_p[v&0x7F]&0x20));}
	bool isBitmask(uint v){return(v<numvars&&_p[v&0x7F]&7)==0;}
	bool lenOK(uint v,uint len){
		if(len==4)len--;
		return(_p[v&0x7F]&(1<<(len-1)))!=0;
	}
	AUTO_ARRAY(uchar)
	SINGLETON(Vars)
};

class D:public Guintp{
public:
	uint maxpatchvar,maxop;
	SINGLETON(D)
};

Vars::Vars(){
	FILE*pFile=myfopen(79Dv);
	_p=new uchar[numvars=GetCheckByte(79Dv)];
	myfread(_p,numvars,79Dv);
	fclose(pFile);
	numvars|=0x80;
}

D::D(){
	FILE*pFile=myfopen(D);
	maxpatchvar=GetCheckByte(D);
	maxop=GetCheckByte(D);
	_p=new uint[MaxFeature()+1];
	for(uint i=0;i<=MaxFeature();i++)
		_p[i]=GetCheckWord(D);
	fclose(pFile);
}

#define SetSize(x)\
	if(desiredSize!=x)desiredSize=0;\
	else(void(0))

#define CheckSize(var)\
	if(!Vars::Instance().lenOK(var,desiredSize)) desiredSize=0;\
	else(void(0))

struct act7{
	act7();
	act7(uint act,uint skips):act(act),spriteno(_spritenum),skips(skips){}
	uint act,spriteno,skips;
};

static vector<act7>jumps;

int Check7(PseudoSprite&data){
	uint desiredSize=data.Length()-5;
	data.SetAllHex();
	uint var=data.ExtractByte(1),var_size=data.ExtractByte(2),cond=data.ExtractByte(3);
	data.SetOpByte(3, '7');
	if(cond>0xE)IssueMessage(ERROR,BAD_CONDITION,cond);
	else if(cond>5&&cond<0xB){
		if(var!=0x88){
			IssueMessage(ERROR,GRFCOND_NEEDS_GRFVAR,cond);
			desiredSize=0;
		}
		data.SetGRFID(4);
	}else if(cond>0xA){
		data.SetText(4,4);
		SetSize(4);
	}
	if(cond<6&&var==0x88){
		IssueMessage(ERROR,GRFVAR_NEEDS_GRFCOND);
		desiredSize=0;
	}
	if(cond<2){
		SetSize(1);
		var_size=1;
	}
	if(!Vars::Instance().canRead7(var))IssueMessage(ERROR,NONEXISTANT_VARIABLE,1,var);
	else if(var>0x7F&&cond>1&&Vars::Instance().isBitmask(var)){
		IssueMessage(ERROR,BITTEST_VARIABLE,var);
		desiredSize=0;
	}
	if(var_size==8&&var==0x88){
		if(desiredSize>=8){
			uint grfid = data.ExtractDword(4), mask = data.ExtractDword(8);
			if((~mask&grfid)!=0)
				IssueMessage(WARNING1,MASKED_BIT_SET);
		}
	}else if(var_size==0||var_size==3||var_size>4){
		IssueMessage(FATAL,BAD_VARSIZE,var_size);
		return 0;
	}else if((cond==0xB||cond==0xC)&&var_size!=4)
		IssueMessage(WARNING2,COND_SIZE_MISMATCH,cond,4);
	else if(var>0x7F&&cond>1){
		CheckSize(var);
		if(!Vars::Instance().lenOK(var,var_size))
			IssueMessage(WARNING2,VARIABLE_SIZE_MISMATCH,var_size,var);
	}
	if(_autocorrect&&desiredSize&&desiredSize!=data.ExtractByte(2)){
        IssueMessage(0,CONSOLE_AUTOCORRECT,_spritenum);
		IssueMessage(0,AUTOCORRECTING,2,VARSIZE,data.ExtractByte(2),desiredSize);
		data.SetByteAt(2,var_size=desiredSize);
	}
	if(cond<2?
		CheckLength(data.Length(),6,BAD_LENGTH,COND,VAL,cond,6):
		CheckLength(data.Length(),5+var_size,BAD_LENGTH,VARSIZE,VAL,data.ExtractByte(2),var_size+5))return 0;
	jumps.push_back(act7(data.ExtractByte(0),data.ExtractByte(4+var_size)));
	return data.ExtractByte(4+var_size);
}

#undef SetSize

void Init7(){
	jumps.clear();
}

void final7(){
	bool header=false;
	for(uint i=0;i<jumps.size();i++)
		if(!IsLabel(jumps[i].skips)&&jumps[i].skips+jumps[i].spriteno>_spritenum){
			if(!header){
				header=true;
				IssueMessage(WARNING2,LONG_JUMPLEAD);
				IssueMessage(WARNING2,UNEXP_EOF_LONGJUMP);
			}
			IssueMessage(WARNING2,LONG_JUMP,jumps[i].act,jumps[i].spriteno);
		}
}

bool CheckD(PseudoSprite&data,uint length){
	if(length<5){IssueMessage(FATAL,INVALID_LENGTH,ACTION,0xD,ONE_OF,5,9);return false;}
	data.SetAllHex();
	uint target=data.ExtractByte(1),op=data.ExtractByte(2),src1=data.ExtractByte(3),src2=data.ExtractByte(4);
	data.SetPositionalOpByte(2, 'D');
	if(!Vars::Instance().canWriteD(target))IssueMessage(ERROR,INVALID_TARGET);
	if((op&0x7F)>D::Instance().maxop)IssueMessage(ERROR,INVALID_OP,2,op);
	if((src1!=0xFE||src2!=0xFE)&&!Vars::Instance().canReadD(src1))IssueMessage(ERROR,INVALID_SRC,1);
	if(op&&src2!=0xFE&&!Vars::Instance().canReadD(src2))IssueMessage(ERROR,INVALID_SRC,2);
	if((op&0x7F)&&src1==0xFF&&src2==0xFF)IssueMessage(ERROR,ONLY_ONE_DATA);
	if(src1==0xFF||src2==0xFF||src2==0xFE){
		if(CheckLength(length,9,INVALID_LENGTH,ACTION,0xD,ONE_OF,5,9))return false;
		if(src2==0xFE){
			uint info=data.ExtractDword(5);
			if((info&0xFF)!=0xFF){
				if(op&0x7F)IssueMessage(ERROR,INVALID_OP,2,op);
				if(src1!=0xFE&&src1&0x80)IssueMessage(ERROR,INVALID_SRC,1);
				data.SetGRFID(5);
			}else if(info==0xFFFF){
				if(op&0x7F)IssueMessage(ERROR,INVALID_OP,2,op);
				if(src1>D::Instance().maxpatchvar)IssueMessage(ERROR,INVALID_SRC,1);
			}else{
				if(op)IssueMessage(ERROR,INVALID_OP,2,op);
				data.SetPositionalOpByte(3, 'D');
				uint feat=(info>>8)&0xFF,count=info>>16;
				if(src1>6)IssueMessage(ERROR,INVALID_SRC,1);
				if(feat>MaxFeature()||!D::Instance()[feat])IssueMessage(ERROR,INVALID_FEATURE);
				else if(count>D::Instance()[feat])IssueMessage(WARNING1,OOR_COUNT);
				return true;
			}
		}
	}else if(length>5)IssueMessage(WARNING2,INVALID_LENGTH,ACTION,0xD,ONE_OF,5,9);
	return false;
}
