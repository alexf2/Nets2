#include <windowsx.h>

#define TFULLSCR_DLL
#include "fullscr.hpp"

//#define TDIBAPI_DLL
#include "dibapi.h"

#define MYDIB_DLL
#include "mydib.hpp"


#include "mydib.rh"
//#include <stdio.h>

//extern HINSTANCE hInstRC;
//extern PTModule pMRC;

BEGIN_MESSAGE_MAP( TFullScrWindow, CWnd )
	ON_WM_CREATE()
	ON_WM_DESTROY()
    ON_WM_PAINT()

	ON_WM_ERASEBKGND()

	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()

	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()

	ON_WM_SETCURSOR()

	ON_WM_QUERYNEWPALETTE()
	ON_WM_PALETTECHANGED()

END_MESSAGE_MAP()
  
    
int TSysPaletteSave::iIndexPal[ iNumbEnt ] =
 {
   COLOR_ACTIVEBORDER,
   COLOR_ACTIVECAPTION,
   COLOR_APPWORKSPACE,
   COLOR_BACKGROUND,
   COLOR_BTNFACE,
   COLOR_BTNHIGHLIGHT,
   COLOR_BTNSHADOW,
   COLOR_BTNTEXT,
   COLOR_CAPTIONTEXT,
   COLOR_GRAYTEXT,
   COLOR_HIGHLIGHT,
   COLOR_HIGHLIGHTTEXT,
	COLOR_INACTIVEBORDER,
   COLOR_INACTIVECAPTION,
   COLOR_INACTIVECAPTIONTEXT,
   COLOR_MENU,
   COLOR_MENUTEXT,
   COLOR_SCROLLBAR,
   COLOR_WINDOW,
   COLOR_WINDOWFRAME,
   COLOR_WINDOWTEXT
 };

COLORREF TSysPaletteSave::rgb[ iNumbEnt ];


TSysPaletteSave::~TSysPaletteSave()
 {
   ~*this;
 }

void TSysPaletteSave::operator()( HDC hdc_ )
 {
   Reset();   

	iNumColors = GetDeviceCaps( hdc, NUMCOLORS );
   if( !hdc_ || !iNumColors || iNumColors > 20 ) return;
   hdc = hdc_;

   for( int i = 0; i < iNumbEnt; i++ )
     TSysPaletteSave::rgb[ i ] = GetSysColor( TSysPaletteSave::iIndexPal[i] );
 }

void TSysPaletteSave::operator~()
 {
   if( hdc )
     SetSysColors( iNumbEnt , TSysPaletteSave::iIndexPal,
		   TSysPaletteSave::rgb );
   else return;

   Reset();
 }

void TSysPaletteSave::Reset()
 {
	hdc = NULL;
 }

int TFullScrWindow::iMouseX, TFullScrWindow::iMouseY;

//TFullScrWindow( CWnd* parent, HDIB  pbmpDIB_, HINSTANCE hiRc, HPALETTE hpalAlias_ );
TFullScrWindow::TFullScrWindow( CWnd* pPar, HDIB  pbmpDIB_, HINSTANCE hiRc, HPALETTE hpalAlias_ ):
  CWnd(),
  arcDIB( (HGLOBAL)pbmpDIB_ )
 {
    m_hinstRC = hiRc;
	m_pPar_ = pPar;

	bFlOldDisable = FALSE;
	hpalAlias = hpalAlias_;
	//Attr.Style = WS_BORDER | WS_MAXIMIZE | WS_VISIBLE | WS_POPUP;
	
	LPSTR lp = (LPSTR)arcDIB.Lock();
	iBmpWidth = DIBWidth( lp );
	iBmpHigh = DIBHeight( lp );
	arcDIB.Unlock();
	hcHand = NULL;
	//hdc = NULL;

	iStaOrgX = iStaOrgY = 0;
	bFlTrack = FALSE;

	//_fsetmem( &r, sizeof(r), 0 );

	CreateDIBPalette( pbmpDIB_, &pal );
 }

//void TFullScrWindow::Destroy( int retVal )
void TFullScrWindow::OnDestroy()
 {
	//if( m_pPar_ )
	  //m_pPar_->EnableWindow( bFlOldDisable ? FALSE:TRUE );

	::SetSystemPaletteUse( arcDC()->m_hDC, SYSPAL_STATIC );
	~spsSysPalette;

	arcDC.FreeRc();

	CWnd::OnDestroy();
 }

void TFullScrWindow::PostNcDestroy()
 {
   CWnd::PostNcDestroy();
   delete this;
 }

TFullScrWindow::~TFullScrWindow()
 {
	if( hcHand ) { DestroyCursor(hcHand); hcHand = NULL; }		
 }


/*void TFullScrWindow::GetWindowClass( WNDCLASS & AWndClass )
 {
	TWindow::GetWindowClass( AWndClass );

	AWndClass.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
	AWndClass.style |= CS_SAVEBITS | CS_OWNDC;
 }*/

BOOL TFullScrWindow::PreCreateWindow( CREATESTRUCT& cs )
 {
   if( !CWnd::PreCreateWindow(cs) ) return FALSE;

   cs.style = WS_MAXIMIZE | WS_VISIBLE | WS_POPUP;

   cs.lpszClass = AfxRegisterWndClass( CS_OWNDC,
	  AfxGetApp()->LoadStandardCursor(IDC_ARROW),
	  (HBRUSH)GetStockObject(BLACK_BRUSH) );

   return  cs.lpszClass ? TRUE:FALSE;   	
 }

void TFullScrWindow::Create()
 {
   CRect r(0, 0, GetSystemMetrics(SM_CXMAXIMIZED), GetSystemMetrics(SM_CYMAXIMIZED) ); 
   CWnd::Create( NULL, NULL, 0, r, m_pPar_, 0 );	 
 }

//BOOL TFullScrWindow::Create()
int TFullScrWindow::OnCreate( LPCREATESTRUCT lpC )
 {
	if( CWnd::OnCreate(lpC) == -1 ) return -1;

	//if( m_pPar_ )
	  //bFlOldDisable = m_pPar_->EnableWindow( FALSE );

	//hdc = GetDC( HWindow );
	arcDC = new CClientDC( this );

	spsSysPalette( arcDC()->m_hDC );
	SetSystemPaletteUse( arcDC()->m_hDC, SYSPAL_NOSTATIC );

	CRect r;
	GetClientRect( &r );
	iBmpViewWidth = r.right; iBmpViewHigh = r.bottom;
	hcHand = LoadCursor( GetDLL_MyDIB_ServisModule(), MAKEINTRESOURCE(IDC_CURSOR_HAND) );

	if( iBmpWidth < r.right ) iStaOrgX = (r.right - iBmpWidth) / 2;
	if( iBmpHigh < r.bottom ) iStaOrgY = (r.bottom - iBmpHigh) / 2;
	arcDC()->SetViewportOrg( iStaOrgX, iStaOrgY );

	//InvalidateRect( NULL, TRUE );

	return 0;
 }

//bool TFullScrWindow::EvEraseBkgnd( HDC hdc )
BOOL TFullScrWindow::OnEraseBkgnd( CDC* pDC )
 { 
    //CRect r; GetClientRect( &r );
	//if( iBmpWidth >= r.right && iBmpHigh >= r.bottom ) return 1;

    
	CPoint cp;
	if( arcDC() )  
	 {        
		cp = arcDC()->GetViewportOrg();
		arcDC()->SetViewportOrg( 0, 0 );	  
	 }

	BOOL bRes = CWnd::OnEraseBkgnd( pDC );

	if( arcDC() )  
	 {        	
		arcDC()->SetViewportOrg( cp );	  
	 }

	return bRes;
 }

void TFullScrWindow::DrawBitmapDIB( CDC& rHdc, int X, int Y )
 {
   //LPSTR lp = (LPSTR)arcDIB.Lock();
   CRect rDst( X, Y, X+iBmpWidth, Y+iBmpHigh );
   CRect rSrc( 0, 0, rDst.Width(), rDst.Height() );
   //arcDIB.Unlock();      
   
   PaintDIB( (HDC)rHdc, rDst, (HDIB)arcDIB(), rSrc, NULL );	
 }


void TFullScrWindow::DrawBitmapDIBWithPalette( CDC& rHdc, HPALETTE hpalAlias, int X, int Y )
 {
   HPALETTE hpal = hpalAlias ? hpalAlias:(HPALETTE)pal;//TPalette


   //LPSTR lp = (LPSTR)arcDIB.Lock();
   CRect rDst( X, Y, X+iBmpWidth, Y+iBmpHigh );
   CRect rSrc( 0, 0, rDst.Width(), rDst.Height() );
   //arcDIB.Unlock();      

   CPalette *pPal = CPalette::FromHandle( hpal );
   PaintDIB( (HDC)rHdc, rDst, (HDIB)arcDIB(), rSrc, pPal );
 }


//void TFullScrWindow::Paint( TDC& dc, bool erase, TRect& rect )
void TFullScrWindow::OnPaint()
 {
	if( arcDIB() )
	  DrawBitmapDIBWithPalette( *arcDC(), hpalAlias, 0, 0 );

    CRect ptR(0, 0, iBmpWidth-1, iBmpHigh-1 );
	arcDC()->LPtoDP( ptR );    
	ValidateRect( &ptR );

	CPoint cp;	
	cp = arcDC()->GetViewportOrg();
	arcDC()->SetViewportOrg( 0, 0 );	  
	
	        	
	Default();
	
	arcDC()->SetViewportOrg( cp );	  	
 }

void TFullScrWindow::PrepareKey( UINT uiKey )
 {
	if( arcDIB() )
	 {
		int iPoW = iStaOrgX,
	  iPoH = iStaOrgY;
		int iDx = iBmpWidth - iBmpViewWidth,
	  iDy = iBmpHigh - iBmpViewHigh;
		int iVStep = double(iBmpViewHigh) * 0.2,
	  iHStep = double(iBmpViewWidth) * 0.2;

		switch( uiKey )
		 {
			case VK_UP:
			  iPoH++;
		break;

			case VK_DOWN:
			  iPoH--;
		break;

			case VK_LEFT:
			  iPoW++;
		break;

	 case VK_RIGHT:
		iPoW--;
		break;

	 case VK_HOME:
		if( GetKeyState(VK_SHIFT) & 0xFF00 )
		  iPoH = 0;
			  else
		  iPoW = 0;
		break;

	 case VK_END:
		if( GetKeyState(VK_SHIFT) & 0xFF00 )
		  iPoH = -iDy;
			  else
		  iPoW = -iDx;
		break;

	 case VK_PRIOR:
		if( GetKeyState(VK_SHIFT) & 0xFF00 )
		  iPoW += iHStep;
			  else
		  iPoH += iVStep;
		break;

	 case VK_NEXT:
		if( GetKeyState(VK_SHIFT) & 0xFF00 )
		  iPoW -= iHStep;
			  else
		  iPoH -= iVStep;
		break;

	 default:
			  int cod = uiKey;
		//TWindow::DefWndProc( msg );
		//DefaultProcessing();
        Default();
		if( cod != VK_SHIFT )
		  //CloseWindow();
				 PostMessage( WM_CLOSE, 0, 0 );
			  return;
		 }


		if( iDx > 0 || iDy > 0 )
		 {
			if( iPoW > 0 ) iPoW = 0;
			else if( iPoW < -iDx ) iPoW = -iDx;
			if( iPoH > 0 ) iPoH = 0;
	 else if( iPoH < -iDy ) iPoH = -iDy;

	 if( iPoW != iStaOrgX || iPoH != iStaOrgY )
	  {
		 iStaOrgX = (iDx> 0) ? iPoW:iStaOrgX;
		 iStaOrgY = (iDy > 0) ? iPoH:iStaOrgY;
		 arcDC()->SetViewportOrg( iStaOrgX, iStaOrgY );

		 InvalidateRect( NULL, TRUE );
	  }
		 }
	 }
	else PostMessage( WM_CLOSE, 0, 0 );//CloseWindow();

	//DefaultProcessing();
	Default();
 }


//void TFullScrWindow::EvKeyDown( uint key, uint repeatCount, uint flags )
void TFullScrWindow::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
 {
	PrepareKey( nChar );
 }

//void TFullScrWindow::EvSysKeyDown( uint key, uint repeatCount, uint flags )
void TFullScrWindow::OnSysKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
 {
	PrepareKey( nChar );
 }

//void TFullScrWindow::EvLButtonDown( uint modKeys, TPoint& point )
void TFullScrWindow::OnLButtonDown( UINT nFlags, CPoint point )
 {
	//TWindow::EvLButtonDown( modKeys, point );
    CWnd::OnLButtonDown( nFlags, point );
	::SetCursor( hcHand );
	bFlTrack = TRUE;
 }

void TFullScrWindow::OnRButtonUp( UINT nFlags, CPoint point )
 {
    CWnd::OnRButtonUp( nFlags, point );
	//CloseWindow();
	PostMessage( WM_CLOSE, 0, 0 );
 }

//bool TFullScrWindow::EvSetCursor( HWND hWndCursor, uint hitTest, uint mouseMsg )
BOOL TFullScrWindow::OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message )
 {
	if( bFlTrack == TRUE )
	 {
		::SetCursor( hcHand );
		return 1;
	 }
	else return CWnd::OnSetCursor( pWnd, nHitTest, message );
	 //TWindow::EvSetCursor( hWndCursor, hitTest, mouseMsg );
 }

//void TFullScrWindow::EvLButtonUp( uint modKeys, TPoint& point )
void TFullScrWindow::OnLButtonUp( UINT nFlags, CPoint point )
 {
	//TWindow::EvLButtonUp( modKeys, point );
    CWnd::OnLButtonUp( nFlags, point );
	::SetCursor( ::LoadCursor(NULL, IDC_ARROW) );
	bFlTrack = FALSE;
 }

//void TFullScrWindow::EvRButtonUp( uint modKeys, TPoint& point )
void TFullScrWindow::OnRButtonDown( UINT nFlags, CPoint point )
 {
	//TWindow::EvRButtonUp( modKeys, point );
    CWnd::OnRButtonDown( nFlags, point );
	//CloseWindow();
	PostMessage( WM_CLOSE, 0, 0 );
 }

//void TFullScrWindow::EvMouseMove( uint modKeys, TPoint& point )
void TFullScrWindow::OnMouseMove( UINT nFlags, CPoint point )
 {
	if( nFlags & MK_LBUTTON )
	 {
	   if( arcDIB() )
		{
	      int iPoW = iStaOrgX,
		      iPoH = iStaOrgY;
		  int iDx = iBmpWidth - iBmpViewWidth,
		      iDy = iBmpHigh - iBmpViewHigh;
																  //WM_MOUSEMOVE
	      iPoW -= (TFullScrWindow::iMouseX - point.x);
	      iPoH -= (TFullScrWindow::iMouseY - point.y);

	      if( iDx > 0 || iDy > 0 )
		   {
	         if( iPoW > 0 ) iPoW = 0;
	         else if( iPoW < -iDx ) iPoW = -iDx;

		     if( iPoH > 0 ) iPoH = 0;
		     else if( iPoH < -iDy ) iPoH = -iDy;

		     if( iPoW != iStaOrgX || iPoH != iStaOrgY )
			  {
		        iStaOrgX = (iDx > 0) ? iPoW:iStaOrgX; 
		        iStaOrgY = (iDy > 0) ? iPoH:iStaOrgY;
		        arcDC()->SetViewportOrg( iStaOrgX, iStaOrgY );

		        InvalidateRect( NULL, TRUE );
			  }
		   }
		}
		//msg.Result = 0;
	 }
	else Default();

	TFullScrWindow::iMouseX = point.x;
	TFullScrWindow::iMouseY = point.y;
 }

//void TFullScrWindow::EvPaletteChanged( HWND hWndPalChg )
void TFullScrWindow::OnPaletteChanged( CWnd* pFocusWnd )
 {
	if( pFocusWnd->m_hWnd == m_hWnd ) 
	  CWnd::OnPaletteChanged( pFocusWnd );
	else OnQueryNewPalette();
 }
  
//bool TFullScrWindow::EvQueryNewPalette()
BOOL TFullScrWindow::OnQueryNewPalette()
 {
	HPALETTE HpalOld;
	int i;
	HPALETTE hp = hpalAlias ? hpalAlias:(HPALETTE)pal;
					  //TPalette
	if( !hp ) return 0;

	//if( !hdc ) hdc = ::GetDC( HWindow );
	HpalOld = ::SelectPalette( arcDC()->m_hDC, hp, FALSE );

	i = ::RealizePalette( arcDC()->m_hDC );

	::SelectPalette( arcDC()->m_hDC, HpalOld, FALSE );

	if (i > 0)
	  InvalidateRect( NULL, TRUE);

	return i;
 }

