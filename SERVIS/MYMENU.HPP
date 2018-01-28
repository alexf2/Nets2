#if !defined(_MYMENU_)
#define _MYMENU_

#include "stdafx.h"
//#include "docexp.hpp"
#include "autorc.hpp"

#include <vector>
#include <afxcmn.h>

using namespace std ;

#if defined(MUMENU_DLL)
  #define EXPORT_MYMENU __declspec(dllexport)
#else 
  #define EXPORT_MYMENU __declspec(dllimport)
#endif


class EXPORT_MYMENU TIcon
 {
public:
  TIcon();
  virtual ~TIcon();

  void Load( UINT, HINSTANCE );
  operator HICON() const { return m_hIcon; }

private:
  HICON m_hIcon;
  UINT m_uiRC;
 };


class EXPORT_MYMENU TMSItem: public CButton
 {
  DECLARE_DYNCREATE( TMSItem )

friend class TMSStyleTooolBar;
friend BOOL CALLBACK EXPORT MSDrawFunc( HDC, LPARAM, int );

public:
  enum TMS_Type { MS_Normal, MS_Selected, MS_Grayed, MS_Pressed };

  static const float fFildV;
  static const float fFildH;
  static const int iSkipBmpTOStr;

  TMSItem();  
  virtual ~TMSItem();

  void Init( LPSTR lpStr, UINT uiIdRcBmp, UINT m_uiCommand, bool bFlRight = false, bool bPushBtn = true );
  void Draw( LPDRAWITEMSTRUCT, TMS_Type, CFont *pFnt );  
  void DrawInDC( CDC *pDc, TMS_Type msType );

  void SetPar( TMSStyleTooolBar *p ) { m_pPar = p; }
  afx_msg void OnMouseMove( UINT nFlags, CPoint point );
  afx_msg void OnTimer( UINT nIDEvent );
  afx_msg void OnDestroy();

  afx_msg BOOL OnEraseBkgnd( CDC * );

  //afx_msg void OnChar( UINT nChar, UINT nRepCnt, UINT nFlags );  
  afx_msg void OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags );
  
  afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
  //afx_msg BOOL OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message );

  //virtual void MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct );
  virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );  
  

  void Mask( CRect&, CDC* );
 
  virtual int OnToolHitTest( CPoint point, TOOLINFO* pTI ) const;

  bool IsChecked() const { return m_bFlChecked; }
  void SetCheckMy( bool bFl ); 
  bool GetCheckMy() const { return m_bFlChecked; }

protected:
  CString m_str;
  CBitmap m_bmp;
  UINT m_uiCommand;    
  bool m_bPushBtn;

  UINT m_uiTimer;  
  
  bool m_bFlChecked;

  TMSStyleTooolBar *m_pPar;

  CSize sz;
  CRect m_Rect;

  bool m_bRight;

//private:
  CBitmap m_bmpNormal;
  CBitmap m_bmpSelected;
  CBitmap m_bmpGrayed;
  CBitmap m_bmpPressed;
  bool m_bChaced;

  static CBrush m_MaskBrush;  
  static CBrush m_MaskBrush2;  

  static TIcon m_iconOK;  
  static TIcon m_iconCancel;  

  bool m_bFlAtCursor;

  CString m_strHint;
  bool m_bMoved;

  bool m_bFlNestedTimer;

  DECLARE_MESSAGE_MAP()  
 };

class EXPORT_MYMENU TMSStyleTooolBar: public CToolBar
 {
   DECLARE_DYNCREATE( TMSStyleTooolBar )

public:
  TMSStyleTooolBar( CWnd *pParent_ );
  TMSStyleTooolBar() { pParent = NULL; }

  virtual ~TMSStyleTooolBar();

  TMSStyleTooolBar& operator,( TMSItem* pIt )
   {
     m_Dta.push_back( pIt );
	 pIt->SetPar( this );
	 return *this;
   }  

  virtual void OnUpdateCmdUI( CFrameWnd* pTarget, BOOL bDisableIfNoHndler ); 

  virtual BOOL PreCreateWindow( CREATESTRUCT& cs );

  void SetFont( CFont* pF ) { m_arcFnt = pF; }  
  void Init( bool bFlEqualWidth = true, bool bFlEqualHigh = true ); //call after all operator, immediatly

  int IDtoIND( int );
  CFont* GetFntPtr() const { return m_arcFnt(); }

  afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );

  void RecalcPosAll();
  int GetWidth() const { return m_iWidthToolbar; }
  bool ShiftRight();

  afx_msg void OnSize( UINT nType, int cx, int cy );  
  

  //afx_msg void OnDrawItem( int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct );  

  HWND FindItem_( CPoint& point, CRect& rect, LPSTR& lpszText ) const;

  bool FindMoved();
  void CheckOff( TMSItem* );

  int GetReqvWidth() const { return m_iWidthToolbar; }

  CWnd *GetParent() { return pParent; }

private:
  vector<TMSItem*> m_Dta;  
  TAutoMemoryRc<CFont*> m_arcFnt;

  //CToolTipCtrl m_ToolTip;

  CWnd *pParent;

  int m_iMaxW, m_iMaxH;
  int m_iWidthToolbar;

  DECLARE_MESSAGE_MAP()  
 };

#endif
