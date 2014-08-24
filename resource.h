/*
   resource.h
   Developer: Rchockxm (rchockxm.silver@gmail.com)
   Copyright: Silence Unlimited (rchockxm.com)
   Reference: Google C++ Style Guide
     
   This program is free software unless you got it under another license directly
   from the author. You can redistribute it and/or modify it under the terms of
   the GNU General Public License as published by the Free Software Foundation.
   Either version 2 of the License, or (at your option) any later version.
*/

/* Define Constant */
#ifndef _RESOURCE_H
#define _RESOURCE_H

/* Define Language */
#ifndef _APPLANGUAGE
#define _APPLANGUAGE
#define MSG_SUCCESS_TITLE "Success"
#define MSG_SUCCESS_LOG_CLEAR "Clear the last logged success!!"
#define MSG_SUCCESS_SMS_SEND "SMS message send success!!"
#define MSG_ERR_TITLE "Error"
#define MSG_ERR_OPENINSTANCE "Already running?"
#define MSG_ERR_DLGCLS_CREATE "Error creating class."
#define MSG_ERR_DLG_CREATE "Error creating application."
#define MSG_ERR_DLL_LOAD "Load Library Failed While Registering DLL File."
#define MSG_ERR_COM_REGISTER "Load Library Failed While Registering COM interface."
#define MSG_ERR_SQLITE_OPENDB "Can't open SQLite database."
#define MSG_ERR_PORT_INIT "Initiation SerialPort Failed While Open COM port."
#define MSG_ERR_PORT_BUSY "Jobman is busy!! "
#define MSG_ERR_PHONE_MSG "User phone or message empty!!"
#define MSG_ERR_SMS_SEND_FAIL "SMS message send fail!!"
#define MSG_ABOUT_TITLE "About"
#define MSG_ABOUT_CONTENT "Powered by Rchockxm\r\n\r\nE-Mail: rchockxm.silver@gmail.com\r\nWebsiye: rchockxm.com"
#define MSG_EXIT_TITLE "Exit"
#define MSG_EXIT_WITH_JOBMAN "Jobman is running!! Really want to exit?"
#define DLG_FILE_OPEN "Open xml file"
#define STATUS_COM_INIT "Init com port..."
#define STATUS_COM_LISTINIT "Init com list..."
#define STATUS_COM_UNINIT "Released com port..."
#define STATUS_ERR_COM_UNINIT "Released com failed!!"
#define STATUS_JOBMAN_READY "Jobman ready..."
#define STATUS_JOBMAN_INIT "Jobman startup..."
#define STATUS_JOBMAN_SEND "Jobman sending sms..."
#define STATUS_JOBMAN_PREPARENEXT "Jobman preparing next..."
#define STATUS_JOBMAN_FINISHED "Jobman finished..."
#define STATUS_JOBMAN_RESUME "Jobman resume!!"
#define STATUS_JOBMAN_PAUSE "Jobman paused!!"
#define STATUS_JOBMAN_STOP "Jobman stopped!!"
#define STATUS_JOBMAN_FETCHING "Fetching..."
#define STATUS_JOBMAN_SKIPFETCHING "Skip Fetching..."
#define STATUS_ERR_SELECT_PARH "Select path falied!!"
#define STATUS_ERR_SELECT "Select falied!!"
#endif // _APPLANGUAGE

#endif // _RESOURCE_H
