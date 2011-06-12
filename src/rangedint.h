/*
 * rangedint.h
 *
 * Declarations for class of integers with a smaller than normal max.
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

#ifndef _RENUM_RANGEDINT_H_INCLUDED_
#define _RENUM_RANGEDINT_H_INCLUDED_

class RangedUint{
public:
	//RangedUint();
	explicit RangedUint(uint max);
	RangedUint(uint val,uint max);

	operator uint()const;
	const RangedUint&operator+=(uint);
	const RangedUint&operator-=(uint);
	const RangedUint&operator/=(uint);
	const RangedUint&operator*=(uint);
	const RangedUint&operator=(uint);

	//SetRange(uint min,uint max);

	bool LastOpOverflow()const{return m_overflow;}

private:
	uint m_max,m_val;
	bool m_overflow;
};

class RangedInt{
public:
	//RangedUint();
	explicit RangedInt(const RangedUint&);
	explicit RangedInt(int max);
	RangedInt(int min,int max);
	RangedInt(int val,int min,int max);

	operator int()const;
	const RangedInt&operator+=(int);
	const RangedInt&operator-=(int);
	const RangedInt&operator/=(int);
	const RangedInt&operator=(int);

	//SetRange(uint min,uint max);

	bool LastOpOverflow()const{return m_overflow;}

private:
	RangedInt(const RangedInt&);
	int m_min,m_max,m_val;
	bool m_overflow;
};

#endif//_RENUM_RANGEDINT_H_INCLUDED_
