/*
 * messages.h
 * Contains messages output by NFORenum, and declares related functions
 *
 * Copyright 2004-2006 by Dale McCoy.
 * Copyright 2006, Dan Masek.
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

#ifndef _RENUM_MESSAGES_H_INCLUDED_
#define _RENUM_MESSAGES_H_INCLUDED_

#include<cstdarg>

// Message properties
#define MAKE_COMMENT 1 //Indicates that the message should be prefixed with "//!!"
#define USE_PREFIX 2  //Use the appropriate PREFIX_* string, as determined by the minSan argument to IssueMessage
#define HAS_OFFSET 4  //Prefix "Offset %d"
#define NO_CONSOLE 8  //Don't auto-send message to console

// Message output stream
#define TO_ERROR 0
#define TO_OUT 0x10
#define TO_NFO 0x20
#define TO_NULL 0x30
#define TO_MASK 0x30
#define TO_SHIFT 4

// --------

// Generate type RenumMessageId to be an enum of
// all the supported message IDs
#ifndef MESSAGE
#define MESSAGE(name,message,props)name,
#define START_MESSAGES(lang) typedef enum {
#define END_MESSAGES() } RenumMessageId;
#endif//MESSAGE

#ifndef ERR_MESSAGE
#define ERR_MESSAGE(name,message,props) MESSAGE(name,message,NO_CONSOLE|props|TO_ERROR)
#define OUT_MESSAGE(name,message,props) MESSAGE(name,message,NO_CONSOLE|props|TO_OUT)
#define NFO_MESSAGE(name,message,props) MESSAGE(name,message,MAKE_COMMENT|props|TO_NFO)
#endif

#ifndef MESSAGE_EX
#define MESSAGE_EX(name,msg,props,loc,base) MESSAGE(name,msg,props|TO_##loc)
#endif

#ifndef MESSAGE_UNUSED
#define MESSAGE_UNUSED(name) MESSAGE(unused_##name,"",0)
#endif

//#ifndef VMESSAGE // Verbose output messages
//#define VMESSAGE(name,message) NFO_MESSAGE(V_##name,message,NO_CONSOLE|HAS_OFFSET)
//#endif

#include "lang/message_english.h"

// --------

// Generate type RenumExtraTextId to be an enum of
// all the supported extra text IDs
#ifndef EXTRA
#define EXTRA(name,str)name,
#define START_EXTRA_STRINGS(lang) typedef enum {
#define END_EXTRA_STRINGS() __LAST_EXTRA} RenumExtraTextId;
#endif //EXTRA
#include "lang/extra_english.h"

// -------

std::string vIssueMessage(int,RenumMessageId,std::va_list&);
std::string IssueMessage(int,RenumMessageId,...);
void AutoConsoleMessages();
void ManualConsoleMessages();
std::string mysprintf(const char*,...);
std::string myvsprintf(const char*,std::va_list&);

#endif//_RENUM_MESSAGES_H_INCLUDED_
