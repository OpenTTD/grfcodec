/*
 * act123.cpp
 * Contains definitions for checking actions 1-3.
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

#include"inlines.h"
#include"ExpandingArray.h"
#include"sanity_defines.h"
#include"data.h"
#include"rangedint.h"
#include"pseudo.h"
#include"act123.h"
#include"command.h"
#include"messages.h"

#define MAX_TTD_SPRITE 4894

/*int parity(uint x){
	int ret=0;
	while(x){
		ret+=x%2;
		x/=2;
	}
	return ret;
}*/


void CheckSpriteNum(uint num,uint offset,act123::Act1&act1,uint feature,bool&mismatch,bool&hasGround){
	uint sprite=num&0x3FFF;
	if(num&1<<31){
		if(feature!=act1.feature&&!mismatch){
			IssueMessage(ERROR,FEATURE_MISMATCH,1,act1.spritenum);
			mismatch=true;
		}
		if(sprite>=act1.numsets)
			IssueMessage(ERROR,UNDEFINED_SPRITE_SET,offset,sprite,act1.spritenum);
		else act1.use(sprite);
	}else if(sprite>MAX_TTD_SPRITE)
		IssueMessage(ERROR,SPRITENUM_TOO_HIGH,offset);
	hasGround|=((num&1<<30)!=0);
	int translate=num>>14&3,color=num>>16&0x3FFF;
	if(translate==3)IssueMessage(ERROR,INVALID_COLOR_TRANS,offset);
	else if((translate==1||translate==2&&color)&&(color<775||color>790)&&(color<795||color>803))
		IssueMessage(WARNING1,INVALID_COLOR_SPRITE,offset+2,color);
}

bool check_id(uint offset,unsigned int id,act123::IDarray&IDs){
	if(id&0x8000)return false;//callback
	else if(id>>8){
		IssueMessage(ERROR,NEITHER_ID_CALLBACK,id,CID);
		return true;
	}
	return IDs.test(offset,id)?(IDs.use(id),IDs.checks1C(id)):true;
}

void invalidate_act3(){
	act123::Instance().act3spritenum=0;
}

void CheckCallback(uint offs,uint feature,uint cb){
	Callbacks&cbs=Callbacks::Instance();
	if(cb&&(cb>=cbs.numcallbacks||cb<0x10||(cbs[cb]&0x80?!(cbs[cb]&(1<<feature)):feature!=cbs[cb])))
		IssueMessage(ERROR,INVALID_CALLBACK,offs,cb);
}

int Check1(PseudoSprite&data){
	act123::Act1&act1=act123::Instance().act1;
	const uint length=data.Length();
	if(act1.spritenum)
		for(unsigned int i=0;i<act1.numsets;i++)
			if(!act1.is_used(i))IssueMessage(WARNING1,UNUSED_SET,i,act1.spritenum);
	act1.init();
	if(CheckLength(length,(data.ExtractByte(3)==0xFF)?6:4,INVALID_LENGTH,ACTION,1,ONE_OF,4,6))return 0;
	if(!IsValidFeature(ACT1,act1.feature=data.ExtractByte(1)))IssueMessage(ERROR,INVALID_FEATURE);
	act1.numsets=data.ExtractByte(2);
	if(!act1.numsets)IssueMessage(WARNING1,NO_SETS,1);
	int numsprites=data.ExtractExtended(3);
	if(!numsprites&&!IsValidFeature(EMPTY1,act1.feature))IssueMessage(WARNING1,NO_SPRITES,1);
	if((numsprites>4&&(act1.feature==7||act1.feature==9))||(numsprites>8&&act1.feature<4)||
		(numsprites>1&&act1.feature==0x0B))IssueMessage(WARNING1,SET_TOO_LARGE);
	else if(numsprites>4&&numsprites<8&&act1.feature<4)IssueMessage(WARNING1,STRANGE_SET_SIZE);
	act1.spritenum=_spritenum;
	return numsprites*act1.numsets;
}

void Check2(PseudoSprite&data){
	data.SetAllHex();
	act123::Act1&act1=act123::Instance().act1;
	uint feature=data.ExtractByte(1),id=data.ExtractByte(2);
	if(!IsValidFeature(ACT2,feature)){
		IssueMessage(FATAL,INVALID_FEATURE);
		return;
	}
	Define2 defineID(feature,id);
	uint nument1=data.ExtractByte(3),nument2=data.ExtractByte(4),length=data.Length(),i,j;
	switch(nument1){
	case 0x80:
	case 0x83:
	{
		bool isRand=false;
		unsigned int prevID=(unsigned)-1,rID;
		nument2=data.ExtractByte(6);
		if((nument2-1)&nument2/*tests for only one bit set*/||!nument2)IssueMessage(ERROR,RAND_2_NUMSETS);
		else if(nument2==1)IssueMessage(WARNING3,ONLY_ONE_CHOICE);
		rand2::Instance().CheckRand(feature,nument1,data.ExtractByte(4),data.ExtractByte(5),nument2);
		bool ret=CheckLength(length,7+2*nument2,BAD_LENGTH,"nrand","%2x",nument2,7+2*nument2);
		if(!_autocorrect||!(length%2)){
			if(ret)return;
		}else if(nument2*2!=(length-7)&&(length-7)/2<256&&!(((length-7)/2-1)&(length-7)/2)){
			IssueMessage(0,CONSOLE_AUTOCORRECT,_spritenum);
			IssueMessage(0,AUTOCORRECTING,6,"nrand",nument2,(length-7)/2);
			data.SetByteAt(6,uchar(nument2=(length-7)/2));
		}
		for(i=0;i<nument2;i++){
			rID=data.SetNoEol(7+2*i).ExtractWord(7+2*i);
			check_id(7*i+2,rID,act123::Instance().defined2IDs[feature]);
			if(prevID!=(unsigned)-1)
				isRand|=(prevID!=rID);
			prevID=rID;
		}
		if(!isRand&&nument2!=1)
			IssueMessage(WARNING3,NOT_RANDOM);
		return;
	}
	case 0x81:case 0x82:
	case 0x85:case 0x86:
	case 0x89:case 0x8A:
	{
		uint extract=1<<((nument1>>2)&3),off=4,var,param=0,shift,op=(uint)-1;
		bool isvar=false,isadv=false;
		varRange ranges(extract);
		while(true){//read <var> [<param>] <varadjust> [<op> ...]. off reports byte to be read.
			if(((var=data.ExtractByte(off++))&0xE0)==0x60)param=data.ExtractByte(off++);
			Check2v::Instance().Check(feature,nument1,var,param,(shift=data.ExtractByte(off++))&0x1F);
			if((shift&0xC0)==0xC0){
				IssueMessage(FATAL,INVALID_SHIFT,off-1);
				return;
			}
			if(isadv||(shift&0x20))data.SetEol(off-(((var&0xE0)==0x60)?4:3),2);
			ranges.UpdateRange(op,shift,data,off);
			defineID.Check(var);
			isvar|=(var!=0x1A&&var!=0x1C);
			if(!(shift&0x20))break;
			isadv=true;
			if((op=data.ExtractByte(off++))>0x0D)IssueMessage(ERROR,INVALID_OP,off,op);
		}
		nument2=data.ExtractByte(off);//off switches to byte-just-read.
		if(isadv)data.SetEol(off-1,1);
		else if(nument2>1)data.SetEol(off,1);
		if(!isvar)IssueMessage(WARNING4,NOT_VARIATIONAL);
		uint width=2+extract*2,end=++off+width*nument2;
		bool ret=CheckLength(length,end+2,BAD_LENGTH,"nvar","%2x",nument2,end+2);
		int change=int(length-end-2)/int(width);
		if(end+2!=length&&_autocorrect&&!((length-end-2)%width)&&nument2+change<256){
			IssueMessage(0,CONSOLE_AUTOCORRECT,_spritenum);
			IssueMessage(0,AUTOCORRECTING,off-1,"nvar",nument2,nument2+change);
			data.SetByteAt(off-1,uchar(nument2+=change));
			end=length-2;
		}else if(ret)return;
		uint def=data.ExtractWord(end),vID;
		for(i=off;i<end;i+=width){//read <ID> <min> <max> [...]
			vID=data.ExtractWord(i);
			isvar=check_id(i,vID,act123::Instance().defined2IDs[feature]);
			uint min=data.ExtractVariable(i+2,extract),max=data.ExtractVariable(i+2+extract,extract);
			if(nument2>1)data.SetEol(i+1+extract*2,isadv?2:1);
			if(min>max)IssueMessage((i==off)?WARNING4:WARNING1,UNREACHABLE_VAR,(i-off)/width);
			else{
				ranges.AddRange(min,max);
				if(def==vID&&(nument2!=1||!isvar))IssueMessage(WARNING3,REUSED_DEFAULT);
			}
			if(var==0x0C&&!isadv){
				if(min!=max&&!(
					(feature==7&&min==0x1B&&max==0x1C)||
					(feature==0&&min==0x10&&max==0x11&&(vID==0x8000||vID==0xFF00))
					))
					IssueMessage(WARNING1,CHECK_0C_RANGE,i+2,min,max);
				else{
					CheckCallback(i+2,feature,min);
				}
			}
		}
		ranges.CheckDefault();
		check_id(end,def,act123::Instance().defined2IDs[feature]);
		return;
	}
	default:
		if(nument1&0x80)IssueMessage(FATAL,INVALID_TYPE);
		else if(feature<6||feature==0x0B){
			bool mismatch=false,no1=false;
			if(CheckLength(length,2*(nument1+nument2)+5,BAD_LENGTH,VARS,"nument1","nument2",VALS,nument1,nument2,
						   2*(nument1+nument2)+5))
				break;
			switch(feature){
			case 0x0B:
			case 5:
				if(nument1!=1)IssueMessage(ERROR,INVALID_LITERAL,3,nument1,1);
				if(nument2)IssueMessage(ERROR,INVALID_LITERAL,4,nument2,0);
				break;
			default:
				if(!nument1&&feature!=4)IssueMessage(ERROR,NO_REQD_SETS,LOADED);
				if(!nument2)IssueMessage(ERROR,NO_REQD_SETS,(feature==4)?LOTS:LOADING);
			}
			for(i=0;i<nument1+nument2;i++){
				j=data.ExtractWord(2*i+5);
				if(j&0x8000);//Callback
				else if(j>>8)IssueMessage(ERROR,NEITHER_ID_CALLBACK,j,SET);
				else if(act1.spritenum==0&&!no1){
					IssueMessage(ERROR,NO_ACT1);
					no1=true;
				}else if(feature!=act1.feature&&!mismatch&&!no1){
					mismatch=true;
					IssueMessage(ERROR,FEATURE_MISMATCH,1,act1.spritenum);
				}else if(j>=act1.numsets)
					IssueMessage(ERROR,UNDEFINED_SPRITE_SET,2*i+5,j,act1.spritenum);
				else act1.use(j);
			}
		}else if(feature==7||feature==9){
			uint ground=data.ExtractDword(4);
			bool mismatch=false,hasGround=(ground!=0);
			if(ground)CheckSpriteNum(ground,4,act1,feature,mismatch,hasGround);
			if(nument1){//Extended format
				data.SetEol(3,3);//EOL before ground sprite
				data.SetEol(7,1);//EOL after ground sprite
				uint off=7;
				for(i=0;i<nument1;/*increment in last statement in try block*/){
					try{
						uint building=data.ExtractDword(++off),xoff=data.ExtractByte(off+=4),
							yoff=data.ExtractByte(++off),zoff=data.ExtractByte(++off);
						CheckSpriteNum(building,off-6,act123::Instance().act1,feature,mismatch,hasGround);
						if(!building)IssueMessage(ERROR,NO_BUILDING_SPRITE,off-6);
						if(zoff!=0x80){
							uint x=data.ExtractByte(++off),y=data.ExtractByte(++off);//,z=data.ExtractByte(++off);
							++off;
							if(xoff>15)IssueMessage(WARNING3,TOO_LARGE,off-5,"xoff",15);
							else if(xoff+x>16)IssueMessage(WARNING3,TOO_LARGE,off-5,"xoff+xextent",16);
							if(yoff>15)IssueMessage(WARNING3,TOO_LARGE,off-4,"yoff",15);
							else if(yoff+y>16)IssueMessage(WARNING3,TOO_LARGE,off-4,"yoff+yextent",16);
							//if(zoff+z>0x87)IssueMessage(WARNING1,TOO_LARGE,off-3,"zoff+zextent",0x87);
						}else if(i==0)IssueMessage(ERROR,FIRST_SPRITE_CANNOT_SHARE);
						if(++i!=nument1)data.SetEol(off,2);
					}catch(...){
						IssueMessage(FATAL,OVERRAN_SPRITE,i);
						return;
					}
				}
				if(++off!=length)IssueMessage(WARNING2,EXTRA_DATA,off);
				if(!hasGround)IssueMessage(WARNING2,NO_GROUNDSPRITE,NONTRANS);
			}else{//Basic format
				if(!ground)IssueMessage(ERROR,NO_GROUNDSPRITE,GROUND);
				if(CheckLength(length,17,INVALID_LENGTH,TYPE,BASICSTD2,HOUSE_INSTYTILE,17))break;
				uint building=data.ExtractDword(8),xoff=data.ExtractByte(12),yoff=data.ExtractByte(13),
					x=data.ExtractByte(14),y=data.ExtractByte(15);//,z=data.ExtractByte(16);
				if(building){
					CheckSpriteNum(building,8,act1,feature,mismatch,hasGround);
					if(xoff>15)IssueMessage(WARNING3,TOO_LARGE,12,"<xoff>",15);
					else if(xoff+x>16)IssueMessage(WARNING3,TOO_LARGE,12,"xoff+xextent",16);
					if(yoff>15)IssueMessage(WARNING3,TOO_LARGE,13,"yoff",15);
					else if(yoff+y>16)IssueMessage(WARNING3,TOO_LARGE,13,"yoff+yextent",16);
					//if(z>0x87)IssueMessage(WARNING1,TOO_LARGE,16,"zextent",0x87);
				}
			}
		}else if(feature==10){
			if(CheckLength(length,15,INVALID_LENGTH,TYPE,PROD2,INDUSTRIES,15))break;
			if(data.ExtractByte(3)!=0)
				IssueMessage(ERROR,INVALID_LITERAL,3,data.ExtractByte(3),0);
		}else
			INTERNAL_ERROR(feature,feature);
	}
}

void Check3(PseudoSprite&data){
	const uint feature=data.ExtractByte(1),length=data.Length();
	if(!IsValidFeature(ACT3,feature)){
		IssueMessage(FATAL,INVALID_FEATURE);
		return;
	}
	bool isOverride=((data.ExtractByte(2)&0x80)!=0);
	if(isOverride&&!IsValidFeature(OVERRIDE3,feature))IssueMessage(ERROR,INVALID_FEATURE);
	unsigned int numIDs=data.ExtractByte(2)&0x7F,numCIDs=data.ExtractByte(3+numIDs);
	if(numCIDs&&feature>4)IssueMessage(WARNING1,NO_CARGOTYPES);
	if(!isOverride){
		act123::Instance().act3feature=feature;
		act123::Instance().act3spritenum=_spritenum;
	}else if(!act123::Instance().act3spritenum)IssueMessage(ERROR,NO_STD_3);
	if(CheckLength(length,6+numIDs+3*numCIDs,BAD_LENGTH,VARS,"n-id","num-cid",VALS,numIDs,numCIDs,6+numIDs+3*numCIDs))return;
	unsigned int id,def=data.ExtractWord(4+numIDs+3*numCIDs),i,j;
	for(i=3;i<3+numIDs;i++)
		CheckID(feature,data.ExtractByte(i));
	for(i=4+numIDs;i<4+numIDs+3*numCIDs;i+=2){
		j=data.ExtractByte(i);
		if(j==0xFE&&feature!=4)
			IssueMessage(ERROR,INVALID_CARGO_TYPE,i,j);
		i++;
		check_id(i,id=data.ExtractWord(i),act123::Instance().defined2IDs[feature]);
		if(def==id)
			IssueMessage(WARNING1,REUSED_DEFAULT);
	}
	check_id(4+numIDs+3*numCIDs,def,act123::Instance().defined2IDs[feature]);
}

void Init123(){act123::Instance().init();}

void final123(){
	ManualConsoleMessages();
	bool header=false;
	if(act123::Instance().act1.spritenum)
		for(unsigned int i=0;i<act123::Instance().act1.numsets;i++)
			if(!act123::CInstance().act1.is_used(i)){
				IssueMessage(WARNING1,UNUSED_SET,i,act123::Instance().act1.spritenum);
				if(!header){
					IssueMessage(WARNING1,UNEXP_EOF_STD2);
					header=true;
				}
			}
	int j;
	for(uint i=0;i<=MaxFeature();i++){
		header=false;
		for(j=0;j<256;j++)
			if(act123::CInstance().defined2IDs[i].is_defined(j)&&!act123::CInstance().defined2IDs[i].is_used(j)){
				if(!header){
					IssueMessage(WARNING1,UNUSED2IDLEAD,i);
					IssueMessage(WARNING1,UNEXP_EOF_CARGOID,i);
					header=true;
				}
				IssueMessage(WARNING1,UNUSEDIDFINAL,j,act123::CInstance().defined2IDs[i].defined_at(j));
			}
	}
}
