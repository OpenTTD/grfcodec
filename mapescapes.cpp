/*
 * mapescapes.cpp
 * Helper functions for using boost::bimapped escapes
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <string>

using namespace std;

#include "renum.h"
#include "inlines.h"
#include "messages.h"
#include "mapescapes.h"

string FindEscape(char action, int byte) {
	// Look for a custom escape
	foreach(const nfe_rpair& p, nfo_escapes.right.equal_range(byte))
		if (p.second[0] == action)
			return mysprintf(" \\%t", p.second.c_str());

	// Look for a built-in escape
	foreach(const esc& e, escapes)
		if (e.action==ctoi(action) && e.byte==byte)
			return mysprintf(" %t", e.str);
	return "";
}

string FindEscape(char action, int byte, uint offset) {
	// This time, look for a built-in escape first
	foreach(const esc& e, escapes)
		if (e.action==ctoi(action) && e.byte==byte && e.pos==offset)
			return mysprintf(" %t", e.str);
	// Look for a custom escape
	foreach(const nfe_rpair& p, nfo_escapes.right.equal_range(byte))
		if (p.second[0] == action)
			return mysprintf(" \\%t", p.second.c_str());
	return "";
}

int FindEscape(string str) {
	foreach(esc e, escapes)
		if(str == e.str+1)
			return e.byte;
	nfe_left_iter ret = nfo_escapes.left.find(str);
	if(ret == nfo_escapes.left.end())
		return -1;
	return ret->second;
}
