/****************************************************************************
 *                          utf8.h
 * This header file defines constants and macros to be used for utf8
 * decoding.
 *
 ***************************************************************************/

/* Original version (such as it is) at http://xsb.sourceforge.net/api/utf8_8h-source.html*/

#ifndef UTF8_H_INCLUDED
#define UTF8_H_INCLUDED

uint GetUtf8Char(PseudoSprite&,uint&,bool,bool&);
string GetUtf8Encode(uint);

#endif /*UTF8_H_INCLUDED*/
