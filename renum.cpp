/*
 * renum.cpp 
 * Renumbers and lints an NFO file specified on the commandline or piped
 * in via standard input.
 *
 * Return values:
 * See README[.txt]
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

#include<iostream>
#include<string>
#include<sstream>
#include<iomanip>
#include<fstream>
#include<cerrno>
#include<cassert>

using namespace std;

#include"renum.h"
#include"globals.h"
#include"inlines.h"
#include"sanity.h"
#include"command.h"
#include"messages.h"
#include"inject.h"
#include"pseudo.h"
#ifdef _WIN32
#   include "win32.h"
#endif

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

void SetCode(int x){_retval=max(_retval,x);}

void doexit(){exit(_retval);}

void ShowHelp(){
	cout<<"Usage: renum [options] [file1 [file2 [file3 [...]]]]\n"
		"   Any number of files may be specified.\n"
		"   If no files or options are specified, NFORenum will read a file from\n"
		"   standard input and write it to standard output.\n"
		"Options:\n"
		"   --comments=<type> -c <type>\n"
		"       <type> is one character, either /, ;, or #, and specifies the comment\n"
		"       style that NFORenum will use. This will not change the header, because\n"
		"       grfcodec requires that the header be commented in C++-style.\n"
		"   --data[=<dir>] -D[<dir>]\n"
		"       If <dir> is specified, look for the .renum directory in <dir>, and\n"
		"       create it if not found. Regardless of whether or not <dir> is\n"
		"       specified, eliminate the 5-second wait used to ensure that those not\n"
		"       running NFORenum from a command line can see the directory-created\n"
		"       message.\n"
		"       Default: Look for  the .renum directory in the current directory, and\n"
		"       then in the environment variable HOME, if defined. If not found attempt\n"
		"       to create in HOME, then in .\n"
		"   --force -f\n"
		"       Forces processing of files that do not look like NFO files.\n"
		"       The default is to treat such files as if they specified a too-high info\n"
		"       version.\n"
		"   --help -?\n"
		"       Display this message.\n"
		"   --lock\n"
		"       Locks the current comment command state. Commands will continue to be\n"
		"       parsed as normal (so NOPRESERVE will be honored, @@DIFF will be\n"
		"       removed, &c.) but their changes (such as turning on the diff-assister)\n"
		"       will not be honored.\n"
		"   --no-replace --keep-old -k\n"
		"       Do not replace the old NFO file; write new file to file.new.nfo.\n"
		"       Default: Use file[.nfo].new as temporary, rename it to file[.nfo]\n"
		"       when done.\n"
		"   --auto-correct -a\n"
		"       Perform some rudimentary automatic correction of incorrect pseudo\n"
		"       sprites. This may be specified twice to enable the corrections that\n"
		"       are more likely to be incorrect.\n"
		"       See the README for detailed information on the auto-correcter.\n"
		"\n"
		"The following options cause NFORenum to behave as if all files started with\n"
		"the associated command. The readme has full details on the comment commands.\n"
		"Options associated with comment commands that require additional information\n"
		"take that information as an argument. With the exception of -L/--let, the\n"
		"options to the command line versions are case insensitive.\n"
		"\"ON\" and \"OFF\" may be specified with \"+\" and \"-\", respectively.\n"
		"   --beautify -b               @@BEAUTIFY\n"
		"   --diff -d                   @@DIFF\n"
		"   --let -L                    @@LET\n"
		"   --lint -l                   @@LINT\n"
		"   --preserve-messages -p      @@PRESERVEMESSAGES\n"
		"   --real-sprites -r           @@REALSPRITES\n"
		"   --use-old-nums -o           @@USEOLDSPRITENUMS\n"
		"   --warning-disable -w        @@WARNING DISABLE\n"
		"   --warning-enable -W         @@WARNING ENABLE\n"
		"       -w and -W (and their long counterparts) also accept a comma-separated\n"
		"       list of messages, all of which will be ENABLEd or DISABLEd.\n\n"
		"NFORenum is Copyright 2004-2006 by Dale McCoy (dalestan@gmail.com)\n"
		"You may copy and redistribute it under the terms of the GNU General Public\n"
		"License, as stated in the file 'COPYING'.\n\n"
		"If this message scrolls by too quickly, you may want to try\n"
		"   renum -? | more\n"
		;
}

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
		{"help",no_argument,NULL,'?'},
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
	pOut=argc==1?&cerr:&cout;
	IssueMessage(0,STARTUP);
	ifstream fin;
	ofstream fout;
	while(argc>1){
		if(opt!=EOF)opt=getopt_long(argc,argv,"D::k?c:fa" "dL:l:pw:W:r:b:e:",optlist,&longind);
		switch(opt){
		case 0:continue;
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
		case'?':
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
		default:CLCommand(opt);continue;
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
		IssueMessage(0,PROCESSING_FILE,basename.c_str());
		result=process_file(fin);
		fin.close();
		fout.close();
		if(result)unlink(outfilename.c_str());
		else if(replace){
			if(rename(infilename.c_str(),bakfilename.c_str())&&errno==EEXIST){
				if(unlink(infilename.c_str())){
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
		IssueMessage(0,PROCESSING_COMPLETE);
	}
	pNfo=&cout;
	IssueMessage(0,PROCESSING);
	process_file(cin);
	IssueMessage(0,PROCESSING_COMPLETE);
	return _retval;
}

#define flush_buffer()\
	if(true){\
		if(buffer!=""){\
			output_buffer(buffer,isPatch,spriteno);\
			buffer="";\
		}\
		spriteno=temp;\
	}else\
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
		}else{
			IssueMessage(0,UNKNOWN_VERSION,1);
			IssueMessage(0,PARSING_FILE);
		}
		getline(in,sprite);//first sprite
	}


	int temp=-1,size,spriteno=-1;
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
		if(spritestream.peek()==EOF)buffer+='\n';
		else if(COMMENT.find((char)spritestream.peek())!=NPOS){//comment
			if(is_command(sprite))
				flush_buffer();
			if(parse_comment(sprite))
				buffer+=sprite+'\n';
		}else{//sprite
			if(!eat_white(spritestream>>temp)){
				spritestream.clear();
				temp=-1;
			}
			if(spritestream.peek()=='*'){
				if(spritestream.ignore().peek()=='*'){
					SetVersion(6);
					getline(eat_white(spritestream.ignore()),datapart);
					flush_buffer();
					if(isPatch)check_sprite(INCLUDE);
					else IssueMessage(ERROR,UNEXPECTED,BIN_INCLUDE);
					_spritenum++;
					(*pNfo)<<setw(5)<<spritenum()<<" **\t "<<datapart<<'\n';
				}else{
					eat_white(spritestream>>size);
					flush_buffer();
					if(_spritenum==(uint)-1){
						isPatch=true;
						if(size!=4)
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
				if(!spritestream||firstnotpseudo==NPOS||(datapart[firstnotpseudo]=='"'?(SetVersion(5),true):false)||
					(datapart[firstnotpseudo]=='\\'?TrySetVersion(7):false)||COMMENT.find_first_of(datapart[firstnotpseudo])!=NPOS){
					if(PseudoSprite::MayBeSprite(buffer)){
						buffer+=sprite+'\n';
					}else{
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
						IssueMessage(0,PARTIAL_PARSE_FAILURE,--_spritenum);
					}
				}
			}
		}
		if(peek(in)==EOF){
			flush_buffer();
			(*real_out)<<NFO_HEADER(NFOversion);
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
	int var_list[9]={0,0,1,1,1,0,0},&xpos=var_list[0],&ypos=var_list[1],&comp=var_list[2],&ysize=var_list[3],&xsize=var_list[4],
		&xrel=var_list[5],&yrel=var_list[6];
	const char*const format_list[7]={
		"%d%s",
		"%d %d%s",NULL,
		"%d %d %2x %d%s",
		"%d %d %2x %d %d%s",
		"%d %d %2x %d %d %d%s",
		"%d %d %2x %d %d %d %d%s"};
	const char*const var_names[7]={"xpos","ypos","comp","ysize","xsize","xrel","yrel"};
	string meta=data.substr(loc+5);
	size_t offs=NPOS;
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
		// **FIXME** 64-bit safety (?) (next 2 lines)
		//store pointer to remainder of metadata immediately after calculated datum, and ...
		(const char*&)(var_list[var+1])=meta.c_str()+offs;
		//... pass it as one or two 32-bit integers, to be read with va_arg(ap,char*).
		meta=mysprintf(format_list[var],xpos,ypos,comp,ysize,xsize,xrel,yrel,var_list[7],var_list[8]);
	}
	if(state)data=data.substr(0,loc+5)+meta;
	if(xpos<0)IssueMessage(ERROR,REAL_VAL_TOO_SMALL,"xpos",0);
	if(ypos<0)IssueMessage(ERROR,REAL_VAL_TOO_SMALL,"ypos",0);
	if(!(comp&1)||(comp&0x0B)!=comp)IssueMessage(comp==0xFF?ERROR:WARNING1,REAL_BAD_COMP,comp);
	if(xsize<1)IssueMessage(ERROR,REAL_VAL_TOO_SMALL,"xsize",1);
	else if(xsize>0xFFFF)IssueMessage(ERROR,REAL_VAL_TOO_LARGE,"xsize",0xFFFF);
	if(ysize<1)IssueMessage(ERROR,REAL_VAL_TOO_SMALL,"ysize",1);
	else if(ysize>0xFF)IssueMessage(ERROR,REAL_VAL_TOO_LARGE,"ysize",0xFF);
	if(xsize*ysize>0xFFFF)IssueMessage(ERROR,REAL_SPRITE_TOO_LARGE);
	if(xrel<-32768)IssueMessage(ERROR,REAL_VAL_TOO_SMALL,xrel,-32768);
	else if(xrel>32767)IssueMessage(ERROR,REAL_VAL_TOO_LARGE,xrel,32767);
	if(yrel<-32768)IssueMessage(ERROR,REAL_VAL_TOO_SMALL,yrel,-32768);
	else if(yrel>32767)IssueMessage(ERROR,REAL_VAL_TOO_LARGE,yrel,32767);
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
