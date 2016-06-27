/*
 * pseudo.cpp
 * Implementation of the PseudoSprite class.
 *
 * Copyright 2006-2010 by Dale McCoy.
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

#include<sstream>
#include<iostream>
#include<iomanip>
#include<cstdarg>
#include<cstdio>

/* If your compiler errors on the following lines, boost is not
 * properly installed.
 * Get boost from http://www.boost.org */
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/tokenizer.hpp>

using namespace boost::gregorian;
#define foreach BOOST_FOREACH
using namespace std;

#include"nforenum.h"
#include"pseudo.h"
#include"globals.h"
#include"messages.h"
#include"inlines.h"
#include"command.h"
#include"utf8.h"
extern int NFOversion;

bool TrySetVersion(int);

//QESC: \n or \xx
//		\" and \\ are implicit in TEXT.
//QEXT: \Uxxxx, usually.
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

int FindEscape(string str);
string FindEscape(char, int);
string FindEscape(char, int, uint);

PseudoSprite::PseudoSprite(const string&sprite,int oldspritenum):
	orig(sprite),
	valid(true),
	useorig(false),
	oldspritenum(oldspritenum),
	extract_offs(0)
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
							LinkBytes(3,out.str().length());
							continue;
						}else if(x>0x7F){
							SetUTF8(next_pos(),2);
							out.write(GetUtf8Encode(x).c_str(),2);
							LinkBytes(2,out.str().length());
							continue;
						}else{
							out.put(ch);
							LinkBytes(1,out.str().length());
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
		case'\\':
			if(TrySetVersion(7)){
				ProcessWhite();
				newline=false;
				in.ignore();
				uint x;
				switch(in.get()){
				case'b':
					if(in.peek()=='*'){// \b*
						x = ReadValue(in.ignore(), _BX_);
						if(!in||x>0xFFFF)break;//invalid
						if(x>0xFE){
							out.put('\xFF').write(itoa(x,256,2).c_str(),2);
							LinkBytes(3,out.str().length());
						}else{
							out.put((char)x);
							LinkBytes(1,out.str().length());
						}
						continue;
					}
					x = ReadValue(in, _B_);
					if(!in||x>0xFF)break;//invalid
					out.put((char)x);
					LinkBytes(1,out.str().length());
					continue;
				case'w':
					x = ReadValue(in, _W_);
					if(!in||x>0xFFFF)break;//invalid
					out.write(itoa(x,256,2).c_str(),2);
					LinkBytes(2,out.str().length());
					continue;
				case'd':
					x = ReadValue(in, _D_);
					if(!in)break;
					out.write(itoa(x,256,4).c_str(),4);
					LinkBytes(4,out.str().length());
					continue;
				default:{
					in.unget();
					string esc;
					in>>esc;
					int byte = FindEscape(esc);
					if(byte == -1)
						break;
					out.put((char)byte);
					LinkBytes(1,out.str().length());
					continue;
				}}

 				IssueMessage(0,INVALID_EXTENSION);
				Invalidate();
				return;
			}
			//fall through to default when !GetState(EXTENSIONS)
		default:
			if (is_comment(in)) {
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
			} else if(isspace(in.peek()))
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

void PseudoSprite::LinkBytes(int count, size_t e){
	int end = (int)e;
	for(int i=end-count,high=0; i<end; i++,high++)
		linkage[i]=high<<8|count;
}

bool PseudoSprite::ignorelinkage=false;

void PseudoSprite::CheckLinkage(int ofs, int count)const{
	if(!ignorelinkage) {
		for(int i=0;i<count;i++) {
			if(linkage[ofs+i] != 0 && linkage[ofs+i] != (i<<8 | count)) {
				IssueMessage(WARNING2,EXTENSION_MISMATCH,ofs+i,((linkage[ofs+i]>>8)&0xFF)+1,linkage[ofs+i]&0xFF,i+1,count);
				return;
			}
		}
	}
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
PseudoSprite&PseudoSprite::SetHex(uint i,uint num){while(num--)beauty[i++]=HEX;return*this;}
PseudoSprite&PseudoSprite::SetAllHex(){beauty.clear();return*this;}
PseudoSprite&PseudoSprite::SetUTF8(uint i,uint len){
	while(--len)
		beauty[i++]=uchar(UTF8|NOBREAK);
	beauty[i++]=UTF8;
	return*this;
}
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

PseudoSprite&PseudoSprite::SetOpByte (uint i, char action) {
	string s = FindEscape(action, ExtractByte(i));
	if (s != "") SetEscape(i, false, s, 1);
	return *this;
}
PseudoSprite&PseudoSprite::SetPositionalOpByte (uint i, char action) {
	string s = FindEscape(action, ExtractByte(i), i);
	if (s != "") SetEscape(i, false, s, 1);
	return *this;
}

PseudoSprite&PseudoSprite::SetDate(uint i, uint num) {
	assert(num==1 || num==2 || num==4);
	switch(num){
	case 1:
		return SetEscape(i, false, mysprintf(" \\b%d", 1920+ExtractByte(i)), 1);
	case 2:{
		date::ymd_type ymd = (date(1920,1,1) + days(ExtractWord(i))).year_month_day();
		ushort y = ymd.year, m = ymd.month, d = ymd.day;
		return SetEscape(i, false, mysprintf(" \\w%d/%d/%d", y, m, d), 2);
	} case 4: {
		const int min = 511340,		// == PseudoSprite("\\d1400-1-1", 0).ExtractDword(0)
			max = 3652424,			// == PseudoSprite("\\d9999-12-31", 0).ExtractDword(0)
			base = 701265;			// == PseudoSprite("\\d1920-1-1", 0).ExtractDword(0)
		int yearmod = 0;
		int val = ExtractDword(i);
		while (val < min) {
			val += 365*400 + 97;
			yearmod -= 400;
		}
		while (val > max) {
			val -= 365*400 + 97;
			yearmod += 400;
		}
		date::ymd_type ymd = (date(1920,1,1) + days(val-base)).year_month_day();
		uint y = ymd.year+yearmod, m = ymd.month, d = ymd.day;
		return SetEscape(i, false, mysprintf(" \\d%d/%d/%d", y, m, d), 4);
	}}
	return SetDec(i, num);
}

PseudoSprite&PseudoSprite::SetBE(uint i, uint num) {
	assert(num>0 && num<5);
	switch (num) {
	case 2:
		return SetEscape(i, false, mysprintf(" \\wx%x", ExtractWord(i)), 2);
	case 3:
		return SetEscape(i, false, mysprintf(" \\b*x%x", ExtractExtended(i)), ExtendedLen(i));
	case 4:
		return SetEscape(i, false, mysprintf(" \\dx%x", ExtractDword(i)), 4);
	}
	return SetHex(i, num);
}

PseudoSprite&PseudoSprite::SetDec(uint i, uint num) {
	assert(num>0 && num<5);
	switch (num) {
	case 1:
		return SetEscape(i, false, mysprintf(" \\b%d", ExtractByte(i)), 1);
	case 2:
		return SetEscape(i, false, mysprintf(" \\w%d", ExtractWord(i)), 2);
	case 3:
		return SetEscape(i, false, mysprintf(" \\b*%d", ExtractExtended(i)), ExtendedLen(i));
	case 4:
		return SetEscape(i, false, mysprintf(" \\d%d", ExtractDword(i)), 4);
	}
	return SetHex(i, num);
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
		while(num--)
			beauty[i++]=QESC;
		return *this;
	}
	return SetHex(i,num);
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
	if(GetState(LINEBREAKS))beauty[i]|=NOBREAK;
	return*this;
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
	SetHex(off);		// Remove any escape the beautifier might have generated from the old value.
	return*this;
}

PseudoSprite&PseudoSprite::SetWordAt(uint off, uint word){
	VERIFY(off+1<packed.length(),off);
	assert(word<0x10000);
	packed[off]=(uchar)word;
	packed[off+1]=(uchar)(word>>8);
	SetHex(off,2);		// Remove any escape the beautifier might have generated from the old value.
	return*this;
}

PseudoSprite&PseudoSprite::SetDwordAt(uint off, uint dword){
	VERIFY(off+3<packed.length(),off);
	packed[off]=(uchar)dword;
	packed[off+1]=(uchar)(dword>>8);
	packed[off+2]=(uchar)(dword>>16);
	packed[off+3]=(uchar)(dword>>24);
	SetHex(off,4);		// Remove any escape the beautifier might have generated from the old value.
	return*this;
}


PseudoSprite&PseudoSprite::Append(uchar byte){
	context.resize(Length()+1);
	context[Length()]=context[Length()-1];
	context[Length()-1].clear();
	packed.append(1,byte);
	return*this;
}

PseudoSprite&PseudoSprite::ColumnAfter(uint i){
	context[i]+='\t';
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

uint PseudoSprite::LinkSafeExtractByte(uint offs)const{
	VERIFY(IsValid(),IsValid());
	if(Length()<=offs)
		throw offs|1<<24;
	return(uchar)packed[offs];
}

uint PseudoSprite::ExtractByte(uint offs)const{
	CheckLinkage(offs,1);
	return LinkSafeExtractByte(offs);
}

uint PseudoSprite::ExtractWord(uint offs)const{
	try{
		CheckLinkage(offs,2);
		return LinkSafeExtractByte(offs)|LinkSafeExtractByte(offs+1)<<8;
	}catch(unsigned int){
		throw offs|2<<24;
	}
}

uint PseudoSprite::ExtractExtended(uint offs)const{
	uint val=LinkSafeExtractByte(offs);
	if(val!=0xFF){
		CheckLinkage(offs,1);
		return val;
	}
	if(linkage[offs]!=0 && linkage[offs]!=1) {
		CheckLinkage(offs,3);
		ignorelinkage = true;
	}
	val = ExtractWord(offs+1);
	ignorelinkage = false;
	return val;
}

uint PseudoSprite::ExtractDword(uint offs)const{
	try{
		CheckLinkage(offs,4);
		ignorelinkage=true;
		uint val = ExtractWord(offs)|ExtractWord(offs+2)<<16;
		ignorelinkage=false;
		return val;
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
		string::size_type offs=context[i].find_first_of('\n');
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

// Modified from a tokenizer in the C++ Programming HOW-TO by Al Dev (Alavoor Vasudevan)
// http://oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html#ss7.3
// "Copyright policy is GNU/GPL as per LDP (Linux Documentation project)."
// http://oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-22.html
vector<string> Tokenize(const string& str, char delimiter) {
	vector<string> tokens;
    // Skip delimiters at beginning.
	string::size_type lastPos = str.find_first_not_of(delimiter);
    // Find first "non-delimiter".
	string::size_type pos     = str.find_first_of(delimiter, lastPos);

    while (string::npos != pos || string::npos != lastPos) {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiter, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiter, lastPos);
    }
	return tokens;
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

	ostringstream outbuf;	// buffer output for potential tab expansion

//This section contains a rewrite of lines 402-438 or thereabouts of info.cc
//from grfcodec v0.9.7: http://www.ttdpatch.net/grfcodec
//grfcodec is Copyright 2000-2005 Josef Drexler
	ignorelinkage = true;
	for(uint i=0;i<Length();i++) {
		// Most output strings contain a leading space, but this should be omitted if CONVERTONLY.
		// ... unless there was no space between the preceding byte and this one.
		const int skipspace = (GetState(CONVERTONLY)&&i&&context[i-1]!="")?1:0;
		if(DoQuote(i)){
			if (!instr){
				outbuf<< (skipspace ? "\"" : " \"");
				count+=3-skipspace;// count close-quote here.
				instr=true;
			}
			if(NFOversion>6){
				if((beauty[i]&~NOBREAK)==QESC){
					if((*this)[i]=='\r'){
						outbuf<<"\\n";
						count=+2;
					}else{
						outbuf<<mysprintf("\\%2x",(*this)[i]);
						count+=3;
					}
				}else if((beauty[i]&~NOBREAK)==QEXT){
					outbuf<<ext_print[i];
					count+=(uint)ext_print[i].size();
				}else if((*this)[i]=='"'){
					outbuf<<"\\\"";
					count+=2;
				}else if((*this)[i]=='\\'){
					outbuf<<"\\\\";
					count+=2;
				}else{
					outbuf<<(char)(*this)[i];
					count++;
				}
			}else{
				outbuf<<(char)(*this)[i];
				count++;
			}
			noendl=false;
		} else {
			if(instr){
				outbuf<<'"';
				instr=false;
			}
			string str;
			if(NFOversion>6 && (beauty[i]&~NOBREAK)==NQEXT)
				str = ext_print[i].c_str()+skipspace;
			else
				str = mysprintf(skipspace ? "%2x" : " %2x",(*this)[i]);
			outbuf<<str;
			count+=(uint)str.size();
			noendl=false;
		}

		//Context control (comments, beautifier controlled newlines, &c.)
		if(IsEot(i)&&instr){
			outbuf<<'"';
			instr=false;
		}
		if(context[i]==" " && ((instr && i+1<Length() && (IsText(i)||(IsUTF8(i)&&GetState(QUOTEUTF8)))) || (beauty[i]&~NOBREAK)==NQEXT)) {
			context[i]="";
		} else if(context[i]!="") {
			if(instr){
				outbuf<<'"';
				instr=false;
			}
			for(const char*ch=context[i].c_str();*ch;ch++){
				count++;
				if(*ch=='\n'){
					count=0;
					noendl=true;
				}
				outbuf<<*ch;
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
				outbuf<<'"';
				instr=false;
			}
			outbuf<<(noendl?"":"\n")<<string(count=GetState(LEADINGSPACE,2),' ');
		}
	}
	ignorelinkage=false;
	if (instr)outbuf<<'"';

	// Collected all output; perform tab expansion
	string buffer = outbuf.str();
	if(buffer.find('\t')!=NPOS){
		// Split into columns
		vector<vector<string> > sections;
		foreach(const string &line, (Tokenize(buffer, '\n')))
			sections.push_back(Tokenize(line, '\t'));

		// Count the columns
		uint columns = (uint)max_element(sections.begin(),sections.end(), boost::lambda::bind(&vector<string>::size,boost::lambda::_1) < boost::lambda::bind(&vector<string>::size,boost::lambda::_2))->size();

		// For each column,
		for(uint i=0;i<columns;i++){
			// determine how wide it must be,
			string::size_type padWidth = 0;
			foreach(const vector<string> &section, sections)
				if(section.size()>i+1) padWidth = max(padWidth, section[i].length()+1);
			// and make it that wide.
			foreach(vector<string> &section, sections)
				if(section.size()>i+1) section[i] += string(padWidth - section[i].length(), ' ');
		}

		// Tabs are expanded, write each line
		foreach(const vector<string>&line, sections)
			for_each(line.begin(),line.end(),out<<boost::lambda::_1)('\n');

	}else out<<buffer;

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
	int type = beauty[i]&~NOBREAK;
	if (NFOversion>6)
		return type==TEXT || type==ENDQUOTE || type==QESC || type==QEXT;
	else
		return type==TEXT || type==ENDQUOTE;
}
bool PseudoSprite::IsUTF8(uint i)const{
	return (beauty[i]&~NOBREAK)==UTF8;
}
bool PseudoSprite::IsEot(uint i)const{
	return (beauty[i]&~NOBREAK)==ENDQUOTE;
}
bool PseudoSprite::IsLinePermitted(uint i)const{
	return!(beauty[i]&NOBREAK);
}

bool PseudoSprite::UseOrig()const{
	return useorig||!GetState(BEAUTIFY);
}

uint PseudoSprite::ReadValue(istream& in, width w) {
	if (in.peek() == 'x') {		// Read any hex value
		uint ret;
		in.ignore()>>setbase(16)>>ret>>setbase(10);
		return ret;
	}
	if (in.peek() == '(') {		// Read any RPN value
		if(RPNOFF){
			IssueMessage(0, INVALID_EXTENSION);
			return 0;
		}
		int val, err = 0, s = in.tellg();
		val = DoCalc(in.ignore(),err);
		if (err>0)
			return 0;

		// Replace the original RPN with value
		int e = in.tellg(),
			p = orig.find(((istringstream&)in).str().substr(s, e - s));
		orig.erase(p, e - s);
		ostringstream Val;
		Val << val;
		orig.insert(p, Val.str());
		return val;
	}

	// Read any other value
	string str;
	// can't use operator>> -- that will consume comments in cases like \w12000//comment
	eat_white(in); // skip whitespace at front
	while(in && !is_comment(in) && !isspace(in.peek()) && in.peek() != EOF)
		str += (char)in.get();

	char c1, c2;
	int y, m, d, count = sscanf(str.c_str(), "%d%c%d%c%d", &y, &c1, &m, &c2, &d);

	if (count==1) {
		// Got a decimal number
		if (w==_B_ && y>1920) y-=1920;	// special case for byte-sized years
		return y;
	}

	// May have a date. Check, fiddle, and invoke date_time.
	if (count == 5 && c1 == c2 && (c1 == '-' || c1 == '/')) {
		int extra = 0;

		if (w == _W_) {
			// word date
			if (d==0 || (d>31 && d<100) || d>1919) swap(y, d);	// Try DMY instead
			if (y==0) y = 2000;
			else if (y>31 && y<100) y+=1900;
		} else if (w == _D_) {
			// dword date
			extra = 701265;
			if (d >= 32) swap(y, d); // Try DMY instead
			// Boost doesn't support years out of the range 1400..9999
			while (y>9999) {
				y -= 400;
				extra += 365*400 + 97; // 97 leap years every 400 years.
			}
			while (y<1400) {
				y += 400;
				extra -= 365*400 + 97;
			}
		} else goto fail;		// I can't read a date of that width.

		try {
			return (date((ushort)y, (ushort)m, (ushort)d) - date(1920, 1, 1)).days() + extra;
		} catch (std::out_of_range) {
			// Fall through to fail
		}
	}

fail:	// Nothing worked
	in.clear(ios::badbit);
	return (uint)-1;
}
