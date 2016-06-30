//********************************************************************************
//  Copyright (c) 2009-2013  Daniel D Miller
//  wfontlist.exe - A WinApi font-lister program
//  getfontfile.cpp - walk the registry to find the filename related to
//  the selected font name.
//  
//  Written by:   Daniel D. Miller
//********************************************************************************
// http://www.installsetupconfig.com/win32programming/windowsregistryapis6_7.html
//  provided working code for walking the registry tree
//********************************************************************************
//  Copyright (C) 2001 Hans Dietrich
//  http://www.codeproject.com/Articles/1235/Finding-a-Font-file-from-a-Font-name
//
//  This provided the concept of looking in the registry to find the filename
//  related to a given font name.  However, his registry walking code did
//  not work properly, and I had to turn to the preceding example for that.
//
//********************************************************************************

#include <windows.h>
#include <tchar.h>
#include <limits.h>
#include <shlobj.h>

#include "common.h"
#include "commonw.h"

//  wfontlist.cpp
extern void redraw_font_list(void);

// #define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

static TCHAR system_font_path[PATH_MAX] ;

//*******************************************************************
void get_font_path(void)
{
   system_font_path[0] = 0 ;
   // int result = 
   SHGetFolderPath(NULL, CSIDL_FONTS, NULL, 0, system_font_path) ;
   // if (result == S_OK) {
   //    syslogW(_T("font path=[%s]\n"), system_font_path) ;
   // } else {
   //    syslogW(_T("SHGetFolderPath: something went wrong\n")) ;
   // }
}

//*******************************************************************
// user app path=[C:\Users\derelict\AppData\Roaming]
//*******************************************************************
static TCHAR user_home_path[PATH_MAX] ;

void get_user_app_path(void)
{
   user_home_path[0] = 0 ;
   // int result = 
   SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, user_home_path) ;
   // if (result == S_OK) {
      syslogW(_T("user app path=[%s]\n"), user_home_path) ;
   // } else {
   //    syslogW(_T("SHGetFolderPath: something went wrong\n")) ;
   // }
}

//********************************************************************************
extern uint operating_mode ;  //  0=remove, 1=enumerate

static void QueryKey (HKEY hKey, LPCTSTR lpszFontName)
{
   // WCHAR achKey[MAX_KEY_LENGTH]; // buffer for subkey name
   // DWORD cbName;                // size of name string
   WCHAR achClass[MAX_PATH] = TEXT ("");  // buffer for class name
   DWORD cchClassName = MAX_PATH;   // size of class string
   DWORD cSubKeys = 0;          // number of subkeys
   DWORD cbMaxSubKey;           // longest subkey size
   DWORD cchMaxClass;           // longest class string
   DWORD cValues;               // number of values for key
   DWORD cchMaxValue;           // longest value name
   DWORD cbMaxValueData;        // longest value data
   DWORD cbSecurityDescriptor;  // size of security descriptor
   FILETIME ftLastWriteTime;    // last write time

   DWORD i, retCode;

   WCHAR achValue[MAX_VALUE_NAME];
   DWORD cchValue = MAX_VALUE_NAME;

   // Get the class name and the value count.
   retCode = RegQueryInfoKey (hKey, // key handle
      achClass,                 // buffer for class name
      &cchClassName,            // size of class string
      NULL,                     // reserved
      &cSubKeys,                // number of subkeys
      &cbMaxSubKey,             // longest subkey size
      &cchMaxClass,             // longest class string
      &cValues,                 // number of values for this key
      &cchMaxValue,             // longest value name
      &cbMaxValueData,          // longest value data
      &cbSecurityDescriptor,    // security descriptor
      &ftLastWriteTime);        // last write time

   // wprintf (L"RegQueryInfoKey() returns %u\n", retCode);

   //  Enumerate the subkeys, until RegEnumKeyEx() fails.
   //  subkeys not relevant, in this example
//    if (cSubKeys != 0) {
//       syslog("Number of subkeys: %d\n", cSubKeys);
// 
//       for (i = 0; i < cSubKeys; i++) {
//          cbName = MAX_KEY_LENGTH;
//          retCode = RegEnumKeyEx (hKey, i, achKey, &cbName, NULL, NULL, NULL, &ftLastWriteTime);
//          if (retCode == ERROR_SUCCESS) {
//             syslog("(%d) %s\n", i + 1, achKey);
//          }
//       }
//    }
//    else
//       syslog("No subkeys to be enumerated!\n");

   // Enumerate the key values
   if (cValues) {
      // Number of values: 497, seeking Almagro Regular
      if (operating_mode == 1) {
         syslogW(_T("Number of values: %d, seeking %s\n"), cValues, lpszFontName);
      }

      // char szValueName[MAX_PATH];
      // DWORD dwValueNameSize = sizeof(szValueName)-1;
      BYTE szValueData[MAX_PATH];
      DWORD dwValueDataSize = sizeof(szValueData)-1;
      DWORD dwType = 0;

      TCHAR font_file_name[PATH_MAX];
      uint removed_elements = 0 ;
      retCode = ERROR_SUCCESS ;  //  not really needed
      for (i = 0; i < cValues; i++) {
         cchValue = MAX_VALUE_NAME;
         achValue[0] = '\0';
         dwValueDataSize = sizeof(szValueData)-1;
         dwType = 0;
         retCode = RegEnumValue (hKey, i, achValue, &cchValue, NULL, &dwType, szValueData, &dwValueDataSize);
         // if (retCode == ERROR_SUCCESS  ||  retCode == ERROR_MORE_DATA) {
         if (retCode == ERROR_SUCCESS) {
            if (operating_mode == 1) {
               // (351) value=Native  Normal (TrueType), data=nativ__r.ttf
               syslogW(_T("(%d) value=%s, data=%s\n"), i + 1, achValue, (char *)szValueData);
            }

            if (_tcsstr(achValue, lpszFontName) != NULL  &&  system_font_path[0] != 0) {
               TCHAR szData[PATH_MAX] ;
               lstrcpy(szData, (TCHAR *)szValueData);
               
               //  RemoveFontResource() requires that filename be fully-qualified path
               //  i.e., c:\windows\fonts\fontname.ttf
               wsprintf(font_file_name, _T("%s\\%s"), system_font_path, szData) ;
               if (operating_mode == 0) {
                  removed_elements++ ;
                  // [4892] removing C:\Windows\Fonts\abbey_me.TTF
                  if (RemoveFontResource(font_file_name)) {
                     syslogW(_T("%s removed\n"), font_file_name) ;
                     //  Again, I am using SendNotifyMessage because SendMessage caused the code to hang.
                     SendMessage(HWND_BROADCAST, WM_FONTCHANGE, (WPARAM) 0, (LPARAM) 0);
                     // SendNotifyMessage(HWND_BROADCAST, WM_FONTCHANGE, NULL, NULL);
                     // _unlink(CompleteLocalPath.c_str());
                     // return kSuccess;
                  } else {
                     syslog("%s: %s\n", font_file_name, get_system_message()) ;
                  }
               } else {
                  syslogW(_T("@@@  found (%d) value=%s, data=%s\n"), i + 1, achValue, (char *)szValueData);
               }
            }
         } else {
            syslog("### (%d) [%u] %s\n", i + 1, (uint) retCode, get_system_message(retCode));
         }
      }
      //  if any fonts were deleted, redraw the font list
      if (removed_elements > 0) {
         redraw_font_list() ;
      } else {
         if (operating_mode == 0) 
            syslogW(_T("cannot match %s\n"), lpszFontName);
      }
   }
   else
      syslog("No values to be enumerated!\n");
}

//********************************************************************************
static bool enum_registry(HKEY key, TCHAR *path, LPCTSTR lpszFontName)
{
   HKEY hTestKey;

   // Change the key and subkey accordingly...
   // In this case: HKEY_USERS\\S-1-5-18\\...
   // if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\Setup") /*L"S-1-5-18"*/,  
   // if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, strFont,
   if (RegOpenKeyEx (key, path, 0, KEY_READ, &hTestKey) != ERROR_SUCCESS) {
      syslog("RegOpenKeyEx() failed!\n");
      return false;
   }
   // syslogW(_T("opened Registry key %s\n"), path);
   QueryKey (hTestKey, lpszFontName);
   RegCloseKey (hTestKey);
   return true;
}

//********************************************************************************
// GetFontFile
//
// Note:  This is *not* a foolproof method for finding the name of a font file.
//        If a font has been installed in a normal manner, and if it is in
//        the Windows "Font" directory, then this method will probably work.
//        It will probably work for most screen fonts and TrueType fonts.
//        However, this method might not work for fonts that are created 
//        or installed dynamically, or that are specific to a particular
//        device, or that are not installed into the font directory.
//********************************************************************************
bool GetFontFile(LPCTSTR lpszFontName)
{
   if (lpszFontName == NULL)
      return false;

   // int nVersion;
   // CString strVersion;
   // GetWinVer(strVersion, &nVersion);
   // syslog(_T("strVersion=%s\n"), strVersion);
   // get_user_app_path();

   //  this won't work for Win98 (or WinCE)
   // if ((nVersion >= WNTFIRST) && (nVersion <= WNTLAST)) 
   //lint -esym(1778, strFont)  
   //  Assignment of string literal to variable 'strFont' (line 209) is not const safe
   TCHAR *strFont = _T("Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts");
   // else
   //   strFont = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Fonts");

   return enum_registry(HKEY_LOCAL_MACHINE, strFont, lpszFontName);
}

