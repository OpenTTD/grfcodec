/*
 * singleton.h
 *
 * Singleton macros
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
 * along with this program; if not, write to the Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _RENUM_SINGLETON_H_INCLUDED_
#define _RENUM_SINGLETON_H_INCLUDED_

//Mutable singleton
#define SINGLETON(class)\
public:\
	static class&Instance(){static class obj;return obj;}\
	static const class&CInstance(){return Instance();}\
private:\
	class();\
	class(const class&);\
	void operator=(const class&);

//Immutable singleton
#define C_SINGLETON(class)\
public:\
	static const class&Instance(){static const class obj;return obj;}\
private:\
	class();\
	class(const class&);\
	void operator=(const class&);

//Non-object class
#define STATIC(class)\
private:\
	class();\
	class(const class&);\
	void operator=(const class&);

#endif // _RENUM_SINGLETON_H_INCLUDED_
