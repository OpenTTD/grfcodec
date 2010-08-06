/****************************************************************************
 *                               utf8.c[pp]
 * This file contains functions to decode utf8 characters data the input
 * stream.
 *
 **************************************************************************/

/* Original version at http://xsb.sourceforge.net/api/utf8_8c-source.html */

#include<cassert>
#include<cstdlib>

#include"nforenum.h"
#include"pseudo.h"
#include"utf8.h"
#include"messages.h"
#include"command.h"
#include"globals.h"

#define CONT(i) ((data[off+i]&0xc0) == 0x80)
#define VAL(i, s) ((data[off+i]&0x3f) << s)

uint PseudoSprite::ExtractUtf8(uint& off, bool& valid){
	PseudoSprite& data = *this;		// Historical
	valid=false;
	if(data[off]<0xC0||data[off]>0xFC){
		CheckLinkage(off,1);
		return data[off++];
	}
	valid=true;
										/* 2-byte, 0x80-0x7ff */
	if ( (data[off]&0xe0) == 0xc0 && CONT(1) ){
		uint ret = ((data[off]&0x1f) << 6)|VAL(1,0);
		if(ret<0x80){
			IssueMessage(ERROR,OVERLENGTH_UTF8,off);
			valid=false;
			return data[off++];
		}
		data.SetUTF8(off,2);
		CheckLinkage(off,2);
		off+=2;
		return ret;
	}									/* 3-byte, 0x800-0xffff */
	if ( (data[off]&0xf0) == 0xe0 && CONT(1) && CONT(2) ){
		uint ret = ((data[off]&0xf) << 12)|VAL(1,6)|VAL(2,0);
		if(ret<0x800){
			IssueMessage(ERROR,OVERLENGTH_UTF8,off);
			valid=false;
			return data[off++];
		}
		data.SetUTF8(off,3);
		CheckLinkage(off,3);
		if (ret>0xE07A && ret<0xE100)
			SetEscape(off, true, mysprintf("\\U%x", ret), 3);
		off+=3;
		return ret;
	}									/* 4-byte, 0x10000-0x1FFFFF */
	if ( (data[off]&0xf8) == 0xf0 && CONT(1) && CONT(2) && CONT(3) ){
		if((((data[off]&0x7) << 18)|VAL(1,12)/*|VAL(2,6)|VAL(3,0)*/)<0x10000){
			IssueMessage(ERROR,OVERLENGTH_UTF8,off);
			valid=false;
			return data[off++];
		}
		data.SetUTF8(off,4);
		off+=4;
		return(uint)-4;
	}									/* 5-byte, 0x200000-0x3FFFFFF */
	if ( (data[off]&0xfc) == 0xf8 && CONT(1) && CONT(2) && CONT(3) && CONT(4) ){
		if((((data[off]&0x3) << 24)|VAL(1,18)/*|VAL(2,12)|VAL(3,6)|VAL(4,0)*/)<0x200000){
			IssueMessage(ERROR,OVERLENGTH_UTF8,off);
			valid=false;
			return data[off++];
		}
		data.SetUTF8(off,5);
		off+=5;
		return(uint)-5;
	}									/* 6-byte, 0x4000000-0x7FFFFFFF */
	if ( (data[off]&0xfe) == 0xfc && CONT(1) && CONT(2) && CONT(3) && CONT(4) && CONT(5) ){
		if((((data[off]&0x1) << 30)|VAL(1,24)/*|VAL(2,18)|VAL(3,12)|VAL(4,6)|VAL(5,0)*/)<0x4000000){
			IssueMessage(ERROR,OVERLENGTH_UTF8,off);
			valid=false;
			return data[off++];
		}
		data.SetUTF8(off,6);
		off+=6;
		return(uint)-6;
	}

	IssueMessage(WARNING2,INVALID_UTF8,off);
	return data[off++];
}

#define CHAR(x) (char(((ch>>((x)*6))&0x3F)|0x80))

string GetUtf8Encode(uint ch){
	if(ch<0x80)return string()+char(ch);
	if(ch<0x800)return string()+char(((ch>>6 )&0x1F)|0xC0)+CHAR(0);
	if(ch<0x10000)return string()+char(((ch>>12)&0x0F)|0xE0)+CHAR(1)+CHAR(0);
	if(ch<0x200000)return string()+char(((ch>>18)&0x07)|0xF0)+CHAR(2)+CHAR(1)+CHAR(0);
	if(ch<0x4000000)return string()+char(((ch>>24)&0x03)|0xF8)+CHAR(3)+CHAR(2)+CHAR(1)+CHAR(0);
	if(ch<0x80000000)return string()+char(((ch>>30)&0x01)|0xFC)+CHAR(4)+CHAR(3)+CHAR(2)+CHAR(1)+CHAR(0);
	INTERNAL_ERROR(ch,ch);
}
