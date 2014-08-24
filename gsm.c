/*
   gsm.c
   Developer: Rchockxm (rchockxm.silver@gmail.com)
   Copyright: Silence Unlimited (rchockxm.com)
   Reference: Google C++ Style Guide

   This program is free software unless you got it under another license directly
   from the author. You can redistribute it and/or modify it under the terms of
   the GNU General Public License as published by the Free Software Foundation.
   Either version 2 of the License, or (at your option) any later version.
*/

#define COMMOMEXDLL

#include <windows.h>
#include <windowsx.h>
#include "gsm.h"

// Returns a boolean of true, false if error.
// lpPort is char based port name and 
// without spaces.
BOOL gsmModemInit(HMODULE hCommonEx, char* lpPort) 
{
    if (hCommonEx)
    {
        ModemInitFunc* ModemInit = (ModemInitFunc*)GetProcAddress(hCommonEx, Func_ModemInit);

        return ModemInit(lpPort);
    }

    return TRUE;
}

// Returns a boolean of true, false if error.
// Modem connect instance is define on <commonex> 
// and no needed to define again.
BOOL gsmModemUnInit(HMODULE hCommonEx)
{
    if (hCommonEx)
    {
        ModemUnInitFunc* ModemUnInit = (ModemUnInitFunc*)GetProcAddress(hCommonEx, Func_ModemUnInit);

        return ModemUnInit();
    }

    return FALSE;
}

// Returns a int of pointer number, 0 if error.
// pData is char based ANSI string and 
// without UTF-8 string.
INT gsmModemRead(HMODULE hCommonEx, char* pData, int nLength)
{
    if (hCommonEx)
    {
        ModemReadFunc* ModemRead = (ModemReadFunc*)GetProcAddress(hCommonEx, Func_ModemRead);

        return ModemRead(pData, nLength);
    }

    return 0;
}

// Returns a int of pointer number, 0 if error.
// pData is char based ANSI string and 
// without UTF-8 string.
INT gsmModemWrite(HMODULE hCommonEx, char* pData, int nLength)
{
    if (hCommonEx)
    {
        ModemWriteFunc* ModemWrite = (ModemWriteFunc*)GetProcAddress(hCommonEx, Func_ModemWrite);

        return ModemWrite(pData, nLength);
    }

    return 0;
}

// Returns a int of pointer number, 0 if error.
// pData is char based ANSI string and 
// without UTF-8 string.
char* gsmModemExecCommand(HMODULE hCommonEx, char* pData, int nLength)
{
    if (hCommonEx)
    {
        ModemExecCommandFunc* ModemExecCommand = (ModemExecCommandFunc*)GetProcAddress(hCommonEx, Func_ModemExecCommand);

        return ModemExecCommand(pData, nLength);
    }

    return "";
}

// Returns a boolean of true, false if error.
// lpNumber and lpMsg is char based ANSI string and 
// without UTF-8 string.
BOOL gsmModemSendSMSByText(HMODULE hCommonEx, char* lpNumber, char* lpMsg)
{
    if (hCommonEx)
    {
        ModemSendSMSFunc* ModemSendSMS = (ModemSendSMSFunc*)GetProcAddress(hCommonEx, Func_ModemSendSMS);
        
        return ModemSendSMS(lpNumber, lpMsg);
    }

    return FALSE;
}
