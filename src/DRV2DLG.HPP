#if !defined(_DRV2DLG_)
#define _DRV2DLG_

#include "stdafx.h"
#include "autorc.hpp" 
//#include "fsrv.hpp" 
#include "RC\net.rh" 

class TDriveDlg: public CDialog
 {
public:
 TDriveDlg( CWnd *pPar, char *pBuf ): pB(pBuf), CDialog( IDD_DLG_DRIVE, pPar )
  {
  }

  virtual BOOL OnInitDialog();
  virtual void DoDataExchange( CDataExchange* pDX );

//private:
  char *pB;
  CStatic m_Icon;
 };

class TSelectDialog: public CDialog
 {
public:
  TSelectDialog( CWnd *pPar, char *pBuf, char* pHint ): 
    m_csMask( pBuf ), 
	m_csHint( pHint ),
    CDialog( IDD_DLG_SELECT, pPar )
   {
     m_mPar = pPar;
   }

  virtual BOOL OnInitDialog();
  virtual void DoDataExchange( CDataExchange* pDX );

  CString m_csMask;  

private:
  CString m_csHint;
  CWnd* m_mPar;
 };

class TMaskTemplate
 {
public:
  TMaskTemplate( LPCSTR lp );
  ~TMaskTemplate();

  bool Check( LPCSTR );

private:
  CString m_csName;
  CString m_csExt;
  LPSTR m_lpName, m_lpExt;

  bool CmpTmpl( LPSTR, LPSTR );
 };


#endif
