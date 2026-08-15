//****************************************************************************
//  Copyright (c) 2008-2026  Daniel D Miller
//  config.cpp - manage configuration data file
//
//  Produced and Directed by:  Dan Miller
//****************************************************************************
//  Filename will be same as executable, but will have .ini extensions.
//  Config file will be stored in same location as executable file.
//  Comments will begin with '#'
//  First line:
//  device_count=%u
//  Subsequent file will have a section for each device.
//****************************************************************************
#include <windows.h>
#include <cstdio>   //  fopen, etc
#include <cstdlib>  //  atoi()
#include <memory>
#include <tchar.h>

#include "common.h"
#include "wfontlist.h"

uint window_left = 500 ;
uint window_top = 200 ;
uint client_height = 300 ;
static TCHAR ini_name[MAX_PATH_LEN+1] = _T("") ;

//****************************************************************************
//  debug: message-reporting data
//****************************************************************************
uint dbg_flags =
               // DBG_WINMSGS ||
               0 ;

//****************************************************************************
static void strip_comments(TCHAR *bfr)
{
   TCHAR *hd = _tcschr(bfr, '#') ;
   if (hd != 0)
      *hd = 0 ;
}

//****************************************************************************
LRESULT save_cfg_file(void)
{
   client_height = cyClient ;
   TCHAR *fname = ini_name ;
   // FILE *fd = fopen(fname, "wt") ;
   unique_file fd(_tfopen(fname, L"wt")) ;
   if (fd == 0) {
      LRESULT result = (LRESULT) GetLastError() ;
      syslog(_T("%s open: %s\n"), fname, get_system_message(result)) ;
      return result;
   }
   //  save any global vars
   fprintf(fd.get(), "dbg_flags=0x%X\n", dbg_flags) ;
   fprintf(fd.get(), "window_top=%u\n", window_top) ;
   fprintf(fd.get(), "window_left=%u\n", window_left) ;
   fprintf(fd.get(), "client_height=%u\n", client_height) ;
   // fclose(fd) ;
   return ERROR_SUCCESS;
}

//****************************************************************************
//  - derive ini filename from exe filename
//  - attempt to open file.
//  - if file does not exist, create it, with device_count=0
//    no other data.
//  - if file *does* exist, open/read it, create initial configuration
//****************************************************************************
LRESULT init_config(void)
{
   TCHAR inpstr[128] ;
   LRESULT result ;
   
   result = derive_filename_from_exec(ini_name, _T(".ini")) ;
   if (result != 0)
      return result;
   // if (dbg_flags & DBG_VERBOSE)
   //    syslog("INI fname=%s\n", ini_name) ;

   // FILE *fd = fopen(ini_name, "rt") ;
   unique_file fd(_tfopen(ini_name, L"rt")) ;
   if (fd == 0) {
      return save_cfg_file() ;
   }

   // uint local_max_devs = 0 ;
   while (_fgetts(inpstr, sizeof(inpstr), fd.get()) != 0) {
      strip_comments(inpstr) ;
      strip_newlines(inpstr) ;
      if (_tcslen(inpstr) == 0)
         continue;

      TCHAR *tl = _tcschr(inpstr, L'=') ;
      //  if '=' not found, try ':' as separator
      if (tl == NULL) {
         tl = _tcschr(inpstr, L':') ;
         if (tl == NULL) 
            continue;
      }
      *tl++ = 0 ; //  split field name from value ;

      if (_tcscmp(inpstr, L"dbg_flags") == 0) {
         dbg_flags = _tcstoul(tl, NULL, 10);    
      } else
      if (_tcscmp(inpstr, L"window_top") == 0) {
         window_top = (unsigned) _tcstol(tl, NULL, 10) ;
      } else
      if (_tcscmp(inpstr, L"window_left") == 0) {
         window_left = (unsigned) _tcstol(tl, NULL, 10) ;
      } else
      if (_tcscmp(inpstr, L"client_height") == 0) {
         client_height = (unsigned) _tcstol(tl, NULL, 10) ;
      } else
      {
         // syslog("unknown: [%s]\n", inpstr) ;
      }

   }
   // fclose(fd) ;
   return 0;
}

