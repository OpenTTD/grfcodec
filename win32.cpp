/*
 * win32.cpp
 * Windows specific stuff for NFORenum.
 *
 * Copyright 2004-2005 by Dale McCoy.
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

#ifdef _WIN32

#include<string>

using namespace std;

#include"win32.h"

string GetName(const string&arg){
    static bool checkargs=true;
    if(!checkargs)return arg;
    if(arg=="--"){
        checkargs=false;
        return "";
    }
    if(arg=="-i"||arg=="--install"){
        HKEY nfo;
        if(RegCreateKeyEx(HKEY_CLASSES_ROOT,".nfo",0,NULL,KEY_READ,&nfo)){
            LPVOID lpOutBuf;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,NULL,GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR)&lpOutBuf,0,NULL);
            IssueMessage(REG_FAILURE,"HKCR\\.nfo",lpOutBuf);
            LocalFree(lpOutBuf);
            return"";
        }
    }
}

#endif//_WIN32
