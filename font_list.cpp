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

// CFontList::FontVListView  not directly freed or zeroed by destructor ... it shouldn't be...
//lint -esym(1740, CFontList::FontVListView)

//*************************************************************************
CFontList::CFontList(CVListView *VListView) :
   FontVListView(VListView),
#ifdef  USE_VECTOR
   font_list {},
#else
   font_list (nullptr),
   font_tail (nullptr),
#endif   
   font_count(0),
   max_font_len(0)
{
}

//*************************************************************************
void CFontList::delete_font_list(void)
{
#ifdef  USE_VECTOR
   font_list.clear() ;
#else
   font_list_p fontptr = font_list;
   font_list = NULL ;
   font_tail = NULL ;
   max_font_len = 0 ;
   while (fontptr != NULL) {
      font_list_p fontkill = fontptr;
      fontptr = fontptr->next ;
      delete fontkill ;
   }
#endif   
   font_count = 0 ;
}

//*************************************************************************
CFontList::~CFontList()
{
#ifdef  USE_VECTOR
   font_list.clear() ;
#else
   font_list_p fontptr = font_list;
   font_list = NULL ;
   font_tail = NULL ;
   while (fontptr != NULL) {
      font_list_p fontkill = fontptr;
      fontptr = fontptr->next ;
      delete fontkill ;
   }
#endif   
   font_count = 0 ;
}

//****************************************************************************
void CFontList::mark_element(uint idx)
{
#ifdef  USE_VECTOR
   if (idx >= font_list.size()) {
      return ;
   }
   font_list_p rptr = &font_list[idx];  //  NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
   rptr->marked = true ;
#else
   font_list_p rptr ;
   // for (term_lview_item_p lvptr = tlv_top; lvptr != NULL; lvptr = lvptr->next) {
   uint list_idx = 0 ;
   for (rptr = font_list; rptr != NULL; rptr = rptr->next, list_idx++) {
      if (list_idx == idx) {
         rptr->marked = true ;
         break;
      }
   }
#endif
}

//****************************************************************************
void CFontList::clear_marked_elements(void)
{
#ifdef  USE_VECTOR
   for(auto &fptr : font_list) {
      fptr.marked = false ;
   }
#else
   font_list_p rptr ;
   for (rptr = font_list; rptr != NULL; rptr = rptr->next) {
      rptr->marked = false ;
   }
#endif   
}

//*************************************************************************
font_list_p CFontList::find_font_element(uint target_idx)
{
#ifdef  USE_VECTOR
   if (target_idx >= font_list.size()) {
      return nullptr ;
   }
   font_list_p rptr = &font_list[target_idx];  //  NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
   return rptr ;
#else
   font_list_p rptr ;
   uint idx = 0 ;
   for (rptr = font_list; rptr != NULL; rptr = rptr->next, idx++) {
      if (idx == target_idx)
         return rptr;
   }
   return NULL;
#endif   
}

#ifdef  USE_VECTOR
//*********************************************************
static bool const sort_name (font_list_s const &a, font_list_s const &b)
{
   return (_tcsicmp (a.name, b.name) < 0) ;
}

#else
//*********************************************************
static int sort_name(font_list_p a, font_list_p b)
{
   return(_tcsicmp(a->name, b->name)) ;
}
#endif

//*********************************************************
#ifndef  USE_VECTOR
//*********************************************************
static font_list_p z = NULL ;

//*********************************************************
//  This routine merges two sorted linked lists.
//*********************************************************
static font_list_p merge(font_list_p a, font_list_p b)
{
   font_list_p c = z ;

   do {
      int x = sort_name(a, b) ;
      if (x <= 0) {
         c->next = a ;
         c = a ;
         a = a->next ;
      } else {
         c->next = b ;
         c = b ;
         b = b->next ;
      }
   } while ((a != NULL) && (b != NULL));

   if (a == NULL)  c->next = b ;
             else  c->next = a ;
   return z->next ;
}

//*********************************************************
//  This routine recursively splits linked lists
//  into two parts, passing the divided lists to
//  merge() to merge the two sorted lists.
//*********************************************************
static font_list_p merge_sort(font_list_p c)
{
   font_list_p a ;
   font_list_p b ;
   font_list_p prev ;
   int pcount = 0 ;
   int j = 0 ;

   if ((c != NULL) && (c->next != NULL)) {
      a = c ;
      while (a != NULL) {
         pcount++ ;
         a = a->next  ;
      }
      a = c ;
      b = c ;
      prev = b ;
      while (j <  pcount/2) {
         j++ ;
         prev = b ;
         b = b->next ;
      }
      prev->next = NULL ;  //lint !e771

      return merge(merge_sort(a), merge_sort(b)) ;
   }
   return c ;
}
#endif

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
#ifdef  USE_VECTOR
   std::sort(font_list.begin(), font_list.end(), sort_name);
#else
   if (z == 0) {
      // z = new ffdata ;
      // z = (struct ffdata *) malloc(sizeof(ffdata)) ;
      z = (font_list_p) new font_list_t ;
      if (z == NULL)
         return ;
      memset((char *) z, 0, sizeof(font_list_t)) ;
   }
   font_list = merge_sort(font_list) ;
#endif   
}

//***********************************************************************
bool CFontList::check_for_dupe(TCHAR *face_name)
{
#ifdef  USE_VECTOR
   for(auto &fptr : font_list) {
      if (_tcscmp(face_name, fptr.name) == 0)
         return true;
   }
#else
   font_list_p fptr ;
   for (fptr=font_list; fptr != 0; fptr = fptr->next) {
      if (_tcscmp(face_name, fptr->name) == 0)
         return true;
   }
#endif   
   return false;
}

//***********************************************************************
//  this needs to drop duplicate entries, though...
//***********************************************************************
void CFontList::add_font_to_list(TCHAR *facename, uint charset, u8 pitch, u8 family)
{
   if (check_for_dupe(facename))
      return ;
      
#ifdef  USE_VECTOR
   font_list_p fptr = &font_list.emplace_back();
   // font_list_p fptr = &font_list.back();
#else      
   font_list_p fptr = new font_list_t ;
   ZeroMemory((char *) fptr, sizeof(font_list_t)) ;
#endif   
   uint slen = _tcslen(facename) ;
   if (max_font_len < slen)
       max_font_len = slen ;
   _tcsncpy(fptr->name, facename, slen) ;
   fptr->charset = charset ;
   fptr->pitch = pitch ;
   fptr->family = family ;
   fptr->hfont = build_font(fptr->name, 20, EZ_ATTR_NORMAL) ;

#ifndef  USE_VECTOR
   //  add new entry to list
   if (font_list == 0)
      font_list = fptr ;
   else
      font_tail->next = fptr ;
   font_tail = fptr ;
#endif   

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
