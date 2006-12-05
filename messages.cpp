/*
 * messages.cpp
 * defines message processing functions.
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

#include<cstdarg>
#include<string>
#include<cassert>
//#include<sstream>

using namespace std;

// The prefered method for generating version.h for Visual Studio is to
// install Cygwin and use the command "make version.h".
// If this is not an option, create a file with the two lines
// #define VERSION "v3.3.1 r<revision>"
// #define YEARS "2004-2006"
// where <revision> is the current revision of the nforenum source
// Increment the 3.3.1 as necessary to agree with version.def
#include"version.h"
#include"renum.h"
#include"inlines.h"
#include"messages.h"
#include"sanity_defines.h"
#include"strings.h"
#include"command.h"

//#define MSG_ARRAYS_INCLUDE_TIMES 3
//#include"msg_arrays.h"

static const char*STACK[]={NULL,"byte","word","textID","dword","date"};

static bool bAutoMessage;

void AutoConsoleMessages(){
	bAutoMessage=true;
}

void ManualConsoleMessages(){
	bAutoMessage=false;
}

#undef _RENUM_MESSAGES_H_INCLUDED_
#undef MESSAGE
#undef START_MESSAGES
#undef END_MESSAGES

#define START_MESSAGES() const messageData message[]={
#define MESSAGE(name,message,props){message,props},
#define END_MESSAGES() };

#undef EXTRA
#undef START_EXTRA_STRINGS
#undef END_EXTRA_STRINGS

#define EXTRA(name,str)str,
#define START_EXTRA_STRINGS() static const char* const extra [] = {
#define END_EXTRA_STRINGS() };

string myvsprintf(const char*,va_list&);

struct messageData{
	char*text;
	char props;
	bool ConsoleMessage()const{return!(props&NO_CONSOLE);}
	string display(const string&prefix,va_list&ap)const{
		string ret=myvsprintf(GetMessage(prefix).c_str(),ap);
		ostream*const stream[]={pErr,pOut,pNfo};
		if((props&TO_MASK)!=TO_NULL)(*(stream[(props&TO_MASK)>>TO_SHIFT]))<<ret;
		return ret;
	}
	const static string commentPrefix;
private:
	string GetMessage(const string&prefix="")const;
};
const string messageData::commentPrefix="!!";

#include "messages.h"

string messageData::GetMessage(const string&prefix)const{
	string ret=text;
	if(props&HAS_OFFSET)ret=extra[OFFSET]+ret;
	if(props&USE_PREFIX)ret=prefix+ret;
	if(props&MAKE_COMMENT)ret=COMMENT_PREFIX+commentPrefix+ret;
	return ret;
}

string mysprintf(const char*str,...){
	WrapAp(str);
	return myvsprintf(str,ap);
}

#ifdef DEBUG
static int curMessage;
#endif

string IssueMessage(int minSan,int id,...){
	WrapAp(id);
	return vIssueMessage(minSan,id,ap);
}

string vIssueMessage(int minSan,int id,va_list arg_ptr){
#ifdef DEBUG
	curMessage=id;
#endif
	/*if(minSan<0){
		if(GetState(VERBOSE)<-minSan)return"";
	}else*/if(!GetWarn(id,minSan))return"";
	if(message[id].props&MAKE_COMMENT&&GetState(DIFF))return"";
	int prefix=PREFIX_LINT_WARNING;
	switch(minSan){
		case-1:case-2:break;
		case 0:
			prefix=PREFIX_UNPARSEABLE;
			break;
		case FATAL:
		case ERROR:
			prefix=minSan==FATAL?PREFIX_LINT_FATAL:PREFIX_LINT_ERROR;
		case WARNING1:
		case WARNING2:
		case WARNING3:
		case WARNING4:
			if(message[id].ConsoleMessage()&&bAutoMessage)
				IssueMessage(0,CONSOLE_LINT_WARNING-(minSan<WARNING1?WARNING1-minSan:0),_spritenum,minSan-ERROR);
			break;
		DEFAULT(minSan)
	}
	if(message[id].props&MAKE_COMMENT){
		if(minSan==FATAL||minSan==ERROR)SetCode(EERROR);
		else if(minSan>=0)SetCode(EWARN);
	}
	try{
		return message[id].display(mysprintf(extra[prefix],id),arg_ptr);
	}catch(...){
		(*pErr)<<message[FATAL_MESSAGE_ERROR].text<<id<<endl;
		assert(false);
		exit(EFATAL);
	}
}

string myvsprintf(const char*fmt,va_list&arg_ptr){
	string ret;
	int i=-1,pad;
	while(fmt[++i]!='\0'){
		if(fmt[i]!='%')ret+=fmt[i];
		else{
			if(isdigit(fmt[++i]))i+=(int)itoa(pad=atoi(fmt+i)).length();
			else pad=0;
			switch(fmt[i]){
			case'c':
				ret+=(char)va_arg(arg_ptr,int);
				break;
			case'd':
				ret+=itoa(va_arg(arg_ptr,int),10,pad);
				break;
			case't': // If an EXTRA cannot be used (eg for __FILE__), use %t, not %s.
				ret+=(char*)va_arg(arg_ptr,char*);
				break;
			case's':{
				int x=(int)va_arg(arg_ptr,int);
				if(x>=__LAST_EXTRA){
#ifdef DEBUG
					IssueMessage(0,BAD_STRING,x,curMessage,_spritenum);
#else
					IssueMessage(0,BAD_STRING,x);
#endif
					assert(false);
					exit(EFATAL);
				}
				if(x!=-1)
					ret+=extra[x];
				break;
			}case'S':{
				int x=(int)va_arg(arg_ptr,int);
				if(x>=__LAST_EXTRA){
#ifdef DEBUG
					IssueMessage(0,BAD_STRING,x,curMessage,_spritenum);
#else
					IssueMessage(0,BAD_STRING,x);
#endif
					assert(false);
					exit(EFATAL);
				}
				if(x!=-1)
					ret+=myvsprintf(extra[x],arg_ptr);
				break;
			}case'x':{
				uint val=va_arg(arg_ptr,int);
				if(pad&&!(pad&1)){
					while(pad||val){
						ret+=itoa(val&0xFF,16,2);
						val>>=8;
						if((pad?pad-=2:0)||val)ret+=' ';
					}
					break;
				}
				ret+=itoa(val,16);
				break;
			}case'L':{
				uint langID=va_arg(arg_ptr,int);
				ret+=mysprintf(GetLangName(langID).c_str(),langID);
				break;
			}case'K':
				assert(false);
				ret+=STACK[va_arg(arg_ptr,int)];//_msgArrays[STACK].array[va_arg(arg_ptr,int)];
				break;
			/*case'Y':{
				uint array=va_arg(arg_ptr,uint),offset=va_arg(arg_ptr,uint);
				VERIFY(array>=INVALID_MSG_ARRAY,vIssueMessage,array);
				if(offset>=_msgArrays[array].length)offset=_msgArrays[array].length-1;
				ret+=_msgArrays[array].array[offset];
				break;
			}*/default:
				ret+=fmt[i];
			}
		}
	}
	return ret;
}
