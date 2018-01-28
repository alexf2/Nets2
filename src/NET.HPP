#if !defined(_NET_)
#define _NET_

#include "stdafx.h"
#include "..\\RUDAKOW\\SERVIS\\bwcc.hpp"

class TNetApp: public CWinApp
 {
public:
  TNetApp();  

  virtual BOOL InitInstance();
  virtual BOOL OnIdle( LONG lCount );
  

  TBWCC bwcc;

  LPCSTR GetStartPath() const { return m_Startpath; }

private:  
  CString m_Startpath;
  

  DECLARE_MESSAGE_MAP()
 };

#endif
