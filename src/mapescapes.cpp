/*
 * mapescapes.cpp
 * Helper functions for using escapes
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

#include <algorithm>
#include <string>


#include "nforenum.h"
#include "inlines.h"
#include "messages.h"
#include "mapescapes.h"

nfe_map nfo_escapes;

void InsertEscape(const std::string &key, int byte) {
	auto new_pair = std::make_pair(key, byte);
	auto it = std::find(nfo_escapes.begin(), nfo_escapes.end(), new_pair);
	if (it == nfo_escapes.end()) {
		nfo_escapes.push_back(new_pair);
	}
}

void RemoveEscape(const std::string &key) {
	auto it = std::remove_if(nfo_escapes.begin(), nfo_escapes.end(), [key](const auto &p) {
		return p.first == key;
	});
	nfo_escapes.erase(it, nfo_escapes.end());
}

std::string FindEscape(char action, int byte) {
	// Look for a custom escape
	auto it = std::find_if(nfo_escapes.begin(), nfo_escapes.end(), [action, byte](const auto &p) {
		return p.first[0] == action && p.second == byte;
	});
	if (it != nfo_escapes.end()) {
		return " \\" + it->first;
	}

	// Look for a built-in escape
	for (const esc &e : escapes) {
		if (e.action==ctoi(action) && e.byte==byte) return ' ' + std::string(e.str);
	}
	return "";
}

std::string FindEscape(char action, int byte, uint offset) {
	// This time, look for a built-in escape first
	for (const esc &e : escapes) {
		if (e.action==ctoi(action) && e.byte==byte && e.pos==offset) return ' ' + std::string(e.str);
	}
	// Look for a custom escape
	auto it = std::find_if(nfo_escapes.begin(), nfo_escapes.end(), [action, byte](const auto &p) {
		return p.first[0] == action && p.second == byte;
	});
	if (it != nfo_escapes.end()) {
		return " \\" + it->first;
	}
	return "";
}

int FindEscape(const std::string &str) {
	for (const esc &e : escapes) {
		if(str == e.str+1) return e.byte;
	}

	const auto &it = std::find_if(nfo_escapes.begin(), nfo_escapes.end(), [str](const auto &p) {
		return p.first == str;
	});
	if (it != nfo_escapes.end()) {
		return it->second;
	}

	return -1;
}
