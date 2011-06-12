/*
 * help.cpp
 * Defines functions for displaying program help in multiple languages.
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

#include<iostream>
#include<string>

#include "language_mgr.h"
#include "help.h"

using namespace std;

// --------

// Generate a ShowHelp function for each supported language.
#define START_HELP_TEXT(lang) void ShowHelp_##lang(){ \
	cout << ""
#define END_HELP_TEXT() ; }
#include "lang/all_help.h"

// --------

// Generate a master ShowHelp function to switch to the correct
// language specific one based on the current language.
#undef START_LANGUAGES
#undef RENUM_LANGUAGE
#undef END_LANGUAGES
#define START_LANGUAGES() void ShowHelp() { \
	switch(LanguageMgr::Instance().GetCurrentLanguage()) {
#define RENUM_LANGUAGE(name,code)case name: ShowHelp_##name(); break;
#define END_LANGUAGES() } }
#include "lang/language_list.h"
