/*
 * data.cpp
 * Data file contents and implementation of helper functions.
 *
 * Copyright 2005-2007 by Dale McCoy.
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

#ifdef _WIN32
#include"win32.h"
#endif

#include<cstdio>
#include<cstdlib>
#include<cassert>
#include<string>
#include<sys/stat.h>
#include<cstdarg>
#include<errno.h>
#ifndef _WIN32
#   include<unistd.h>
#endif

using namespace std;

#include"renum.h"
#include"inlines.h"
#include"globals.h"
#include"data.h"
#include"messages.h"
#include"win32.h"
#include"sanity_defines.h"

//Let's dump the data files into the source code. That sounds like fun, right?


//Format: Bitmask
//Bit(s)			Meaning
//0,1,2				valid width(s) (B,W,D) (If all are clear, this is a bitmask-variable.)
//5	(20h)			write in D
//6	(40h)			read in D
//7 (80h)			read in 7
//                                          80                              88                              90                              98                              A0
static const char _dat79Dv[]="\x20\x03\x22\x00\xC1\x00\xC1\xC5\x80\x80\x00\x84\x00\x00\xC6\x00\xC1\xE1\xE4\x00\x00\xC1\xE6\xE6\xE6\xE6\xE1\x00\x24\x87\x00\x00\xC4\xE4\x24\x00\x84";

static const char _datB[]="\x01\x02\x03\x07\x02\x02\x02\x03\x02\x02\x02";

// Bit 15 set if recolor block, bit 14 set if mixed block, bit 13 set if 80+x valid, bit 
// Remaining bits are number of sprites expected. If 0, sprite count is not checked
// Flags:
// 01 -- recolor
// 02 -- mixed
// 04 -- word
// 08 -- allow 80+x
static const char _dat5[]="\x04\x01"
	"\x13\x30\x70\xF0"			//4
	"\x11\x30"					//5
	"\x12\x4A\x5A"				//6
	"\x31\x4B\x41\x06"			//7..9
"\x85\x11\x00\x01"				//A
	"\x31\x71\x85\x10"			//B..D
"\x82\x11\x00"					//E
	"\x51\x0C\x0F\x08\x08\x37"	//F..13
"\x88\x21\x1D\x88"				//14..15
"\x00";

static const char _datTextIDs[]="\x04\x08\x35\x03\x11\x00\x25\x00\x19\x00\x5D\x00\x11\x00\x6D\x00\x08\x00\x11\x00\x3C\x00\x2A\x00\x08\x00\x19\x00\x39\x00\x80\x00\x00\x00\x08\x01\x6C\x00\x38\x00\x43\x00\x44\x00\x00\x00\x06\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x4D\x00\x00\x00\x00\x00\x68\x01"
//Count of IDs in class:                   -0000-                          -2000-                          -4000-                          -6000-                          -8000-                          -A000-                          -C000-                          -E000-
	"\x03\xC4\xC5\xC9";

//                                     --08--                          --0C--                          --10--
static const char _dat0f8[]="\x02\x00\x30\x30\x00\xFC\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x00\x00";

// Count of CBs, then list of
// 1) feature for callback
// 2) one-byte bitmask of features for callback | 80h
// 3) 7Fh followed by word-sized bitmask
static const char _datcallbacks[]="\x05\x0C\x4D\x01"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x83\x8F\x04\x04\x8F\x83\x07\x9F\x8F\x07\x07\x07\x00\x07\x07"
//v 20             x4              x8              xC
"\x07\x07\x0A\x8F\x04\x09\x09\x09\x0A\x0A\x07\x09\x09\x8F\x07\x09"
"\x09\x8F\x8F\xCF\x8F\x0A\x8F\x0A\x0A\x0B\x0A\x0A\x09\x0A\x80\x80"
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
//v 60             x4              x8              xC
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
//v A0             x4              x8              xC
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
//v E0             x4              x8              xC
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
//v 120            x4              x8              xC
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
"\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
"\x04\x04\x04\x07\x0C\x0B\x0E\x05\x07\x04\x0A\x0A\x0A";

static const char _datlangs[]="\x00\x02"
//		x0				x1				x2				x3				x4				x5				x6				x7				x8				x9				xA				xB				xC				xD				xE				xF
/*0x*/	"American\n"	"English\n"		"German\n"		"French\n"		"Spanish\n"		"Esperanto\n"	"\n"			"Russian\n"		"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"
/*1x*/	"\n"			"\n"			"\n"			"\n"			"\n"			"Czech\n"		"Slovak\n"		"\n"			"Bulgarian\n"	"\n"			"\n"			"Afrikaans\n"	"\n"			"\n"			"Greek\n"		"Dutch\n"
/*2x*/	"\n"			"\n"			"Catalan\n"		"\n"			"Hungarian\n"	"\n"			"\n"			"Italian\n"		"Romanian\n"	"Icelandic\n"	"Latvian\n"		"Lithuanian\n"	"Slovenian\n"	"Danish\n"		"Swedish\n"		"Norwegian\n"
/*3x*/	"Polish\n"		"Galician\n"	"Frisian\n"		"Ukrainian\n"	"Estonian\n"	"Finnish\n"		"Portuguese\n"	"Brazilian Portuguese\nCroatian\nJapanese\n"	"Korean\n"		"\n"			"\n"			"\n"			"Turkish\n"		"\n"
/*4x*/	"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"
/*5x*/	"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"
/*6x*/	"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"
/*7x*/	"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"\n"			"any\n";

static const char _datversions[]="\x00\x04\x09"
"\xD8\x01\x31\x06\x81\x07\x70\x11";

/*****************************************************************************
 *
 * Files that are defined above this line (more accurately, those that appear
 * before the DATA_FILE(feat) line in data.h) have two leading bytes that are
 * checked and then eaten by myfopen. The first byte is a format byte; start
 * it at \x00. Each time the file format gets updated (ie the file reader
 * changes), increment it and reset the version byte to \x00. The second byte
 * is a version byte; increment it every time the file gets updated. 
 *
 *****************************************************************************/

// The above also applies for feat.dat.

static const char _datfeat[]="\x11\x07\x0F"
// 00              04              08              0C
"\xFF\xDF\xDF\xFF\x5F\x8F\xD9\x0F\x01\x0F\x0D\x8F\x01\x00\x50\x09"
"\x00\x00\x00\x00\x00\x00\x00\x01\xFF\x01\x02\x00\xFF\xFF\x00\xFF";
// First line: OR of the appropriate bits from enum ActBit
//    {ACT0=1,ACT1=2,ACT3=4,ACT4=8,EMPTY1=0x10,OVERRIDE3=0x20,GENERIC3=0x40,ACT3_BEFORE_PROP08=0x80};
// Second line: Std action 2 format:
// 0: Vehicle style		1: House style		2: Ind. prod style
// FF: No std 2 for this feature

//third chawmp: 0x:default 1x:quote 2x:decimal 3x:B-E hex (2x and 3x are currently unsupported)
//The upper chawmp, which only applies in FE strings:
//0x:default  Cx:linebreak;long lead
//Full description in act0.cpp.
static const char _dat0[]="\x0C\x05\x0F"
"\x22\xFF\x01\x21\x21\x01\x01\x21\x01\x22\xFF\x22\xFF\x01\x04\xFF\xFF\xFF\x01\x01\x21\x01\x01\x01\x01\x01\x01\x22\x01\x34\x01\x01\x01\x01\x01\x21\x01\x01\x21\x01\x32\x32\x04\x00"
"\x22\xFF\x01\x21\x21\xFF\x01\x21\x21\x01\x04\xFF\xFF\xFF\x01\x21\x01\x01\x01\x21\x21\x21\x34\x01\x01\x01\x01\x21\x01\x32\x32\x04\x00"
//                                 08                              10                              18                              20                              28
"\x22\xFF\x01\x21\x21\xFF\x01\x21\x01\x01\x01\x21\x01\x22\xFF\x01\x01\x34\x01\x01\x01\x01\x21\x01\x32\x32\x04\x00"
"\x22\xFF\x01\x21\x21\xFF\x01\x21\x01\x01\x01\x01\x21\x01\x01\x22\xFF\x21\x01\x34\x01\x01\x21\x01\x32\x32\x04\x00"
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x14\xFE\x01\x01\x01\x01\xFE\x01\x22\x01\x34\x01\x01\x01\x32\x21\x02\x00"
  "\x03r\xFE\x80\x00"//                ^^
	"l\\\x00l\\\x00l\\\x00l\\\x00|\x34*\xFE\x01\x80\x00"
	  "\x01\x01\x01\x01\x01\x01\x34\x00"
  "*\xFE\x02\\\x00\\\x00\x00" //                           ^^
	"\x01\x01r\x01x\x80\x81\xC0\x00"
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01\x01\x00"
//                                 08                              10
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x21\x21\x21\x01\x22\xFE\x01\x04\x00"
  "\x01\x01r\x34x\x81\x20\xC0\x00"//                   ^^
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01\x01\x02\x01\x01\x01\x01\x01\x22\x01\x32\x02\x01\x01\x01\x04\x01\x01\x01\x01\x01\x01\x04\x01\xFE\x00"
  "\x01r\x01\x80\x00"
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01\x14\x32\x24\x12\x14\x14\x22\xFE\x00"
  "r\xFE\x0B\xFE\x00"//                                            ^^
	"r\x01\x20\xC0\x00"
	"r\x01\x20\x00"
//                                 08                              10                              18                              20
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01\x01\x02\x02\x02\x01\x01\x02\x01\x01\x01\x00"
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01\x01\xFE\x01\x02\x02\x02\x01\x02\x04\x01\x01\x01\xFE\xFE\x01\x01\x01\x04\x02\x04\x04\x04\x02\x04\x01\x01\x24\x32\x00"
  "\x01\xF4r\xFE\x80\x00"//                ^^
	"l\xFE\x01\xC1|*\xFE\x02\\\x00\x80\x00"
	  "\x00"
	  "\x01\x01\xFE\xC0\x00"
		"l\xFE\x02|\x01\x00"
		  "\x00"
  "\x01r\x01\x80\x00"//                                                                ^^
  "r\x01\x03\x00"//                                                                        ^^
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x21\x32\x32\x32\x32\x32\x32\x01\x01\x01\x34\x01\x01\x01\x32\x14\x01\x32\x01\x00"
//                                 08                              10                              18
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01\x01\x01\x00"
"\x00"
"\x00"
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x14\x02\x02\x00"
;

static const char _dat2v[]="\x0D\x09\x0F\x13"
	"\x00\x82\x01\x81\x02\x81\x03\x81\x09\x82\x0A\x82\x0C\x82\x10\x84\x12\x81\x18\x84\x1A\xC4\x1B\x81\x1C\x84\x20\x81\x7D\x84\xFF\x7E\xC2\xFF\x7F\x84\x7F\xFF\xF0"
"\x00\x40\x83\x41\x83\x42\x84\x43\x84\x45\x83\x46\x84\x47\x84\x48\x81\x5F\x81\x60\x81\x73\xFF\xF0"
"\x01\x40\x83\x41\x83\x42\x83\x43\x84\x45\x83\x46\x84\x47\x84\x48\x81\x5F\x81\xFF\xF0"
"\x02\x42\x81\x43\x84\x46\x84\x47\x84\x48\x81\x5F\x81\xFF\xF0"
"\x03\x40\x83\x42\x81\x43\x84\x46\x84\x48\x81\x44\x82\x47\x84\x5F\x81\xFF\xF0"
"\x08\x11\x81\x40\x84\x41\x84\x42\x82\x43\x83\x44\x81\x45\x82\x46\x84\x47\x84\x48\x85\x49\x84\x4A\x81\x5F\x83\x60\x82\x1F\x61\x81\xFF\x62\x84\xFF\x63\x81\xFF\x64\x82\xFF\x65\x81\xFF\x66\x84\xFF\x67\x82\xFF\x68\x82\xFF\x69\x81\xFF\xFF\xF0"
"\x05\x84\x80"
"\x00\x40\x83\x41\x83\x42\x84\x43\x84\x45\x83\x46\x84\x47\x84\x48\x81\x5F\x81\x60\x81\x73\xFF\xF0"
"\x08\x40\x81\x41\x81\x42\x81\x43\x81\x44\x84\x45\x81\x46\x81\x5F\x81\x60\x82\x6B\x61\x82\xFF\x62\x84\xFF\x63\x81\xFF\x64\x81\xFF\x80\x80"
"\x08\x40\x81\x41\x81\xDE\x80"
"\x0A\x40\x81\x41\x81\x42\x81\x43\x83\x44\x81\x5F\x81\x60\x84\xFF\x61\x84\xFF\x62\x82\xFF\x80\x80"
"\x08\x40\x82\x41\x82\x42\x82\x43\x82\x44\x81\x45\x84\x60\x82\xFF\x61\x81\xFF\x62\x84\xFF\x63\x84\xFF\x64\x84\xFF\x65\x83\xFF\x66\x83\xFF\x67\x84\xFF\x68\x84\xFF\x7C\x84\x0F\xB6\x80"
"\x0B\x80\x80"
"\x0C\x80\x80"
"\x0D\x80\x80"
"\x0E\x60\x84\xFF\x80\x80"
"\x0F\x80\x80"
;

static const char _datD[]="\x14\x02\x0F"
	"\x11\x0C"            /*GRM count:*/"\x74\x00\x58\x00\x0B\x00\x29\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1E\x13\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00\x00\x00";
//Feature:                                 --00--                          --04--                          --08--                          --0C--                          --10--
static const char _datIDs[]="\x77\x01\x0F\x73\x00\x57\x00\x0A\x00\x28\x00\xFF\x00\x07\x00\x0A\x00\xFF\x00\x00\x00\xFF\x00\x24\x00\x1F\x00\xFF\xFF\x00\x00\x00\x00\xFF\x00";
static const char _dat4[]="\x03\x02\x0F"
"\x04\x04\x04\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
// 00              04              08              0C              10
"\x1C\x1C\x1C\x1C\x04\x04\x00\x04\x00\x1C\x3D\x06\x00\x00\x00\x00";
//First line: rules for one-byte IDs
//Second line: rules for two-byte IDs
//enum {CTRL_FONT_LARGE=1, CTRL_FONT_SMALL=2, CTRL_SPACE=4, CTRL_NEWLINE=8
//		CTRL_COLOR=0x10, CTRL_NO_STACK_CHECK=0x20}

//byte triples: num 80 bits/num 83 bits/num triggers
//                                        --  00  --                                      --  04  --                                      --  08  --                                      --  0C  --                                      --  10  --
static const char _dat2r[]="\x02\x01\x0F\x08\x08\x05\x08\x08\x05\x08\x08\x05\x08\x08\x05\x20\x00\x06\x00\x00\x00\x00\x00\x00\x08\x00\x02\x00\x00\x00\x08\x10\x03\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

/* Files that depend on feat.dat (appear below it in data.h), have three
 * leading bytes that are eaten by myfopen. The first two are as described
 * above, near line 120. The third is the maximum feature supported by this
 * file. This should usually match the third byte of _datfeat, but under some
 * circumstances, it is appropriate to increment the this byte without
 * changing _datfeat. */

struct dat{
	char*data,*name;
	uint len;
};

#undef _RENUM_DATA_H_INCLUDED_
#undef DATA
#undef DATA_FILE
#define DATA() static const dat data[]={
#define DATA_FILE(name)\
	{(char*)_dat##name,"/.renum/" #name ".dat",sizeof(_dat##name)-1},\

#include "data.h"

bool makedir(const string&dir,bool dieonfail=false){
	if(dir==""){
		if(dieonfail)exit(EDATA);
		return false;
	}
	if(mkdir((dir+"/.renum").c_str(),0755)){
		IssueMessage(0,CREATE_FAILED,dir.c_str(),errno);
		perror(NULL);
		if(dosleep)sleep(5);
		if(dieonfail)exit(EDATA);
		return false;
	}else{
		IssueMessage(0,CREATED_DIR,dir.c_str());
		if(dosleep)sleep(5);
		return true;
	}
}

bool finddir(string&dir){
	if(dir=="")return false;
	struct stat Stat;
	if(dir[dir.length()-1]=='\\'||dir[dir.length()-1]=='/')
		dir[dir.length()-1]='\0';
	if(stat((dir+"/.renum").c_str(),&Stat))return false;
	else if(Stat.st_mode&S_IFREG)return false;
	return true;
}

string getdir(){
	string *pret;
	string cwd,home,homedrpath;
	if(datadir!=""){
		verify(finddir(datadir)||makedir(datadir,true));
		pret=&datadir;
	}else{
		char*pcwd=getcwd(NULL,0);
		cwd=pcwd;
		home=safetostring(getenv("HOME"));
		homedrpath=safetostring(getenv("HOMEDRIVE"))+safetostring(getenv("HOMEPATH"));
		free(pcwd);
		if(finddir(cwd))pret=&cwd;
		else if(finddir(home))pret=&home;
		else if(finddir(homedrpath)||makedir(homedrpath))pret=&homedrpath;
		else if(makedir(home))pret=&home;
		else{
			verify(makedir(cwd,true));
			pret=&cwd;
		}
	}
	if(!dosleep)IssueMessage(0,DATA_FOUND_AT,pret->c_str());
	return *pret;
}

FILE*tryopen(const char*name,const char*mode,bool allownull=false){
	static string dir=getdir();
	FILE*pFile=fopen((dir+name).c_str(),mode);
	if(pFile||allownull)return pFile;
	IssueMessage(0,DATAFILE_ERROR,OPEN,name+1,ERRNO,errno);
	perror(NULL);
	assert(false);
	exit(EDATA);
}

FILE*_myfopen(files file){
	FILE*pFile=tryopen(data[file].name,"rb",true);
	if(pFile){
		if(fgetc(pFile)==data[file].data[0]&&fgetc(pFile)>=data[file].data[1]){
			if(file>datfeat && (uint)fgetc(pFile)<MaxFeature()){
				IssueMessage(0,DATAFILE_MISMATCH,data[file].name+8);
				assert(false);
				exit(EDATA);
			}
			return pFile;
		}
		fclose(pFile);
	}
	pFile=tryopen(data[file].name,"wb");
	if(fwrite(data[file].data,1,data[file].len,pFile)!=data[file].len){
		IssueMessage(0,DATAFILE_ERROR,WRITE,data[file].name+1,-1);
		assert(false);
		exit(EDATA);
	}
	fclose(pFile);
	pFile=tryopen(data[file].name,"rb");
	fgetc(pFile);
	fgetc(pFile);
	if(file>datfeat && (uint)fgetc(pFile)<MaxFeature()){
		IssueMessage(0,DATAFILE_MISMATCH,data[file].name+8);
		assert(false);
		exit(EDATA);
	}
	return pFile;
}

int _CheckEOF(int dat,files file,const char*src,int line){
	if(dat==EOF){
		IssueMessage(0,DATAFILE_ERROR,LOAD,data[file].name+8,FILELINE,src,line);
		assert(false);
		exit(EDATA);
	}
	return dat;
}

int _GetCheckWord(FILE*pFile,files file,const char*src,int line){
	int ret=fgetc(pFile);
	return ret|(_CheckEOF(fgetc(pFile),file,src,line)<<8);
}

void _myfread(FILE*pFile,uchar*target,uint count,files file,const char*src,int line){
	if(fread(target,1,count,pFile)!=count){
		IssueMessage(0,DATAFILE_ERROR,LOAD,data[file].name+8,FILELINE,src,line);
		assert(false);
		exit(EDATA);
	}
}
