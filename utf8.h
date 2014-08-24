/*
   utf8.h
   Developer: Rchockxm (rchockxm.silver@gmail.com)
   Copyright: Silence Unlimited (rchockxm.com)
   Reference: Google C++ Style Guide
     
   This program is free software unless you got it under another license directly
   from the author. You can redistribute it and/or modify it under the terms of
   the GNU General Public License as published by the Free Software Foundation.
   Either version 2 of the License, or (at your option) any later version.
*/

/* Define Constant */
#ifndef _UTF8_H
#define _UTF8_H

#define MAX_SMSLIMIT 2048
#define UTF8_WITHOUT_BOM 10320
#define UTF8_WITH_BOM 10321

// Returns true if the char buffer contain UTF-8 char.
BOOL IsStrUTF8(const void* pBuffer, long size);

// Returns pchar if the char buffer convert to ANSI char buffer.
PCHAR wideStrToANSI(BSTR BStr);

// Returns bstr if the char buffer convert to UTF-8 char buffer.
BSTR ANSIToWideStr(char* AStr);

// Returns bstr if the char buffer convert to UTF-8 char buffer.
BSTR wideStrToWideStr(char* AStr);

#endif // _UTF8_H
