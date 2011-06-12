/*
 * language_list.h
 * List of definitions of supported languages.
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

// Each language definition has the following format:
// RENUM_LANGUAGE(<identifier>,<code string>)
// - <identifier>  Used to refer to this language elsewhere in the program.
//                 Will become part of RenumLanguageId enumeration.
// - <code string> A string used to select this language.
START_LANGUAGES()
RENUM_LANGUAGE(RL_ENGLISH,"en")
//RENUM_LANGUAGE(RL_CZECH,"cz")

// ^
// |
// +-- Add new definitions to the end of the list
END_LANGUAGES()
