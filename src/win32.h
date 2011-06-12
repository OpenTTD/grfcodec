/*
 * win32.h
 * Windows specific stuff for NFORenum.
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

#ifndef _RENUM_WIN32_H_INCLUDED_
#define _RENUM_WIN32_H_INCLUDED_

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include<windows.h>
#include<direct.h>

#undef ERROR

#define sleep(x) Sleep(x*1000)

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
// mkdir is deprecated in MSVS 8.0+
inline int mkdir(const char*x,int){return _mkdir(x);}
#else
inline int mkdir(const char*x,int){return mkdir(x);}
#endif

//string GetOpt(char*);

#endif//_WIN32

#endif//_RENUM_WIN32_H_INCLUDED_
