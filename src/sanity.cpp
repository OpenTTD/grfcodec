/*
 * sanity.cpp
 * Defines functions for sprite sanity checking.
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

#include<iostream>
#include<string>
#include<cassert>
#include<cstdarg>
#include<errno.h>
#include<cstdlib>
using namespace std;

#include"nforenum.h"
#include"globals.h"
#include"inlines.h"
#include"command.h"
#include"messages.h"
#include"sanity.h"
#include"ExpandingArray.h"
#include"sanity_defines.h"
#include"data.h"
#include"strings.h"
#include"pseudo.h"

bool _base_grf = false;

class features{
public:
	uint maxFeat;
	struct featdat{
		uchar validbits;
		uchar act2type;
	};
	AUTO_ARRAY(featdat);
	SINGLETON(features)
};

features::features(){
	FILE*pFile=myfopen(feat);
	maxFeat=GetCheckByte(feat);
	_p=new features::featdat[maxFeat+1];
	uint i=0;
	for(;i<=maxFeat;i++) _p[i].validbits=(uchar)GetCheckByte(feat);
	for(i=0;i<=maxFeat;i++) _p[i].act2type=(uchar)GetCheckByte(feat);
	fclose(pFile);
}

bool IsValidFeature(int act,uint feat){
	if(feat>features::Instance().maxFeat)return false;
	return(features::Instance()[feat].validbits&act)!=0;
}

bool IsValid2Feature(uint feat){
	if(feat>features::Instance().maxFeat)return false;
	return features::Instance()[feat].act2type!=0xFF;
}

uchar Get2Type(uint feat){
	if(feat>features::Instance().maxFeat)return 0xFF;
	return features::Instance()[feat].act2type;
}

uint MaxFeature(){return features::Instance().maxFeat;}

#define MAX_TTD_SPRITE 4894

static struct sanity{
	sanity(){init();}
	void init(){
		state=FIND_PSEUDO;
		defined10IDs.init();
		act8=act11=act14=0;
	}
	sanstate state;
	unsigned int expectedsprites,seensprites,spritenum,act8,act11,act14,minnextlen;
	class labelArray{
	public:
		labelArray(){init();}
		void init(){used.resize(0);sprite.resize(0);}
		bool is_defined(int id)const{return sprite[id]!=0;}
		bool is_used(int id)const{return used[id];}
		uint defined_at(int id)const{return sprite[id];}
		void define(uint id){sprite[id]=_spritenum;used[id]=false;}
		void use(int id){used[id]=true;}
	protected:
		Expanding0Array<bool>used;
		Expanding0Array<uint>sprite;
	}defined10IDs;
}status;
static const sanity&crStatus=status;

void Init123();
void InitF();
void ClearTextIDs();
void Check0(PseudoSprite&);
int  Check1(PseudoSprite&);
void Check2(PseudoSprite&);
void Check3(PseudoSprite&);
int  Check5(PseudoSprite&,sanstate&);
uint Check6(PseudoSprite&);
int  Check7(PseudoSprite&);
void CheckB(PseudoSprite&);
bool CheckD(PseudoSprite&,uint);
void CheckF(PseudoSprite&);
void Check13(PseudoSprite&);
void Check14(PseudoSprite&);
void invalidate_act3();

void reset_sanity(){
	status.init();
	Init0();
	Init123();
	Init7();
	InitF();
	ClearTextIDs();
}

bool IsLabel(uint x){
	VERIFY(x<0x100,x);
	return status.defined10IDs.is_defined(x);
}

void Seen8(int action){
	if(status.act8==0){
		IssueMessage(ERROR,MISSING_8,action);
		status.act8=(unsigned)-1;
	}
}

void Before8(int action){
	if(status.act8!=0){
		IssueMessage(ERROR,BEFORE_8,action);
	}
}

bool CheckLength(int alen,int elen,RenumMessageId message,...){
	WrapAp(message);
	if(alen<elen){
		vIssueMessage(FATAL,message,ap);
		return true;
	}
	if(alen>elen)
		vIssueMessage(WARNING2,message,ap);
	return false;
}

void SetState(sanstate x){
	if((signed)status.expectedsprites==-1)status.state=UNKNOWN;
	else if(status.expectedsprites==0)status.state=FIND_PSEUDO;
	else status.state=x;
}

void check_sprite(SpriteType type){
	if(GetState(LINT)==OFF)return;
	switch(status.state){
	case FIND_PSEUDO:
		if(type==REAL){
			IssueMessage(ERROR,UNEXPECTED,REAL_S);
			status.state=UNKNOWN;
		}else if(type==INCLUDE){
			IssueMessage(ERROR,UNEXPECTED,BIN_INCLUDE);
			status.state=UNKNOWN;
		}
		return;
	case FIND_REAL:
	case FIND_REAL_OR_RECOLOR:
		if(type==INCLUDE){
			IssueMessage(ERROR,UNEXPECTED,BIN_INCLUDE);
			status.state=UNKNOWN;
		}else if(++status.seensprites==status.expectedsprites)
			status.state=FIND_PSEUDO;
		else if(status.seensprites>status.expectedsprites){
			IssueMessage(ERROR,EXTRA,REAL_S,status.expectedsprites,status.spritenum);
			status.state=UNKNOWN;
		}
		return;
	case FIND_INCLUDE:
		if(type==REAL){
			IssueMessage(ERROR,UNEXPECTED,REAL_S);
			status.state=UNKNOWN;
		}else if(++status.seensprites==status.expectedsprites)
			status.state=FIND_PSEUDO;
		else if(status.seensprites>status.expectedsprites){
			IssueMessage(ERROR,EXTRA,BIN_INCLUDE,status.expectedsprites,status.spritenum);
			status.state=UNKNOWN;
		}
		return;
	case FIND_RECOLOR:
		if(type==REAL){
			IssueMessage(ERROR,UNEXPECTED,REAL_S);
			status.state=UNKNOWN;
		}else if(type==INCLUDE){
			IssueMessage(ERROR,UNEXPECTED,BIN_INCLUDE);
			status.state=UNKNOWN;
		}else if(++status.seensprites==status.expectedsprites)
			status.state=FIND_PSEUDO;
		return;
	case UNKNOWN:return;
	DEFAULT(status.state)
	}
}

void check_sprite(PseudoSprite&data){
	if(GetState(LINT)==OFF)return;
	const uint length=data.Length();
	if(length==1&&data.ExtractByte(0)==0&&status.state==FIND_REAL){
		check_sprite(REAL);
		return;
	}
	if((status.state==FIND_RECOLOR||status.state==FIND_REAL_OR_RECOLOR)&&length==257&&data.ExtractByte(0)==0){
		check_sprite(RECOLOR);
		data.SetAllHex();
		data.SetEol(32,1);
		data.SetEol(64,1);
		data.SetEol(96,1);
		data.SetEol(128,1);
		data.SetEol(160,1);
		data.SetEol(192,1);
		data.SetEol(224,1);
		return;
	}
	if(data.ExtractByte(0)==0xFF||data.ExtractByte(0)==0xFE)check_sprite(INCLUDE);
	else{
		switch(status.state){
		case FIND_REAL:
		case FIND_REAL_OR_RECOLOR:
		case FIND_INCLUDE:
		case FIND_RECOLOR:
			IssueMessage(ERROR,INSUFFICIENT,
				status.state==FIND_INCLUDE?BIN_INCLUDE:(status.state==FIND_RECOLOR?RECOLOR_TABLE:REAL_S),
				status.spritenum,status.expectedsprites,status.seensprites);
			status.state=UNKNOWN;
		case FIND_PSEUDO:
		case UNKNOWN:break;
		DEFAULT(status.state)
		}
	}
	if(status.minnextlen>length)
		IssueMessage(ERROR,MODIFY_BEYOND_LENGTH);
	status.minnextlen=0;
	try{
	const int act=data.ExtractByte(0);
	if(act!=3&&act!=6&&act!=7&&act!=9&&act!=0xC)invalidate_act3();
	if(act == 0x14)
		Before8(act);
	else if(act<6 || act==0xA || act==0xF || act>0x11)
		Seen8(act);
	switch(act){
	case 0:
		data.SetAllHex();
		Check0(data);
		break;
	case 1:
		data.SetAllHex();
		status.expectedsprites=Check1(data);
		SetState(FIND_REAL);
		status.spritenum=_spritenum;
		status.seensprites=0;
		break;
	case 2:
		Check2(data);
		break;
	case 3:
		data.SetAllHex();
		Check3(data);
		break;
	case 4:
		Check4(data);
		break;
	case 5:
		data.SetAllHex();
		status.expectedsprites=Check5(data,status.state);
		SetState(status.state);
		status.spritenum=_spritenum;
		status.seensprites=0;
		break;
	case 6:
		data.SetAllHex();
		status.minnextlen=Check6(data);
		break;
	case 7:case 9:{
		status.defined10IDs.use(Check7(data));
		break;
	}case 8:{
		if(status.act8!=0&&status.act8!=(unsigned)-1)IssueMessage(ERROR,DUPLICATE_ACT,8,status.act8);
		status.act8=_spritenum;
		_grfver=data.ExtractByte(1);
		if(_grfver<2||_grfver>8)IssueMessage(ERROR,INVALID_VERSION,GRF);
		if(status.act14!=0&&_grfver<7)IssueMessage(ERROR,INVALID_VERSION_ACT14,_grfver);
		if(_act14_pal==0&&_grfver>=7)IssueMessage(WARNING1,MISSING_PALETTE_INFO);
		const uint GRFid=data.ExtractDword(2);
		data.SetGRFID(2);
		_base_grf=(GRFid&0xFFFFFF)==0x00544FFF;
		if(!_base_grf&&(GRFid&0xFF)==0xFF&&GRFid!=0xFFFFFFFF)IssueMessage(WARNING1,RESERVED_GRFID);
		if(length<8){
			IssueMessage(ERROR,INVALID_LENGTH,ACTION,8,AT_LEAST,8);
			break;
		}
		uint offs=6;
		CheckString(data,offs,CTRL_COLOR);
		try{
			if(data[offs])data.SetEol(offs-1,1);
		}catch(uint){
			IssueMessage(WARNING1,NO_NULL_FOUND);
		}
		CheckString(data,offs,CTRL_COLOR|CTRL_NEWLINE);
		if(offs<length)IssueMessage(WARNING2,EXTRA_DATA,length,offs);
		break;
	}case 0xA:
		data.SetAllHex();
		status.state=UNKNOWN;
		status.seensprites=data.ExtractByte(1);
		if(_autocorrect&&length%3==2&&length/3!=status.seensprites&&length/3<256){
			IssueMessage(0,CONSOLE_AUTOCORRECT,_spritenum);
			IssueMessage(0,AUTOCORRECTING,1,NUMSETS,data.ExtractByte(1),length/3);
			data.SetByteAt(1,status.seensprites=length/3);
		}
		if(!status.seensprites)IssueMessage(WARNING1,NO_SETS,0x0A);
		if(CheckLength(length,2+3*status.seensprites,BAD_LENGTH,BYTE1,VAL,status.seensprites,2+3*status.seensprites))
			return;
		status.expectedsprites=0;
		for(uint i=0;i<status.seensprites;i++){
			int sprites=data.ExtractByte(2+3*i);
			if(!sprites)
				IssueMessage(WARNING1,SET_WITH_NO_SPRITES,2+3*i,i);
			else if(data.ExtractWord(3+3*i)+sprites-1>MAX_TTD_SPRITE)
				IssueMessage(ERROR,SPRITENUM_TOO_HIGH,2+3*i);
			status.expectedsprites+=sprites;
		}
		status.spritenum=_spritenum;
		status.seensprites=0;
		SetState(FIND_REAL_OR_RECOLOR);
		break;
	case 0xB:
		data.SetAllHex();
		CheckB(data);
	case 0xC:break;
	case 0xD:
		if(CheckD(data,length))Seen8(0xD);
		break;
	case 0xE:{
		uint numids=data.SetHex(1).ExtractByte(1);
		if(_autocorrect&&length%4==2&&length/4!=numids&&length/4<256){
			IssueMessage(0,CONSOLE_AUTOCORRECT,_spritenum);
			IssueMessage(0,AUTOCORRECTING,1,NUM,numids,length/4);
			data.SetByteAt(1,numids=length/4);
		}
		if(CheckLength(length,2+4*numids,BAD_LENGTH,NUM,VAL,numids,2+4*numids))return;
		if(!numids)IssueMessage(WARNING1,NO_GRFIDS);
		if(numids<2)return;
		uint*id=new uint[numids];
		for(uint i=0;i<numids;i++){
			if(((id[i]=data.SetGRFID(2+i*4).ExtractDword(2+i*4))&0xFF)==0xFF)IssueMessage(WARNING1,INVALID_GRFID,2+i*4,0xE);
			for(uint j=0;j<i;j++)
				if(id[i]==id[j])IssueMessage(WARNING1,DUPLICATE_GRFID,2+i*4,2+j*4,id[i]);
		}
		delete[]id;
		break;
	}case 0xF:
		CheckF(data);
		break;
	case 0x10:{
		if(length<2){
			IssueMessage(FATAL,INVALID_LENGTH,ACTION,0x10,AT_LEAST,2);
			break;
		}
		int label=data.SetHex(1).ExtractByte(1);
		if(!crStatus.defined10IDs.is_used(label)){
			if(crStatus.defined10IDs.is_defined(label))
				IssueMessage(WARNING1,UNUSED_LABEL_PREV_DEF,label,crStatus.defined10IDs.defined_at(label));
			else
				IssueMessage(WARNING2,UNUSED_LABEL_NO_PREV_DEF,label);
		}
		status.defined10IDs.define(label);
		break;
	}case 0x11:
		status.state=UNKNOWN;
		data.SetAllHex();
		if(CheckLength(length,3,INVALID_LENGTH,ACTION,0x11,EXACTLY,3))return;
		if(status.act11)IssueMessage(ERROR,DUPLICATE_ACT,0x11,status.act11);
		status.expectedsprites=data.ExtractWord(1);
		status.act11=status.spritenum=_spritenum;
		SetState(FIND_INCLUDE);
		status.seensprites=0;
		break;
	case 0x12:
		data.SetAllHex();
		status.state=UNKNOWN;
		status.seensprites=data.ExtractByte(1);
		if(_autocorrect&&length%4==2&&length/4!=status.seensprites&&length/4<256){
			IssueMessage(0,CONSOLE_AUTOCORRECT,_spritenum);
			IssueMessage(0,AUTOCORRECTING,1,NUMDEFS,status.seensprites,length/4);
			data.SetByteAt(1,status.seensprites=length/4);
		}
		if(!status.seensprites)IssueMessage(WARNING1,NO_SETS,0x12);
		if(CheckLength(length,2+4*status.seensprites,BAD_LENGTH,BYTE1,VAL,status.seensprites,2+4*status.seensprites))
			return;
		status.expectedsprites=0;
		for(uint i=0;i<status.seensprites;i++){
			uint sprites=data.ExtractByte(2+4*i);
			if(sprites>3)IssueMessage(ERROR,INVALID_FONT,2+4*i,sprites);
			sprites=data.ExtractByte(3+4*i);
			if(!sprites)
				IssueMessage(WARNING1,SET_WITH_NO_SPRITES,3+4*i,i);
			else if((data.ExtractWord(4+4*i)&~0x7F)!=((data.ExtractWord(4+4*i)+sprites-1)&~0x7F))
				IssueMessage(ERROR,SPANS_BLOCKS,3+4*i,i);
			status.expectedsprites+=sprites;
		}
		status.spritenum=_spritenum;
		status.seensprites=0;
		SetState(FIND_REAL);
		break;
	case 0x13:
		Check13(data);
		break;
	case 0x14:
		if (status.act14==0) status.act14=_spritenum;
		Check14(data);
		break;
	case 0xFE:{
		if(CheckLength(length,8,INVALID_LENGTH,IMPORTS,EXACTLY,8))return;
		uint feat=data.SetAllHex().ExtractByte(1);
		if(feat)IssueMessage(ERROR,INVALID_LITERAL,1,feat,0);
		if(data.ExtractByte(2)==0xFF)IssueMessage(WARNING1,INVALID_GRFID,2,0x11);
		data.SetGRFID(2);
		break;
	}case 0xFF:
		//if(data.ExtractByte(data.ExtractByte(1)+2))
		break;
	default:
		IssueMessage(FATAL,INVALID_ACTION);
	}
	}catch(unsigned int off){
		IssueMessage(FATAL,TOO_SHORT,EXPECTED_LOC(off),EXPECTED_BYTES(off));
	}

}

void final_sanity(){
	if(GetState(LINT)==OFF)return;
	if(status.seensprites<status.expectedsprites){
		switch(status.state){
		case FIND_REAL:
		case FIND_REAL_OR_RECOLOR:
			IssueMessage(ERROR,INSUFFICIENT,REAL_S,status.spritenum,status.expectedsprites,status.seensprites);
			break;
		case FIND_INCLUDE:
			IssueMessage(ERROR,INSUFFICIENT,BIN_INCLUDE,status.spritenum,status.expectedsprites,status.seensprites);
			break;
		case FIND_RECOLOR:
			IssueMessage(ERROR,INSUFFICIENT,RECOLOR_TABLE,status.spritenum,status.expectedsprites,status.seensprites);
		default:;//Intentionally ignoring FIND_PSEUDO and UNKNOWN
		}
	}
	if(status.minnextlen)
		IssueMessage(ERROR,NO_FOLLOWING_SPRITE);
	final123();
	final7();
	finalF();
}
