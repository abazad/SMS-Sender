/*
   utf8.c
   Developer: Rchockxm (rchockxm.silver@gmail.com)
   Copyright: Silence Unlimited (rchockxm.com)  
   Reference: Google C++ Style Guide

   This program is free software unless you got it under another license directly
   from the author. You can redistribute it and/or modify it under the terms of
   the GNU General Public License as published by the Free Software Foundation.
   Either version 2 of the License, or (at your option) any later version.
*/

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include "utf8.h"

// Returns a boolean of true, false if error.
// pBuffer is char based ANSI/UTF-8 string and 
// without Unicode string.
BOOL IsStrUTF8(const void* pBuffer, long size)
{
    BOOL IsUTF8 = TRUE;

    unsigned char* start = (unsigned char*)pBuffer;
    unsigned char* end = (unsigned char*)pBuffer + size;

    while (start < end)
    {
        if (*start < 0x80)
        {
            start += 1;
        }
        else if (*start < (0xC0))
        {
            IsUTF8 = FALSE;

            break;
        }
        else if (*start < (0xE0)) 
        {
            if (start >= end - 1)
            {
                break;
            }

            if ((start[1] & (0xC0)) != 0x80)
            {
                IsUTF8 = FALSE;

                break;
            }

            start += 2;
        }
        else if (*start < (0xF0))
        {
            if (start >= end - 2)
            {
                break;
            }

            if ((start[1] & (0xC0)) != 0x80 || (start[2] & (0xC0)) != 0x80)
            {
                IsUTF8 = FALSE;

                break;
            }

            start += 3;
        }
        else
        {
            IsUTF8 = FALSE;

            break;
        }
    }

    return IsUTF8;
}

// Returns a string of pchar, null if error.
// BStr is char based UTF-8 string and 
// without Unicode string.
PCHAR wideStrToANSI(BSTR BStr)
{
    PCHAR AStr = NULL;

    int bufferSize;
    int retSize; 

    bufferSize = WideCharToMultiByte(CP_UTF8, 0, BStr, -1, NULL, 0, NULL, NULL);

    if (bufferSize > 0)
    {
        AStr = malloc(bufferSize * sizeof(CHAR));
        retSize = WideCharToMultiByte(CP_ACP, 0, BStr, -1, AStr, bufferSize, NULL, NULL);
    }

    SysFreeString(BStr);

    return AStr;
}

// Returns a string of bstr, null if error.
// AStr is char based ANSI string and 
// without UTF-8 string.
BSTR ANSIToWideStr(char* AStr)
{
    BSTR BStr = NULL;   

    long wstrLen;
    int retSize;

    wstrLen = MultiByteToWideChar(CP_ACP, 0, AStr, strlen(AStr), 0, 0);
    BStr = SysAllocStringLen(0, wstrLen);
    retSize = MultiByteToWideChar(CP_ACP, 0, AStr, strlen(AStr), BStr, wstrLen);
    SysFreeString(BStr);

    return BStr;
}

// Returns a string of bstr, null if error.
// AStr is char based UTF-8 string and 
// without Unicode string.
BSTR wideStrToWideStr(char* AStr)
{
    BSTR BStr = NULL; 
  
    long wstrLen;
    int retSize;

    wstrLen = MultiByteToWideChar(CP_UTF8, 0, AStr, strlen(AStr), 0, 0);
    BStr = SysAllocStringLen(0, wstrLen);
    retSize = MultiByteToWideChar(CP_UTF8, 0, AStr, strlen(AStr), BStr, wstrLen);
    SysFreeString(BStr);

    return BStr;
}
