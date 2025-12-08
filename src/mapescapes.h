/*
 * mapescapes.h
 * Helper definitions for using NFO escapes
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

#include <vector>

#include "typesize.h"
#include "escapes.h"

using nfe_map = std::vector<std::pair<std::string, int>>;

void InsertEscape(const std::string &key, int val);
void RemoveEscape(const std::string &key);
std::string FindEscape(char action, int byte);
std::string FindEscape(char action, int byte, uint offset);
int FindEscape(const std::string &str);

extern nfe_map nfo_escapes;
