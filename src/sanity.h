/*
 * sanity.h
 * Declares general functions for sprite sanity checking.
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

#ifndef _RENUM_SANITY_H_INCLUDED_
#define _RENUM_SANITY_H_INCLUDED_

class PseudoSprite;

enum SpriteType{REAL,PSEUDO,INCLUDE,RECOLOR};

void reset_sanity();
void check_sprite(SpriteType);//real/include
void check_sprite(PseudoSprite&);//Pseudo
void final_sanity();
void sanity_use_id(int);
void sanity_use_set(int);
void sanity_test_id(int);
int sanity_locate_id(int);
void sanity_define_id(int,int);
int sanity_get_feature(int);

#endif//_RENUM_SANITY_H_DEFINED
