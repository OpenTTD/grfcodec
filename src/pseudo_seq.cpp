/*
 * pseudo_seq.cpp
 * Sequential access implementation for the PseudoSprite classes.
 * Separated due to preprocessor magic.
 *
 * Copyright 2009 by Dale McCoy.
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

#ifndef INCLUDING

#include <boost/preprocessor/cat.hpp>

#include "nforenum.h"
#include "pseudo.h"

#define INCLUDING
#define FOR_CLASS Byte
#define WIDTH(x) 1
#include "pseudo_seq.cpp"

#define FOR_CLASS ExtByte
#define EXTRACT Extended
#define WIDTH(x) ExtendedLen(x)
#include "pseudo_seq.cpp"

#define FOR_CLASS Word
#define WIDTH(x) 2
#include "pseudo_seq.cpp"

#define FOR_CLASS Dword
#define WIDTH(x) 4
#include "pseudo_seq.cpp"

#undef INCLUDING

uint PseudoSprite::BytesRemaining() const {
	return (uint)packed.length() - extract_offs;
}

PseudoSprite& PseudoSprite::seek(uint off) {
	extract_offs = off;
	return *this;
}

PseudoSprite::Byte& PseudoSprite::Byte::set(uint u) {
	p->SetByteAt(offs, u);
	return *this;
}

PseudoSprite::Word& PseudoSprite::Word::set(uint u) {
	p->SetByteAt(offs, u&0xFF);
	p->SetByteAt(offs, u>>8);
	return *this;
}

#else // INCLUDING

#ifndef EXTRACT
# define EXTRACT FOR_CLASS
#endif

PseudoSprite::FOR_CLASS::FOR_CLASS() :
	p(NULL)
{}


uint PseudoSprite::FOR_CLASS::val() const {
	return p->BOOST_PP_CAT(Extract, EXTRACT(offs));
}

uint PseudoSprite::FOR_CLASS::loc() const {
	return offs;
}

PseudoSprite& PseudoSprite::operator >>(PseudoSprite::FOR_CLASS& b) {
	b.p = this;
	b.offs = extract_offs;
	extract_offs += WIDTH(extract_offs);
	return *this;
}

PseudoSprite& PseudoSprite::Extract(PseudoSprite::FOR_CLASS& b, uint off) {
	b.p = this;
	b.offs = off;
	return *this;
}

#undef FOR_CLASS
#undef EXTRACT
#undef WIDTH


#endif //INCLUDING
