/*
 * inject.h
 * Contains definitions for injecting new sprites into the NFO.
 *
 * Copyright 2005 by Dale McCoy.
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

#ifndef _RENUM_INJECT_H_INCLUDED_
#define _RENUM_INJECT_H_INCLUDED_

std::istream&inj_getline(std::istream&,std::string&);
int peek(std::istream&);
void inject(const std::string&);
void inject_into(const std::istream&);

#endif//_RENUM_INJECT_H_INCLUDED_
