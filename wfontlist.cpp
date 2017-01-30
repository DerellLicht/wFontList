//**********************************************************************
//  Copyright (c) 2009-2017  Daniel D Miller
//  wfontlist.exe - A WinApi font-lister program
//  
//  Written by:   Daniel D. Miller
//**********************************************************************
//  version		changes
//	 =======		======================================
//    1.00     original, derived from winagrams.cpp
//    1.01     Remove winmsgs.cpp from common_funcs.cpp
//    1.02     Convert various modules into classes
//    1.03     Convert Virtual Listview control into a class
//    1.04     Incorporate my generic VListView class
//    1.05     Display font name using that font
//    1.06     > Add separate display window to show more chars
//             > move common libs and classes to separate directory
//    1.07     Add option to remove font(s), via right-click menu
//    1.08     Fix ERROR_MORE_DATA in RegEnumValue()
//    1.09     > Automatically load font list at startup
//             > make dialog height resizable
//****************************************************************************

#include <windows.h>
#include <tchar.h>
// #include <shlobj.h>  //  including this, causes CDDS_PREPAINT and others to be undefined !!

static const TCHAR* VerNum = _T("V1.09") ;
static char szClassName[] = "WFontList" ;

#include "resource.h"
#include "common.h"
#include "commonw.h" //  build_font()
#include "statbar.h"
#include "vlistview.h"
#include "font_list.h"
#include "system.h"

//  getfontfile.cpp
extern void get_font_path(void);
extern bool GetFontFile(LPCTSTR lpszFontName);

//lint -esym(715, hwnd, private_data, message, wParam, lParam)
//***********************************************************************

static HINSTANCE g_hinst = 0;

static uint current_cursor = 0 ;

//  headers for the main listview control
static lv_cols_t my_lv_cols[] = {
{ _T("Font Name"), 520 },
{ _T("Charset"),   100 },
{ _T("Pitch"),     110 },
{ _T("Family"),      0 },
{ 0, 50 }} ;

static CStatusBar *MainStatusBar = NULL;
static CVListView *VListView = NULL ;
static CFontList *FontList = NULL;  //  must be instantiated AFTER VListView

static uint cxClient = 0;
static uint cyClient = 0;   //  subtrace height of status bar

static HFONT hfontDefault = 0 ;
static HMENU hPopMenu = 0 ;

static HWND hwndMain = NULL ;
static HWND hwndSample = 0 ;
//  note: in sample_text string, I had to include TWO '&' to display one!
//  Otherwise, I got "underlined *" instead of "&*"
static TCHAR const * const sample_text = TEXT("ABCDEFabcdef0123456789`~!@#$%^&&*()_+-={}|[]\\:\";'<>?,./") ;

#define  TERM_MIN_DX    860  
#define  TERM_MIN_DY    1024  

static uint term_window_width  = TERM_MIN_DX ;
static uint term_window_height = TERM_MIN_DY ;

//*******************************************************************
static void status_message(TCHAR *msgstr)
{
   MainStatusBar->show_message(msgstr);
}

//*******************************************************************
static void status_message(uint idx, TCHAR *msgstr)
{
   MainStatusBar->show_message(idx, msgstr);
}

//*******************************************************************
void redraw_font_list(void)
{
   PostMessage(hwndMain, WM_COMMAND, ((BN_CLICKED << 16) | IDB_SHOW_FONTS), 0);
}

//****************************************************************************
static uint get_terminal_top(void)
{
   static uint local_ctrl_top = 0 ;
   if (local_ctrl_top == 0) {
      local_ctrl_top = get_bottom_line(hwndMain, IDC_WORDS) ;
      local_ctrl_top += 3 ;
      // syslog("CommPort: ctrl_top = %u, or %u\n", local_ctrl_top, win_ctrl_top+3) ;
   }
   return local_ctrl_top ;
}  //lint !e715

//****************************************************************************
static HFONT get_font_pointer(uint item_num)
{
   // MessageBox(hwnd, ListItem[index].szItem, "Doubleclicked on this item", MB_OK);
   // syslog("clicked on element %d\n", index) ;
   font_list_p fptr = FontList->find_font_element((uint) item_num) ;
   if (fptr == NULL) {
      syslog("get_font: cannot find item %u\n", item_num) ;
      return 0;
   } else {
      // wsprintf(msgstr, "L%u: %s", (uint) index, fptr->name) ;
      // status_message(msgstr) ;
      return fptr->hfont ;
   }
}

//****************************************************************************
//  This function handles the WM_NOTIFY:NM_CLICK message
//****************************************************************************
static void vlview_single_click(LPARAM lParam)
{
   int index = -1 ;
   TCHAR msgstr[LF_FULLFACESIZE + 20] ;
   NMHDR *hdr = (NMHDR *) lParam;
   if (VListView->is_lview_hwnd(hdr->hwndFrom)) {
      index = VListView->get_next_listview_index(index) ;
      if (index != -1) {
         // MessageBox(hwnd, ListItem[index].szItem, "Doubleclicked on this item", MB_OK);
         // syslog("clicked on element %d\n", index) ;
         // font_list_p fptr = FontList->find_font_element((uint) index) ;
         font_list_p fptr = FontList->find_font_element((uint) index) ;
         if (fptr == NULL) {
            syslog("cannot find item %u\n", index) ;
         } else {
            current_cursor = index ;
            wsprintf(msgstr, _T("L%u: %s"), (uint) index, fptr->name) ;
            status_message(msgstr) ;

            if (fptr->hfont != 0) {
               PostMessage(hwndSample, WM_SETFONT, (WPARAM) fptr->hfont, (LPARAM) true) ;
            }
         }

      }
   }
}

//****************************************************************************
//  This function handles the WM_NOTIFY:LVN_GETDISPINFO message
//****************************************************************************
static void vlview_get_terminal_entry(LPARAM lParam)
{
   static char const * const pitch_str[4] = { "default", "fixed", "variable", "bad_pitch" };
   // syslog("listview notify [%u]\n", this_port->curr_row) ;
   // LV_DISPINFO *lpdi = (LV_DISPINFO *) lParam;
   TCHAR msgstr[LF_FULLFACESIZE+1];

   LV_DISPINFO *lpdi = (LV_DISPINFO *) lParam;
   // term_lview_item_p lvptr = find_term_element(tiSelf, lpdi->item.iItem) ;
   // snmp_request_p snmp_req = lvptr->snmp_entry ;
   font_list_p fptr = FontList->find_font_element((uint) lpdi->item.iItem) ;
   if (fptr == NULL) {
      syslog("cannot find item %u, sub-item %u\n", lpdi->item.iItem, lpdi->item.iSubItem) ;
      return ;
   }
   // syslog("plotting item %u, sub-item %u\n", lpdi->item.iItem, lpdi->item.iSubItem) ;
   if (lpdi->item.mask & LVIF_TEXT) {
      switch (lpdi->item.iSubItem) {
      case 0:
         wsprintf(msgstr, _T("%s"), fptr->name);
         break;

      case 1:
         wsprintf(msgstr, _T("%u"), fptr->charset) ;
         break;

      case 2:
         wsprintf(msgstr, _T("%s"), ascii2unicode((char *) pitch_str[fptr->pitch & 3])) ; //lint !e1773
         break;

      case 3:
         switch (fptr->family) {
         case 0x00: wsprintf(msgstr, _T("dont-care")) ;  break ;
         case 0x08: wsprintf(msgstr, _T("mono")) ;  break ;
         case 0x10: wsprintf(msgstr, _T("roman")) ;  break ;
         case 0x20: wsprintf(msgstr, _T("swiss")) ;  break ;
         case 0x40: wsprintf(msgstr, _T("script")) ;  break ;
         case 0x30: wsprintf(msgstr, _T("modern")) ;  break ;
         case 0x50: wsprintf(msgstr, _T("decorative")) ;  break ;
         default:
            wsprintf(msgstr, _T("unknown (0x%02X)\n"), fptr->family) ;
            break;
         }
         break;

      default:
         wsprintf(msgstr, _T("??? item %d"), lpdi->item.iSubItem) ;  //lint !e585
         break;
      }
      lstrcpy (lpdi->item.pszText, msgstr);
   }
}

//****************************************************************
//  Well, this isn't being called...
//****************************************************************
static LRESULT ProcessCustomDraw(LPARAM lParam)
{
   //  remember which item number we are currently dealing with.
   LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW) lParam;
   uint item_num = lplvcd->nmcd.dwItemSpec ;
   HFONT hfontCurr ;

   switch (lplvcd->nmcd.dwDrawStage) {
   case CDDS_PREPAINT:       //Before the paint cycle begins
      // syslog("CDDS_PREPAINT\n") ;
      //request notifications for individual listview items
      return CDRF_NOTIFYITEMDRAW;
      // return (CDRF_NOTIFYPOSTPAINT | CDRF_NOTIFYITEMDRAW);
      // return CDRF_NOTIFYSUBITEMDRAW;

   case CDDS_ITEMPREPAINT:  //Before an item is drawn
      // syslog("CDDS_ITEMPREPAINT\n") ;
      return CDRF_NOTIFYSUBITEMDRAW;

   case CDDS_SUBITEM | CDDS_ITEMPREPAINT: //Before a subitem is drawn
      // {
      // if (lplvcd->nmcd.dwDrawStage == CDDS_SUBITEM)
      //    syslog("CDDS_SUBITEM\n") ;
      // else
      //    syslog("CDDS_ITEMPREPAINT\n") ;
      // }         
      switch (lplvcd->iSubItem) {
      case 0:
         // lplvcd->clrText = GetSysColor(COLOR_WINDOWTEXT) ;
         // lplvcd->clrTextBk = GetSysColor(COLOR_MENUTEXT);  
         hfontCurr = get_font_pointer(item_num) ;
         if (hfontCurr != 0) {
            // per Bengi: To set a custom font:
            SelectObject(lplvcd->nmcd.hdc, hfontCurr);
         }
         return CDRF_NEWFONT;

      default:
         // lplvcd->clrText = GetSysColor(COLOR_WINDOWTEXT) ;
         // lplvcd->clrTextBk = GetSysColor(COLOR_WINDOW) ;
         SelectObject(lplvcd->nmcd.hdc, hfontDefault);

         return CDRF_NEWFONT;
         // return CDRF_DODEFAULT ;
      }

   default:
      syslog("Unknown CDDS code %d\n", lplvcd->nmcd.dwDrawStage) ;
      break;
   }
   return CDRF_DODEFAULT;
}

//*******************************************************************
static bool do_init_dialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
   RECT mainRect ;
   TCHAR msgstr[81] ;
   wsprintf(msgstr, _T("wFontList %s"), VerNum) ;
   SetWindowText(hwnd, msgstr) ;
   hwndMain = hwnd ;
   get_monitor_dimens(hwnd);

   SendMessage(hwnd, WM_SETICON, ICON_BIG,   (LPARAM) LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_APPICON)));
   SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM) LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_APPICON)));

   //  get dimensions of main client area
   // GetWindowRect(hwnd, &mainRect) ;
   GetClientRect(hwnd, &mainRect) ;
   cxClient = (uint) (mainRect.right  - mainRect.left) ;
   cyClient = (uint) (mainRect.bottom - mainRect.top ) ;
   term_window_width  = cxClient ;
   term_window_height = cyClient ;
   // Client area: 883x784
   // stTop=752, cyStatus=22
   /* syslog("Client area: %ux%u\n", cxClient, cyClient); */

   //****************************************************************
   //  create/configure status bar first
   //****************************************************************
   MainStatusBar = new CStatusBar(hwnd) ;
   MainStatusBar->MoveToBottom(term_window_width, term_window_height-1) ;

   //  re-position status-bar parts
   int sbparts[3];
   sbparts[0] = (int) (6 * cxClient / 10) ;
   sbparts[1] = (int) (8 * cxClient / 10) ;
   sbparts[2] = -1;
   MainStatusBar->SetParts(3, &sbparts[0]);
   
   //****************************************************************
   //  create listview class second, needs status-bar height
   //****************************************************************
   {
   // uint lvy0 = 55 ;  //  bottom of other controls
   uint lvy0 = get_terminal_top();
   uint fudge_factor = 0 ;
   uint lvdy = term_window_height - fudge_factor - get_terminal_top() - MainStatusBar->height() ;   //lint !e737
   VListView = new CVListView(hwnd, IDC_TERMINAL, g_hinst, 0, lvy0, cxClient-5, lvdy,
         LVL_STY_VIRTUAL | LVL_STY_EX_GRIDLINES);
   VListView->set_listview_font("Times New Roman", 140, 0) ;
   VListView->lview_assign_column_headers(&my_lv_cols[0], (LPARAM) 0) ;
   }

   hfontDefault = build_font(_T("Times New Roman"), 20, EZ_ATTR_NORMAL) ;
   hwndSample = GetDlgItem(hwnd, IDC_WORDS) ;
   SetWindowText(hwndSample, sample_text) ;
   // PostMessage(hwndSample, WM_SETFONT, hfontDefault, (LPARAM) true) ;

   //****************************************************************
   //  create font-list class third, needs VListView control
   //****************************************************************
   FontList = new CFontList(VListView) ;

   redraw_font_list();
   return true ;
}

//****************************************************************************
static int enum_selected_rows(void)
{
   int result = 0 ;
   int nCurItem = -1 ;
   while (LOOP_FOREVER) {
      nCurItem = VListView->get_next_listview_index(nCurItem) ;
      if (nCurItem < 0)
         break;

      font_list_p fptr = FontList->find_font_element((uint) nCurItem) ;
      if (fptr == NULL) {
         syslog("cannot find item %u\n", nCurItem) ;
      } else {
         GetFontFile(fptr->name) ;
      }
   }
   return result;
}

//*******************************************************************
uint operating_mode = 0 ;  //  0=remove, 1=enumerate

static bool do_command(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
   TCHAR msgstr[81] ;
   DWORD cmd = HIWORD (wParam) ;
   DWORD target = LOWORD(wParam) ;
   // termout(&this_term, "WM_COMMAND: cmd=%u, target=%u", cmd, target) ;
   if (cmd == BN_CLICKED) {
      switch (target) {
      case IDB_SHOW_FONTS:
         // min_word_len = read_max_chars() ;
         // PostMessage(hwndCommTask, WM_DO_COMM_TASK, (WPARAM) 0, 0) ;
         //  if list already exists, delete it and prepare for a new read
         if (FontList->get_font_count() > 0) {
            VListView->clear_listview() ;
            FontList->delete_font_list();
         }
         FontList->build_font_list() ;
         FontList->sort_font_list() ;
         wsprintf(msgstr, _T("found %u fonts"), FontList->get_font_count()) ;
         status_message(msgstr) ;
         //  last but not least, we would like to restore "current cursor" position,
         //  if such a thing existed...
         if (current_cursor > 0) {
            //  restore cursor position
            wsprintf(msgstr, _T("move to row %u"), current_cursor) ;
            status_message(1, msgstr) ;
            
            VListView->goto_element(current_cursor);
            current_cursor = 0 ;
         } else {
            status_message(1, _T("Ready")) ;
         }

         return true;

      case IDM_DELETE_FONTS:
         operating_mode = 0 ;
         enum_selected_rows();
         return true;

      case IDM_ENUM_FONTS:
         syslogW(_T("enumerating fonts\n"));
         operating_mode = 1 ;
         enum_selected_rows();
         return true;
      }  //lint !e744
   } 
   return false ;
}

//***************************************************************************
//  right-click popup menu for device actions
//***************************************************************************
static void show_popup_menu(HWND hwnd, uint clicked_row)
{
   // if (clicked_row >= remote_count) {
   //    return ;
   // }

   // device_p this_dev = &ip3_devices[clicked_row] ;
   // menu_dev = this_dev ;
   //  delete any existing menu
   if (hPopMenu != 0) {
      DeleteMenu(hPopMenu, 3, MF_BYPOSITION) ;
      DeleteMenu(hPopMenu, 2, MF_BYPOSITION) ;
      DeleteMenu(hPopMenu, 1, MF_BYPOSITION) ;
      DeleteMenu(hPopMenu, 0, MF_BYPOSITION) ;
      // DrawMenuBar(hPopMenu) ;
   }

   POINT lpClickPoint;
   GetCursorPos(&lpClickPoint);
   hPopMenu = CreatePopupMenu();
   AppendMenu(hPopMenu, MF_STRING | MF_DISABLED, IDM_PUP_LABEL, _T("Font Actions"));
   AppendMenu(hPopMenu, MF_SEPARATOR, 0, NULL) ;
   AppendMenu(hPopMenu, MF_STRING, IDM_DELETE_FONTS, _T("Delete selected fonts"));
   AppendMenu(hPopMenu, MF_STRING, IDM_ENUM_FONTS, _T("Enumerate fonts"));
   SetForegroundWindow(hwnd);
   // TrackPopupMenu(hPopMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_BOTTOMALIGN,
   TrackPopupMenu(hPopMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_TOPALIGN,
      lpClickPoint.x, lpClickPoint.y, 0, hwnd, NULL);
}  //lint !e715

//*******************************************************************
static bool do_notify(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
   NMHDR *hdr ;
   int clicked_row, clicked_column ;
   LPNMLISTVIEW pnm ;
   int msg_code = (int) ((NMHDR FAR *) lParam)->code ;
   // syslog("Cport_notify: HWND=%08X, msg=%d\n", hwnd, msg_code) ;
   switch (msg_code) {

   //**********************************************************
   //  terminal listview notifications
   //**********************************************************
   case LVN_GETDISPINFO:   //lint !e650  negative constant out of range
      // syslog("trap: LVN_GETDISPINFO\n") ;
      vlview_get_terminal_entry(lParam) ;
      return true;

   case NM_CLICK:
      vlview_single_click(lParam) ;
      return true;

   case NM_RCLICK:
      // if (LOWORD (wParam) == IDC_LISTVIEW1) {
      hdr = (NMHDR *) lParam;
      if (VListView->is_lview_hwnd(hdr->hwndFrom)) {
         VListView->find_selected_row((LPNMHDR) lParam, &clicked_row, &clicked_column) ;
         // syslog("right-click: row=%d, column=%d\n", clicked_row, clicked_column) ;
         if (clicked_column == 0) {
            show_popup_menu(hwnd, clicked_row) ;
         }
      } 
      break;

   case NM_CUSTOMDRAW:
      pnm = (LPNMLISTVIEW) lParam;
      if (VListView->is_lview_hwnd((HWND) pnm->hdr.hwndFrom)) {
         SetWindowLongA(hwnd, DWL_MSGRESULT, (LONG) ProcessCustomDraw (lParam));
         return true ;  //  the absense of this, is why no more occurred!!
      }
      break;

   default:
      // if (dbg_flags & DBG_WINMSGS)
      //    syslog("Trap WM_NOTIFY: [%d] %s\n", msg_code, lookup_winmsg_name(msg_code)) ;
      break;
   }
   return false;
}

//*******************************************************************
static bool do_close(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
   DestroyWindow(hwnd);
   return true ;
}

//*******************************************************************
static bool do_destroy(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
   PostQuitMessage(0);
   return true ;
}

//********************************************************************************************
//  okay, this function originally gave inaccurate results,
//  because the rectangle passed by WM_SIZING was from GetWindowRect(),
//  which included the unwanted border area, rather than from
//  GetClientRect(), which works with get_bottom_line().
//********************************************************************************************
//  04/26/13 
//  WM_SIZING is generated every pixel or two of movement; we *wont* want to be resizing
//  the entire dialog that frequently!!  We need to somehow slow this down a bit...
//********************************************************************************************
// #define  TERM_INIT_XOFFSET  1
// #define  TERM_INIT_YOFFSET  1

static void resize_font_dialog(bool resize_on_drag)
{
   RECT myRect ;
   int dx_offset = 5, dy_offset = 0;
   // syslog("resize terminal, drag=%s\n", (resize_on_drag) ? "true" : "false") ;

   uint new_window_width, new_window_height ;
   if (resize_on_drag) {
      //  if resizing on drag-and-drop, re-read main-dialog size
      GetClientRect(hwndMain, &myRect) ;
      new_window_width  = (uint) (myRect.right - myRect.left) ;
      new_window_height = (uint) (myRect.bottom - myRect.top) ;

      if (term_window_height == new_window_height)
          return ;

      // dx_offset = 6 ;
      // dy_offset = 5 ;
      // CPortTabControl->resize_window(new_window_width-dx_offset, new_window_height-dy_offset) ;
      // term_window_width  = new_window_width  ;
      term_window_height = new_window_height ;

      // change_terminal_pixels(term_window_width, term_window_height) ;
      // dx_offset =  3 ;
      dy_offset =  4 ;
      if (!are_normal_fonts_active()) {
         // syslog("acting on large fonts\n") ;
         // dx_offset = 7 ;
         dy_offset = 2 ;
      }

   } 
   // else {
   //    resize_window(hwndTerminal, term_window_width, term_window_height) ;
   //    dx_offset = TERM_INIT_XOFFSET ;
   //    dy_offset = TERM_INIT_YOFFSET ;
   //    // if (are_large_fonts_active(hwndTerminal)) {
   //    if (!are_normal_fonts_active()) {
   //       dx_offset -= 5 ;
   //       dy_offset -= 6 ;
   //    }
   // }

   MainStatusBar->MoveToBottom(term_window_width, term_window_height-1) ;
   //  resize the terminal (cols)
   int dxi = term_window_width  - dx_offset ;   //lint !e737
   int dyi   = term_window_height - dy_offset - get_terminal_top() - MainStatusBar->height() ;   //lint !e737
   // VListView->resize_terminal_pixels(dxi, dyi) ;
   VListView->resize(dxi, dyi); //  dialog is actually drawn a few pixels too small for text
   // set_terminal_dimens() ;  //  do this *after* resize()
//   VListView->resize_column(dxi-25) ; //  make this narrower than new_dx, to allow for scroll bar
   // set_terminal_sizing(true);
   // if (resize_on_drag) {
   //    save_cfg_file() ;
   // }
}

//*******************************************************************
static bool do_sizing(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
   //  handle main-dialog resizing
   switch (message) {
   case WM_SIZING:
      switch (wParam) {
      case WMSZ_BOTTOMLEFT:
      case WMSZ_BOTTOMRIGHT:
      case WMSZ_TOPLEFT:
      case WMSZ_TOPRIGHT:
      case WMSZ_LEFT:
      case WMSZ_RIGHT:
      case WMSZ_TOP:
      case WMSZ_BOTTOM:
         resize_font_dialog(true);
         return true;

      default:
         break;
      }
      break;
   }
   return false ;
}

//*******************************************************************
//  DDM 01/29/17 - These minima are not actually working;
//  Perhaps this is due to Windowblinds ??
//*******************************************************************
static bool do_getminmaxinfo(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
   switch (message) {
   case WM_GETMINMAXINFO:
      {
      LPMINMAXINFO lpTemp = (LPMINMAXINFO) lParam;
      POINT        ptTemp;
      // syslog("set minimum to %ux%u\n", cxClient, cyClient);
      //  set minimum dimensions
      ptTemp.x = term_window_width + 11;  //  empirical value
      ptTemp.y = term_window_height ;  //  empirical value
      lpTemp->ptMinTrackSize = ptTemp;
      //  set maximum dimensions
      ptTemp.x = term_window_width + 11;
      ptTemp.y = get_screen_height() ;
      lpTemp->ptMaxTrackSize = ptTemp;
      // lpTemp->ptMaxSize = ptTemp;
      }         
      return 0 ;

   default:
      break;
   }
   return 1 ;
}


//*******************************************************************
// typedef struct winproc_table_s {
//    uint win_code ;
//    bool (*winproc_func)(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data) ;
// } winproc_table_t ;

static winproc_table_t const winproc_table[] = {
{ WM_INITDIALOG,     do_init_dialog },
{ WM_COMMAND,        do_command },
// { WM_COMM_TASK_DONE, do_comm_task_done },
{ WM_NOTIFY,         do_notify },
{ WM_SIZING,         do_sizing },
{ WM_GETMINMAXINFO,  do_getminmaxinfo },
{ WM_CLOSE,          do_close },
{ WM_DESTROY,        do_destroy },

{ 0, NULL } } ;

//*******************************************************************
static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   uint idx ;
   for (idx=0; winproc_table[idx].win_code != 0; idx++) {
      if (winproc_table[idx].win_code == message) {
         return (*winproc_table[idx].winproc_func)(hwnd, message, wParam, lParam, NULL) ;
      }
   }
   return false;
}  //lint !e715

//***********************************************************************
static BOOL WeAreAlone(LPSTR szName)
{
   HANDLE hMutex = CreateMutexA(NULL, TRUE, szName);
   if (GetLastError() == ERROR_ALREADY_EXISTS)
   {
      CloseHandle(hMutex);
      return FALSE;
   }
   return TRUE;
}

//*********************************************************************
int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                  LPSTR szCmdLine, int iCmdShow)   
{
   if (!WeAreAlone (szClassName)) {
      syslog("wFontList is already running!!\n") ;
      return 0;
   }

   g_hinst = hInstance;
   load_exec_filename() ;     //  get our executable name
   get_font_path();

   //  create the main application
   HWND hwnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN_DIALOG), NULL, (DLGPROC) WndProc);
   if (hwnd == NULL) {
      // Notified your about the failure
      syslog("CreateDialog (main): %s [%u]\n", get_system_message(), GetLastError()) ;
      // Set the return value
      return FALSE;
   }
   ShowWindow (hwnd, SW_SHOW) ;
   UpdateWindow(hwnd);

   MSG msg ;
   while (GetMessage (&msg, NULL, 0, 0)) {
      if (!IsDialogMessage(hwnd, &msg)) {
         TranslateMessage (&msg) ;
         DispatchMessage (&msg) ;
      }
   }
   return (int) msg.wParam ;
}  //lint !e715

