#if !defined(_FULLSCR_WINDOW_)
#define  _FULLSCR_WINDOW_

#if defined(TFULLSCR_DLL)
  #define EXPORT_TFULLSCR_ __declspec(dllexport)
#else 
  #define EXPORT_TFULLSCR_ __declspec(dllimport)
#endif


#include "stdafx.h"
#include "autorc.hpp"
#include "dibapi.h"


const int iNumbEnt = 21;

class EXPORT_TFULLSCR_ TSysPaletteSave
 {
public:
  TSysPaletteSave() { hdc = NULL; }
  ~TSysPaletteSave();

  void operator()( HDC );
  void operator~();

  void Reset();

private:
  static int iIndexPal[ iNumbEnt ];
  static COLORREF rgb[ iNumbEnt ];
  HDC hdc;
  int iNumColors;
 };

class  EXPORT_TFULLSCR_ TFullScrWindow: public CWnd
 {
public:
  TFullScrWindow( CWnd* pPar, HDIB  pbmpDIB_, HINSTANCE hiRc, HPALETTE hpalAlias_ );

  virtual ~TFullScrWindow();

  //virtual void Destroy( int retVal = 0 );
  afx_msg void OnDestroy();
  void Create();

protected:
  //virtual LPSTR GetClassName();      //TMDIFrame
  //virtual void GetWindowClass( WNDCLASS & AWndClass );
  
  //virtual BOOL Create();
  afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
  
  virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
  

  //virtual void Paint( TDC& dc, bool erase, TRect& rect );
  afx_msg void OnPaint();

  //void WMKeyDown( RTMessage ) = [WM_FIRST + WM_KEYDOWN];
  //void WMSysKeyDown( RTMessage ) = [WM_FIRST + WM_SYSKEYDOWN];
  //void WMMouseMove( RTMessage msg ) = [WM_FIRST + WM_MOUSEMOVE];
  //void WMLButtonDown( RTMessage ) = [WM_FIRST + WM_LBUTTONDOWN];
  //void WMRButtonUp( RTMessage ) = [WM_FIRST + WM_RBUTTONUP];
  //void WMLButtonUp( RTMessage ) = [WM_FIRST + WM_LBUTTONUP];

  //void WMSetCursor( RTMessage ) = [WM_FIRST + WM_SETCURSOR];

  //void WMQueryNewPalette( RTMessage ) = [WM_FIRST + WM_QUERYNEWPALETTE];
  //void WMPaletteChanged( RTMessage ) = [WM_FIRST + WM_PALETTECHANGED];


  /*bool EvEraseBkgnd( HDC );
  void EvKeyDown( uint key, uint repeatCount, uint flags );
  void EvSysKeyDown( uint key, uint repeatCount, uint flags );
  void EvMouseMove( uint modKeys, TPoint& point );
  void EvLButtonDown( uint modKeys, TPoint& point );
  void EvLButtonUp( uint modKeys, TPoint& point );
  void EvRButtonUp( uint modKeys, TPoint& point );
  bool EvSetCursor( HWND hWndCursor, uint hitTest, uint mouseMsg );
  bool EvQueryNewPalette();
  void EvPaletteChanged( HWND hWndPalChg );*/

  afx_msg BOOL OnEraseBkgnd( CDC* pDC );

  afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
  afx_msg void OnSysKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );

  afx_msg void OnMouseMove( UINT nFlags, CPoint point );
  afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
  afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
  afx_msg void OnRButtonUp( UINT nFlags, CPoint point );
  afx_msg void OnRButtonDown( UINT nFlags, CPoint point );

  afx_msg BOOL OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message );

  afx_msg BOOL OnQueryNewPalette( );
  afx_msg void OnPaletteChanged( CWnd* pFocusWnd );

  virtual void PostNcDestroy();

private:
  CPalette pal;  

  int iBmpWidth, iBmpHigh;
  int iBmpViewWidth, iBmpViewHigh;

  //TDib*  pbmpDIB;  
  TAutoMemoryRc_Win arcDIB;

  HINSTANCE m_hinstRC;
  CWnd *m_pPar_;

  HPALETTE hpalAlias;
  HCURSOR hcHand;
  //HDC hdc;
  TAutoMemoryRc<CClientDC*> arcDC;
  BOOL bFlTrack;
  int iStaOrgX, iStaOrgY;
  

  TSysPaletteSave spsSysPalette;

  static int iMouseX;
  static int iMouseY;

  BOOL bFlOldDisable;

  void PrepareKey( UINT uiKey );

  void DrawBitmapDIBWithPalette( CDC& rHdc, HPALETTE hpalAlias, int X, int Y );
  void DrawBitmapDIB( CDC& rHdc, int X, int Y );

  //DECLARE_RESPONSE_TABLE( TFullScrWindow );
  DECLARE_MESSAGE_MAP()
 };

#endif

