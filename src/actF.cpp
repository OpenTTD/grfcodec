/*
 * actF.cpp
 * Contains definitions for checking action Fs.
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
#include<sstream>

using namespace std;

#include"nforenum.h"
#include"inlines.h"
#include"pseudo.h"
#include"messages.h"
#include"ExpandingArray.h"
#include"strings.h"
#include"command.h"

class IDarray{
public:
	void init(){used.resize(0);sprite.resize(0);}
	bool is_defined(int id)const{return sprite[id]!=0;}
	bool is_used(int id)const{return used[id];}
	uint defined_at(int id)const{return sprite[id];}
	void define(unsigned int id){
		used[id]=false;
		sprite[id]=_spritenum;
	}
	void use(int id){used[id]=true;}
protected:
	Expanding0Array<uint>sprite;
	Expanding0Array<bool>used;
}status;
static const IDarray&crStatus=status;

void InitF(){status.init();}

#define BITS(first, num) (((1U << (num)) - 1) << (first))

void CheckF(PseudoSprite&data){
	data.SetAllHex();
	uint id=data.ExtractByte(1),offset=2,langs,oldoff=0;
	bool isFinal=(id>=0x80);
	id&=0x7F;
	if(crStatus.is_defined(id)&&!status.is_used(id))IssueMessage(WARNING1,UNUSED_ID,id,status.defined_at(id));
	status.define(id);
	if(isFinal){
		status.use(id);
		ExpandingArray<uint>nameLocs;
		const ExpandingArray<uint>&cNameLocs=nameLocs;
		uint names=0;
		langs=data.ExtractByte(offset);
		do{
			if(langs&0x80)IssueMessage(WARNING2,UNKNOWN_LANG_BIT,offset,langs);
			langs&=0x7F;
			names++;
			if(names>1)data.SetEol(oldoff-1,1);
			oldoff=offset;
			if(_grfver<7){
				if(langs!=0x7F&&langs&0x60)IssueMessage(WARNING3,UNKNOWN_LANG_BIT,offset,langs);
				if(langs&0x10){if(nameLocs[4])IssueMessage(WARNING2,DUPLICATE_LANG_NAME,offset,4,nameLocs[4]);nameLocs[4]=offset;}
				if(langs&0x08){if(nameLocs[3])IssueMessage(WARNING2,DUPLICATE_LANG_NAME,offset,3,nameLocs[3]);nameLocs[3]=offset;}
				if(langs&0x04){if(nameLocs[2])IssueMessage(WARNING2,DUPLICATE_LANG_NAME,offset,2,nameLocs[2]);nameLocs[2]=offset;}
				if(langs&0x02){if(nameLocs[1])IssueMessage(WARNING2,DUPLICATE_LANG_NAME,offset,1,nameLocs[1]);nameLocs[1]=offset;}
				if(langs&0x01){if(nameLocs[0])IssueMessage(WARNING2,DUPLICATE_LANG_NAME,offset,0,nameLocs[0]);nameLocs[0]=offset;}
			}else{
				CheckLangID(langs,offset);
				if(nameLocs[langs])
					IssueMessage(WARNING2,DUPLICATE_LANG_NAME,offset,langs,nameLocs[langs]);
				nameLocs[langs]=offset;
			}
			if(CheckString(data,++offset,0)){
				IssueMessage(FATAL,OVERRAN_F_NAME,oldoff+1,langs);
				return;
			}
		}while((langs=data.ExtractByte(offset))!=0);
		if(names>1)data.SetEol(oldoff-1,1);
		if(_grfver<7){
			if(!nameLocs[4])IssueMessage(WARNING1,MISSING_LANG_NAME,4);
			if(!nameLocs[3])IssueMessage(WARNING1,MISSING_LANG_NAME,3);
			if(!nameLocs[2])IssueMessage(WARNING1,MISSING_LANG_NAME,2);
			if(!nameLocs[1])IssueMessage(WARNING1,MISSING_LANG_NAME,1);
			if(!nameLocs[0])IssueMessage(WARNING1,MISSING_LANG_NAME,0);
		}else if(!cNameLocs[0x7F])IssueMessage(WARNING1,MISSING_FALLBACK);
		offset++;
	}
	uint num_parts=data.SetEol(offset,1).ExtractByte(offset);
	if(isFinal)data.SetEol(offset-1,1);
	if(!num_parts)IssueMessage(ERROR,NO_PARTS,offset);
	uint bitsused=0;
	for(uint i=0;i<num_parts;i++){
		uint textcount=data.ExtractByte(++offset);
		if(!textcount)IssueMessage(ERROR,NO_PARTS,offset);
		uint total_prob=0,firstbit=data.ExtractByte(++offset),fb_offs=offset,numbits=data.ExtractByte(++offset);
		if(textcount>1){
			if(bitsused&BITS(firstbit,numbits)){
				ostringstream s;
				int first=-1,bits=bitsused&BITS(firstbit,numbits);
				for(int j=0;j<32;j++){
					if(bits&(1<<j)){
						if(first==-1)first=j;
						continue;
					}
					if(first==-1)continue;
					if(first==j-1)s<<first<<", ";
					else if(first==j-2)s<<first<<", "<<j-1<<", ";
					else s<<first<<".."<<j-1<<", ";
					first=-1;
				}
				string str=s.str();
				str[str.length()-2]='\0';
				IssueMessage(WARNING1,BITS_OVERLAP,fb_offs,i,str.c_str());
			}
			bitsused|=BITS(firstbit,numbits);
		}
		data.SetEol(offset,2);
		if(firstbit+numbits>32)IssueMessage(ERROR,OUT_OF_RANGE_BITS,fb_offs,32);
		for(uint j=0;j<textcount;/*Increment in SetEol call*/){
			uint prob=data.ExtractByte(++offset);
			total_prob+=prob&0x7F;
			if(!(prob&0x7F))IssueMessage(WARNING1,NO_PROBABILITY,offset);
			if(prob&0x80){
				uint newid=data.ExtractByte(++offset)&0x7F;
				if(!status.is_defined(newid))IssueMessage(ERROR,UNDEFINED_ID,offset,newid);
				else if(newid==id)IssueMessage(ERROR,RECURSIVE_F,offset);
				else status.use(newid);
			}else{
				oldoff=++offset;
				if(CheckString(data,offset,0)){
					IssueMessage(FATAL,OVERRAN_F_PART,oldoff);
					return;
				}
				offset--;
			}
			data.SetEol(offset,(++j==textcount)?1:2);
		}
		total_prob--;//beause 2^n has n+1 bits but only needs n bits of randomness.
		uint minbits=1;
		while(total_prob>>=1)minbits++;
		if(numbits<minbits)IssueMessage(WARNING1,INSUFFICIENT_BITS,fb_offs+1,minbits,numbits);
	}
	if(++offset!=data.Length())IssueMessage(WARNING2,EXTRA_DATA,data.Length(), offset);
}

void finalF(){
	ManualConsoleMessages();
	bool header=false;
	for(uint i=0;i<128;i++)
		if(crStatus.is_defined(i)&&!crStatus.is_used(i)){
			if(!header){
				IssueMessage(WARNING1,UNUSEDFIDLEAD,i);
				IssueMessage(WARNING1,UNEXP_EOF_TOWNNAMES,i);
				header=true;
			}
			IssueMessage(WARNING1,UNUSEDIDFINAL,i,crStatus.defined_at(i));
		}
}
