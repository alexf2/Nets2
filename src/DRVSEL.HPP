#if !defined(_DRVSEL_)
#define _DRVSEL_

#include "stdafx.h"
#include "autorc.hpp" 
#include "fsrv.hpp" 
#include "RC\net.rh" 

#include <afxcmn.h>

#include <vector>
using namespace std;

class CMyListBox: public CListBox
 {
public:
  CMyListBox( TFileServer& rSrv ): 
    CListBox(), m_rSrv(rSrv) 
	{};

  virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
  //virtual void MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct );

  //afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
  afx_msg BOOL OnEraseBkgnd( CDC* pDC );
  int Init();

  int m_itemHeight;
  int m_itemWidth;

private:
  TFileServer& m_rSrv;

  static CImageList m_img;
  static CFont m_Fnt;
  static bool m_bInit;
  static int m_CX, m_CY;
  static int m_cxFild;

  int IndexOfType( int );
  LPSTR TextOf( int );

  virtual int CharToItem( UINT nKey, UINT nIndex );

  DECLARE_MESSAGE_MAP()
 };


class TDriveSelDlg: public CDialog
 {
public:
 TDriveSelDlg( CWnd *pPar, TFileServer& rSrv, int& iSel, LPSTR lpTtl ): 
   m_rSrv( rSrv ), 
   m_iSel( iSel ),
   m_lstBox( rSrv ),
   m_sTtl( lpTtl ),
   CDialog( IDD_DLG_DRIVESEL, pPar )
  {
    m_pPar = pPar;
  }

  virtual BOOL OnInitDialog();
  virtual void DoDataExchange( CDataExchange* pDX );  
  afx_msg void OnLb_DblClk();

private:
  TFileServer& m_rSrv;
  int& m_iSel;
  CString m_sTtl;
  CWnd *m_pPar;

  CMyListBox m_lstBox;

  virtual void OnOK();

  DECLARE_MESSAGE_MAP()
 };


#endif
