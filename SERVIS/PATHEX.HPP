#if !defined(_TPATHEX_)
#define _TPATHEX_

#include "stdafx.h"


#if defined(TPATHEX_DLL)
  #define EXPORT_TPATHEX_ __declspec(dllexport)
#else 
  #define EXPORT_TPATHEX_ __declspec(dllimport)
#endif

void EXPORT_TPATHEX_ CompressPath( CString& rStr, CDC& rDC, 
				                   CRect& rT
						         );
void EXPORT_TPATHEX_ CreateShortName( LPSTR LpFullName, int iLen,
			                          char *pOut );


#endif
