//  wfontlist.cpp
extern uint cyClient ;

//  getfontfile.cpp
extern void get_font_path(void);
extern bool GetFontFile(LPCTSTR lpszFontName);

//  config.cpp
extern uint dbg_flags ;
extern uint window_top ;
extern uint window_left ;
extern uint client_height ;

LRESULT save_cfg_file(void);
LRESULT init_config(void);
