/*
 * renum.cpp
 * Renumbers and lints an NFO file specified on the commandline or piped
 * in via standard input.
 *
 * Return values:
 * See README[.txt]
 *
 * Copyright 2004-2007,2009 by Dale McCoy.
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

#include<iostream>
#include<string>
#include<sstream>
#include<iomanip>
#include<fstream>
#include<cerrno>
#include<cassert>
#include<cstdlib>
#include<getopt.h>

#ifdef MINGW
	#include <io.h>
	#define mkdir(a,b) mkdir(a)
	#define isatty _isatty
#elif defined(_MSC_VER)
	#include <io.h>
	#include <direct.h>
	#define mkdir(a,b) mkdir(a)
	#define F_OK 0
	#define isatty _isatty
#else
	#include <unistd.h>
#endif//_MSC_VER

using namespace std;

#include"getopt.h"
#include"globals.h"
#include"inlines.h"
#include"sanity.h"
#include"command.h"
#include"messages.h"
#include"inject.h"
#include"pseudo.h"
#include"help.h"
#ifdef _WIN32
#   include "win32.h"
#endif

#include "mapescapes.h"

nfe_map nfo_escapes;

#ifndef _MSC_VER
//Cygwin's GCC #defines __cdecl, but other GCCs do not
//#undef it to prevent GCC from warning on the #define,
//then #define it to prevent errors on main's definition.
#undef __cdecl
#define __cdecl
#endif

int process_file(istream&);
void output_buffer(const string&,bool,int);
bool verify_real(string&);

static int _retval=EOK;
static int _force=0;
bool _interactive;

void SetCode(int x){_retval=max(_retval,x);}
void doexit(){exit(_retval);}

//The VS project file specifies __stdcall as the default convention,
//so main must be explicitly __cdecl'd
int __cdecl main(const int argc,char**argv){
	string infilename,outfilename,bakfilename,basename;
	int result,longind,opt=0,replace=1;
	static const option optlist[]={
		{"data",optional_argument,NULL,'D'},
		{"comments",required_argument,NULL,'c'},
		{"force",no_argument,&_force,1},
		{"lock",no_argument,NULL,256},
		{"no-replace",no_argument,&replace,0},
		{"keep-old",no_argument,&replace,0},
		{"help",no_argument,NULL,'h'},
		{"silent",no_argument,NULL,'s'},
		{"auto-correct",no_argument,NULL,'a'},
		{"beautify",required_argument,NULL,'b'},
		{"diff",no_argument,NULL,'d'},
		{"extentions",required_argument,NULL,'e'},
		{"let",required_argument,NULL,'L'},
		{"lint",optional_argument,NULL,'l'},
		{"preserve-messages",no_argument,NULL,'P'},
		{"real-sprites",required_argument,NULL,'r'},
		{"use-old-nums",required_argument,NULL,'o'},
		{"warning-disable",required_argument,NULL,'w'},
		{"warning-enable",required_argument,NULL,'W'},
		{NULL,0,0,0}
	};
	_interactive = (isatty(fileno(stdout)) != 0);
	bool seen_startup_message = false;
	pOut=argc==1?&cerr:&cout;
	ifstream fin;
	ofstream fout;
	while(argc>1){
		if(opt!=EOF)opt=getopt_long(argc,argv,"D::kvshc:fa" "dL:l:pw:W:r:b:e:",optlist,&longind);
		switch(opt){
		case 0:continue;
		case 's':
			_interactive = false;
			continue;
		case 'v':
			IssueMessage(0,STARTUP);
			return 0;
		case'D':
			if(optarg)datadir=optarg;
			dosleep=false;
			continue;
		case'k':replace=0;continue;
		case'c':
			switch(*optarg){
			case'#':COMMENT_PREFIX="#";break;
			case';':COMMENT_PREFIX=";";break;
			case'/':COMMENT_PREFIX="//";
			}
			continue;
		case'h':
			IssueMessage(0,STARTUP);
			ShowHelp();
			return 0;
		case'f':_force=1;continue;
		case'a':
			_autocorrect++;
			if(!GetState(BEAUTIFY)){
				optarg=(char*)"convertonly+";
				CLCommand('b');
			}
			continue;
		case EOF:
			if(optind==argc)doexit();
			basename=argv[optind++];break;
		case '?':
			ShowHelp();
			exit(1);
		default:
			if(!CLCommand(opt))
				IssueMessage(0,BAD_CL_ARG,opt,optarg);
			continue;
		}

		if (!seen_startup_message && _interactive) {
			seen_startup_message = true;
			IssueMessage(0,STARTUP);
		}
		pNfo=&fout;
		bakfilename=basename+bak_ext;
		fin.open((infilename=basename).c_str());
		if(!fin.is_open()){
			fin.open((infilename=basename+nfo_ext).c_str());
			bakfilename=basename+nfo_ext+bak_ext;
		}
		if(fin.is_open()){
			fout.open((outfilename=(replace?basename+new_ext:basename+new_ext+nfo_ext)).c_str());
		}else{
			bakfilename=dirname+(basename+bak_ext);
			fin.open((infilename=dirname+basename).c_str());
			if(!fin.is_open()){
				bakfilename=dirname+(basename+nfo_ext+bak_ext);
				fin.open((infilename=dirname+basename+nfo_ext).c_str());
			}
			if(fin.is_open()){
				fout.open((outfilename=dirname+(replace?basename+new_ext:basename+new_ext+nfo_ext)).c_str());
			}else{
				IssueMessage(0,NO_INPUT_FILE,basename.c_str());
				SetCode(EFILE);
				continue;
			}
		}
		if(!fout.is_open()){
			IssueMessage(0,NO_OUTPUT_FILE,outfilename.c_str(),basename.c_str());
			SetCode(EFILE);
			fin.close();
			fout.clear();
			continue;
		}
		fin.clear();
		reset_sanity();
		reset_commands();
		_grfver=0;
		if (_interactive) IssueMessage(0,PROCESSING_FILE,basename.c_str());
		result=process_file(fin);
		fin.close();
		fout.close();
		if(result){
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
// unlink is deprecated in MSVS 8.0+
			_unlink(outfilename.c_str());
#else
			unlink(outfilename.c_str());
#endif
		}else if(replace){
			if(rename(infilename.c_str(),bakfilename.c_str())&&errno==EEXIST){
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
// unlink is deprecated in MSVS 8.0+
				if(_unlink(infilename.c_str()))
#else
				if(unlink(infilename.c_str()))
#endif
				{
					IssueMessage(0,DELETE_FAILED,infilename.c_str(),errno);
					perror(NULL);
					SetCode(EFILE);
				}
			}
			if(rename(outfilename.c_str(),infilename.c_str())){
				IssueMessage(0,REPLACE_FAILED,infilename.c_str(),outfilename.c_str(),errno);
				perror(NULL);
				SetCode(EFILE);
			}
		}
		if (_interactive) IssueMessage(0,PROCESSING_COMPLETE);
	}
	pNfo=&cout;
	IssueMessage(0,PROCESSING);
	process_file(cin);
	IssueMessage(0,PROCESSING_COMPLETE);
	return _retval;
}

#define flush_buffer()\
	if(true) {\
		if(buffer!="") {\
			output_buffer(buffer,isPatch,oldspritenum);\
			buffer="";\
		}\
		oldspritenum=temp;\
	} else\
		(void(0))

#define SetVersion(x)\
	(NFOversion=max((x),NFOversion))

static bool hasHeader;
int NFOversion;

bool TrySetVersion(int x){
	if(hasHeader&&NFOversion>=x)return true;
	if(hasHeader&&NFOversion<=6&&x>=7)return false;
	SetVersion(x);
	return true;
}

string smash(const string&,int&);

int process_file(istream&in){
	NFOversion=4;
	string sprite,datapart,buffer;
	inject_into(in);
	nfo_escapes.clear();
	vector<string> extra_lines;

	if(string(" \t\n*0123456789/;#*").find((char)in.peek())==NPOS){
		IssueMessage(0,APPARENTLY_NOT_NFO);
		if(!_force){
			IssueMessage(0,SKIPPING_FILE);
			SetCode(EPARSE);
			return-1;
		}
	}

//This section is a rewrite of lines 35-50 or thereabouts of info.cc
//from grfcodec: http://www.ttdpatch.net/grfcodec
//grfcodec is Copyright 2000-2003 Josef Drexler
	getline(in,sprite);//Info header or first sprite
	hasHeader=false;
	if (sprite.substr(0,3)=="// "){
		hasHeader=true;
		getline(in,sprite);//Info version (first sprite for ancient NFO versions)
		if(sscanf(sprite.c_str(),"// (Info version %d)",&NFOversion)){
			if(NFOversion<4||NFOversion>7){
				IssueMessage(0,UNKNOWN_VERSION,NFOversion);
				if(NFOversion>7){
					IssueMessage(0,SKIPPING_FILE);
					SetCode(EPARSE);
					return-1;
				}
				IssueMessage(0,PARSING_FILE);
			}
			if(NFOversion>2)
				getline(in,sprite);//Format line
			if (NFOversion>6) {
				while (strncmp(sprite.c_str(), "// Format: ", 11)) {
					if (!strncmp(sprite.c_str(), "// Escapes: ", 12)) {	// Actually, that was an escapes line. Read it.
						istringstream esc(sprite);
						string str;
						int byte = 0;
						esc.ignore(12);
						while (esc>>str)
							if (str == "=") byte--;
							else if (str[0] == '@')
								byte = strtol(str.c_str()+1,NULL,16);
							else nfo_escapes.insert(nfe_pair(str, byte++));
					} else if (strncmp(sprite.c_str(), "// ",3)) { // EEEP! No "Format:" line in this file!
						IssueMessage(0, APPARENTLY_NOT_NFO);
						SetCode(EPARSE);
						return -1;
					} else extra_lines.push_back(sprite); // store unknown lines

					getline(in,sprite);	// Try again to skip "Format: " line
				}
				// Now remove all defaults. This serves two purposes:
				// 1) Prevent incorrectly specified defaults from causing problems later.
				// 2) Allow the beautifier to select custom escapes over built-ins.
				foreach(const esc& e, escapes)
					nfo_escapes.left.erase(e.str+1);
			}
		}else{
			IssueMessage(0,UNKNOWN_VERSION,1);
			IssueMessage(0,PARSING_FILE);
		}
		getline(in,sprite);//first sprite
	}


	int temp=-1,size,oldspritenum=-1;
	_spritenum=(unsigned)-1;
	string::size_type firstnotpseudo;
	bool isPatch=false;
	ostringstream outbuffer;
	ostream*real_out=pNfo;
	pNfo=&outbuffer;
	SetVersion(4);
	AutoConsoleMessages();
	while(true){
		//IssueMessage(0,SPRITE,spritenum+1,sprite.c_str());
		istringstream spritestream(sprite);
		eat_white(spritestream);
		if(spritestream.peek()==EOF) {
            buffer+='\n';
		} else if(is_comment(spritestream)){
			if(is_command(sprite)) {
				flush_buffer();
            }
			if(parse_comment(sprite)) {
				buffer+=sprite+'\n';
            }
		} else {//sprite
			if(!eat_white(spritestream>>temp)){
				spritestream.clear();
				temp=-1;
			}
			if(spritestream.peek()=='*'){
				if(spritestream.ignore().peek()=='*'){
					SetVersion(6);
					getline(eat_white(spritestream.ignore()),datapart);
					flush_buffer();
					if(isPatch) {
						check_sprite(INCLUDE);
					} else {
						IssueMessage(ERROR,UNEXPECTED,BIN_INCLUDE);
					}
					_spritenum++;
					(*pNfo)<<setw(5)<<spritenum()<<" **\t "<<datapart<<'\n';
				}else{
					(spritestream>>size).clear();
					eat_white(spritestream);
					flush_buffer();
					if(_spritenum==(uint)-1){
						isPatch=true;
						if(size!=4 && size!=0)
							outbuffer<<COMMENT_PREFIX<<sprite<<endl;
						_spritenum=0;
					}else{
						getline(spritestream,buffer);
						buffer+='\n';
					}
				}
			}else{
				getline(spritestream,datapart);
				firstnotpseudo=datapart.find_first_not_of(VALID_PSEUDO);
				if(!spritestream || firstnotpseudo==NPOS
							|| datapart[firstnotpseudo]=='"'
							|| (datapart[firstnotpseudo]=='\\'?TrySetVersion(7):false)
							|| is_comment(datapart,firstnotpseudo)) {
					if(PseudoSprite::MayBeSprite(buffer)) {
						buffer+=sprite+'\n';
					} else {
						IssueMessage(0,NOT_IN_SPRITE,_spritenum+1);
						buffer+="//"+sprite+'\n';
					}
				}else{
					_spritenum++;
					if(verify_real(datapart)){
						_spritenum--;
						flush_buffer();
						_spritenum++;
						if(isPatch)check_sprite(REAL);
						(*pNfo)<<setw(5)<<spritenum()<<' '<<datapart<<endl;
					}else{
						buffer+="//"+sprite+'\n';
						SetCode(EPARSE);
						IssueMessage(0,PARTIAL_PARSE_FAILURE,_spritenum--);
					}
				}
			}
		}
		if(peek(in)==EOF){
			flush_buffer();
			(*real_out)<<NFO_HEADER(NFOversion);
			if (NFOversion > 6) {
					// (re)insert default escapes
					foreach(const esc& e, escapes)
						nfo_escapes.insert(nfe_pair(e.str+1, e.byte));
					(*real_out)<<"// Escapes:";
					int oldbyte = -1;

					for (int act = 0; act < 255; act++) {
						foreach (const nfe_rpair& p, nfo_escapes.right) {
							if (p.second[0] != act) continue;

							if (p.first == oldbyte) {
								(*real_out)<<" =";
								--oldbyte;
							} else if (p.first < oldbyte) {
								(*real_out)<<"\n// Escapes:";
								oldbyte = -1;
							}
							while (++oldbyte != p.first)
								(*real_out)<<" "<<nfo_escapes.right.begin()->second;
							(*real_out)<<" "<<p.second;
						}
					}
					(*real_out)<<"\n";
				for (uint i=0; i<extra_lines.size(); i++)
					(*real_out)<<extra_lines[i]<<"\n";
			}
			(*real_out)<<NFO_FORMAT;
			if(isPatch)(*real_out)<<"    0 * 4\t "<<mysprintf("%8x\n",GetState(DIFF)?0:_spritenum);
			(*real_out)<<outbuffer.str();
			pNfo=real_out;
			final_sanity();
			return 0;
		}
		inj_getline(in,sprite);
	}
}

bool verify_real(string&data){
	string::size_type loc=NPOS;
	string udata=UCase(data);
	while(true){
		loc=udata.find(".PCX",loc+1);
		if(loc==NPOS){
			IssueMessage(0,REAL_NO_FILENAME);
			return false;
		}
		if(isspace(data[loc+4]))break;
	}
	string name=data.substr(0,loc+4);
	int var_list[7]={0,0,1,1,1,0,0},
		&xpos=var_list[0],
		&ypos=var_list[1],
		&comp=var_list[2],
		&ysize=var_list[3],
		&xsize=var_list[4],
		&xrel=var_list[5],
		&yrel=var_list[6];
	const char*const var_names[7]={"xpos","ypos","comp","ysize","xsize","xrel","yrel"};
	string meta=data.substr(loc+5);
	string::size_type offs=NPOS;
	uchar state=0; // bitmask of calculated variables
	int var;
	while((var=sscanf(meta.c_str(),"%d %d %2x %d %d %d %d",&xpos,&ypos,&comp,&ysize,&xsize,&xrel,&yrel))!=7){
		if(RPNOFF()||
			//Calculation on comp, already tried to calculate this datum, or no more parens->broken sprite
			var==2||state&(1<<var)||(offs=meta.find('('))==NPOS||
			//If OK so far, calculate and store datum. If calc fails, offs == NPOS; sprite is broken
			(var_list[var]=DoCalc(meta,offs),offs==NPOS)){
			if(COMMENTON()||RPNON()||(RPNOFF()&&meta.find('(')==NPOS))IssueMessage(0,REAL_MISSING_DATA,var_names[var]);
			return COMMENTOFF();
		}
		state|=1<<var;//Mark calculation
		const char* rest(meta.c_str()+offs);
		switch(var) {
		case 0: meta=mysprintf("%d%t",xpos,rest); break;
		case 1: meta=mysprintf("%d %d%t",xpos,ypos,rest); break;
		case 3: meta=mysprintf("%d %d %2x %d%t",xpos,ypos,comp,ysize,rest); break;
		case 4: meta=mysprintf("%d %d %2x %d %d%t",xpos,ypos,comp,ysize,xsize,rest); break;
		case 5: meta=mysprintf("%d %d %2x %d %d %d%t",xpos,ypos,comp,ysize,xsize,xrel,rest); break;
		default: meta=mysprintf("%d %d %2x %d %d %d %d%t",xpos,ypos,comp,ysize,xsize,xrel,yrel,rest); break;
		}
	}
	if(state)data=data.substr(0,loc+5)+meta;
	if(xpos<0)IssueMessage(ERROR,REAL_VAL_TOO_SMALL,XPOS,0);
	if(ypos<0)IssueMessage(ERROR,REAL_VAL_TOO_SMALL,YPOS,0);
	if(!(comp&1)||(comp&0x4B)!=comp)IssueMessage(comp==0xFF?ERROR:WARNING1,REAL_BAD_COMP,comp);
	if(xsize<1)IssueMessage(ERROR,REAL_VAL_TOO_SMALL,XSIZE,1);
	else if(xsize>0xFFFF)IssueMessage(ERROR,REAL_VAL_TOO_LARGE,XSIZE,0xFFFF);
	if(ysize<1)IssueMessage(ERROR,REAL_VAL_TOO_SMALL,YSIZE,1);
	else if(ysize>0xFF)IssueMessage(ERROR,REAL_VAL_TOO_LARGE,YSIZE,0xFF);
	if(xsize*ysize>0xFFFF)IssueMessage(ERROR,REAL_SPRITE_TOO_LARGE);
	if(xrel<-32768)IssueMessage(ERROR,REAL_VAL_TOO_SMALL,XREL,-32768);
	else if(xrel>32767)IssueMessage(ERROR,REAL_VAL_TOO_LARGE,XREL,32767);
	if(yrel<-32768)IssueMessage(ERROR,REAL_VAL_TOO_SMALL,YREL,-32768);
	else if(yrel>32767)IssueMessage(ERROR,REAL_VAL_TOO_LARGE,YREL,32767);
	return true;
}

void output_buffer(const string&sprite,bool isPatch,int spriteno){
	PseudoSprite data(sprite,spriteno);
	if(data.IsValid()){
		_spritenum++;
		if(isPatch)check_sprite(data);
		else{
			data.SetAllHex();
			data.SetEol(32,1);
			data.SetEol(64,1);
			data.SetEol(96,1);
			data.SetEol(128,1);
			data.SetEol(160,1);
			data.SetEol(192,1);
			data.SetEol(224,1);
		}
	}
	(*pNfo)<<data;
}
