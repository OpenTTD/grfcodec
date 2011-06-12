/*
 * mapescapes.h
 * Helper definitions for using boost::bimap
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

/* If your compiler errors on the following lines, boost is not
 * properly installed, or your version of boost is too old.
 * Get boost from http://www.boost.org */
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/foreach.hpp>

using namespace boost::bimaps;

#define foreach BOOST_FOREACH

typedef unsigned char U8;
#include "escapes.h"

typedef bimap<string, multiset_of<int> > nfe_map;
typedef nfe_map::value_type nfe_pair;
typedef nfe_map::left_value_type nfe_lpair;
typedef nfe_map::right_value_type nfe_rpair;
typedef nfe_map::left_iterator nfe_left_iter;
typedef nfe_map::right_iterator nfe_right_iter;

extern nfe_map nfo_escapes;
