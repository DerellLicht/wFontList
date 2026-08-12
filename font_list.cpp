//***********************************************************************
//  Sample code from MSDN - virtual listview control
// 
//  compile with:  g++ -Wall -s -O3 flist.cpp -o flist.exe -lgdi32
//***********************************************************************

#include <windows.h>
#include <tchar.h>

// NOLINTBEGIN(cppcoreguidelines-owning-memory)
#include "common.h"
#include "commonw.h" //  build_font()
#include "vlistview.h"
#include "font_list.h"

//*************************************************************************
CFontList::CFontList(CVListView *VListView) :
   FontVListView(VListView),
   font_list {},
   font_count(0),
   max_font_len(0)
{
}

//*************************************************************************
void CFontList::delete_font_list(void)
{
   font_list.clear() ;
   max_font_len = 0 ;
   font_count = 0 ;
}

//*************************************************************************
CFontList::~CFontList()
{
   font_list.clear() ;
   max_font_len = 0 ;
   font_count = 0 ;
}

//****************************************************************************
void CFontList::mark_element(uint idx)
{
   if (idx >= font_list.size()) {
      return ;
   }
   font_list_p rptr = &font_list[idx];  //  NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
   rptr->marked = true ;
}

//****************************************************************************
void CFontList::clear_marked_elements(void)
{
   for(auto &fptr : font_list) {
      fptr.marked = false ;
   }
}

//*************************************************************************
font_list_p CFontList::find_font_element(uint target_idx)
{
   if (target_idx >= font_list.size()) {
      return nullptr ;
   }
   font_list_p rptr = &font_list[target_idx];  //  NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
   return rptr ;
}

//*********************************************************
static bool const sort_name (font_list_s const &a, font_list_s const &b)
{
   return (_tcsicmp (a.name, b.name) < 0) ;
}

//*********************************************************
//  This intermediate function is used because I want
//  merge_sort() to accept a passed parameter,
//  but in this particular application the initial
//  list is global.  This function sets up the global
//  comparison-function pointer and passes the global
//  list pointer to merge_sort().
//*********************************************************
void CFontList::sort_font_list(void)
{
   std::sort(font_list.begin(), font_list.end(), sort_name);
}

//***********************************************************************
bool CFontList::check_for_dupe(TCHAR *face_name)
{
   for(auto &fptr : font_list) {
      if (_tcscmp(face_name, fptr.name) == 0)
         return true;
   }
   return false;
}

//***********************************************************************
//  this needs to drop duplicate entries, though...
//***********************************************************************
void CFontList::add_font_to_list(TCHAR *facename, uint charset, u8 pitch, u8 family)
{
   if (check_for_dupe(facename))
      return ;
      
   font_list_p fptr = &font_list.emplace_back();
   // font_list_p fptr = &font_list.back();
   uint slen = _tcslen(facename) ;
   if (max_font_len < slen)
       max_font_len = slen ;
   _tcsncpy(fptr->name, facename, slen) ;
   fptr->charset = charset ;
   fptr->pitch = pitch ;
   fptr->family = family ;
   fptr->hfont = build_font(fptr->name, 20, EZ_ATTR_NORMAL) ;

   FontVListView->listview_update(font_count);
   font_count++ ;
}

//***********************************************************************
// #define DEFAULT_PITCH   0
// #define FIXED_PITCH  1
// #define VARIABLE_PITCH  2
// #define MONO_FONT        8    0x08
// #define FF_DECORATIVE   80    0x50
// #define FF_DONTCARE           0x00
// #define FF_MODERN 48          0x30
// #define FF_ROMAN  16          0x08
// #define FF_SCRIPT 64          0x40
// #define FF_SWISS  32          0x20

// typedef struct tagENUMLOGFONTEX {
//   LOGFONT  elfLogFont;
//   TCHAR  elfFullName[LF_FULLFACESIZE];
//   TCHAR  elfStyle[LF_FACESIZE];
//   TCHAR  elfScript[LF_FACESIZE];
// } ENUMLOGFONTEX, *LPENUMLOGFONTEX;

//***********************************************************************
int CALLBACK CFontList::EnumFontFamiliesExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, 
                                    int FontType, LPARAM lParam )
{
   // bugprone-casting-through-void: don't chain a C-style cast through void* — 
   // if you're going to reinterpret unrelated types, 
   // say so explicitly with reinterpret_cast.
   // CFontList* pThis = (CFontList*)(void*)lParam;
   auto *pThis = reinterpret_cast<CFontList*>(lParam) ;

   LOGFONT *lfptr = &lpelfe->elfLogFont ;
   // printf( "%s, charset=%u, paf=0x%x\n", lfptr->lfFaceName, lfptr->lfCharSet, lfptr->lfPitchAndFamily );
   // printf( "%s, charset=%u, ", lfptr->lfFaceName, lfptr->lfCharSet);
   u8 paf = (u8) lfptr->lfPitchAndFamily ;
   u8 pitch = paf & 0x03 ;
   u8 family = paf & 0xFC;
   // printf( "%s, style=%s, script=%s\n", lpelfe->elfFullName, lpelfe->elfStyle, lpelfe->elfScript) ;
   pThis->add_font_to_list((TCHAR *) lpelfe->elfFullName, lfptr->lfCharSet, pitch, family) ;
   return 1;
}  //lint !e715

//***********************************************************************
void CFontList::build_font_list(void)
{
   HDC hDC = GetDC( NULL );
   LOGFONT lf = { 0, 0, 0, 0, 0, 0, 0, 0, 
      // ANSI_CHARSET,  //  lfCharSet
      DEFAULT_CHARSET,  //  lfCharSet - read everything, all languages
      0, 0, 0, 
      DEFAULT_PITCH,    //  lfPitchAndFamily
      // "Courier New" };
      TEXT("")  //  lfFaceName
      };              
   // EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC) EnumFontFamiliesExProc, 0, 0 );
   // EnumFontFamiliesEx(hDC, &lf, reinterpret_cast<FONTENUMPROC>(EnumFontFamiliesExProc), (LPARAM) (void*) this, 0 );
   EnumFontFamiliesEx(hDC, &lf, reinterpret_cast<FONTENUMPROC>(EnumFontFamiliesExProc), reinterpret_cast<LPARAM>(this), 0 );

   ReleaseDC( NULL, hDC );
}

// NOLINTEND(cppcoreguidelines-owning-memory)
