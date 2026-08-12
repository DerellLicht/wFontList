//***********************************************************************
//  Sample code from MSDN - virtual listview control
//***********************************************************************

#include <vector>

//***********************************************************************
struct font_list_s {
   TCHAR name[LF_FULLFACESIZE] ;
   uint charset ;
   u8 pitch ;
   u8 family ;
   HFONT hfont ;
   bool marked ;
} ;

typedef struct font_list_s *font_list_p ;

//***********************************************************************
class CFontList {
private:
   CVListView *FontVListView ;
   std::vector<font_list_s> font_list ;
   unsigned font_count ;   //  in vector, this will be replaced by font_list.size()
   uint max_font_len ;

   //  private (formerly static) member functions
   bool check_for_dupe(TCHAR *face_name);
   void add_font_to_list(TCHAR *facename, uint charset, u8 pitch, u8 family);
   static int CALLBACK EnumFontFamiliesExProc(
      ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam );

public:
   //  disable default constructor
   CFontList() = delete;
   CFontList(CVListView *VListView);
   ~CFontList() ;
   //  disable copy assignment and copy operators
   CFontList &operator=(const CFontList &src) = delete;
   CFontList(const CFontList&) = delete;
   
   //  disable move assignment and move operators
   CFontList &operator=(const CFontList &&src) = delete;
   CFontList(const CFontList&&) = delete;
   
   void build_font_list(void);
   void sort_font_list(void);
   void delete_font_list(void);
   font_list_p find_font_element(uint target_idx);
   [[nodiscard]] uint get_font_count(void) const {
         return font_count ;
      }
   void mark_element(uint idx);
   void clear_marked_elements(void);
} ;

