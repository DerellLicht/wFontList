//****************************************************************************
//  Copyright (c) 2008-2014  Anacom, Inc
//  AnaComm.exe - a utility for communicating with ODUs and other devices.
//  dlu_mapping.cpp - encapsulate DLU-mapping computations and decisions.
//
//  Produced and Directed by:  Dan Miller
//****************************************************************************

//  dialog dimension functions
uint get_screen_width(void);
uint get_screen_height(void);
bool cp_recalc_dlu_width (uint *psheet_dx);
bool cp_recalc_dlu_height(uint *psheet_dy);
void center_dialog_on_screen(HWND hDlg);

//  general system functions
void get_monitor_dimens(HWND hwnd);
bool are_normal_fonts_active(void);
int wait_for_event(HANDLE hEvent, uint timeout_secs);

