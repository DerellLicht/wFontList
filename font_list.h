//***********************************************************************
//  Sample code from MSDN - virtual listview control
//***********************************************************************

// Info 1704: Constructor 'CFontList::CFontList(const CFontList &)' has private access specification
// Info 1719: assignment operator for class 'CFontList' has non-reference parameter
// Info 1720: assignment operator for class 'CFontList' has non-const parameter
// Info 1722: assignment operator for class 'CFontList' does not return a reference to class
//lint -esym(1704, CFontList::CFontList)
//lint -esym(1719, CFontList)
//lint -esym(1720, CFontList)
//lint -esym(1722, CFontList)
typedef struct font_list_s {
   struct font_list_s *next ;
   TCHAR name[LF_FULLFACESIZE] ;
   uint charset ;
   u8 pitch ;
   u8 family ;
   HFONT hfont ;
   bool marked ;
} font_list_t, *font_list_p ;

//***********************************************************************
class CFontList {
private:
   CVListView *FontVListView ;
   font_list_p font_list ;
   font_list_p font_tail ;
   unsigned font_count ;
   uint max_font_len ;

   //  disable default constructor
   CFontList() ;
   
   //  disable assignment and copy operators
   CFontList operator=(const CFontList src) ;
   CFontList(const CFontList&);

   //  private (formerly static) member functions
   unsigned check_for_dupe(TCHAR *face_name);
   void add_font_to_list(TCHAR *facename, uint charset, u8 pitch, u8 family);
   static int CALLBACK EnumFontFamiliesExProc(
      ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam );

public:
   CFontList(CVListView *VListView);
   ~CFontList() ;
   void build_font_list(void);
   void sort_font_list(void);
   void delete_font_list(void);
   font_list_p find_font_element(uint target_idx);
   uint get_font_count(void) const {
         return font_count ;
      }
   void mark_element(uint idx);
   void clear_marked_elements(void);
} ;

