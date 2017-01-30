//****************************************************************************
//  Copyright (c) 2008-2017  Anacom, Inc
//  svr10.exe - a utility for communicating with ODUs and other devices.
//  dlu_mapping.cpp - encapsulate DLU-mapping computations and decisions.
//
//  Produced and Directed by:  Dan Miller
//****************************************************************************

//  Following is required, to make MONITOR_DEFAULTTONEAREST known
//lint -esym(767, _WIN32_WINNT)
#define  _WIN32_WINNT   0x0501
#include <windows.h>
#include <tchar.h>

#include "common.h"
#include "commonw.h"
#include "system.h"

//lint -esym(759, cp_get_dlu_height, cp_get_dlu_width)
//lint -esym(765, cp_get_dlu_height, cp_get_dlu_width)

static const uint STD_DPI = 96 ;

// static int  curr_dpi = STD_DPI ;
static uint screen_width  = 0 ;
static uint screen_height = 0 ;

//****************************************************************************
//lint -esym(714, get_monitor_dimens)
//lint -esym(759, get_monitor_dimens)
//lint -esym(765, get_monitor_dimens)
void get_monitor_dimens(HWND hwnd)
{
   HMONITOR currentMonitor;      // Handle to monitor where fullscreen should go
   MONITORINFO mi;               // Info of that monitor
   currentMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
   mi.cbSize = sizeof(MONITORINFO);
   if (GetMonitorInfo(currentMonitor, &mi) != FALSE) {
      screen_width  = mi.rcMonitor.right  - mi.rcMonitor.left ;
      screen_height = mi.rcMonitor.bottom - mi.rcMonitor.top ;
   }
   // curr_dpi = GetScreenDPI() ;
}

//****************************************************************************
//lint -esym(714, get_screen_width)
//lint -esym(759, get_screen_width)
//lint -esym(765, get_screen_width)
uint get_screen_width(void)
{
   return screen_width ;
}

//****************************************************************************
//lint -esym(714, get_screen_height)
//lint -esym(759, get_screen_height)
//lint -esym(765, get_screen_height)
uint get_screen_height(void)
{
   return screen_height ;
}

//********************************************************************************
//lint -esym(714, center_dialog_on_screen)
//lint -esym(759, center_dialog_on_screen)
//lint -esym(765, center_dialog_on_screen)
void center_dialog_on_screen(HWND hDlg)
{
   if (screen_width == 0  ||  screen_height == 0) {
      return ;
   }
   RECT rectAbout ;
   GetWindowRect(hDlg, &rectAbout);
   uint dxAbout = rectAbout.right  - rectAbout.left ;
   uint dyAbout = rectAbout.bottom - rectAbout.top ;
   uint xi = (screen_width  - dxAbout) / 2 ;
   uint yi = (screen_height - dyAbout) / 2 ; 
   SetWindowPos(hDlg, HWND_TOP, xi, yi, 0, 0, SWP_NOSIZE);   
}

//****************************************************************************
bool are_normal_fonts_active(void)
{
   uint curr_dpi = GetScreenDPI() ;
   if (curr_dpi == 96)
      return true;
   return false;
}

//****************************************************************************
//  return true if recalculation was required, false otherwise
//****************************************************************************
//lint -esym(714, cp_recalc_dlu_width)
//lint -esym(759, cp_recalc_dlu_width)
//lint -esym(765, cp_recalc_dlu_width)
bool cp_recalc_dlu_width(uint *psheet_dx)
{
   uint curr_dpi = GetScreenDPI() ;
   if (curr_dpi == STD_DPI) 
      return false ;
   *psheet_dx = (*psheet_dx * curr_dpi) / STD_DPI ;
   return true ;
}

//****************************************************************************
//  return true if recalculation was required, false otherwise
//****************************************************************************
//lint -esym(714, cp_recalc_dlu_height)
//lint -esym(759, cp_recalc_dlu_height)
//lint -esym(765, cp_recalc_dlu_height)
bool cp_recalc_dlu_height(uint *psheet_dy)
{
   uint curr_dpi = GetScreenDPI() ;
   if (curr_dpi == STD_DPI) 
      return false ;
   *psheet_dy = (*psheet_dy * curr_dpi) / STD_DPI ;
   return true ;
}
