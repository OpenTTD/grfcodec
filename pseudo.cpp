/*
 * pseudo.cpp
 * Implementation of the PseudoSprite class.
 *
 * Copyright 2006-2008 by Dale McCoy.
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

#include<sstream>
#include<iostream>
#include<iomanip>
#include<cstdarg>

/* If your compiler errors on the following line, boost is not
 * properly installed.
 * Get boost from http://www.boost.org */
#include <boost/date_time/gregorian/gregorian_types.hpp>
using namespace boost::gregorian;

using namespace std;

#include"renum.h"
#include"pseudo.h"
#include"globals.h"
#include"messages.h"
#include"inlines.h"
#include"command.h"
#include"utf8.h"

extern int NFOversion;

bool TrySetVersion(int);

enum{HEX,TEXT,UTF8,ENDQUOTE,QESC,QEXT,NQEXT,NOBREAK=0x80};


#define cur_pos() ((uint)out.str().length()-1)
#define next_pos() ((uint)out.str().length())
#define cur_context() (context[cur_pos()])
#define ProcessWhite()\
	if(white!=""){\
		if(white[white.length()-1]==' '&&!GetState(CONVERTONLY))white=white.substr(0,white.length()-1);\
		if((cur_context()!=""&&cur_context().find('\n')!=NPOS)||GetState(CONVERTONLY))cur_context()+=white;\
		white="";\
	}else ((void)0)


PseudoSprite::PseudoSprite(const string&sprite,int oldspritenum):
	orig(sprite),
	valid(true),
	useorig(false),
	oldspritenum(oldspritenum)
{
	istringstream in(sprite);
	ostringstream out;
	char ch;
	bool newline=true;
	string white;
	while(in){
		switch(in.peek()){
		case EOF:continue;
		case'"':
			TrySetVersion(5);
			in.ignore();
			ProcessWhite();
			while(true){
				if(!in.get(ch)){
					IssueMessage(0,UNTERMINATED_STRING);
					Invalidate();
					return;
				}
				if(ch=='"')break;
				if(ch=='\\'&&TrySetVersion(7)){
					switch(ch=(char)in.get()){
					case'n':
						ch='\r';//TTD uses Mac-style linefeeds (\r)
						break;
					case'"':
					case'\\':
						break;
					case'U':{
						uint x=ReadHex(in,4);
						if(!in){
							IssueMessage(0,INVALID_EXTENSION);
							Invalidate();
							return;
						}
						if(x>0x7FF){
							SetUTF8(next_pos(),3);
							out.write(GetUtf8Encode(x).c_str(),3);
							continue;
						}else if(x>0x7F){
							SetUTF8(next_pos(),2);
							out.write(GetUtf8Encode(x).c_str(),2);
							continue;
						}
						break;
					}default:
						in.unget();
						ch=(char)ReadHex(in,2);
						if(!in){
							IssueMessage(0,INVALID_EXTENSION);
							Invalidate();
							return;
						}
						break;
					}
				}
				SetText(next_pos());
				newline=false;
				out.put(ch);
			}
			break;
		case'/':case'#':case';':{
			string comment;
			getline(in,comment);
			comment=white+comment;
			white="";
			if(is_command(comment)||parse_comment(comment)) {
				if(newline) {
                    AddComment(comment,cur_pos());
				} else {
                    TrailComment(comment,cur_pos());
                }
            }
			newline=true;
			break;
		}case'\\':
			if(TrySetVersion(7)){
				ProcessWhite();
				newline=false;
				in.ignore();
				uint x;
				switch(in.get()){
				case'2':{
					bool sign=true;
					if(in.peek()=='u'){
						sign=false;
						in.ignore();
					}
					switch(in.get()){
					case'+':out.put(0x00);continue;
					case'-':out.put(0x01);continue;
					case'<':out.put(sign?0x02:0x04);continue;
					case'>':out.put(sign?0x03:0x05);continue;
					case'/':out.put(sign?0x06:0x08);continue;
					case'%':out.put(sign?0x07:0x09);continue;
					case'*':out.put(0x0A);continue;
					case'&':out.put(0x0B);continue;
					case'|':out.put(0x0C);continue;
					case'^':out.put(0x0D);continue;
					case's':out.put(0x0E);
						if(in.get()=='t'&&in.peek()=='o'){
							in.ignore();
							continue;
						}
						in.unget();
						continue;
					case'r':{
						char ch=(char)in.get();
						if(ch=='o' && (in.peek()=='r' || in.peek()=='t')){
							out.put(0x11);
							in.ignore();
							continue;
						}
						out.put(0x0F);
						if(ch=='s' && in.peek()=='t'){
							in.ignore();
							continue;
						}
						in.unget();
						continue;
					}}
					break;
				}case'7':{
					switch(in.get()){
					case'1':out.put(0x00);continue;
					case'0':out.put(0x01);continue;
					case'=':out.put(0x02);continue;
					case'!':out.put(0x03);continue;
					case'<':out.put(0x04);continue;
					case'>':out.put(0x05);continue;
					case'G':
						if(in.get()=='G'){// \7GG Is or will be active
							out.put(0x09);
							continue;
						}
						in.unget();
						out.put(0x06);// \7G Is active
						continue;
					case'g':
						switch(in.get()){
						case'g':out.put(0x0A);continue;// \7gg is not and will not be active
						case'G':out.put(0x08);continue;// \7gG is not active but will be
						default:// \7g is not active
							in.unget();
							out.put(0x07);
							continue;
						}
					case'c':out.put(0x0B);continue;
					case'C':out.put(0x0C);continue;
					}
					break;
				}case'D':{
					bool sign=true;
					if(in.peek()=='u'){
						sign=false;
						in.ignore();
					}
					switch(in.get()){
					//<operation>
					case'=':out.put(0x00);continue;
					case'+':out.put(0x01);continue;
					case'-':out.put(0x02);continue;
					case'*':out.put(sign?0x04:0x03);continue;
					case'<':
						if(in.get()=='<'){
							out.put(sign?0x06:0x05);
							continue;
						}
						break;
					case'&':out.put(0x07);continue;
					case'|':out.put(0x08);continue;
					case'/':out.put(sign?0x0A:0x09);continue;
					case'%':out.put(sign?0x0C:0x0B);continue;
					//GRM <operation>
					case'R':out.put(0x00);continue;
					case'F':out.put(0x01);continue;
					case'C':out.put(0x02);continue;
					case'M':out.put(0x03);continue;
					case'n'://no fail
						switch(in.get()){
						case'F':out.put(0x04);continue; //\DnF
						case'C':out.put(0x05);continue; //\DnC
						}
						break;
					case'O':out.put(0x06);continue;
					}
				}case'b':
					if(in.peek()=='*'){// \b*
						in.ignore()>>x;
						if(!in||x>0xFFFF)break;//invalid
						if(x>0xFE)out.put('\xFF').write(itoa(x,256).c_str(),2);
						else out.put((char)x);
						continue;
					}
					in>>x;
					if(x>=1920)x-=1920;//remap years
					if(!in||x>0xFF)break;//invalid
					out.put((char)x);
					continue;
				case'w':
					if(in.peek()=='x'){// \wx
						in.ignore();
						x=ReadHex(in,4);
					}else{
						in>>x;
						//delay fail check until after date parsing.
						if(in.peek()=='/'||in.peek()=='-'){//date
							in.ignore();
							ushort y,z;
							in>>y;
							if(/*!in||*/(in.peek()!='/'&&in.peek()!='-'))//in.peek will return eof if !in
								break;
							in.ignore();
							in>>z;
							if(x==0)x=2000;
							if(x>31&&x<100)x+=1900;
							if(z==0)z=2000;
							if(z>31&&z<100)z+=1900;
							date d;
							try{
								if(x>1919)d=date((ushort)x,y,z);
								else d=date(z,y,(ushort)x);
							}catch(std::out_of_range){
								break;
							}
							x=(d-date(1920,1,1)).days();
						}
					}
					if(!in)break;
					if(x>0xFFFF)break;//invalid
					out.write(itoa(x,256,2).c_str(),2);
					continue;
				case'd':
					if(in.peek()=='x'){// \wx
						in.ignore();
						x=ReadHex(in,8);
					}else{
						in>>x;
						if(in.peek()=='/'||in.peek()=='-'){//date
							in.ignore();
							ushort y,z;
							in>>y;
							if(/*!in||*/(in.peek()!='/'&&in.peek()!='-'))//in.peek will return eof if !in
								break;
							in.ignore();
							in>>z;
							date d;
							int extra=701265;
//Boost doesn't support years out of the range 1400..9999, so move wider dates back in range.
#define adjyear(x) \
	while(x>9999){ \
		x-=400; \
		extra += 365*400 + 97; /* 97 leap years every 400 years. */ \
	} \
	while(x<1400){ \
		x+=400; \
		extra -= 365*400 + 97; \
	}

							try{
								if(z<32){
									adjyear(x);
									d=date((ushort)x,y,z);
								}else{
									adjyear(z);
									d=date(z,y,(ushort)x);
								}
								x=(d-date(1920,1,1)).days();
							}catch(std::out_of_range){
								break;
							}
							x+=extra;
						}
					}
					if(!in)break;
					out.write(itoa(x,256,4).c_str(),4);
					continue;
				}

 				IssueMessage(0,INVALID_EXTENSION);
				Invalidate();
				return;
			}
			//fall through to default when !GetState(EXTENSIONS)
		default:
			if(isspace(in.peek()))
				if(in.peek()=='\n'&&!GetState(CONVERTONLY)){
					if(newline)AddBlank(cur_pos());
					newline=true;
					white="";
					in.ignore();
				}else
					white+=(char)in.get();
			else{
				ch=(char)ReadHex(in,2);
				if(!in){
					in.clear();
					IssueMessage(0,INVALID_CHARACTER,in.peek());
					Invalidate();
					return;
				}
				ProcessWhite();
				newline=false;
				out.put(ch);
			}
		}
	}
	packed=out.str();
	if(white!=""&&Length()){
		if(white[white.length()-1]==' '&&!GetState(CONVERTONLY))white=white.substr(0,white.length()-1);
		if((cur_context()!=""&&cur_context().find('\n')!=NPOS)||GetState(CONVERTONLY))cur_context()+=white;
		white="";
	};
}

bool PseudoSprite::MayBeSprite(const string&sprite){
	istringstream in(sprite);
	char ch;
	while(in.get(ch)){
		if(ch=='"')return true;
		if(COMMENT.find(ch)!=NPOS){
			in.ignore(INT_MAX,'\n');
			continue;
		}
		if(isspace(ch)||string(VALID_PSEUDO).find(ch)==NPOS)continue;
		return true;
	}
	return false;
}

uint PseudoSprite::Length()const{return(uint)(valid?packed.length():0);}

PseudoSprite&PseudoSprite::SetHex(uint i){beauty[i]=HEX;return*this;}
PseudoSprite&PseudoSprite::SetHex(uint i,uint num){while(--num)beauty[i++]=HEX;return*this;}
PseudoSprite&PseudoSprite::SetAllHex(){beauty.clear();return*this;}
PseudoSprite&PseudoSprite::SetUTF8(uint i,uint len){while(--len)beauty[i++]=UTF8;return*this;}
PseudoSprite&PseudoSprite::SetText(uint i){
	if(i&&GetState(CONVERTONLY)){
		if((beauty[i-1]&~NOBREAK)==ENDQUOTE&&context[i-1]=="")context[i-1]=" ";
	}
	beauty[i]=TEXT;
	return*this;
}
PseudoSprite&PseudoSprite::SetText(uint i,uint num){
	for(uint j=i+num-1;i<j;i++)SetText(i);
	return SetEot(i);
}

PseudoSprite&PseudoSprite::SetQEscape(uint i){
	if(GetState(USEESCAPES))
		beauty[i]=QESC;
	else
        SetHex(i);
	return*this;
}

PseudoSprite&PseudoSprite::SetQEscape(uint i,uint num){
	if(GetState(USEESCAPES)){
		while(--num)
			beauty[i++]=QESC;
		return *this;
	}
	return SetHex(i,num);
}

PseudoSprite&PseudoSprite::SetEscapeWord(uint i){
	return SetEscape(i,false,mysprintf(" \\wx%x",ExtractWord(i)),2);
}

PseudoSprite&PseudoSprite::SetEscape(uint i, bool quote, string ext, uint len){
	if(GetState(USEESCAPES)){
		while(len--){
			ext_print[i+len]="";
			beauty[i+len]=char(quote?QEXT:NQEXT);
		}
		ext_print[i]=ext;
		return *this;
	}
	return SetHex(i,len);
}

PseudoSprite&PseudoSprite::SetEot(uint i){beauty[i]=ENDQUOTE;return*this;}
PseudoSprite&PseudoSprite::SetEol(uint i,uint minbreaks,uint lead){
	if(GetState(CONVERTONLY)||GetState(LINEBREAKS)<minbreaks||i+1==Length())return*this;
	if(context[i].find_first_of('\n')==NPOS)context[i]+="\n";
	if(context[i][context[i].length()-1]=='\n')context[i]+=string(GetState(LEADINGSPACE,lead),' ');
	return*this;
}
PseudoSprite&PseudoSprite::SetNoEol(uint i){
	if(GetState(LINEBREAKS))beauty[i]|=NOBREAK;return*this;
}

PseudoSprite&PseudoSprite::SetGRFID(uint i){
	if(CanQuote((*this)[i])&&CanQuote((*this)[i+1])&&(*this)[i]!=0xFF&&(*this)[i+1]!=0xFF&&!GetState(HEXGRFID)){
		SetText(i);
		SetText(i+1);
	}else{
		SetHex(i);
		SetHex(i+1);
	}
	SetHex(i+2);
	SetHex(i+3);
	SetNoEol(i);
	SetNoEol(i+1);
	SetNoEol(i+2);
	return*this;
}

PseudoSprite&PseudoSprite::SetByteAt(uint off,uint byte){
	VERIFY(off<packed.length(),off);
	assert(byte<0x100);
	packed[off]=(uchar)byte;
	return*this;
}

PseudoSprite&PseudoSprite::Append(uchar byte){
	context.resize(Length()+1);
	context[Length()]=context[Length()-1];
	context[Length()-1].clear();
	packed.append(1,byte);
	return*this;
}

PseudoSprite&PseudoSprite::PadAfter(uint i,uint width){
	if(context[i]=="")context[i]=string(width,' ');
	return*this;
}

static uint(PseudoSprite::* const ExtractFuncs[])(uint)const=
	{NULL,&PseudoSprite::ExtractByte,&PseudoSprite::ExtractWord,&PseudoSprite::ExtractExtended,&PseudoSprite::ExtractDword};

uint PseudoSprite::ExtractVariable(uint offs,uint length)const{
	VERIFY((length&&length<5),length);
	return(this->*(ExtractFuncs[length]))(offs);
}

uint PseudoSprite::ExtendedLen(uint offs)const{
	return ExtractByte(offs)!=0xFF?1:3;
}

uint PseudoSprite::ExtractByte(uint offs)const{
	VERIFY(IsValid(),IsValid());
	if(Length()<=offs)
		throw offs|1<<24;
	return(uchar)packed[offs];
}

uint PseudoSprite::ExtractWord(uint offs)const{
	try{
		return ExtractByte(offs)|ExtractByte(offs+1)<<8;
	}catch(unsigned int){
		throw offs|2<<24;
	}
}

uint PseudoSprite::ExtractExtended(uint offs)const{
	uint byte=ExtractByte(offs);
	if(byte!=0xFF)return byte;
	return ExtractWord(offs+1);
}

uint PseudoSprite::ExtractDword(uint offs)const{
	try{
		return ExtractWord(offs)|ExtractWord(offs+2)<<16;
	}catch(unsigned int){
		throw offs|4<<24;
	}
}

void PseudoSprite::AddComment(const string&str,uint i){
	if(i==(uint)-1)NoBeautify();
	else if(context[i]=="")context[i]+='\n'+str+'\n';
	else if(context[i][context[i].length()-1]=='\n')context[i]+=str+'\n';
	else{
		uint offs=(uint)context[i].find_last_of('\n')+1;
		context[i]=context[i].substr(0,offs)+str+context[i].substr(offs);
	}
}

void PseudoSprite::TrailComment(const string&str,uint i){
	if(i==(uint)-1)NoBeautify();
	else if(context[i]=="")context[i]=str+'\n';
	else if(context[i][0]!='\n')return;
	else context[i]=str+'\n'+context[i];
}

void PseudoSprite::AddBlank(uint i){
	if(i==(uint)-1)NoBeautify();
	else if(context[i]=="")context[i]="\n\n";
	else if(!Length())context[i]+='\n';
	else{
		uint offs=(uint)context[i].find_first_of('\n');
		if(offs==NPOS||context[i].substr(offs,2)!="\n\n")
			context[i]=context[i].substr(0,offs)+'\n'+context[i].substr(offs);
	}
}

void PseudoSprite::NoBeautify(){
	useorig=true;
}

ostream&operator<<(ostream&out,PseudoSprite&sprite){
	sprite.output(out);
	return out;
}

ostream&PseudoSprite::output(ostream&out){
	if(!valid){
		istringstream datastream(orig);
		string line;
		getline(datastream,line);
		out<<COMMENT_PREFIX<<"    0 * 0\t "<<line<<endl;
		while(getline(datastream,line)){
			if(!is_comment(line))out<<COMMENT_PREFIX;
			out<<line<<endl;
		}
		return out;
	}
	if(UseOrig()){
		if(Length())
			out<<setw(5)<<spritenum()<<" * "<<(GetState(DIFF)?0:Length())<<"\t ";
		return out<<orig;
	}
	bool instr=false,noendl=false;
	uint count=16;
	out<<setw(5)<<spritenum()<<" * "<<(GetState(DIFF)?0:Length())<<"\t";

//This section contains a rewrite of lines 402-438 or thereabouts of info.cc
//from grfcodec v0.9.7: http://www.ttdpatch.net/grfcodec
//grfcodec is Copyright 2000-2005 Josef Drexler
	for(uint i=0;i<Length();i++) {
		if(DoQuote(i)){
			if (!instr){
				out<<" \""+((GetState(CONVERTONLY)&&i&&context[i-1]!="")?1:0);
				count+=3-((GetState(CONVERTONLY)&&i&&context[i-1]!="")?1:0);// count close-quote here.
				instr=true;
			}
			if(NFOversion>6){
				if((beauty[i]&~NOBREAK)==QESC){
					if((*this)[i]=='\r'){
						out<<"\\n";
						count=+2;
					}else{
						out<<mysprintf("\\%2x",(*this)[i]);
						count+=3;
					}
				}else if((beauty[i]&~NOBREAK)==QEXT){
					out<<ext_print[i];
					count+=(uint)ext_print[i].size();
				}else if((*this)[i]=='"'){
					out<<"\\\"";
					count+=2;
				}else if((*this)[i]=='\\'){
					out<<"\\\\";
					count+=2;
				}else{
					out<<(char)(*this)[i];
					count++;
				}
			}else{
				out<<(char)(*this)[i];
				count++;
			}
			noendl=false;
		} else {
			if(instr){
				out<<'"';
				instr=false;
			}
			if(NFOversion>6 && (beauty[i]&~NOBREAK)==NQEXT){
				out<<ext_print[i];
				count+=(uint)ext_print[i].size();
			}else{
				out<<mysprintf(" %2x"+((GetState(CONVERTONLY)&&i&&context[i-1]!="")?1:0),(*this)[i]);
				count+=3;
			}
			noendl=false;
		}

		//Context control (comments, beautifier controlled newlines, &c.)
		if(IsEot(i)&&instr){
			out<<'"';
			instr=false;
		}
		if(context[i]==" " && ((instr && i+1<Length() && (IsText(i)||(IsUTF8(i)&&GetState(QUOTEUTF8)))) || (beauty[i]&~NOBREAK)==NQEXT)) {
			context[i]="";
		} else if(context[i]!="") {
			if(instr){
				out<<'"';
				instr=false;
			}
			for(const char*ch=context[i].c_str();*ch;ch++){
				count++;
				if(*ch=='\n'){
					count=0;
					noendl=true;
				}
				out<<*ch;
			}
		}
		// Insert line-breaks after current charater:
		// when not CONVERTONLY and
		// when not in GRFids and when MAXLEN !=0 and
		//   after GetState(MAXLEN)-15 non-text characters or
		//   at spaces after GetState(MAXLEN)-15 text characters or
		//   after GetState(MAXLEN) characters
		// and when not last character
		if(!GetState(CONVERTONLY)&&
			IsLinePermitted(i)&&GetState(MAXLEN)&&
			((!instr && count>=GetState(MAXLEN)-15) ||
			(instr && ((count>=GetState(MAXLEN)-15 && (*this)[i]==' ') ||
					   count>=GetState(MAXLEN))))
			&& i<Length()-1) {
			if(instr){
				out<<'"';
				instr=false;
			}
			out<<(noendl?"":"\n")<<string(count=GetState(LEADINGSPACE,2),' ');
		}
	}
	if (instr)out<<'"';
	if(noendl)return out;
	return out<<endl;
}

bool PseudoSprite::CanQuote(uint byte){
	VERIFY(byte<0x100,byte);
	return(byte<0x80||GetState(QUOTEHIGHASCII))&&
	//non-printable ASCII are 00..1F, 22(sometimes), 7F..9F.
	//(String parser is responsible for marking others text/hex/UTF8 as appropriate)
		byte>0x1F&&(byte!='"'||NFOversion>6)&&(byte<0x7F||byte>0x9F);
}

bool PseudoSprite::DoQuote(uint i)const{
	if((beauty[i]&~NOBREAK)==QESC || (beauty[i]&~NOBREAK)==QEXT) return NFOversion>6;	// Quote IFF we have escapes
	return (CanQuote((*this)[i])&&IsText(i))||(IsUTF8(i)&&GetState(QUOTEUTF8));
}

void PseudoSprite::Invalidate(){
	IssueMessage(0,PARSE_FAILURE,_spritenum);
	valid=false;
	SetCode(EPARSE);
}

bool PseudoSprite::IsText(uint i)const{
	int type = uchar(beauty[i])&~NOBREAK;
	if (NFOversion>6)
		type = type==TEXT || type==ENDQUOTE || type==QESC || type==QEXT;
	else
		type = type==TEXT || type==ENDQUOTE;
	return type &&!(i&&(beauty[i-1]&~NOBREAK)==UTF8);
}
bool PseudoSprite::IsUTF8(uint i)const{
	return (beauty[i]&~NOBREAK)==UTF8||(i&&(beauty[i-1]&~NOBREAK)==UTF8);
}
bool PseudoSprite::IsEot(uint i)const{
	return (beauty[i]&~NOBREAK)==ENDQUOTE;
}
bool PseudoSprite::IsLinePermitted(uint i)const{
	return!(i&&((beauty[i-1]&~NOBREAK)==UTF8||beauty[i]&NOBREAK));
}

bool PseudoSprite::UseOrig()const{
	return useorig||!GetState(BEAUTIFY);
}
