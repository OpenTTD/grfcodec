/*
 * rangedint.cpp
 *
 * Definitions for class of integers with a limited range.
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

typedef unsigned int uint;

#include "rangedint.h"

RangedUint::RangedUint(uint max):m_max(max){}
RangedUint::RangedUint(uint val,uint max):m_max(max),m_val(val){}

RangedUint::operator uint()const{return m_val;}

const RangedUint&RangedUint::operator+=(uint right){
	if(m_val+right>=m_val)m_overflow=(m_val+=right)>m_max;
	else m_overflow=true;
	return*this;
}

const RangedUint&RangedUint::operator-=(uint right){
	m_overflow=(m_val-right<=m_val);
	m_val-=right;
	return*this;
}

const RangedUint&RangedUint::operator/=(uint right){
	m_overflow=false;
	m_val/=right;
	return*this;
}

const RangedUint&RangedUint::operator*=(uint right){
	if(right&&(m_val*right)/right!=m_val)
		m_overflow=true;
	else
		m_overflow=(m_val*=right)>=m_max;
	return*this;
}

const RangedUint&RangedUint::operator=(uint right){
	m_overflow=(right>m_max);
	m_val=right;
	return*this;
}
