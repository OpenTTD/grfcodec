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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

class PseudoSprite;

struct act123{
	class Act1{
	public:
		Act1(){init();}
		void init(){spritenum=0;used.resize(0);}
		bool is_used(int set)const{return used[set];}
		void use(int set){used[set]=true;}
		unsigned int feature,numsets,spritenum;
	private:
		Expanding0Array<bool>used;
	}act1;
	class IDarray{
	public:
		void init(){used.resize(0);sprite.resize(0);v1C.resize(0);}
		bool is_defined(int id)const{return sprite[id]!=0;}
		bool is_used(int id)const{return used[id];}
		unsigned short defined_at(int id)const{return sprite[id];}
		void define(unsigned int id,bool checks1C);
		bool checks1C(int id)const{return v1C[id];}
		void use(int id){used[id]=true;}
		bool test(uint,uint)const;
	private:
		Expanding0Array<bool>used;
		Expanding0Array<bool>v1C;
		Expanding0Array<unsigned short>sprite;
	};

	void init();

	ExpandingArray<IDarray>defined2IDs;
	uint act3feature,act3spritenum;
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
		FeatData():var80(VarData(1)){}
		ExpandingArray<VarData>globvars,var40,var60;
		ExpandingFillArray<VarData>var80;
		uint last80,featfor82;
	};
public:
	void Check(uint,uint,uint,uint,uint)const;
	SINGLETON(Check2v)
private:
	ExpandingArray<VarData>globvars;
	auto_array<FeatData>_p;
};

class varRange{
private:
	struct range{
		explicit range(uint max):min(0,max),max(max,max){}
		range(const range&right):min(right.min),max(right.max){}
		range(uint rangemax,uint minval,uint maxval):min(minval,rangemax),max(maxval,rangemax){}
		RangedUint min,max;
	}dflt;
public:
	static const uint rangemax[];
	explicit varRange(uint);
	void UpdateRange(uint op,uint shift,const PseudoSprite&data,uint&offs);
	void AddRange(uint min,uint max);
	void CheckDefault();
private:
	int num;
	uint width;
	vector<range>ranges;
	void AddRangeInternal(uint min,uint max,uint unreachable);
};

class rand2{
private:
	struct rand2info{
		uint bits[2],numtriggers;
	};
	auto_array<rand2info>_p;
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
private:
	uint feature,id;
	bool checks1C;
};

class Callbacks:public auto_array<uchar>{
public:
	uint numcallbacks;
	SINGLETON(Callbacks);
};

