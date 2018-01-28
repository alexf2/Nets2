#if !defined(_TBWCC_)
#define _TBWCC_

#include "stdafx.h"
#include <cstring>
//#include <windows.h>

#if defined(TBWCC_DLL)
  #define EXPORT_TBWCC_ __declspec(dllexport)
#else 
  #define EXPORT_TBWCC_ __declspec(dllimport)
#endif

class EXPORT_TBWCC_ TBWCC
 {
public:
  TBWCC( LPCSTR lpPath = NULL );
  virtual ~TBWCC();

  void EnableBWCC( HINSTANCE hApp, BOOL bFl = TRUE );

  int (WINAPI* BWCCMessageBox)( HWND, LPCSTR lpText, LPCSTR lpCaption, UINT );

private:
  HINSTANCE m_Inst;
  CString str;

  bool (WINAPI* bwccIntlInit)( unsigned int );
  bool (WINAPI* bwccRegister)( HINSTANCE );
  bool (WINAPI* bwccIntlTerm)( void );  
 };

class EXPORT_TBWCC_ TBWCCLocalozeRC
 {
public:
  TBWCCLocalozeRC();
  virtual ~TBWCCLocalozeRC();

  void Localize();
  void Restore();

private:
  HINSTANCE m_hinstOldRC; 
  bool m_bInit;
 };

#endif
