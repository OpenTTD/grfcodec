/*
 * act123_classes.cpp
 * Defines classes and support functions for checking actions 1-3.
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

#include<string>
#include<iostream>
#include<cstdlib>

using namespace std;

#include"inlines.h"
#include"ExpandingArray.h"
#include"sanity_defines.h"
#include"data.h"
#include"rangedint.h"
#include"pseudo.h"
#include"messages.h"
#include"command.h"

#if defined(min)||defined(max)
#error min and max must be implemented as functions
#endif

#include "act123.h"

//****************************************
// act123
//****************************************

act123::act123(){init();}

void act123::init(){
	act1.init();
	defined2IDs.init();
	act3spritenum=0;
}

uint act123::MaxFoundFeat()const{
	const vector<IDarray::info>&m=defined2IDs._m;
	short ret=0;
	for(uint i=0;i<(uint)m.size();i++)
		ret=max<short>(ret,m[i].feature);
	return ret;
}

void act123::IDarray::define(uint feature,unsigned int id,bool checks1C){
	_m[id].used=false;
	_m[id].sprite=_spritenum;
	_m[id].v1C=checks1C;
	_m[id].feature=(ushort)feature;
}

bool act123::IDarray::test(uint offset,uint id)const{
	if(!is_defined(id)){
		IssueMessage(ERROR,UNDEFINED_ID,offset,id);
		return false;
	}
	return true;
}

//****************************************
// Check2v
//****************************************

Check2v::Check2v(){
	FILE*pFile=myfopen(2v);
	VarData tempvar;
	int var;
	maxop=GetCheckByte(2v);
	while(true){
		var=tempvar.Load(pFile);
		if(var==-1)break;
		globvars[var]=tempvar;
	}
	_p=new FeatData[MaxFeature()+1];
	for(uint i=0;i<=MaxFeature();i++){
		_p[i].last80=0xFF;
		_p[i].featfor82=GetCheckByte(2v);
		while(true){
			var=tempvar.Load(pFile);
			if(var==-1)break;
			if(tempvar.width==0x80){
				_p[i].last80=var-1;
				break;
			}
			if(var==0xFF&&tempvar.width==0xF0)break;
			if(var&0x80)_p[i].var80[var&0x7F]=tempvar;
			else _p[i].vars[var]=tempvar;
		}
	}
	fclose(pFile);
}

bool Check2v::IsValid(uint feature, uint var)const{
	if(var>=0x80)
		return _p[feature].var80[var&0x7F].width != 0;
	else
		return _p[feature].vars[var].width || globvars[var].width;
}

uint Check2v::MaxParam(uint feature, uint var)const{
	assert((var&0xE0)==0x60);
	assert(IsValid(feature, var));
	if(_p[feature].vars[var].width)
		return _p[feature].vars[var].maxparam;
	else
		return globvars[var].maxparam;
}

uint Check2v::GetWidth(uint feature, uint var)const{
	assert(IsValid(feature, var));
	if(_p[feature].vars[var].width)
		return _p[feature].vars[var].width;
	else
		return globvars[var].width & ~0x40;
}

uint Check2v::GetEffFeature(uint feature,uint type){
	if((type&3)==2)return Instance()._p[feature].featfor82;
	return feature;
}

void Check2v::Check(uint feature,uint var,uint offs,uint param,uint shift)const{
	if(feature>MaxFeature())return;
	uint real_var=var;
	if(real_var==0x7B){
		var=param;
		if(var<0x60 || var>=0x80) IssueMessage(WARNING1,INDIRECT_VAR_NOT_6X,offs);
	}
	if(var&0x80){
		if(var>_p[feature].last80) IssueMessage(ERROR,NONEXISTANT_VARIABLE,offs,var);
		else if(!IsValid(feature, var)) IssueMessage(WARNING1,NONEXISTANT_VARIABLE,offs,var);
		else if(shift>=_p[feature].var80[var&0x7F].width<<3)IssueMessage(WARNING4,SHIFT_TOO_FAR,offs+1,var);
	}else if(!IsValid(feature,var))
		IssueMessage(WARNING1,NONEXISTANT_VARIABLE,offs,var);
	else{
		if(var==0x7E){
			if(real_var==0x7B){
				IssueMessage(ERROR,INDIRECT_VAR_7E,offs);
				return;
			}
			act123::IDarray&IDs=act123::Instance().defined2IDs;
			if(!IDs.is_defined(param)) IssueMessage(ERROR,UNDEFINED_ID,offs+1,param);
			else{
				if(IDs.GetFeature(param)!=feature)
					IssueMessage(WARNING1,FEATURE_CALL_MISMATCH,offs+1,param,IDs.GetFeature(param));
				IDs.use(param);
			}
		}else if(var>=0x60 && real_var!=0x7B && param>MaxParam(feature,var))
			IssueMessage(WARNING1,PARAM_TOO_LARGE,offs+1,param,var);
		if(shift>=GetWidth(feature, var)<<3)IssueMessage(WARNING1,SHIFT_TOO_FAR,offs+2,var);
	}
}

uint Check2v::Prohibit0Mask(uint var){
	return !(CInstance().globvars[var].width & 0x40);
}

//****************************************
// Check2v::VarData
//****************************************

int Check2v::VarData::Load(FILE*pFile){
	uint var;
	var=GetCheckByte(2v);
	if((width=GetCheckByte(2v))==0xF0&&var==0xFF)
		return-1;
	if(!(width&0x7F))return var;
	assert(width&0x80);
	/*  //Read lots of currently unused bytes.
		CHECK_EOF(min);
		if(min==0xFF){
			uint type;
			bool done=false;
			while(!done){
				CHECK_EOF(type);
				switch(type){
				case'-':
					fgetc(pFile);
				case '+':
					fgetc(pFile);
					break;
				case 'x':
					done=true;
					break;
				DEFAULT(Checkv2::VarData::Init,type);
				}
			}
		}else{
			for(;i>0;i--)
				min|=fgetc(pFile)<<(i*8);//ignore EOF--I'll catch that later
			CHECK_EOF(max);
			for(i=temp.width;i>0;i--)
				max|=fgetc(pFile)<<(i*8);//ignore EOF--I'll catch that later
		}
	}
	*/
	width&=0x7F;
	if((var&0xE0)==0x60)maxparam=GetCheckByte(2v);
	return var;
}

//****************************************
// varRange
//****************************************

const uint varRange::rangemax[]={0,0xFF,0xFFFF,0,0xFFFFFFFF};

varRange::varRange(uint width):dflt(rangemax[width]),num(0),width(width){
	VERIFY(width&&width<5&&width!=3,width);
}

void varRange::UpdateRange(uint Var,uint op,uint shift,const PseudoSprite&data,uint&offs){
	uint add,divmod,nAnd=data.ExtractVariable(offs,width);
	if(Check2v::Prohibit0Mask(Var) && !nAnd)IssueMessage(WARNING1,AND_00,offs);
	offs+=width;
	range var(rangemax[width],0,(0xFFFFFFFF>>shift)&nAnd);
	if(shift&0xC0){
		add=data.ExtractVariable(offs,width);
		divmod=data.ExtractVariable(offs+=width,width);
		if(divmod){
			if((var.min+=add).LastOpOverflow()||(var.max+=add).LastOpOverflow()){
				var.min=0;
				var.max=rangemax[width];
			}
			if(shift&0x80){// %
				var.min=0;
				var.max=min<uint>(var.max,divmod-1);
				add%=divmod;
			}else{// /
				var.min/=divmod;
				var.max/=divmod;
			}
			if(!add&&!(divmod&(divmod-1)))
				IssueMessage(WARNING2,USE_SHIFT_AND,offs-3*width);
		}else{
			var.min=0;
			var.max=rangemax[width];
			IssueMessage(ERROR,DIVIDE_BY_ZERO,offs);
		}
		offs+=width;
	}
	assert(var.min<=var.max);
	assert(dflt.min<=dflt.max);
	switch(op){
	case(uint)-1://First run
	case 0xF:// Discard left
		dflt=var;
	case 0xE:// Store
	case 0x10://Persistent store
		break;
	case 0:// +
	case 0xC:// |
	case 0xD:// ^
		if((dflt.min+=var.min).LastOpOverflow()||(dflt.max+=var.max).LastOpOverflow()){
			dflt.min=0;
			dflt.max=rangemax[width];
		}
		break;
	case 1:// -
		if((dflt.min-=var.max).LastOpOverflow()||(dflt.max-=var.min).LastOpOverflow()){
			dflt.min=0;
			dflt.max=rangemax[width];
		}
		break;
	case 2:case 3:case 6:case 7://SIGNED min,max,/,%
		dflt.min=0;
		dflt.max=rangemax[width];
		//TODO: signed op support
		break;
	case 4:// min
	case 0xB:// &
        dflt.min=min(dflt.min,var.min);
        dflt.max=min(dflt.max,var.max);
		break;
	case 5:// max
        dflt.min=max(dflt.min,var.min);
        dflt.max=max(dflt.max,var.max);
	case 8:// /
		if(var.min==0){
			dflt.min=0;
			dflt.max=rangemax[width];
		}else{
			dflt.min/=var.max;
			dflt.max/=var.min;
		}
		break;
	case 9:// %
		dflt.min=0;
		dflt.max=var.max;
		break;
	case 0xA:// *
		if((dflt.min*=var.min).LastOpOverflow()||(dflt.max*=var.max).LastOpOverflow()){
			var.min=0;
			var.max=rangemax[width];
		}
		break;
	case 0x12:case 0x13:	// <=>
		dflt.min=0;
		dflt.max=2;
		break;
	case 0x11:	// ROR
	case 0x14:	// SHL
	case 0x15:	// SHR
	case 0x16:	// SAR
	default:
		dflt.min=0;
		dflt.max=rangemax[width];
	}
}

void varRange::AddRange(uint min,uint max){
	AddRangeInternal(min,max,UNREACHABLE_VAR);
	num++;
}

void varRange::CheckDefault(){
	num=-1;
	AddRangeInternal(dflt.min,dflt.max,UNREACHABLE_DEFAULT);
}

void varRange::AddRangeInternal(uint min,uint max,RenumMessageId unreachable){
	bool obscured=false,repeat;
	do{
		repeat=false;
		for(int i=0;i<(int)ranges.size();i++){
			if(min>=ranges[i].min&&min<=ranges[i].max){
				if(ranges[i].max>=dflt.max){
					IssueMessage(WARNING1,unreachable,num);
					return;
				}
				repeat=obscured=true;
				min=ranges[i].max+1;
			}
			if(max>=ranges[i].min&&max<=ranges[i].max){
				if(ranges[i].min<=dflt.min){
					IssueMessage(WARNING1,unreachable,num);
					return;
				}
				repeat=obscured=true;
				max=ranges[i].min-1;
			}
			if(min>max){
				IssueMessage(WARNING1,unreachable,num);
				return;
			}
		}
	}while(repeat);
	if(obscured&&num!=-1)
		IssueMessage(WARNING2,OBSCURED_VARIATION,num);
	ranges.push_back(range((uint)-1,min,max));
}

//****************************************
// rand2
//****************************************

rand2::rand2(){
	FILE*pFile=myfopen(2r);
	_p=new rand2info[MaxFeature()+1];
	for(uint i=0;i<=MaxFeature();i++){
		_p[i].bits[0]=fgetc(pFile);
		_p[i].bits[1]=fgetc(pFile);
		_p[i].numtriggers=fgetc(pFile);
	}
	CheckEOF(_p[MaxFeature()].numtriggers,2r);
	fclose(pFile);
}

void rand2::CheckRand(uint feat,uint type,uint triggers,uint first,uint nrand){
	if(feat>MaxFeature())return;
	type&=1;
	uint bits=0;
	while(nrand>>=1)bits++;
	if(first>_p[feat].bits[type])IssueMessage(ERROR,OUT_OF_RANGE_BITS,5,_p[feat].bits[type]);
	else if(first+bits>_p[feat].bits[type])IssueMessage(ERROR,OUT_OF_RANGE_BITS,6,_p[feat].bits[type]);
	triggers&=0x7F;
	if(triggers>>_p[feat].numtriggers)IssueMessage(WARNING1,UNDEFINED_TRIGGER);
}

//****************************************
// Define2
//****************************************

Define2::~Define2(){
	if(act123::CInstance().defined2IDs.is_defined(id)&&!act123::CInstance().defined2IDs.is_used(id))
		IssueMessage(WARNING1,UNUSED_ID,id,act123::CInstance().defined2IDs.defined_at(id));
	act123::Instance().defined2IDs.define(feature,id,checks1C);
}

void Define2::ChangeFeature(uint feat){
	feature=feat;
}

//****************************************
// Callbacks
//****************************************

Callbacks::Callbacks(){
	FILE*pFile=myfopen(callbacks);
	_p=new uint[numcallbacks=GetCheckWord(callbacks)];
	for(uint i=0;i<numcallbacks;i++){
		uint temp=GetCheckByte(callbacks);
		if(temp==0x7F)
			temp = GetCheckWord(callbacks) | (1<<31);
		else if(temp&0x80)temp ^= (1<<31 | 0x80);
		_p[i]=temp;
	}
	fclose(pFile);
}

//****************************************
// GLOBAL FUNCTIONS
//****************************************

void sanity_use_id(int id){
	act123::Instance().defined2IDs.use(id);
}

void sanity_use_set(int set){
	act123::Instance().act1.use(set);
}

void sanity_test_id(int id){
	act123::Instance().defined2IDs.test(0,id&0xFF);
}

int sanity_locate_id(int id){
	return act123::Instance().defined2IDs.defined_at(id&0xFF);
}

void sanity_define_id(int feature,int id){
	act123::Instance().defined2IDs.define(feature,id&0xFF,true);
}

int sanity_get_feature(int id){
	return act123::Instance().defined2IDs.GetFeature(id);
}
