#if !defined(_NCPANE_)
#define _NCPANE_

#include "stdafx.h"
#include "autorc.hpp" 
#include "fsrv.hpp" 
#include "server.hpp" 

#include <afxcmn.h>
#include <io.h>

#include <vector>
using namespace std;

class TNortonLikePane;
struct TMFOptions;

class TNCSplitter: public CSplitterWnd
 {
   DECLARE_DYNAMIC(TNCSplitter)
public:
  TNCSplitter(): CSplitterWnd() 
   {
     m_bModeFull = 0;
	 m_pOpt = NULL;
	 m_pFSrv = NULL;
   }
  virtual ~TNCSplitter(); 

  //virtual void OnDrawSplitter( CDC* pDC, ESplitType nType, const CRect& rect );
  virtual void OnDrawSplitter( CDC* pDC, ESplitType nType, const CRect& rectArg );

  afx_msg BOOL OnEraseBkgnd( CDC* pDC );    

  void Get_SizeBar( int& cx, int& cy ) const
   {
     cx = m_cxSplitter; cy = m_cySplitter;
   }
  void Set_SizeBar( int cx, int cy ) 
   {
     m_cxSplitter = cx; m_cySplitter = cy;
   }

  void Get_Space( int& cx, int& cy ) const
   {
     cx = m_cxBorderShare; cy = m_cyBorderShare;
   }
  void Set_Space( int cx, int cy ) 
   {
     m_cxBorderShare = cx; m_cyBorderShare = cy;
   }

  void Get_Amount( int& cx, int& cy ) const
   {
     cx = m_cxSplitterGap; cy = m_cySplitterGap;
   }
  void Set_Amount( int cx, int cy ) 
   {
     m_cxSplitterGap = cx; m_cySplitterGap = cy;
   }

  void Get_Border( int& cx, int& cy ) const
   {
     cx = m_cxBorder; cy = m_cyBorder;
   }
  void Set_Border( int cx, int cy ) 
   {
     m_cxBorder = cx; m_cyBorder = cy;
   }
  
  TNortonLikePane* operator()( int );
  int GetActivePaneMy() const;
  void SetActivePaneMy( int );
  void SetActivePaneMy( TNortonLikePane* );

  int GetFullMode() const { return m_bModeFull; }
  void SetFullMode( int );

  void RefocusList();
  TNortonLikePane* GetActivePane_Int();

  int GetClmSize( int iPane ) const;
  void SetClmSize( int iPane, int iSize );

  LPCSTR GetPath( int iPane ) const;
  void SetPath( int iPane, LPCSTR lp );

  void SetCfg( TMFOptions* pOpt )
   {
     m_pOpt = pOpt;
   }

  TMFOptions* GetOpt() const { return m_pOpt; }
  TFileServer *GetFServer() const { return m_pFSrv; }
  void AssFServer( TFileServer *pSrv ) { m_pFSrv = pSrv; }

  LPCSTR GetSelectMask( int ) const;
  void SetSelectMask( int, LPCSTR );
  void Select( int iPane, LPCSTR lpMask, bool bFlSel );	

  void Refresh();

protected:
  virtual void StopTracking( BOOL bAccept );
  int m_bModeFull;//0-none, 1 - left is full pane, 2 - right is full pane;

  TFileServer *m_pFSrv;

  TMFOptions* m_pOpt;

  DECLARE_MESSAGE_MAP()
 };

class TUserOfDrive_NCPane;

class CMyBox: public CStatusBar
 {

  friend class TNortonLikePane;

public:
  CMyBox();

  virtual ~CMyBox();

  void Init( COLORREF cBk, COLORREF cFnt, COLORREF cBk2, COLORREF cFnt2, CFont *pFnt, int iCy, bool bOwnDraw = false );
  void SetTtl( TUserOfDrive_NCPane& );   
  void RecalcWorkStr();

  afx_msg void OnSize( UINT nType, int cx, int cy );

  void SetMode( bool bFl ) { m_bFlMode2 = bFl; }

  //afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
  afx_msg BOOL OnEraseBkgnd( CDC* pDC );  
  afx_msg void OnPaint();
  afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );

  afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
  afx_msg void OnRButtonDown( UINT nFlags, CPoint point );


protected:
  CFont *m_pFnt;
  COLORREF m_cBk, m_cFnt, m_cBk2, m_cFnt2;
  TAutoMemoryRc<CBrush*> arcBr, arcBr2;
  TAutoMemoryRc<CPen*> arcPenDark, arcPenLight;
  bool m_bOwnDraw;

  bool m_bFlMode2;

  CString m_Str;
  CString m_StrWork;

  DECLARE_MESSAGE_MAP()
 };

class THeaderCtrlNC: public CHeaderCtrl
 {

public:
  THeaderCtrlNC( int& riSz ): m_riSizeClm(riSz), CHeaderCtrl() {};
  virtual ~THeaderCtrlNC();

  afx_msg void OnSize( UINT, int, int );
  afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
  afx_msg void OnRButtonDown( UINT nFlags, CPoint point );

private:
  int& m_riSizeClm;

  DECLARE_MESSAGE_MAP()
 };


class TNCList: public CListBox
 {

friend class TNortonLikePane;

public:
  TNCList( vector<TFileRecord*>& rLst, int& rszClm, TAutoMemoryRc<CPen*>& rAP, CFont* pFnt ): 
    CListBox(),
    m_rvevFiles( rLst ),
    m_riSizeClm( rszClm ),
    m_rAP( rAP )
	 {
	   m_iOldSel = 0;
	   m_pFnt = pFnt;
	 }

   virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
   virtual void MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct );

   afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
   afx_msg BOOL OnEraseBkgnd( CDC* pDC );    

   afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
   afx_msg void OnRButtonDown( UINT nFlags, CPoint point );

   //afx_msg void OnSetFocus( CWnd* pOldWnd );
   //afx_msg void OnKillFocus( CWnd* pNewWnd );  

   virtual int CharToItem( UINT nKey, UINT nIndex );   
   afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );

   void FillList();
   void UpdateWidth()
	{
      SetColumnWidth( m_riSizeClm );
      Invalidate( NULL );
	}

protected:
  vector<TFileRecord*>& m_rvevFiles;
  int& m_riSizeClm;

  CFont *m_pFnt;

  TAutoMemoryRc<CPen*>& m_rAP;
  int m_iOldSel;

  void KeyCurDir();
  CString m_strKeyDir;
  void SelectKeyedDir();
  void SetInSel();

  DECLARE_MESSAGE_MAP()
 };


class TNortonLikePane;

class TUserOfDrive_NCPane: public TUserOfDrive
 {
public:
  TUserOfDrive_NCPane( TDrive *pDrv ): 
    TUserOfDrive( pDrv )
   {
     m_pOwner = NULL;
   }
  
  bool IsCallingOutside() const;   

  void SetParent( TNortonLikePane *pPar ) 
   {
     m_pOwner = pPar;
   }
  virtual bool NotifyUser( TUserOfDrive::TNotifyCode );

  LONG CalcSelected( unsigned atr = _A_SUBDIR|_A_ARCH|_A_HIDDEN|_A_NORMAL|_A_RDONLY|_A_SYSTEM  );
  LPSTR GetFileAtCursor();
  vector<TFileRecord*>* GetSelectionVector() const;

  TDrive* GetDriveFor( CString& csPath, bool bRemote );

  int GetIndexAtCursor();   
  void SelectIndex( int, bool );

  void InvalidateList();
   

private:
  TNortonLikePane *m_pOwner;
 };



class TNortonLikePane: public CFrameWnd
 {
   DECLARE_DYNCREATE(TNortonLikePane)
public:
  TNortonLikePane();
  virtual ~TNortonLikePane();

  /*CDocument* GetDocument( ) const
   {
     return m_pDocument;
   }*/
  //virtual void OnDraw( CDC* );

  virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
  afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
  afx_msg BOOL OnEraseBkgnd( CDC* pDC );    

  afx_msg void OnPaint();
  afx_msg void OnSize( UINT nType, int cx, int cy );
  //afx_msg void OnActivate( UINT nState, CWnd* pWndOther, BOOL bMinimized );
  afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
  afx_msg void OnRButtonDown( UINT nFlags, CPoint point );

  afx_msg void OnEndTrack( NMHDR *pNMHDR, LRESULT *pRes );

  afx_msg void OnSetFocus( CWnd* pOldWnd );
  //afx_msg void OnKillFocus( CWnd* pNewWnd );  
  afx_msg void OnLb_DblClk();
  afx_msg void OnLb_SelChange();
  

  void SetActivePaneInstance();

  bool IsActive() const { return m_bFlActive; }
  void SetState( bool );

  int GetClmSize() const { return m_iSizeClm; }
  void SetClmSize( int );

  LPCSTR GetPath() const { return (LPCSTR)m_usrUserOfSrv.GetPath(); }
  void SetPath( LPCSTR lp, bool bExtErr = false );
  void SetPath_DrOnly( LPCSTR lp );

  void GetHdrRect( CRect& );
  void GetListRect( CRect& );

  void RefocusList()
   {
     m_nclLst.SetFocus();
   }
  bool GetStateTTl() const { return m_Ttl.m_bFlMode2; }

  bool NotifyUser( TUserOfDrive::TNotifyCode nfc );
  BOOL ChDir( TNCList*, bool& );

  BYTE GetDrive() const { return m_usrUserOfSrv.GetDrive()->Letter(); }
  int GetDriveIndex() 
   { 
     return m_pFSrv->FindCurDriveIndex(m_usrUserOfSrv.GetDrive());
   }
    
  void SetDriveIndex( int );

  LPCSTR GetSelectMask() const { return m_csSelMask; }
  void SetSelectMask( LPCSTR lp )
   {
     m_csSelMask = lp;
   }

  void Select( LPCSTR lpMask, bool bFlSel );

  TUserOfDrive_NCPane* GetUser() { return &m_usrUserOfSrv; }

  LONG CalcSelected( unsigned atr );
  LPSTR GetFileAtCursor();

  vector<TFileRecord*>* GetSelectionVector() const
   {
     return (vector<TFileRecord*>*)&m_rvecFiles;
   }

  TDrive* GetDriveFor( CString& csPath, bool bRemote );

  int GetIndexAtCursor();   
  void SelectIndex( int, bool );
  void InvalidateList()
   {
     m_nclLst.InvalidateRect( NULL );
   }  


protected:
  CMyBox m_Ttl, m_Stat;

  TMFOptions *m_pOpt;
  static CFont m_FntTtl, m_FntStat, m_FntList;
  static bool m_bFlInitFont;

  CString m_csSelMask;

  bool m_bFlActive;

  CRect m_rcClient;

  THeaderCtrlNC m_hdrCtl;
  int m_iSizeClm;

  TNCList m_nclLst;    
  afx_msg void OnIdleUpdateCmdUI();

  //**************** Filler data ****************//
  TFileServer *m_pFSrv;
  TUserOfDrive_NCPane m_usrUserOfSrv;

  vector<TFileRecord*> m_rvecFiles;  
  void ClearFList( vector<TFileRecord*>& rLst );  
  //**************** End of Filler data ****************//
  

  TAutoMemoryRc<CPen*> arcPenBorder;

  DECLARE_MESSAGE_MAP()
 };

inline LONG TUserOfDrive_NCPane::CalcSelected( unsigned atr )
 {
   return m_pOwner->CalcSelected( atr );
 }

inline LPSTR TUserOfDrive_NCPane::GetFileAtCursor()
 {
   return m_pOwner->GetFileAtCursor();
 }

inline int TUserOfDrive_NCPane::GetIndexAtCursor()
 {
   return m_pOwner->GetIndexAtCursor();
 }
inline void TUserOfDrive_NCPane::SelectIndex( int iInd, bool bFl )
 {
   m_pOwner->SelectIndex( iInd, bFl );
 }

inline vector<TFileRecord*>* TUserOfDrive_NCPane::GetSelectionVector() const
 {
   return m_pOwner->GetSelectionVector();
 }

inline TDrive* TUserOfDrive_NCPane::GetDriveFor( CString& csPath, bool bRemote )
 {
   return m_pOwner->GetDriveFor( csPath, bRemote );
 }

inline void TUserOfDrive_NCPane::InvalidateList()
 {
   m_pOwner->InvalidateList();
 }

#endif
