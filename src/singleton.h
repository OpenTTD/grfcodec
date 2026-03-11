/*
 * singleton.h
 *
 * Singleton macros
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

#ifndef _RENUM_SINGLETON_H_INCLUDED_
#define _RENUM_SINGLETON_H_INCLUDED_

/**
 * Base class to make a singleton class.
 * @tparam T the singleton class.
 */
template <typename T>
class Singleton {
public:
	/**
	 * Get the singleton class instance.
	 * @return the singleton class instance.
	 */
	static T &Instance() {
		static T instance{};
		return instance;
	}

	/**
	 * Get the const singleton class instance.
	 * @return the const singleton class instance.
	 */
	static const T &CInstance() { return Instance(); }

	Singleton(const Singleton &) = delete;
	Singleton &operator=(const Singleton) = delete;

protected:
	Singleton() = default;
	virtual ~Singleton() = default;
};

/**
 * Base class to make a const singleton class.
 * @tparam T the singleton class.
 */
template <typename T>
class ConstSingleton {
public:
	/**
	 * Get the const singleton class instance.
	 * @return the const singleton class instance.
	 */
	static const T &Instance() {
		static T instance;
		return instance;
	}

	ConstSingleton(const ConstSingleton &) = delete;
	ConstSingleton &operator=(const ConstSingleton) = delete;

protected:
	ConstSingleton() = default;
	virtual ~ConstSingleton() = default;
};

#endif // _RENUM_SINGLETON_H_INCLUDED_
