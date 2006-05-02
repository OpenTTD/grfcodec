/*
 * readinfo.cc 
 * Reads an NFO file into an array of parsed sprites.
 * 
 * Bastard child of NFORenum's renum.cpp, pseudo.cpp, and inlines.h
 *
 * Copyright 2004-2006 by Dale McCoy.
 * dalestan@gmail.com
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

/*
INFO VERSION HISTORY:
Version 1: First version
Version 2: Add sprite numbers, // (Info version x) line
Version 3: Change real-sprite format
Version 4: Fix bug in real-sprite format
Version 5: Add literal strings
Version 6: Add binary includes

*/


#include<iostream>
#include<string>
#include<sstream>

using namespace std;

#include"nfosprite.h"
#include"allocarray.h"

istream&eat_white(istream&in){
	while(isspace(in.peek()))in.ignore();
	return in;
}

int ctoi(char ch){
	if(ch>='0'&&ch<='9')return ch-'0';
	if(ch>='A'&&ch<='F')return ch-'A'+10;
	if(ch>='a'&&ch<='f')return ch-'a'+10;
	return 0;
}

string UCase(string str){
	size_t len=str.length();
	for(size_t i=0;i<len;i++)
		str[i]=(char)toupper(str[i]);
	return str;
}

string itoa(uint x){
	string ret;
	while(x){
		ret="0123456789"[x%10]+ret;
		x/=10;
	}
	return ret;
}

uint ReadHex(istream&in,uint digits){
	uint ret;
	char ch;
	eat_white(in).get(ch);
	if((ret=ctoi(ch))==0&&ch!='0'){
		in.unget().clear(ios::badbit);
		return ret;
	}
	for(;--digits;){
		in.get(ch);
		if(ctoi(ch)==0&&ch!='0'){
			in.unget();
			return ret;
		}
		ret<<=4;
		ret|=ctoi(ch);
	}
	return ret;
}

const char*const VALID_PSEUDO="0123456789ABCDEFabcdef \t\v\r\n",
	*const WHITESPACE=" \t\v\r\n",
	*const COMMENT="#;/";

#define NPOS (string::npos)

#define checkspriteno()\
	if(spriteno!=-1&&spriteno!=(int)sprites.size()){\
		printf("Warning: Found sprite %d looking for sprite %d.\n",spriteno,sprites.size());\
	}else(void(0))


#define flush_buffer()\
	if(true){\
		if(buffer!=""){\
			checkspriteno();\
			sprites.push_back(Pseudo(sprites.size(),infover,buffer,claimed_size));\
			buffer="";\
		}\
		spriteno=temp;\
	}else\
		(void(0))

void read_file(istream&in,int infover,AllocArray<Sprite>&sprites){
	string sprite,datapart,buffer;

	int temp=-1,spriteno=-1,claimed_size=1;
	string::size_type firstnotpseudo;
	while(true){
		getline(in,sprite);
		//IssueMessage(0,SPRITE,spritenum+1,sprite.c_str());
		istringstream spritestream(sprite);
		eat_white(spritestream);
		if(spritestream.peek()==EOF || // blank
			strchr(COMMENT,spritestream.peek())){ // comment
		}else{//sprite
			if(!eat_white(spritestream>>temp)){
				spritestream.clear();
				temp=-1;
			}
			if(spritestream.peek()=='*'){
				if(spritestream.ignore().peek()=='*'){
					flush_buffer();
					getline(eat_white(spritestream.ignore()),datapart);
					checkspriteno();
					sprites.push_back(Include(datapart));
				}else{
					flush_buffer();
					eat_white(spritestream>>claimed_size);
					getline(spritestream,buffer);
					buffer+='\n';
				}
			}else{
				getline(spritestream,datapart);
				firstnotpseudo=datapart.find_first_not_of(VALID_PSEUDO);
				if((!spritestream||firstnotpseudo==NPOS||
					(datapart[firstnotpseudo]=='"'&&infover>4)||
					(datapart[firstnotpseudo]=='\\'&&infover>6)||
					strchr(COMMENT,datapart[firstnotpseudo]))&&
					Pseudo::MayBeSprite(buffer)){
						buffer+=sprite+'\n';
				}else{
					flush_buffer();
					checkspriteno();
					sprites.push_back(Real(sprites.size(),infover,datapart));
				}
			}
		}
		if(in.peek()==EOF){
			flush_buffer();
			return;
		}
	}
}

Sprite::unparseable::unparseable(string reason,size_t sprite){
	this->reason="Error: "+reason+".\n\tWhile reading sprite:"+itoa((int)sprite)+'\n';
}

Real::Real(size_t sprite,int infover,const string&data){
	string::size_type loc=NPOS;
	string udata=UCase(data);
	while(true){
		loc=udata.find(".PCX",loc+1);
		if(loc==NPOS)
			throw Sprite::unparseable("Could not find filename",sprite);
		if(isspace(data[loc+4]))break;
	}
	if((name=data.substr(0,loc+4))!=prevname){
		prevy=0;
		prevname=name;
	}
	const char*meta=data.c_str()+loc+5;
	if(infover<3){
		unsigned int intinf[8];
		if(sscanf(data.c_str(), "%d %d %x %x %x %x %x %x %x %x",
			&xpos, &ypos,
			&(intinf[0]), &(intinf[1]), &(intinf[2]), &(intinf[3]),
			&(intinf[4]), &(intinf[5]), &(intinf[6]), &(intinf[7]))!=10)
			throw Sprite::unparseable("Insufficient meta-data",sprite);
		for(int i=0;i<8;i++){
			if(intinf[i]>0xFF)
				throw Sprite::unparseable("\"Byte\" "+itoa(i)+" isn't.",sprite);
			inf[i]=U8(intinf[i]);
		}
	}else{
		int sx,sy,rx,ry,comp;
		if(sscanf(meta,"%d %d %2x %d %d %d %d",&xpos,&ypos,&comp,&sy,&sx,&rx,&ry)!=7){
			throw Sprite::unparseable("Insufficient meta-data",sprite);
		}
		if(sx<1)throw Sprite::unparseable("xsize is too small",sprite);
		if(sx>0xFFFF)throw Sprite::unparseable("xsize is too large",sprite);
		if(sy<1)throw Sprite::unparseable("ysize is too small",sprite);
		if(sy>0xFF)throw Sprite::unparseable("ysize is too large",sprite);
		if(rx<-32768)throw Sprite::unparseable("xrel is too small",sprite);
		if(rx>32767)throw Sprite::unparseable("xrel is too large",sprite);
		if(ry<-32768)throw Sprite::unparseable("yrel is too small",sprite);
		if(ry>32767)throw Sprite::unparseable("yrel is too large",sprite);
		inf[0] = U8(comp);
		inf[1] = U8(sy);
		inf[2] = U8(sx & 0xff);
		inf[3] = U8(sx >> 8);
		inf[4] = U8(rx & 0xff);	
		inf[5] = U8(rx >> 8);
		inf[6] = U8(ry & 0xff);
		inf[7] = U8(ry >> 8);
	}
	if (infover < 4)
		ypos++;	// bug, had an extra line at the top
	if(xpos<0)throw Sprite::unparseable("xpos is too small",sprite);
	if(ypos<0)throw Sprite::unparseable("ypos is too small",sprite);
	else if(ypos<prevy)throw Sprite::unparseable("ypos is decreasing",sprite);
	prevy=ypos;
}

string Real::prevname;
int Real::prevy=0;

Pseudo::Pseudo(size_t num,int infover,const string&sprite,int claimed_size){
	istringstream in(sprite);
	ostringstream out;
	char ch;
	while(in){
		eat_white(in);
		switch(in.peek()){
		case EOF:continue;
		case'"':
			in.ignore();
			while(true){
				if(!in.get(ch))
					throw Sprite::unparseable("Unterminated literal string",num);
				if(ch=='"')
					break;
				if(ch=='\\'&&infover>6){
					switch(ch=(char)in.get()){
					case'n':
						ch='\r';//TTD uses Mac-style linefeeds (\r)
						break;
					case'"':
					case'\\':
						break;
					default:
						in.unget();
						ch=(char)ReadHex(in,2);
						if(!in)
							throw Sprite::unparseable("Could not parse quoted escape sequence",num);
						break;
					}
				}
				out.put(ch);
			}
			break;
		case'/':case'#':case';'://comment
			in.ignore(INT_MAX,'\n');
			break;
		default:
			ch=(char)ReadHex(in,2);
			if(!in)
				throw Sprite::unparseable("Encountered invalid character looking for literal byte",num);
			out.put(ch);
		}
	}
	packed=out.str();
	if(!size())
		throw Sprite::unparseable("Found a zero-byte pseudo-sprite",num);
	if(size()!=(uint)claimed_size&&claimed_size!=0)
		printf("Warning: Sprite %d reports %d bytes, but I found %d.\n",num,claimed_size,size());
}

uint Pseudo::size()const{return (uint)packed.size();}

bool Pseudo::MayBeSprite(const string&sprite){
	istringstream in(sprite);
	char ch;
	while(in.get(ch)){
		if(ch=='"')return true;
		if(strchr(COMMENT,ch)){
			in.ignore(INT_MAX,'\n');
			continue;
		}
		if(isspace(ch)||string(VALID_PSEUDO).find(ch)==NPOS)continue;
		return true;
	}
	return false;
}

Include::Include(const string&data):name(data){}
