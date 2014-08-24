/*
   ctrl.h
   Developer: Rchockxm (rchockxm.silver@gmail.com)
   Copyright: Silence Unlimited (rchockxm.com)
   Reference: Google C++ Style Guide
     
   This program is free software unless you got it under another license directly
   from the author. You can redistribute it and/or modify it under the terms of
   the GNU General Public License as published by the Free Software Foundation.
   Either version 2 of the License, or (at your option) any later version.
*/

/* Define Constant */
#ifndef _CTRL_H
#define _CTRL_H

// Edit set text.
void editSetText(HWND hWnd, DWORD ctrlID, LPTSTR lpszFunction);

// Edit get text.
char* editGetText(HWND hWnd, DWORD ctrlID);

// Listbox insert text.
void listboxInsertText(HWND hWnd, DWORD ctrlID, char* lpStr);

// listbox clear all items.
void listboxClearAll(HWND hWnd, DWORD ctrlID);

// Statusbar set text.
void statusSetText(HWND hWnd, DWORD ctrlID, LPTSTR lpszFunction, int dwPart);

// Statusbar set part and parts.
void statusSetParts(HWND hWnd, DWORD ctrlID, int dwPart, int *dwParts);

#endif // _CTRL_H
