/*
   main.c
   Developer: Rchockxm (rchockxm.silver@gmail.com)
   Copyright: Silence Unlimited (rchockxm.com)
   Reference: Google C++ Style Guide

   This program is free software unless you got it under another license directly
   from the author. You can redistribute it and/or modify it under the terms of
   the GNU General Public License as published by the Free Software Foundation.
   Either version 2 of the License, or (at your option) any later version.
*/

#define WIN32_DEFAULT_LIBS
#define WIN32_LEAN_AND_MEAN
/* #define NOCRYPT */
/* #define NOSERVICE */
/* #define NOMCX */
/* #define NOIME */

#define COBJMACROS
#define MAXFLDSIZE 32
#define COMMOMEXDLL

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <tchar.h>
#include <stdio.h>
#include <shellApi.h>  
#include <ShlObj.h>  
#include <objbase.h>
#include <msxml2.h>
#include <time.h>
#include <Win/wininet.h>
#include "sqlite3.h"
#include "resource.h"
#include "main.h"
#include "ctrl.h"
#include "utf8.h"
#include "gsm.h"

#pragma lib "ole32.lib"
#pragma lib "oleaut32.lib"
#pragma lib "uuid.lib"
#pragma lib "shell32.lib"
#pragma lib "Wininet.lib"
#pragma lib "SQLite3.lib"

#define NELEMS(a) (sizeof(a) / sizeof((a)[0]))

// Prototypes
static INT_PTR CALLBACK MainDlgProc(HWND, UINT, WPARAM, LPARAM);
static LRESULT WINAPI ModemDlgProc(HWND, UINT, WPARAM, LPARAM);
static LRESULT WINAPI SMSUrlDlgProc(HWND, UINT, WPARAM, LPARAM);
static LRESULT WINAPI SMSCustomDlgProc(HWND, UINT, WPARAM, LPARAM);

// Global variables
static HANDLE ghInstance;
static HMODULE hCommonEx = NULL;

sqlite3 *db;

TCHAR szMutex[] = "Global\\{E21CE554-B691-4079-A036-8A45BC4EB462}";
HANDLE hMutex;

// Struct Jobman and set task to default.
structJobman minJobman;

// Struct INI file and load from sms.ini.
structINIFile iniFile;

// Struct INI file section.
structINISms iniSms;

// Struct XML file attrib.
structXMLAttrib xmlAttrib;

// Struct SMS file and load just-in-time.
structSMS smsFile[MAX_SMSLIMIT];

// Struct GSM status and set from local com port.
structGSMs gsmStatus;

// Struct sqlite database.
structSqlite sqliteDB;

// Returns a boolean of true, false if error.
// szUrl and szLocalFile is char based UTF-8 string and 
// without relation path.
BOOL getXMLFromUrl(LPCTSTR szUrl, LPCTSTR szLocalFile, int withoutUTF8Bom)
{
    HINTERNET hInet = NULL;  
    HINTERNET hUrl = NULL; 

    int iIndex = 0;

    char buffer[MAX_SMSLIMIT];

    DWORD bytesRead = 0;
    DWORD totalBytesRead = 0;

    FILE* pFile;

    hInet = InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0); 

    if (!hInet) 
    {
        return FALSE;
    }

    hUrl = InternetOpenUrl(hInet, szUrl, NULL, 0, INTERNET_FLAG_TRANSFER_BINARY|INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_RELOAD, 0);

    if (!hUrl) 
    {
        return FALSE;
    }

    pFile = fopen(szLocalFile, "wb");

    while (InternetReadFile(hUrl, buffer, 2048, &bytesRead) && bytesRead != 0)
    {
        buffer[bytesRead] = 0;

        if (pFile) {  
            if (iIndex == 0 && withoutUTF8Bom == UTF8_WITHOUT_BOM)
            {
                if (buffer[0] == 0XEF || buffer[1] == 0XBB || buffer[2] == 0XBF) 
                {                  
                    memmove(buffer, buffer+3, strlen(buffer));
                }
            }

            iIndex += 1;   

            fwrite(buffer, sizeof(char), bytesRead, pFile);
        }

        totalBytesRead += bytesRead;
    }

    fclose(pFile);

    InternetCloseHandle(hUrl);    
    InternetCloseHandle(hInet);

    return TRUE;
}

// Returns a string of bstr, null if error.
// szLocalFile is char based UTF-8 string and 
// with real path.
BSTR readFileFromLocal(LPCTSTR szLocalFile)
{
    BSTR bstrXML = NULL;

    char *strXML;
    long fileSize;
    size_t readSize;

    FILE *pFile = fopen(szLocalFile, "rb");
    
    if (pFile != NULL)
    {
        return L"";
    } 

    fseek(pFile, sizeof(wchar_t) * 3, SEEK_END);
    fileSize = ftell(pFile);
    rewind(pFile);

    strXML = (char*)malloc(fileSize * (sizeof(char)));
    readSize = fread(strXML, 1, fileSize, pFile);
    strXML[readSize] = 0;

    fclose(pFile); 

    if (strXML)
    {
        if (IsStrUTF8(strXML, fileSize)) 
        {
            bstrXML = wideStrToWideStr(strXML);
        }
        else 
        {
            bstrXML = ANSIToWideStr(strXML);
        }

        free(strXML);
    }

    return bstrXML;
}

// Returns a boolean of true, false if error.
// szLocalFile is char based ANSI string and 
// withut Unicode string.
BOOL readXMLFromLocal(char* szLocalFile) 
{
    HRESULT hr;
    CLSID clsid;

    IXMLDOMDocument *pIFace;
    VARIANT xmlSource;
    VARIANT_BOOL isSuccessful;

    if ((hr = CLSIDFromProgID(L"MSXML2.DOMDocument", &clsid)) != S_OK ||
        (hr = CoCreateInstance(&clsid, NULL, CLSCTX_ALL, &IID_IXMLDOMDocument, (void **)&pIFace)) != S_OK) 
    {
        return FALSE;
    }

    if (pIFace)
    {
        VariantInit(&xmlSource); 

        xmlSource.vt = VT_BSTR;
        xmlSource.bstrVal = ANSIToWideStr(szLocalFile);

        pIFace->lpVtbl->put_async(pIFace, VARIANT_FALSE);
        pIFace->lpVtbl->put_validateOnParse(pIFace, VARIANT_FALSE);
        hr = pIFace->lpVtbl->load(pIFace, xmlSource, &isSuccessful);

        if (hr == S_OK) 
        {
            readXMLParser(pIFace);
        }       

        pIFace->lpVtbl->Release(pIFace);

        VariantClear(&xmlSource);
    }    

    return TRUE;
}

// Returns a boolean of true, false if error.
// bstrXML is char based UTF-8 string and 
// withut Unicode string.
BOOL readXMLFromBuffer(BSTR bstrXML)
{
    HRESULT hr;
    CLSID clsid;

    IXMLDOMDocument *pIFace;
    VARIANT_BOOL isSuccessful;

    if ((hr = CLSIDFromProgID(L"MSXML2.DOMDocument", &clsid)) != S_OK ||
        (hr = CoCreateInstance(&clsid, NULL, CLSCTX_ALL, &IID_IXMLDOMDocument, (void **)&pIFace)) != S_OK) 
    {
        return FALSE;
    }

    if (pIFace)
    {
        pIFace->lpVtbl->put_async(pIFace, VARIANT_FALSE);
        pIFace->lpVtbl->put_validateOnParse(pIFace, VARIANT_FALSE);
        hr = pIFace->lpVtbl->loadXML(pIFace, bstrXML, &isSuccessful);

        if (hr == S_OK) 
        {
            readXMLParser(pIFace);
        }       

        pIFace->lpVtbl->Release(pIFace);
    }    

    return TRUE;
}

// Returns a boolean of true, false if error.
// pIFace is xml handle and define on  
// msxml2.h.
BOOL readXMLParser(IXMLDOMDocument *pIFace)
{
    HRESULT hr;

    IXMLDOMNodeList* pNodeList = NULL;
    IXMLDOMNode* Node;
    IXMLDOMNamedNodeMap* NodeMap;
    IXMLDOMNode* IDNode;

    long i;
    long Length;

    BOOL boolRet = FALSE;

    structSMS smsCFile;

    BSTR BStrMsg;
    BSTR BStrPhone;
    BSTR BStrID;
    BSTR BStrLoca;   

    if (pIFace)
    {
        hr = pIFace->lpVtbl->selectNodes(pIFace, L"//root/sms", &pNodeList);

        if (hr == S_OK) 
        {
            minJobman.index = 0;
            minJobman.lndPtr = 0;

            hr = pNodeList->lpVtbl->get_length(pNodeList, &Length);

            for (i=0; i<Length; i++)
            {
                if (pNodeList->lpVtbl->get_item(pNodeList, i, &Node) != S_OK || 
                    Node->lpVtbl->get_text(Node, &BStrMsg) != S_OK) 
                {
                    continue;
                }

                if (Node->lpVtbl->get_attributes(Node, &NodeMap) == S_OK) 
                {
                    if (NodeMap->lpVtbl->getNamedItem(NodeMap, xmlAttrib.lpPhone, &IDNode) == S_OK && 
                        IDNode->lpVtbl->get_text(IDNode, &BStrPhone) == S_OK) 
                    {

                        if (NodeMap->lpVtbl->getNamedItem(NodeMap, xmlAttrib.lpID, &IDNode) == S_OK) 
                        {
                            if (IDNode->lpVtbl->get_text(IDNode, &BStrID) != S_OK) 
                            {
                                continue;
                            }
                        }

                        if (NodeMap->lpVtbl->getNamedItem(NodeMap, xmlAttrib.lpLoca, &IDNode) == S_OK) 
                        {
                            if (IDNode->lpVtbl->get_text(IDNode, &BStrLoca) != S_OK) 
                            {
                                continue;
                            }
                        }

                        smsCFile.lpID = wideStrToANSI(BStrID);
                        smsCFile.lpNumber = wideStrToANSI(BStrPhone);
                        smsCFile.lpMsg = wideStrToANSI(BStrMsg);
                        smsCFile.lpLoca = wideStrToANSI(BStrLoca);
                        smsCFile.dwStatus = 0;
                        smsCFile.dwRetry = 0;

                        jobmanProcessAddjob(smsCFile);

                        SysFreeString(BStrLoca);
                        SysFreeString(BStrMsg);
                        SysFreeString(BStrPhone);
                        SysFreeString(BStrID);

                        IDNode->lpVtbl->Release(IDNode);
                    }

                    NodeMap->lpVtbl->Release(NodeMap);
                }
            }

            boolRet = TRUE;
        }
    }

    return boolRet;
}

// Returns a boolean of true, false if error.
// lpINIPath is char based ANSI string and 
// with real path.
BOOL readINIFileFromLocal(char* lpINIPath)
{
    char szPath[MAX_PATH];
    char szPort[MAX_PATH];
    char szFetchUrl[MAX_PATH];
    char szReportUrl[MAX_PATH];
    char szAutoSMS[MAX_PATH];
    char szLogOutput[MAX_PATH];

    SearchPath(NULL, ".", NULL, sizeof(szPath), szPath, NULL);
    lstrcat(szPath, lpINIPath);

    if (GetPrivateProfileString(iniFile.sms, "Port", "", szPort, sizeof(szPort), szPath)) 
    {
        if (strlen(szPort) > 0)
        {
            lstrcpy((char*)iniSms.port, szPort);
        }        
    }

    if (GetPrivateProfileString(iniFile.sms, "FetchUrl", "", szFetchUrl, sizeof(szFetchUrl), szPath)) 
    {
        if (strlen(szFetchUrl) > 0)
        {
            lstrcpy((char*)iniSms.fetchUrl, szFetchUrl);
        }
    } 

    if (GetPrivateProfileString(iniFile.sms, "ReportUrl", "", szReportUrl, sizeof(szReportUrl), szPath)) 
    {
        if (strlen(szReportUrl) > 0)
        {
            lstrcpy((char*)iniSms.reportUrl, szReportUrl);
        }
    } 

    if (GetPrivateProfileString(iniFile.sms, "AutoSMS", "", szAutoSMS, sizeof(szAutoSMS), szPath)) 
    {
        if (strlen(szAutoSMS) > 0)
        {
            lstrcpy((char*)iniSms.autoSMS, szAutoSMS);
        }
    } 

    if (GetPrivateProfileString(iniFile.sms, "LogOutput", "", szLogOutput, sizeof(szLogOutput), szPath)) 
    {
        if (strlen(szLogOutput) > 0)
        {
            lstrcpy((char*)iniSms.logOutput, szLogOutput);
        }
    } 

    return TRUE;
}

// Init application dll resource from custom.
// This function didn't needs parameter
// and return flags if success.
int appInitDLL(void)
{
    HRESULT hr;

    hCommonEx = LoadLibrary(DLL_COMMOMEX);

    if (!hCommonEx) 
    {
        return DLL_ERROR_COMMONEX;
    }

    if ((hr = CoInitializeEx(NULL, COINIT_MULTITHREADED)) != S_OK) 
    {
        return DLL_ERROR_COMMONEX;
    } 

    sqliteDB.rc = sqlite3_open(SQLITE_DB, &db);

    if (sqliteDB.rc != SQLITE_OK)
    {
        sqlite3_close(db);

        return DLL_ERROR_SQLITE;
    }

    return DLL_RESULT_SUCCESS;
}

// Init application struct for windows load.
// This function didn't needs parameter
// and return void.
void appInitStruct(void)
{
    minJobman.index = 0;
    minJobman.lndPtr = 0;
    minJobman.logPtr = 0;
    minJobman.maxLimit = MAX_SMSLIMIT;
    minJobman.timerSec = FALSE;
    minJobman.timerTask = FALSE;

    lstrcat(minJobman.xmlPath, "jobman/temp/sms.xml");
    lstrcat(minJobman.cachePath, "jobman/temp/sms.cache");

    lstrcat(iniFile.lzPath, "\\sms.ini");
    lstrcpy(iniFile.sms, "SMS");

    xmlAttrib.lpID = L"id";
    xmlAttrib.lpPhone = L"phone";
    xmlAttrib.lpLoca = L"loca";
}

// Init application struct for windows load.
// hWnd is WindowsAPI handle and
// define on windows.h.
void appInitControls(HWND hWnd)
{
    int dwParts[3] = {100, 280, -1};
    int dwPartSize = sizeof(dwParts) / sizeof(dwParts[0]);

    EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMCLOSE), FALSE);
    EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMMODEM), FALSE);
    EnableWindow(GetDlgItem(hWnd, IDC_CHECKBOX_AUTO), FALSE);
    EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDWEB), FALSE);
    EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDFILE), FALSE);
    EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDCUSTOM), FALSE);
    EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMRESUME), FALSE);
    EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMPAUSE), FALSE);
    EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSTOP), FALSE);

    statusSetParts(hWnd, IDC_STATUSBAR, dwPartSize, dwParts);    
}

// Group application control for jobman.
// hWnd is WindowsAPI handle and
// define on windows.h.
void appCtrlGroupStatus(HWND hWnd, int srFlags)
{
    if (srFlags == JOBMAN_CTRL_INIT)
    {
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMCLOSE), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMMODEM), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDWEB), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDFILE), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDCUSTOM), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMPAUSE), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSTOP), TRUE);
    }

    if (srFlags == JOBMAN_CTRL_COMOPEN)
    {
        EnableWindow(GetDlgItem(hWnd, IDC_COMBOBOX_COM), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_CHECKBOX_AUTO), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMOPEN), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMCLOSE), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMMODEM), TRUE);        
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDWEB), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDFILE), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDCUSTOM), TRUE);
    }

    if (srFlags == JOBMAN_CTRL_COMCLOSE)
    {
        EnableWindow(GetDlgItem(hWnd, IDC_COMBOBOX_COM), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_CHECKBOX_AUTO), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMOPEN), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMCLOSE), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMMODEM), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDWEB), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDFILE), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDCUSTOM), FALSE);
    }

    if (srFlags == JOBMAN_CTRL_UNINIT)
    {
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMCLOSE), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMMODEM), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDWEB), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDFILE), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDCUSTOM), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMRESUME), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMPAUSE), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSTOP), FALSE);
    }  
}

// Init application struct for windows load.
// hWnd is WindowsAPI handle and
// define on windows.h.
void appRelease(HWND hWnd)
{
    if (!minJobman.timerSec)
    {
        KillTimer(hWnd, ID_TIMER1);

        minJobman.timerSec = FALSE;
    }

    if (!minJobman.timerTask)
    {
        KillTimer(hWnd, ID_TIMER2);

        minJobman.timerTask = FALSE;
    }

    if (hCommonEx)
    {     
        if (gsmStatus.cmInit)
        {
            gsmModemUnInit(hCommonEx);
        } 

        comClearLists(hWnd, IDC_COMBOBOX_COM);

        FreeLibrary(hCommonEx); 
    }

    if (db)
    {
        sqlite3_close(db);
    }
}

// Init COM port list to listbox.
// hWnd and ctrlID is WindowsAPI handle and
// controlID with numeric.
void comInitLists(HWND hWnd, DWORD ctrlID)
{
    HWND hCombobox;

    HKEY hKey;

    DWORD dwType = 0;
    DWORD dwValName;
    DWORD dwData;
    TCHAR szValueName[30];
    TCHAR szData[1024];
    TCHAR szCOMPrefix[] = "COM";
    BOOL isFind = FALSE;
    
    int i = 0;
    int index = 0;
    int dwStrSize;
    long ilReturn;

    hCombobox = GetDlgItem(hWnd, ctrlID);
    
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Ports", 0, KEY_READ, &hKey)== ERROR_SUCCESS);
    {
        while (!isFind)  
        {
            dwData = sizeof(szData);
            dwValName = sizeof(szValueName);

            ilReturn = RegEnumValue(hKey, i, szValueName, &dwValName, NULL, &dwType, (UCHAR*)szData, &dwData);

            if (strstr(szValueName, szCOMPrefix) != NULL)
            {
                dwStrSize = strlen(szValueName) - 1;

                memmove(&szValueName[dwStrSize] , &szValueName[dwStrSize+1], strlen(szValueName) - dwStrSize);    

                //ComboBox_InsertString(hCombobox, index, szValueName);
                ComboBox_AddString(hCombobox, szValueName);
                index += 1;
            }

            lstrcpy(szValueName, "");
            lstrcpy(szData, "");

            if (ilReturn == ERROR_NO_MORE_ITEMS)
            {
                isFind = TRUE;

                break;
            }

            i += 1;
        }

    }

    RegCloseKey(hKey);

    statusSetText(hWnd, IDC_STATUSBAR, STATUS_COM_LISTINIT, 1); 
}

// Init COM port setting from INI file.
// hWnd and ctrlID is WindowsAPI handle and
// controlID with numeric.
void comInitSetting(HWND hWnd, DWORD ctrlID)
{
    HWND hCombobox;

    char* lpPort;
    char* lpAutoSMS;
    char* lpLogOutput;

    int findIndex;

    hCombobox = GetDlgItem(hWnd, ctrlID);
    lpPort = iniSms.port;
    lpAutoSMS = iniSms.autoSMS;
    lpLogOutput = iniSms.logOutput;

    if (strlen(lpPort) > 0)
    {
        gsmStatus.cmInit = gsmModemInit(hCommonEx, lpPort);

        if (gsmStatus.cmInit) 
        {
            lstrcpy(minJobman.lpPort, lpPort);

            findIndex = ComboBox_FindString(hCombobox, -1, lpPort);

            if (findIndex)
            {
                ComboBox_SetCurSel(hCombobox, findIndex);
                appCtrlGroupStatus(hWnd, JOBMAN_CTRL_COMOPEN);

                if (strlen(lpAutoSMS) > 0)
                {
                    if (strcmp(lpAutoSMS, "true") == 0)
                    {
                        gsmStatus.olAutomatic = TRUE;

                        SendDlgItemMessage(hWnd, IDC_CHECKBOX_AUTO, BM_SETCHECK, BST_CHECKED, 0);
                    }    
                }

                if (strlen(lpLogOutput) > 0)
                {
                    if (strcmp(lpLogOutput, "true") == 0)
                    {
                        gsmStatus.olDisplayLog = TRUE;

                        SendDlgItemMessage(hWnd, IDC_CHECKBOX_LOG, BM_SETCHECK, BST_CHECKED, 0);
                    }  
                }

                statusSetText(hWnd, IDC_STATUSBAR, minJobman.lpPort, 0); 
                statusSetText(hWnd, IDC_STATUSBAR, STATUS_COM_INIT, 1); 
            }
        }
        else 
        {
            MessageBox(0, MSG_ERR_PORT_INIT, MSG_ERR_TITLE, MB_OK);
        }
    }
}

// Released COM port and set to default.
// hWnd and ctrlID is WindowsAPI handle and
// controlID with numeric.
void comClearLists(HWND hWnd, DWORD ctrlID)
{
    gsmStatus.cmInit = FALSE;
    gsmStatus.cmBusy = FALSE;
    gsmStatus.cmSending = FALSE;
    gsmStatus.cmResult = FALSE;
    gsmStatus.olAutomatic = FALSE;
    gsmStatus.olAutoFetching = FALSE;
    gsmStatus.olDisplayLog = FALSE;

    SendDlgItemMessage(hWnd, ctrlID, CB_RESETCONTENT, 0, 0);
}

// Jobman add new job to task list.
// smsFile is structSMS and
// define on main.h.
void jobmanProcessAddjob(structSMS smsCFile)
{
    if (minJobman.index < minJobman.maxLimit) 
    {
        smsFile[minJobman.index].lpID = smsCFile.lpID;
        smsFile[minJobman.index].lpNumber = smsCFile.lpNumber;
        smsFile[minJobman.index].lpMsg = smsCFile.lpMsg;
        smsFile[minJobman.index].lpLoca = smsCFile.lpLoca;
        smsFile[minJobman.index].dwStatus = smsCFile.dwStatus;
        smsFile[minJobman.index].dwRetry = smsCFile.dwRetry;

        if (sizeof(iniSms.reportUrl) > 0)
        {
            lstrcpy((char*)smsFile[minJobman.index].rtUrl, iniSms.reportUrl);
            lstrcat((char*)smsFile[minJobman.index].rtUrl, "?action=uploadxmlresult&id=");
            lstrcat((char*)smsFile[minJobman.index].rtUrl, (const char*)smsCFile.lpID);
            lstrcat((char*)smsFile[minJobman.index].rtUrl, "&");
        }

        minJobman.index += 1;
    }
}

// Jobman process task and send sms message.
// lpNumber with lpMsg is char based ANSI string and
// without UTF-8 string.
void jobmanProcessSendSMS(char* lpNumber, char* lpMsg)
{    
    if (gsmStatus.cmInit)
    {
        gsmStatus.cmResult = gsmModemSendSMSByText(hCommonEx, lpNumber, lpMsg);
    }
}

// Jobman process task report to server.
// szUrl contain real url path and
// url parameter.
void jobmanProcessUploadResult(LPCTSTR szUrl)
{
    LPCTSTR szLocalFile;
    BOOL boolRet;

    szLocalFile = minJobman.cachePath;

    if (gsmStatus.cmInit)
    {
        if (strlen(szUrl) > 0)
        {
            boolRet = getXMLFromUrl(szUrl, szLocalFile, UTF8_WITHOUT_BOM);
        }
    }
}

// Jobman process task from web server.
// hWnd is WindowsAPI handle and szUrl contain
// real url path.
void jobmanProcessJobsByWeb(HWND hWnd, LPCTSTR szUrl)
{
    /*BSTR szXML;
    LPCTSTR szLocalFile;
    BOOL boolRet;

    szLocalFile = minJobman.xmlPath;
    boolRet = getXMLFromUrl(szUrl, szLocalFile, UTF8_WITHOUT_BOM);
    szXML = readFileFromLocal(szLocalFile);
    boolRet = readXMLFromBuffer(szXML);
    SysFreeString(szXML);*/

    BOOL boolRet;

    boolRet = getXMLFromUrl(szUrl, minJobman.xmlPath, UTF8_WITH_BOM);
    boolRet = readXMLFromLocal(minJobman.xmlPath);

    jobOutput(hWnd, DLG_LISTBOX_TASK);

    if (minJobman.index > 0)
    {
        statusSetText(hWnd, IDC_STATUSBAR, STATUS_JOBMAN_INIT, 1); 

        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDWEB), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDFILE), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDCUSTOM), FALSE);

        if (!minJobman.timerTask)
        {
            gsmStatus.cmResult = FALSE;
            minJobman.timerTask = TRUE;

            SetTimer(hWnd, ID_TIMER2, ID_TIMER2_TIME, TimerTaskProc);  
        }     
    }
    else 
    {
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDWEB), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDFILE), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDCUSTOM), TRUE);
    }
}

// Jobman process task from local files.
// hWnd is WindowsAPI handle and szLocalFile contain
// real disk path.
void jobmanProcessJobsByFile(HWND hWnd, char* szLocalFile)
{
    /*BSTR szXML;    
    BOOL boolRet;

    szXML = readFileFromLocal(szLocalFile);
    boolRet = readXMLFromBuffer(szXML);
    SysFreeString(szXML);*/

    BOOL boolRet;

    boolRet = readXMLFromLocal(szLocalFile);

    jobOutput(hWnd, DLG_LISTBOX_TASK);

    if (minJobman.index > 0)
    {
        statusSetText(hWnd, IDC_STATUSBAR, STATUS_JOBMAN_INIT, UTF8_WITHOUT_BOM); 

        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDWEB), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDFILE), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDCUSTOM), FALSE);

        if (!minJobman.timerTask)
        {
            gsmStatus.cmResult = FALSE;
            minJobman.timerTask = TRUE;

            SetTimer(hWnd, ID_TIMER2, ID_TIMER2_TIME, TimerTaskProc);
        }
    }
    else 
    {
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDWEB), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDFILE), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_BTN_COMSENDCUSTOM), TRUE);
    }
}

// Activated jobs list.
// hWnd, ctrlID is WindowsAPI handle and lpszFunction contain
// string.
void jobOutput(HWND hWnd, DWORD ctrlID)
{
    HWND hListbox;

    int i;
    char taskName[120];

    hListbox = GetDlgItem(hWnd, ctrlID);
    ListBox_ResetContent(hListbox);

    if (minJobman.index > 0)
    {
        for (i=0; i<minJobman.index; i++)
        {        
            lstrcpy(taskName, JOBMAN_TASK_PREFIX);     
            lstrcat(taskName, smsFile[i].lpID);            
            ListBox_InsertString(hListbox, i, taskName);
        }
    }
}

// Delete jobs list item.
// hWnd, ctrlID is WindowsAPI handle and lpszFunction contain
// string.
void jobDeleteByStr(HWND hWnd, DWORD ctrlID, char* lpStr)
{
    HWND hListbox;

    int dwFind;

    hListbox = GetDlgItem(hWnd, ctrlID);
    dwFind = ListBox_FindString(hListbox, 0, lpStr);
    ListBox_DeleteString(hListbox, dwFind);
}

// Logged and output jobman information.
// hWnd, ctrlID is WindowsAPI handle and lpszFunction contain
// string.
void logOutput(HWND hWnd, DWORD ctrlID, LPTSTR lpszFunction)
{
    HWND hListbox;

    time_t now;

    struct tm *t;
    char dateTime[40]; 

    hListbox = GetDlgItem(hWnd, ctrlID);
    now = time(NULL);
    t = localtime(&now);      

    if (gsmStatus.olDisplayLog)
    {
        strftime(dateTime, sizeof(dateTime), "%Y-%m-%d %X ", t);
        lstrcat(dateTime, lpszFunction);

        ListBox_InsertString(hListbox, minJobman.logPtr, dateTime);
        minJobman.logPtr += 1;
    }
}

// Clear logged information and set to default.
// hWnd and ctrlID is WindowsAPI handle and
// controlID with numeric.
void logClearAll(HWND hWnd, DWORD ctrlID)
{
    listboxClearAll(hWnd, ctrlID);
    minJobman.logPtr = 0;
}

// Main entrypoint and create dialogform.
// hInstance, hPrevInstance, lpszCmdLine and 
// nCmdShow is define on WindowsAPI.
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    INITCOMMONCONTROLSEX icc;
    WNDCLASSEX wcx;

    ghInstance = hInstance;

    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_WIN95_CLASSES /*|ICC_COOL_CLASSES|ICC_DATE_CLASSES|ICC_PAGESCROLLER_CLASS|ICC_USEREX_CLASSES|... */;
    InitCommonControlsEx(&icc);

    /*
     * TODO: uncomment line below, if you are using the Network Address control (Windows Vista+).
     */
    // InitNetworkAddressControl();

    /* Get system dialog information */
    wcx.cbSize = sizeof(wcx);

    if (!GetClassInfoEx(NULL, MAKEINTRESOURCE(32770), &wcx))
    {
        MessageBox(0, MSG_ERR_DLGCLS_CREATE, MSG_ERR_TITLE, MB_OK);

        return 0;
    }

    wcx.hInstance = hInstance;
    wcx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDR_ICO_MAIN));
    wcx.lpszClassName = _T(DLG_CLASS);

    if (!RegisterClassEx(&wcx))
    {
        MessageBox(0, MSG_ERR_DLG_CREATE, MSG_ERR_TITLE, MB_OK);

        return 0;
    }

    return DialogBox(hInstance, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)MainDlgProc);
}

// Main callback function on Windows event.
// hwndDlg, uMsg, wParam and lParam
// is define on WindowsAPI.
static INT_PTR CALLBACK MainDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hCombobox;

    OPENFILENAME ofn = {0}; 

    int curSel;
    int result;    
    
    BOOL statusCtrl;

    char lpszBuffer[20];
    char lpszFiles[MAX_PATH + 1] = {0}; 

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hMutex = OpenMutex(SYNCHRONIZE, 0, szMutex);

            if (hMutex) {
                MessageBox(0, MSG_ERR_OPENINSTANCE, MSG_ERR_TITLE, MB_OK);
                EndDialog(hwndDlg, TRUE);

                return TRUE;
            }

            hMutex = CreateMutex(0, 0, szMutex);
            result = appInitDLL();

            if (result != DLL_RESULT_SUCCESS)
            {
                if (result == DLL_ERROR_COMMONEX)
                {
                    MessageBox(0, MSG_ERR_DLL_LOAD, MSG_ERR_TITLE, MB_OK);
                }
        
                if (result == DLL_ERROR_COM)
                {
                    MessageBox(0, MSG_ERR_COM_REGISTER, MSG_ERR_TITLE, MB_OK);
                }

                if (result == DLL_ERROR_SQLITE)
                {
                    MessageBox(0, MSG_ERR_SQLITE_OPENDB, MSG_ERR_TITLE, MB_OK);
                }

                EndDialog(hwndDlg, TRUE);

                return TRUE;
            }

            appInitStruct();
            appInitControls(hwndDlg);

            readINIFileFromLocal(iniFile.lzPath);
            comInitLists(hwndDlg, IDC_COMBOBOX_COM);
            comInitSetting(hwndDlg, IDC_COMBOBOX_COM);

            if (!minJobman.timerSec)
            {
                statusSetText(hwndDlg, IDC_STATUSBAR, STATUS_JOBMAN_READY, 1); 

                minJobman.timerSec = TRUE;
                SetTimer(hwndDlg, ID_TIMER1, ID_TIMER1_TIME, TimerProc);
            }

            return TRUE;

        case WM_SIZE:
            return TRUE;

        case WM_NOTIFY:
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDC_BTN_COMOPEN:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        hCombobox = GetDlgItem(hwndDlg, IDC_COMBOBOX_COM);
                        curSel = ComboBox_GetCurSel(hCombobox);      

                        if (curSel >= 0)
                        {
                            ComboBox_GetLBText(hCombobox, curSel, lpszBuffer);

                            gsmStatus.cmInit = gsmModemInit(hCommonEx, lpszBuffer);
                        
                            if (gsmStatus.cmInit) 
                            {
                                lstrcpy(minJobman.lpPort, lpszBuffer);

                                appCtrlGroupStatus(hwndDlg, JOBMAN_CTRL_COMOPEN);
                                statusSetText(hwndDlg, IDC_STATUSBAR, lpszBuffer, 0); 
                                statusSetText(hwndDlg, IDC_STATUSBAR, STATUS_COM_INIT, 1); 
                            }
                            else 
                            {
                                MessageBox(0, MSG_ERR_PORT_INIT, MSG_ERR_TITLE, MB_OK);
                            }
                        }
                        else 
                        {
                            statusSetText(hwndDlg, IDC_STATUSBAR, STATUS_ERR_SELECT, 1); 
                        }
                    }

                    break; 

                case IDC_BTN_COMCLOSE:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        gsmModemUnInit(hCommonEx);

                        if (gsmStatus.cmInit) 
                        {
                            if (!gsmStatus.cmBusy)
                            {
                                lstrcpy(minJobman.lpPort, "");

                                appCtrlGroupStatus(hwndDlg, JOBMAN_CTRL_COMCLOSE);
                                statusSetText(hwndDlg, IDC_STATUSBAR, "N/A", 0);
                                statusSetText(hwndDlg, IDC_STATUSBAR, STATUS_COM_UNINIT, 1); 
                            }
                            else
                            {
                                MessageBox(0, MSG_ERR_PORT_BUSY, MSG_ERR_TITLE, MB_OK);
                            }
                        }   
                        else 
                        {
                            statusSetText(hwndDlg, IDC_STATUSBAR, STATUS_ERR_COM_UNINIT, 1); 
                        }                  
                    }

                    break; 

                case IDC_BTN_COMMODEM:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        if (gsmStatus.cmInit)
                        {
                            if (!gsmStatus.cmBusy)
                            {                        
                                DialogBox(ghInstance, MAKEINTRESOURCE(DLG_MODEM), hwndDlg, (DLGPROC)ModemDlgProc);
                            }
                            else
                            {
                                MessageBox(0, MSG_ERR_PORT_BUSY, MSG_ERR_TITLE, MB_OK);
                            }
                        }
                        else
                        {
                            MessageBox(0, MSG_ERR_PORT_INIT, MSG_ERR_TITLE, MB_OK);
                        }
                    }

                    break;

                case IDC_BTN_COMSENDWEB:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        if (gsmStatus.cmInit)
                        {
                            if (!gsmStatus.cmBusy)
                            {
                                DialogBox(ghInstance, MAKEINTRESOURCE(DLG_SMSURL), hwndDlg, (DLGPROC)SMSUrlDlgProc);

                                if (strlen(minJobman.tempFetchUrl) > 0) 
                                {
                                    gsmStatus.olAutoFetching = TRUE;
                                    jobmanProcessJobsByWeb(hwndDlg, minJobman.tempFetchUrl);
                                }
                            }
                            else
                            {
                                MessageBox(0, MSG_ERR_PORT_BUSY, MSG_ERR_TITLE, MB_OK);
                            }
                        }
                        else
                        {
                            MessageBox(0, MSG_ERR_PORT_INIT, MSG_ERR_TITLE, MB_OK);
                        }
                    }

                    break;

                case IDC_BTN_COMSENDFILE:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        if (gsmStatus.cmInit)
                        {
                            if (!gsmStatus.cmBusy)
                            {
                                ofn.lStructSize = sizeof(OPENFILENAME);
                                ofn.hInstance = ghInstance;
                                ofn.hwndOwner = hwndDlg;
                                ofn.lpstrFile = lpszFiles;
                                ofn.nMaxFile = MAX_PATH;
                                ofn.lpstrTitle = DLG_FILE_OPEN;
                                ofn.nFilterIndex= 1;
                                ofn.lpstrFilter = "XML\0*.xml\0\0";
                                ofn.Flags = OFN_NOVALIDATE;

                                if (GetOpenFileName(&ofn))
                                {
                                    jobmanProcessJobsByFile(hwndDlg, lpszFiles);
                                }
                                else 
                                {
                                    statusSetText(hwndDlg, IDC_STATUSBAR, STATUS_ERR_SELECT_PARH, 1); 
                                }
                            }
                            else
                            {
                                MessageBox(0, MSG_ERR_PORT_BUSY, MSG_ERR_TITLE, MB_OK);
                            }
                        }
                        else
                        {
                            MessageBox(0, MSG_ERR_PORT_INIT, MSG_ERR_TITLE, MB_OK);
                        }
                    }

                    break;

                case IDC_BTN_COMSENDCUSTOM:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        if (gsmStatus.cmInit)
                        {
                            if (!gsmStatus.cmBusy)
                            {
                                DialogBox(ghInstance, MAKEINTRESOURCE(DLG_SMSCUSTOM), hwndDlg, (DLGPROC)SMSCustomDlgProc);
                            }
                            else 
                            {
                                MessageBox(0, MSG_ERR_PORT_BUSY, MSG_ERR_TITLE, MB_OK);
                            }
                        }
                        else
                        {
                            MessageBox(0, MSG_ERR_PORT_INIT, MSG_ERR_TITLE, MB_OK);
                        }
                    }

                    break;

                case IDC_BTN_COMRESUME:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        if (gsmStatus.cmInit)
                        {
                            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_COMRESUME), FALSE);
                            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_COMPAUSE), TRUE);
                            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_COMSTOP), TRUE);

                            statusSetText(hwndDlg, IDC_STATUSBAR, STATUS_JOBMAN_RESUME, 1); 
                        
                            gsmStatus.cmBusy = FALSE;
                        }
                        else
                        {
                            MessageBox(0, MSG_ERR_PORT_INIT, MSG_ERR_TITLE, MB_OK);
                        }
                    }

                    break;

                case IDC_BTN_COMPAUSE:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        if (gsmStatus.cmInit)
                        {
                            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_COMRESUME), TRUE);
                            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_COMPAUSE), FALSE);
                            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_COMSTOP), TRUE);

                            statusSetText(hwndDlg, IDC_STATUSBAR, STATUS_JOBMAN_PAUSE, 1); 
                        
                            gsmStatus.cmBusy = TRUE;
                        }
                        else
                        {
                            MessageBox(0, MSG_ERR_PORT_INIT, MSG_ERR_TITLE, MB_OK);
                        }
                    }

                    break;
                
                case IDC_BTN_COMSTOP:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        if (gsmStatus.cmInit)
                        {
                            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_COMRESUME), FALSE);
                            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_COMPAUSE), FALSE);
                            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_COMSTOP), FALSE);

                            statusSetText(hwndDlg, IDC_STATUSBAR, STATUS_JOBMAN_STOP, 1); 

                            gsmStatus.cmBusy = TRUE;
                            minJobman.lndPtr = minJobman.index;
                        }
                        else
                        {
                            MessageBox(0, MSG_ERR_PORT_INIT, MSG_ERR_TITLE, MB_OK);
                        }
                    }

                    break;

                case IDC_BTN_LOGCLEAR:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        logClearAll(hwndDlg, IDC_LISTBOX_LOG);

                        MessageBox(0, MSG_SUCCESS_LOG_CLEAR, MSG_SUCCESS_TITLE, MB_OK);
                    }

                    break;

                case IDC_CHECKBOX_AUTO:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        if (SendDlgItemMessage(hwndDlg, IDC_CHECKBOX_AUTO, BM_GETCHECK, 0, 0) == BST_CHECKED) 
                        {
                            gsmStatus.olAutomatic = TRUE;
                        }
                        else 
                        {
                            gsmStatus.olAutomatic = FALSE;
                        }
                    }

                    break;

                case IDC_CHECKBOX_LOG:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        if (SendDlgItemMessage(hwndDlg, IDC_CHECKBOX_LOG, BM_GETCHECK, 0, 0) == BST_CHECKED) 
                        {
                            gsmStatus.olDisplayLog = TRUE;
                        }
                        else 
                        {
                            gsmStatus.olDisplayLog = FALSE;
                        }
                    }

                    break;

                case IDC_BTN_ABOUT:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        MessageBox(hwndDlg, MSG_ABOUT_CONTENT, MSG_ABOUT_TITLE, MB_OK);
                    }

                    break;
 
                case IDC_BTN_EXIT:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        statusCtrl = TRUE;

                        if (gsmStatus.cmBusy || gsmStatus.cmSending)
                        {
                            if (MessageBox(hwndDlg, MSG_EXIT_WITH_JOBMAN, MSG_EXIT_TITLE, MB_OKCANCEL) == IDCANCEL) 
                            {
                                statusCtrl = FALSE;
                            }
                        }
                        
                        if (statusCtrl)
                        {
                            EndDialog(hwndDlg, TRUE);

                            return TRUE;
                        }
                    }

                    break;
            }

            break;

        case WM_CLOSE: 
            statusCtrl = TRUE;

            if (gsmStatus.cmBusy || gsmStatus.cmSending)
            {
                if (MessageBox(hwndDlg, MSG_EXIT_WITH_JOBMAN, MSG_EXIT_TITLE, MB_OKCANCEL) == IDCANCEL) 
                {
                    statusCtrl = FALSE;
                }
            }

            if (statusCtrl)
            {
                appRelease(hwndDlg);

                CoUninitialize();           
                EndDialog(hwndDlg, 0);

                if (hMutex)
                {
                    ReleaseMutex(hMutex);
                    CloseHandle(hMutex);
                }

                hCommonEx = NULL;
                ghInstance = NULL;

                return TRUE;
            }
    }

    return FALSE;
}

// Custom at-command callback function on Windows event.
// hwndDlg, uMsg, wParam and lParam
// is define on WindowsAPI.
static LRESULT CALLBACK ModemDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    char* szText;
    char szCmd[256];
    char szAns[128];

    int dwSize;

    switch (uMsg)
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            switch (wParam)
            {
                case IDCC_BTN_ATCLEAR:
                    listboxClearAll(hDlg, IDCC_LISTBOX_ATR);

                    break;

                case IDOK:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {  
                        szText = editGetText(hDlg, IDCC_EDIT_AT);

                        if (strlen(szText) > 0)
                        {
                            lstrcpy(szCmd, szText);
                            lstrcat(szCmd, "\r");                      

                            dwSize = sizeof(szCmd) / sizeof(szCmd[0]);

                            if (dwSize > 0) 
                            {
                                gsmModemWrite(hCommonEx, szCmd, dwSize);
                                gsmModemRead(hCommonEx, szAns, 128);

                                dwSize = sizeof(szAns) / sizeof(szAns[0]);

                                if (dwSize > 0) 
                                {
                                    listboxInsertText(hDlg, IDCC_LISTBOX_ATR, szAns);
                                }
                            }
                        }

                        free(szText);
                    }

                    break;

                case IDCANCEL:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        EndDialog(hDlg, TRUE);

                        return TRUE;
                    }

                    break;
            }

            break;
    }

    return FALSE;
}

// Custom sms-url callback function on Windows event.
// hwndDlg, uMsg, wParam and lParam
// is define on WindowsAPI.
static LRESULT CALLBACK SMSUrlDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    char* szText;

    switch (uMsg)
    {
        case WM_INITDIALOG:
            editSetText(hDlg, IDCC_EDIT_URL, iniSms.fetchUrl);

            EnableWindow(GetDlgItem(hDlg, IDCC_EDIT_URL), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);

            return TRUE;

        case WM_COMMAND:
            switch (wParam)
            {
                case IDOK:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {                     
                        szText = editGetText(hDlg, IDCC_EDIT_URL);

                        if (strlen(szText) > 0)
                        {
                            EnableWindow(GetDlgItem(hDlg, IDCC_EDIT_URL), FALSE);
                            EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);

                            lstrcpy(minJobman.tempFetchUrl, szText);
                        }

                        free(szText);

                        EndDialog(hDlg, TRUE);

                        return TRUE;
                    }

                    break;

                case IDCANCEL:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        EndDialog(hDlg, TRUE);

                        return TRUE;
                    }

                    break;
            }

            break;
    }

    return FALSE;
}

// Custom sms-send callback function on Windows event.
// hwndDlg, uMsg, wParam and lParam
// is define on WindowsAPI.
static LRESULT CALLBACK SMSCustomDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    char* szPhone;
    char* szMsg;

    switch (uMsg)
    {
        case WM_INITDIALOG:
            EnableWindow(GetDlgItem(hDlg, IDCC_EDIT_PHONE), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDCC_EDIT_MSG), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);

            return TRUE;

        case WM_COMMAND:
            switch (wParam)
            {
                case IDOK:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        szPhone = editGetText(hDlg, IDCC_EDIT_PHONE);
                        szMsg = editGetText(hDlg, IDCC_EDIT_MSG);

                        if (strlen(szPhone) > 0 && strlen(szMsg) > 0)
                        {
                            EnableWindow(GetDlgItem(hDlg, IDCC_EDIT_PHONE), FALSE);
                            EnableWindow(GetDlgItem(hDlg, IDCC_EDIT_MSG), FALSE);
                            EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);

                            gsmStatus.cmBusy = TRUE;
                            gsmStatus.cmResult = FALSE;

                            jobmanProcessSendSMS(szPhone, szMsg);

                            if (gsmStatus.cmResult)
                            {
                                MessageBox(0, MSG_SUCCESS_SMS_SEND, MSG_SUCCESS_TITLE, MB_OK);
                            }
                            else
                            {
                                MessageBox(0, MSG_ERR_SMS_SEND_FAIL, MSG_ERR_TITLE, MB_OK);
                            }

                            gsmStatus.cmBusy = FALSE;
                            gsmStatus.cmResult = FALSE;
                        }
                        else 
                        {
                            MessageBox(0, MSG_ERR_PHONE_MSG, MSG_ERR_TITLE, MB_OK);
                        }

                        free(szMsg);
                        free(szPhone);

                        EndDialog(hDlg, TRUE);

                        return TRUE;
                    }

                    break;

                case IDCANCEL:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        EndDialog(hDlg, TRUE);

                        return TRUE;
                    }

                    break;
            }

            break;
    }

    return FALSE;
}

// Timer callback function on Jobman.
// hWnd, uMsg, idEvent and dwTime
// is define on WindowsAPI.
void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    if (gsmStatus.cmInit) 
    {
        if (!gsmStatus.cmBusy && gsmStatus.olAutomatic)
        {
            if (!gsmStatus.olAutoFetching)
            {
                statusSetText(hWnd, IDC_STATUSBAR, STATUS_JOBMAN_FETCHING, 2); 

                gsmStatus.olAutoFetching = TRUE;

                jobmanProcessJobsByWeb(hWnd, iniSms.fetchUrl);
            }
            else
            {
                if (minJobman.index == 0 && minJobman.lndPtr == 0)
                {
                    gsmStatus.olAutoFetching = FALSE;
                }

                statusSetText(hWnd, IDC_STATUSBAR, STATUS_JOBMAN_SKIPFETCHING, 2); 
            }            
        }
    }
}

// Timer callback function on Jobman tasks.
// hWnd, uMsg, idEvent and dwTime
// is define on WindowsAPI.
void CALLBACK TimerTaskProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    PCHAR pID;
    PCHAR pNumber;
    PCHAR pMsg;
    PCHAR pLoca;
    PCHAR szLogLine;

    char szUrl[1024];
    char szParameter[20];
    char taskName[120];

    if (gsmStatus.cmInit) 
    {
        if (!gsmStatus.cmBusy && !gsmStatus.cmSending)
        {
            if (minJobman.index > 0 && minJobman.lndPtr <= minJobman.index)
            {
                if (minJobman.lndPtr == 0)
                {
                    appCtrlGroupStatus(hWnd, JOBMAN_CTRL_INIT);

                    logOutput(hWnd, IDC_LISTBOX_LOG, STATUS_JOBMAN_INIT);
                }

                if (!gsmStatus.cmSending)
                {
                    pID = smsFile[minJobman.lndPtr].lpID;
                    pNumber = smsFile[minJobman.lndPtr].lpNumber;
                    pMsg = smsFile[minJobman.lndPtr].lpMsg;
                    pLoca = smsFile[minJobman.lndPtr].lpLoca;

                    gsmStatus.cmBusy = TRUE;
                    gsmStatus.cmSending = TRUE;
                    szLogLine = pMsg;

                    //logOutput(hWnd, IDC_LISTBOX, szLogLine);
                    statusSetText(hWnd, IDC_STATUSBAR, STATUS_JOBMAN_SEND, 1); 

                    jobmanProcessSendSMS(pNumber, pMsg);

                    smsFile[minJobman.index].dwStatus = (gsmStatus.cmResult) ? 1 : 0;
                    smsFile[minJobman.index].dwRetry += (gsmStatus.cmResult) ? 0 : 1;

                    if (gsmStatus.cmResult)
                    {
                        lstrcpy(taskName, JOBMAN_TASK_PREFIX);     
                        lstrcat(taskName, smsFile[minJobman.index].lpID);      
                        jobDeleteByStr(hWnd, DLG_LISTBOX_TASK, taskName);
                    }

                    if (strlen(smsFile[minJobman.lndPtr].rtUrl) > 0 && strlen(pID) > 0)
                    {
                        lstrcpy(szUrl, smsFile[minJobman.lndPtr].rtUrl);
                        lstrcpy(szParameter, "result=");

                        if (gsmStatus.cmResult)
                        {
                            lstrcat(szParameter, "1");
                            lstrcat(szUrl, szParameter);
                        }
                        else 
                        {
                            lstrcat(szParameter, "0");
                            lstrcat(szUrl, szParameter);
                        }

                        jobmanProcessUploadResult(szUrl);
                    }

                    gsmStatus.cmBusy = FALSE;
                }
            }
        }
        else 
        {
            if (gsmStatus.cmSending)
            {
                statusSetText(hWnd, IDC_STATUSBAR, STATUS_JOBMAN_PREPARENEXT, 1); 

                gsmStatus.cmSending = FALSE;
                minJobman.lndPtr += 1;
            }
        }

        if (minJobman.index > 0 && minJobman.lndPtr >= minJobman.index)
        {
            if (minJobman.timerTask)
            {
                minJobman.timerTask = FALSE;

                KillTimer(hWnd, ID_TIMER2);
            }

            gsmStatus.cmBusy = FALSE;
            gsmStatus.cmSending = FALSE;
            gsmStatus.cmResult = FALSE;

            minJobman.index = 0;
            minJobman.lndPtr = 0;

            appCtrlGroupStatus(hWnd, JOBMAN_CTRL_UNINIT);
            logOutput(hWnd, IDC_LISTBOX_LOG, STATUS_JOBMAN_FINISHED);
            statusSetText(hWnd, IDC_STATUSBAR, STATUS_JOBMAN_FINISHED, 1); 

            if (gsmStatus.olAutoFetching)
            {
                gsmStatus.olAutoFetching = FALSE;                           
            }
        }
    }
}
