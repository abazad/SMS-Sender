/*
   ctrl.c
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
#include <commctrl.h>
#include "ctrl.h"

// Set text to edit.
// hWnd and ctrlID is WindowsAPI handle and
// controlID with numeric.
void editSetText(HWND hWnd, DWORD ctrlID, LPTSTR lpszFunction)
{
    SendDlgItemMessage(hWnd, ctrlID, WM_SETTEXT, 0, (LPARAM)lpszFunction); 
}

// Get text to edit.
// hWnd and ctrlID is WindowsAPI handle and
// controlID with numeric.
char* editGetText(HWND hWnd, DWORD ctrlID)
{
    HWND hEdit;
    DWORD fs;

    char* fd;

    hEdit = GetDlgItem(hWnd, ctrlID);
    fs = (SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0) + 1 * sizeof(char));
    fd = malloc(fs);

    if (fs)
    {   
        SendMessage(hEdit, WM_GETTEXT, fs, (LPARAM)fd);
    }

    return fd;
}

// Insert text to listbox.
// hWnd and ctrlID is WindowsAPI handle and
// controlID with numeric.
void listboxInsertText(HWND hWnd, DWORD ctrlID, char* lpStr)
{
    HWND hListbox;

    int dwCount;

    hListbox = GetDlgItem(hWnd, ctrlID);
    dwCount = ListBox_GetCount(hListbox);
    ListBox_InsertString(hListbox, dwCount, lpStr);
}

// Clear text from default.
// hWnd and ctrlID is WindowsAPI handle and
// controlID with numeric.
void listboxClearAll(HWND hWnd, DWORD ctrlID)
{
    HWND hListbox;
        
    hListbox = GetDlgItem(hWnd, ctrlID);
    ListBox_ResetContent(hListbox);
}

// Set text to statusbar and display.
// hWnd and ctrlID is WindowsAPI handle and
// controlID with numeric.
void statusSetText(HWND hWnd, DWORD ctrlID, LPTSTR lpszFunction, int dwPart)
{
    HWND hStatusbar;

    hStatusbar = GetDlgItem(hWnd, ctrlID);
    SendMessage(hStatusbar, SB_SETTEXT, dwPart, (LPARAM)lpszFunction);
}

// Set parts to statusbar and display.
// hWnd and ctrlID is WindowsAPI handle and
// controlID with numeric.
void statusSetParts(HWND hWnd, DWORD ctrlID, int dwPart, int *dwParts)
{
    HWND hStatusbar;

    hStatusbar = GetDlgItem(hWnd, ctrlID);
    SendMessage(hStatusbar, SB_SETPARTS, (WPARAM)dwPart, (LPARAM)dwParts);
}
