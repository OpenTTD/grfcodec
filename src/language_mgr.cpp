/*
 * language_mgr.cpp
 * Defines classes and functions for management of program language.
 *
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

#include<cstdlib>

#include "language_mgr.h"

using namespace std;

#include "inlines.h"

LanguageMgr::LanguageMgr() {
	InitLanguageMap();
	currentId = DetectLanguage();
}

RenumLanguageId LanguageMgr::DetectLanguage() const {
	string langCode = safetostring(getenv("LANG"));
	return DecodeLanguageCode(langCode);
}

RenumLanguageId LanguageMgr::DecodeLanguageCode(const string& code) const {
	str2lang_map::const_iterator pos = codeIdMap.find(code);
	if(pos != codeIdMap.end())
		return pos->second;
	return RL_DEFAULT;
}

// --------

// Generate language list initializer containing supported languages.
#undef START_LANGUAGES
#undef RENUM_LANGUAGE
#undef END_LANGUAGES
#define START_LANGUAGES() void LanguageMgr::InitLanguageMap() {
#define RENUM_LANGUAGE(name,code) \
	codeIdMap.insert(make_pair(code,name));
#define END_LANGUAGES() }
#include "lang/language_list.h"
