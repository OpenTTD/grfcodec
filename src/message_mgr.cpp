/*
 * message_mgr.cpp
 * Defines classes and functions for handling of program messages in multiple languages.
 *
 * Copyright (c) 2004-2006, Dale McCoy.
 * Copyright (c) 2006, Dan Masek.
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

#include <iostream>
#include <cassert>

#include "version.h"
#include "globals.h"
#include "message_mgr.h"

using namespace std;

const string MessageData::commentPrefix = "!!";

MessageData::MessageData(char props_) : textMap() {
	props = props_;
}

MessageData::MessageData(char props_, const string& default_text) : textMap() {
	props = props_;
	SetText(RL_DEFAULT, default_text);
}

string MessageData::Display(const string& prefix, va_list& ap) const {
	string ret = myvsprintf(GetMessage(prefix).c_str(), ap);
	ostream* const stream[] = {pErr, pOut, pNfo};
	if((props & TO_MASK) != TO_NULL)
		(*(stream[(props & TO_MASK) >> TO_SHIFT])) << ret;
	return ret;
}

const string& MessageData::GetText() const {
	lang2str_map::const_iterator pos = textMap.find(
		LanguageMgr::Instance().GetCurrentLanguage());
	if(pos == textMap.end()) {
		pos = textMap.find(RL_DEFAULT);
		if(pos == textMap.end()) {
			return MessageMgr::UNDEFINED_TEXT;
		}
	}
	return pos->second;
}

void MessageData::SetText(RenumLanguageId lang, const string& text) {
	textMap[lang] = text;
}

string MessageData::GetMessage(const string& prefix) const {
	string ret = GetText();
	if(props & HAS_OFFSET)
		ret = MessageMgr::Instance().GetExtraText(OFFSET) + ret;
	if(props & USE_PREFIX)
		ret = prefix + ret;
	if(props & MAKE_COMMENT)
		ret = COMMENT_PREFIX + commentPrefix + ret;
	return ret;
}

// --------

const string MessageMgr::UNDEFINED_TEXT = "UNDEFINED_TEXT";
const MessageData MessageMgr::UNKNOWN_MESSAGE = MessageData(0,"UNKNOWN_MESSAGE");

MessageMgr::MessageMgr() {
	InitMessages();
	InitMessageTexts();
	InitExtraTexts();
}

const MessageData& MessageMgr::GetMessageData(RenumMessageId i) const {
	msgid2data_map::const_iterator pos = msgDataMap.find(i);
	if(pos != msgDataMap.end()) {
		return pos->second;
	}
	return UNKNOWN_MESSAGE;
}

const std::string& MessageMgr::GetExtraText(RenumExtraTextId i) const {
	extid2data_map::const_iterator pos = extraTextMap.find(i);
	if(pos == extraTextMap.end()) {
		return UNDEFINED_TEXT;
	}
	const lang2str_map& textMap = pos->second;
	lang2str_map::const_iterator textPos = textMap.find(
		LanguageMgr::Instance().GetCurrentLanguage());
	if(textPos == textMap.end()) {
		textPos = textMap.find(RL_DEFAULT);
		if(textPos == textMap.end()) {
			return UNDEFINED_TEXT;
		}
	}
	return textPos->second;
}

bool MessageMgr::AddMessage(RenumMessageId i, char props) {
	return msgDataMap.insert(make_pair(i, MessageData(props))).second;
}

bool MessageMgr::SetMessageText(RenumMessageId i, RenumLanguageId lang, const std::string& text) {
	msgid2data_map::iterator pos = msgDataMap.find(i);
	if(pos != msgDataMap.end()) {
		pos->second.SetText(lang, text);
		return true;
	}
	return false;
}

void MessageMgr::SetExtraText(RenumExtraTextId i, RenumLanguageId lang, const std::string& text) {
	extid2data_map::iterator pos = extraTextMap.find(i);
	if(pos == extraTextMap.end()) {
		pos = extraTextMap.insert(make_pair(i,lang2str_map())).first;
	}
	pos->second.insert(make_pair(lang, text));
}

// -------

// Generate a function to initialize the message map with message data containing
// the properties for all the supported messages.
#undef MESSAGE
#undef START_MESSAGES
#undef END_MESSAGES
#define START_MESSAGES(lang) void MessageMgr::InitMessages() {
#define MESSAGE(name,message,props) AddMessage(name, props);
#define END_MESSAGES() }
#include "lang/message_english.h"

// -------

// Generate functions to add message texts to all the messages
// for each of the supported languages.
#undef MESSAGE
#undef START_MESSAGES
#undef END_MESSAGES
#define START_MESSAGES(lang) void InitMessageTexts_##lang(MessageMgr& msgmgr) { \
	RenumLanguageId langId = lang;
#define MESSAGE(name,message,props) msgmgr.SetMessageText(name,langId, message);
#define END_MESSAGES() }
#include "lang/all_messages.h"

// -------

// Generate a master function to call all the individual message text
// initialization functions.
#undef START_LANGUAGES
#undef RENUM_LANGUAGE
#undef END_LANGUAGES
#define START_LANGUAGES() void MessageMgr::InitMessageTexts() {
#define RENUM_LANGUAGE(name,code) InitMessageTexts_##name(*this);
#define END_LANGUAGES() }
#include "lang/language_list.h"

// -------

// Generate functions to initialize extra texts for each of the
// supported languages.
#undef EXTRA
#undef START_EXTRA_STRINGS
#undef END_EXTRA_STRINGS
#define START_EXTRA_STRINGS(lang) void InitExtraTexts_##lang(MessageMgr& msgmgr) { \
	RenumLanguageId langId = lang;
#define EXTRA(name,str) msgmgr.SetExtraText(name, langId, str);
#define END_EXTRA_STRINGS() }
#include "lang/all_extra.h"

// -------

// Generate a master function to call all the individual extra text
// initialization functions.
#undef START_LANGUAGES
#undef RENUM_LANGUAGE
#undef END_LANGUAGES
#define START_LANGUAGES() void MessageMgr::InitExtraTexts() {
#define RENUM_LANGUAGE(name,code) InitExtraTexts_##name(*this);
#define END_LANGUAGES() }
#include "lang/language_list.h"
