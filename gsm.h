/*
   gsm.h
   Developer: Rchockxm (rchockxm.silver@gmail.com)
   Copyright: Silence Unlimited (rchockxm.com)
   Reference: Google C++ Style Guide

   This program is free software unless you got it under another license directly
   from the author. You can redistribute it and/or modify it under the terms of
   the GNU General Public License as published by the Free Software Foundation.
   Either version 2 of the License, or (at your option) any later version.
*/

/* Define Constant */
#ifndef _GSM_H
#define _GSM_H

#define DLL_COMMOMEX "CommonEx.dll"
#define DLL_MSVCR120 "msvcr120.dll"
#define DLL_MSVCR120D "msvcr120d.dll"

#ifdef COMMOMEXDLL
#define Func_ModemInit "ModemInit"
#define Func_ModemUnInit "ModemUnInit"
#define Func_ModemRead "ModemRead"
#define Func_ModemWrite "ModemWrite"
#define Func_ModemExecCommand "ModemExecCommand"
#define Func_ModemSendSMS "ModemSendSMS"
#endif // COMMOMEXDLL

/* Type Struct */
#ifdef COMMOMEXDLL
typedef BOOL __cdecl ModemInitFunc(char*);
typedef BOOL __cdecl ModemUnInitFunc(void);
typedef INT __cdecl ModemReadFunc(char*, int);
typedef INT __cdecl ModemWriteFunc(char*, int);
typedef char* __cdecl ModemExecCommandFunc(char*, int);
typedef BOOL __cdecl ModemSendSMSFunc(char*, char*);
#endif // COMMOMEXDLL

typedef struct structGSMs {
    BOOL cmInit;
    BOOL cmBusy;
    BOOL cmSending;
    BOOL cmResult;
    BOOL olAutomatic;
    BOOL olAutoFetching;
    BOOL olDisplayLog;
} structGSMs;

/* Declare Function */
#ifdef COMMOMEXDLL
// Returns true if the modem initiation connect success.
BOOL gsmModemInit(HMODULE hCommonEx, char* lpPort);

// Returns true if the modem close success.
BOOL gsmModemUnInit(HMODULE hCommonEx);

// Returns none-zero if modem read success.
INT gsmModemRead(HMODULE hCommonEx, char* pData, int nLength);

// Returns none-zero if modem write success.
INT gsmModemWrite(HMODULE hCommonEx, char* pData, int nLength);

// Returns char buffer if modem respond.
char* gsmModemExecCommand(HMODULE hCommonEx, char* pData, int nLength);

// Returns true if the modem send AT Command success.
BOOL gsmModemSendSMSByText(HMODULE hCommonEx, char* lpNumber, char* lpMsg);
#endif // COMMOMEXDLL

#endif // _GSM_H
