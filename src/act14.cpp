/*
 * act14.cpp
 * Contains definitions for checking action 14s.
 *
 * Copyright 2005-2006 by Dale McCoy.
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

#include<string>
#include<cassert>
#include<sstream>

using namespace std;

#include"nforenum.h"
#include"pseudo.h"
#include"messages.h"
#include"strings.h"
#include"command.h"

static bool Check14(PseudoSprite&data, uint&offset, vector<uint>&idstack)
{
	/* NFORenum reads the NFO, which is a text file. As per definition the
	 * NFO is LE ordered. If characters are interpreted as bytes they will
	 * therefore be read in LE order. ExtractDword does interpret the
	 * characters as bytes and construct a host endian ordered integer. 
	 * Consequently, there is no need to swap endian for the read data; it 
	 * will always be in the host order, or the constant below as long as
	 * they have the expected integer value, thus reverse due to LE. */
	static const uint ID_INFO = 0x4F464E49; // INFO in reverse order (LE)
	static const uint ID_PALS = 0x534C4150; // PALS in reverse order (LE)

	uint type = data.ExtractByte(offset++);
	while (type != 0) {
		uint id = data.ExtractDword(offset);
		offset += 4;
		idstack.push_back(id);

		switch (type) {
			case 'C':
				if (!Check14(data, offset, idstack)) return false;
				break;

			case 'T': {
				uint langs = data.ExtractByte(offset);
				extern uint _grfver;
				/* Hardcode GRF version to 7 as that's needed for the language ids
				 * and because we at least require version 7; that is actually
				 * checked a little bit later during Action 8. */
				_grfver = 7;
				if (langs & 0x80) IssueMessage(WARNING2, UNKNOWN_LANG_BIT, offset, langs);
				langs &= 0x7F;
				CheckLangID(langs, offset++);
				CheckString(data, offset, CTRL_COLOR | CTRL_NEWLINE);
				break;
			}

			case 'B': {
				extern uint _act14_pal;
				uint size = data.ExtractWord(offset);
				offset += 2;
				if (idstack.size()==2 && idstack[0]==ID_INFO && idstack[1]==ID_PALS) {
					uint pal=data.ExtractByte(offset);
					if (size==1 && (pal=='D' || pal=='W' || pal=='A')) {
						_act14_pal=pal;
					} else IssueMessage(ERROR, INVALID_PALETTE_INFO, offset);
				}
				offset += size;
				break;
			}

			default:
				IssueMessage(FATAL, UNKNOWN_ACT14_TYPE, offset, type);
				return false;
		}
		idstack.pop_back();
		type = data.ExtractByte(offset++);
	}
	return true;
}

void Check14(PseudoSprite&data)
{
	vector<uint>idstack;
	uint offset = 1;
	Check14(data, offset, idstack);
}
