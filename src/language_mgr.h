/*
 * language_mgr.h
 * Declares classes and functions for management of program language.
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

#ifndef _RENUM_LANGUAGE_MGR_H_INCLUDED_
#define _RENUM_LANGUAGE_MGR_H_INCLUDED_

#include <string>

#ifdef _MSC_VER
#pragma warning (disable : 4702)  // warning C4702: unreachable code
#endif

#include <map>

#ifdef _MSC_VER
#pragma warning (default : 4702)
#endif

#include "singleton.h"

// -------

// Generate type RenumLanguageId to be an enum of
// all the supported language IDs.
#ifndef RENUM_LANGUAGE
#define START_LANGUAGES() typedef enum {
#define RENUM_LANGUAGE(name,code)name,
#define END_LANGUAGES() } RenumLanguageId;
#endif // RENUM_LANGUAGE
#include "lang/language_list.h"

// -------

/*! Default language identifier */
#define RL_DEFAULT RL_ENGLISH

/*! Collection mapping strings to language IDs. */
typedef std::map<std::string,RenumLanguageId> str2lang_map;

/*! Handles program language detection and selection. */
class LanguageMgr {
	SINGLETON(LanguageMgr);
public:
	/*! Gets the current language ID.
		\return ID of the current language.
	*/
	RenumLanguageId GetCurrentLanguage() const { return currentId; };

	/*! Change the current language.
		\param id Language ID to set the current language to.
	*/
	void ChangeLanguage(RenumLanguageId id) { currentId = id; };

	/*! Detects the current language based on environment variables. */
	RenumLanguageId DetectLanguage() const;

	/*! Decodes a language code into its identifier.
		\param code A string containing the language code.
		\return Corresponding language ID, if such a language is supported.
			Otherwise the function returns RL_DEFAULT.
	*/
	RenumLanguageId DecodeLanguageCode(const std::string& code) const;

private:
	/*! Initializes the language map with data about supported languages. */
	void InitLanguageMap();

	RenumLanguageId currentId; /*!< Current language ID */
	str2lang_map codeIdMap; /*!< Maps language codes to identifiers */
};

#endif // _RENUM_LANGUAGE_MGR_H_INCLUDED_
