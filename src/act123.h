/*
 * act123.h
 * Declares classes to support checking actions 1-3.
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

#include <set>
#include <map>
#include <vector>

#include "singleton.h"
#include "message_mgr.h"

class PseudoSprite;

struct act123{
	void init();
	uint MaxFoundFeat()const;

	class Act1{
	public:
		Act1() = default;
		void init() { spritenum = 0; used.clear(); }
		bool is_used(int set) const { return used.contains(set); }
		void use(int set) { used.insert(set); }
		unsigned int feature,numsets,spritenum = 0;
	private:
		std::set<int> used;
	}act1;

	class IDarray{
	public:
		void init() { _m.clear(); }
		bool is_defined(int id) const {
			if (_m.contains(id)) return _m.at(id).sprite != 0;
			return false;
		}
		bool is_used(int id) const {
			if (_m.contains(id)) return _m.at(id).used;
			return false;
		}
		unsigned int defined_at(int id) const {
			if (_m.contains(id)) return _m.at(id).sprite;
			return 0;
		}
		void define(uint feature, int id, bool checks1C);
		bool checks1C(int id) const {
			if (_m.contains(id)) return _m.at(id).v1C;
			return false;
		}
		void use(int id) { _m[id].used = true; }
		bool test(uint, int) const;
		unsigned short GetFeature(int id) const {
			if (_m.contains(id)) return _m.at(id).feature;
			return (unsigned short)-1;
		}
		friend uint act123::MaxFoundFeat()const;
	private:
		struct info{
			info():used(false),v1C(false),feature((unsigned short)-1),sprite(0){}
			bool used,v1C;
			unsigned short feature;
			unsigned int sprite;
		};
		std::map<int, info> _m;
	};

	IDarray defined2IDs;

	uint act3feature,act3spritenum = 0;
	SINGLETON(act123)
};

class Check2v{
	struct VarData{
		VarData():width(0){}
		VarData(int x):width(x){}
		int Load(FILE*);
		uint min,max,width,maxparam;
	};
	struct FeatData{
		FeatData() = default;
		std::map<uint, VarData>vars;
		std::map<uint, VarData>var80;
		uint last80,featfor82;
	};
public:
	void Check(uint,uint,uint,uint,uint)const;
	static uint GetMaxOp(){return Instance().maxop;}
	static uint Prohibit0Mask(uint);
	static uint GetEffFeature(uint,uint);
	bool IsValid(uint feature, uint var)const;
	SINGLETON(Check2v)
private:
	std::map<uint, VarData> globvars;
	std::vector<FeatData> data;
	uint maxop;
	uint MaxParam(uint feature, uint var)const;
	uint GetWidth(uint feature, uint var)const;
};

class varRange{
private:
	struct range{
		explicit range(uint max):min(0,max),max(max,max){}
		range(const range&right):min(right.min),max(right.max){}
		range(uint rangemax,uint minval,uint maxval):min(minval,rangemax),max(maxval,rangemax){}
		range&operator=(const range&right){min=right.min; max=right.max; return *this;}
		RangedUint min,max;
	}dflt;
public:
	static const uint rangemax[];
	explicit varRange(uint);
	void UpdateRange(uint var,uint op,uint shift,const PseudoSprite&data,uint&offs);
	void AddRange(uint min,uint max);
	void CheckDefault();
private:
	int num;
	uint width;
	std::vector<range>ranges;
	void AddRangeInternal(uint min,uint max,RenumMessageId unreachable);
};

class rand2{
private:
	struct rand2info{
		uint bits[2],numtriggers;
	};
	std::vector<rand2info> data;
public:
	void CheckRand(uint feat,uint type,uint triggers,uint first,uint nrand);
	SINGLETON(rand2);
};

//An object of this class will check and define the given ID when it is destroyed.
class Define2{
public:
	Define2(uint feat,uint id):feature(feat),id(id),checks1C(false){}
	~Define2();
	void Check(uint var){checks1C|=(var==0x1C);}
	void ChangeFeature(uint);
private:
	uint feature,id;
	bool checks1C;
};

class Callbacks {
public:
	std::vector<uint> flags;
	SINGLETON(Callbacks);
};

