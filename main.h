/*
   main.h
   Developer: Rchockxm (rchockxm.silver@gmail.com)
   Copyright: Silence Unlimited (rchockxm.com)
   Reference: Google C++ Style Guide
     
   This program is free software unless you got it under another license directly
   from the author. You can redistribute it and/or modify it under the terms of
   the GNU General Public License as published by the Free Software Foundation.
   Either version 2 of the License, or (at your option) any later version.
*/

/* Define Constant */
#ifndef _MAIN_H
#define _MAIN_H

/* {E21CE554-B691-4079-A036-8A45BC4EB462} */
DEFINE_GUID(NONAME,0xe21ce554,0xb691,0x4079,0xa0,0x36,0x8a,0x45,0xbc,0x4e,0xb4,0x62);

// DLL and COM library error flags.
#define DLL_RESULT_SUCCESS 10
#define DLL_ERROR_COMMONEX 20
#define DLL_ERROR_COM 30 
#define DLL_ERROR_SQLITE 40

// Dialog form and control ids.
#define DLG_CLASS "smsClass"
#define DLG_MAIN 1001
#define DLG_MODEM 1004
#define DLG_SMSURL 1002
#define DLG_SMSCUSTOM 1003
#define DLG_LISTBOX_TASK 4022
#define IDC_LISTBOX_LOG 4001
#define IDC_CHECKBOX_AUTO 4003
#define IDC_CHECKBOX_LOG 4016
#define IDC_COMBOBOX_COM 4004
#define IDC_BTN_COMOPEN 4006
#define IDC_BTN_COMCLOSE 4007
#define IDC_BTN_COMMODEM 4009
#define IDC_BTN_COMSENDWEB 4010
#define IDC_BTN_COMSENDFILE 4011
#define IDC_BTN_COMSENDCUSTOM 4018
#define IDC_BTN_COMRESUME 4013
#define IDC_BTN_COMPAUSE 4014
#define IDC_BTN_COMSTOP 4019
#define IDC_BTN_LOGCLEAR 4012
#define IDC_BTN_EXIT 4008
#define IDC_BTN_ABOUT 4023
#define IDC_STATUSBAR 4017

#define IDCC_EDIT_URL 5001
#define IDCC_EDIT_PHONE 5030
#define IDCC_EDIT_MSG 5031
#define IDCC_EDIT_AT 4015
#define IDCC_LISTBOX_ATR 4020
#define IDCC_BTN_ATCLEAR 4021

#define IDR_ICO_MAIN 8001
#define IDR_MANIFEST 9001

// Timer thread define.
#define ID_TIMER1 1771
#define ID_TIMER1_TIME 18000
#define ID_TIMER2 1772
#define ID_TIMER2_TIME 3000

// Jobman folder and constant define.
#ifndef _JOBMAN
#define _JOBMAN
#define JOBMAN "jobman"
#define JOBMAN_TASK_PREFIX "Jobman Task - "
#define JOBMAN_FAIL "jobman/fail"
#define JOBMAN_SUCCESS "jobman/success"
#define JOBMAN_TEMP "jobman/temp"

#define JOBMAN_CTRL_INIT 473510
#define JOBMAN_CTRL_UNINIT 473511
#define JOBMAN_CTRL_COMOPEN 473512
#define JOBMAN_CTRL_COMCLOSE 476513
#endif // _JOBMAN

#ifndef _SQLITE
#define _SQLITE
#define SQLITE_DB "sms.db"
#endif // _SQLITE

/* Type Struct */
typedef struct structJobman {
	int index;
	int lndPtr;
    int logPtr;
	int maxLimit;
    BOOL timerSec;
    BOOL timerTask;
    char lpPort[20];
    char xmlPath[MAX_PATH];
    char cachePath[MAX_PATH];
    char tempFetchUrl[MAX_PATH];
} structJobman;

typedef struct structXMLAttrib {
    BSTR lpID;
    BSTR lpPhone;
    BSTR lpLoca;
} structXMLAttrib;

typedef struct structSMS {
    PCHAR lpID;
    PCHAR lpNumber;
    PCHAR lpMsg;
    PCHAR lpLoca;
    char rtUrl[MAX_PATH]; 
    int dwStatus;
    int dwRetry;
} structSMS;

typedef struct structINIFile {
    char lzPath[MAX_PATH];
    char sms[20];
} structINIFile;

typedef struct structINISms {
    char port[MAX_PATH];
    char fetchUrl[MAX_PATH];
    char reportUrl[MAX_PATH];
    char autoSMS[MAX_PATH];
    char logOutput[MAX_PATH];
} structINISms;

typedef struct structSqlite {
    char *zErrMsg;
    int rc;

} structSqlite;

// Returns true if get file from internet and download to file.
BOOL getXMLFromUrl(LPCTSTR szUrl, LPCTSTR szLocalFile, int withoutUTF8Bom);

// Returns bstr if read file from local disk.
BSTR readFileFromLocal(LPCTSTR szLocalFile);

// Returns true if read XML file from local file success.
BOOL readXMLFromLocal(char* szLocalFile);

// Returns true if read XML file from char buffer success.
BOOL readXMLFromBuffer(BSTR bstrXML);

// Returns true if XML is parse success.
BOOL readXMLParser(IXMLDOMDocument *pIFace);

// Returns true if read INI file from local dsik success.
BOOL readINIFileFromLocal(char* lpINIPath);

// Init main function and create dll resource.
int appInitDLL(void);

// Init main function struct for application.
void appInitStruct(void);

// Init main function for application controls.
void appInitControls(HWND hWnd);

// Ctrl group control for application on jobman.
void appCtrlGroupStatus(HWND hWnd, int srFlags);

// Released all resource from application created.
void appRelease(HWND hWnd);

// Fetch COM port and set ready init status.
void comInitLists(HWND hWnd, DWORD ctrlID);

// Init COM setting from INI file and user.
void comInitSetting(HWND hWnd, DWORD ctrlID);

// Clear and released COM port set to default.
void comClearLists(HWND hWnd, DWORD ctrlID);

// Jobman process task and add new job.
void jobmanProcessAddjob(structSMS smsFile);

// Jobman process task and send text message sms.
void jobmanProcessSendSMS(char* lpNumber, char* lpMsg);

// Jobman process task report result to server from user.
void jobmanProcessUploadResult(LPCTSTR szUrl);

// Jobman process task thread from web.
void jobmanProcessJobsByWeb(HWND hWnd, LPCTSTR szUrl);

// Jobman process task thread from file.
void jobmanProcessJobsByFile(HWND hWnd, char* szLocalFile);

// Output jobman task and display to listbox.
void jobOutput(HWND hWnd, DWORD ctrlID);

// Delete jobman list to listbox.
void jobDeleteByStr(HWND hWnd, DWORD ctrlID, char* lpStr);

// Output jobman log and display to listbox.
void logOutput(HWND hWnd, DWORD ctrlID, LPTSTR lpszFunction);

// Clear all logs from listbox and set to default.
void logClearAll(HWND hWnd, DWORD ctrlID);

// Timer thread for dispatch interface and process jobman.
void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);

// Timer thread for dispatch jobman task from exists job list.
void CALLBACK TimerTaskProc(HWND, UINT, UINT_PTR, DWORD);

#endif // _MAIN_H
