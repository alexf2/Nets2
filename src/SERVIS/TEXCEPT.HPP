#if !defined(_TEXCEPT_)
#define _TEXCEPT_

#include "stdafx.h"
#include <cstring>

#if defined(TEXCEPT_DLL)
  #define EXPORT_TEXCEPT_ __declspec(dllexport)
#else 
  #define EXPORT_TEXCEPT_ __declspec(dllimport)
#endif

class  EXPORT_TEXCEPT_ TExcept
 {
public:
  TExcept( bool bShDel );
  TExcept( bool bShDel, LPCSTR lpMsg, unsigned int uiTtlID );
  TExcept( bool bShDel, unsigned int uiRcID, unsigned int uiTtlID );
 
  TExcept( const TExcept& );  
  TExcept& operator=( const TExcept& );

  virtual ~TExcept();

  void Delete();
  void ReportExcept();

private:
  bool m_bShDel;

  unsigned int m_uiRcID, m_uiTtlID;
  CString  m_cstrMsg;
 };

#endif
