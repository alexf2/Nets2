#define TBWCC_DLL
#include "bwcc.hpp"
#define MUMENU_DLL
#include "mymenu.hpp"

#define TEXCEPT_DLL
#include "texcept.hpp"

#include "bwcc.rh"

//#include "vs.rh"

#include <afxcmn.h>

IMPLEMENT_DYNCREATE( TMSStyleTooolBar, CToolBar )
IMPLEMENT_DYNCREATE( TMSItem, CButton )

BEGIN_MESSAGE_MAP( TMSStyleTooolBar, CToolBar )
	//ON_WM_SHOWWINDOW()	
	//ON_WM_WINDOWPOSCHANGING()
	ON_WM_CREATE()
	ON_WM_SIZE()	

END_MESSAGE_MAP()


BEGIN_MESSAGE_MAP( TMSItem, CButton )
	//ON_WM_SHOWWINDOW()	
	//ON_WM_WINDOWPOSCHANGING()
	ON_WM_ERASEBKGND()	
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_DESTROY()

	ON_WM_CREATE()

	ON_WM_KEYUP()

END_MESSAGE_MAP()


TIcon::TIcon()
 {
   m_hIcon = NULL;
   m_uiRC = 0;
 }

TIcon::~TIcon()
 {
   if( m_hIcon )
	{
      DestroyIcon( m_hIcon );
	  m_hIcon = NULL;
	}
 }

void TIcon::Load( UINT uiRC, HINSTANCE hiRC )
 {
   if( uiRC == m_uiRC ) return;

   if( m_hIcon ) DestroyIcon( m_hIcon );
   
   m_hIcon = LoadIcon( hiRC, MAKEINTRESOURCE(uiRC) );
   m_uiRC = uiRC;
 }


int TMSItem::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const
 {   
   if( m_uiCommand == ID_SEPARATOR ) return -1;

   CRect r;
   GetClientRect( r );
   if( r.PtInRect(point) == false ) return -1;

   //pTI->uFlags = TTF_IDISHWND | TTF_SUBCLASS;
   pTI->uFlags = TTF_IDISHWND;
   pTI->hwnd = m_pPar->m_hWnd;
   pTI->uId = (UINT)m_hWnd;
   pTI->rect = r;
   //pTI->hinst = AfxGetResourceHandle();
   pTI->hinst = 0;
   //pTI->lpszText = MAKEINTRESOURCE( m_uiCommand );
   pTI->lpszText = _strdup( (LPCSTR)m_strHint );

   return 1;
 }


BOOL TMSItem::OnEraseBkgnd( CDC * )
 {
   return TRUE;
 }

struct TDrawData
 {
   TMSItem::TMS_Type msType; 
   TMSItem* pMSIt;   
 };
BOOL CALLBACK EXPORT MSDrawFunc( HDC, LPARAM, int );

const float TMSItem::fFildV = 4;
const float TMSItem::fFildH = 4;
const int TMSItem::iSkipBmpTOStr = 1;

TIcon TMSItem::m_iconOK;  
TIcon TMSItem::m_iconCancel;  

BOOL CALLBACK EXPORT MSDrawFunc( HDC hdc, LPARAM lp, int )
 {
   TDrawData *pDta = (TDrawData *)lp;

   CDC dc; dc.Attach( hdc );
   CDC dcMem; dcMem.CreateCompatibleDC( &dc );
   
   CBitmap *pOldBmp = dcMem.SelectObject( &(pDta->pMSIt->m_bmp) );

   /*dcMem.SetMapMode( dc.GetMapMode() );
   dcMem.SetViewPortOrg( dc.GetViewPortOrg() );
   dcMem.SetWindowOrg( dc.GetWindowOrg() );*/

   BITMAP bm;
   ::GetObject( (HBITMAP)pDta->pMSIt->m_bmp, sizeof(BITMAP), &bm );

   //CPoint pt( (pDta->pMSIt->sz.cx - bm.bmWidth) / 2, float(pDta->pMSIt->sz.cy) * TMSItem::fFildV );!!!!!!
   CPoint pt( (pDta->pMSIt->sz.cx - bm.bmWidth) / 2, TMSItem::fFildV );
   dc.DPtoLP( &pt );   
   int iDx, iDy;
   if( pDta->msType == TMSItem::MS_Pressed )
	{
      iDx = 2; iDy = 2;
	}
   else
	{
      iDx = 0; iDy = 0;
	}
   dc.BitBlt( pt.x+iDx, pt.y+iDy, bm.bmWidth, bm.bmHeight, &dcMem, 0, 0, SRCCOPY );

   //CSize sz = dc.GetTextExtent( m_str );   
   
   UINT uiOldAln = dc.SetTextAlign( TA_CENTER | TA_BOTTOM );
   int iOldBk = dc.SetBkMode( TRANSPARENT );
   int iLbl =  pDta->pMSIt->m_str.Find( '~' );
   int iLbl2 =  pDta->pMSIt->m_str.ReverseFind( '~' );
   int y = pDta->pMSIt->sz.cy - TMSItem::fFildV + iDy;
   if( iLbl != -1 )
	{
      CString strLeft = pDta->pMSIt->m_str.Left( iLbl );
	  CString strRight = pDta->pMSIt->m_str.Right( pDta->pMSIt->m_str.GetLength() - iLbl2 - 2 );
	  CString strCh( pDta->pMSIt->m_str.Mid(iLbl+1, iLbl2-iLbl-1) );

	  CString strFull = strLeft + strCh + strRight;
	  CString strFull2 = strLeft + strCh;

	  CSize ext = dc.GetTextExtent( strFull );
	  CSize ext2 = dc.GetTextExtent( strLeft );
	  CSize ext3 = dc.GetTextExtent( strFull2 );

	  int x = (pDta->pMSIt->sz.cx - ext.cx) / 2 + iDx;

	  dc.SetTextAlign( TA_LEFT | TA_BOTTOM );
	  dc.TextOut( x, y, strLeft );
	  //dc.SetTextAlign( TA_LEFT | TA_BOTTOM | TA_UPDATECP );
	  COLORREF refOld = dc.SetTextColor( RGB(255,0,0) );
	  dc.TextOut( x+ext2.cx, y, strCh );
	  dc.SetTextColor( refOld );
	  dc.TextOut( x+ext3.cx, y, strRight );	  
	}
   else
     dc.TextOut( pDta->pMSIt->sz.cx / 2 + iDx, 
	             y, 
			     pDta->pMSIt->m_str 
			   );

   if( pDta->pMSIt->m_bPushBtn == false )
	{
      if( !(HICON)TMSItem::m_iconOK )
	   {
	     TBWCCLocalozeRC local;
         local.Localize();

         TMSItem::m_iconOK.Load( IDI_OK, AfxGetResourceHandle() );
		 TMSItem::m_iconCancel.Load( IDI_CANCEL, AfxGetResourceHandle() );		 
	   }
	  HICON hi = NULL;
	  switch( pDta->msType )
	   {
	     case TMSItem::MS_Normal:
		   hi = (HICON)TMSItem::m_iconCancel;
		   break;

		 case TMSItem::MS_Selected:
		   hi = (HICON)TMSItem::m_iconCancel;
		   break;

		 case TMSItem::MS_Grayed:
		   break;

		 case TMSItem::MS_Pressed:
		   hi = (HICON)TMSItem::m_iconOK;
		   break;		  
	   };
      dc.DrawIcon( 0, 0, hi );  
	}

   dc.SetBkMode( iOldBk );
   dc.SetTextAlign( uiOldAln );   

   dcMem.SelectObject( pOldBmp );
   dcMem.DeleteDC();
   dc.Detach();

   return TRUE;
 }


CBrush TMSItem::m_MaskBrush;
CBrush TMSItem::m_MaskBrush2;
 

TMSItem::TMSItem(): CButton()
 {
   m_bFlChecked = false;

   m_uiCommand = 0;
   m_bChaced = false;
   m_pPar = NULL;

   m_bPushBtn = true;

   m_bMoved = false;

   m_uiTimer = 0;   
   m_bFlNestedTimer = false;
   

   m_bFlAtCursor = false;

   if( !(HBRUSH)TMSItem::m_MaskBrush )
	{
	  CBitmap bmp, bmp2;

	  TBWCCLocalozeRC local;
      local.Localize();

	  bmp.LoadBitmap( MAKEINTRESOURCE(IDB_MASK) );
	  m_MaskBrush.CreatePatternBrush( &bmp );

	  bmp2.LoadBitmap( MAKEINTRESOURCE(IDB_MASK2) );
	  m_MaskBrush2.CreatePatternBrush( &bmp2 );
	}
 }

TMSItem::~TMSItem()
 {
 }

void TMSItem::SetCheckMy( bool bFl )
 { 
   bool bKey = m_bFlChecked;
   m_bFlChecked = bFl;

   if( bKey != m_bFlChecked ) InvalidateRect( NULL );
 }

int TMSItem::OnCreate( LPCREATESTRUCT lpCreateStruct )
 {
   int iRes = CButton::OnCreate( lpCreateStruct );
   if( iRes == -1 ) return -1;

   EnableToolTips( TRUE );

   return 0;
 }

void TMSItem::OnDestroy()
 {
   if( m_uiTimer )
	{
      KillTimer( m_uiTimer );
	  m_uiTimer = 0;
	}

   CButton::OnDestroy();
 }

void TMSItem::Init( LPSTR lpStr, UINT uiIdRcBmp, UINT m_uiCommand_, bool bFlRight, bool bPushBtn )
 {
   m_uiCommand = m_uiCommand_;
   m_str = lpStr;
   m_bPushBtn = bPushBtn;
   if( m_uiCommand_ != ID_SEPARATOR ) 
	{
	  m_bmp.LoadBitmap( uiIdRcBmp );
	  m_strHint.LoadString( m_uiCommand_ );
	  int iInd = m_strHint.Find( '\n' );
	  if( iInd != -1 ) m_strHint = m_strHint.Right( m_strHint.GetLength() - iInd - 1 );
	}

   m_bRight = bFlRight;     
 }

void TMSItem::OnTimer( UINT nIDEvent )
 {
   if( nIDEvent == 1 )
	{
	  CPoint pt;
	  GetCursorPos( &pt );
	  ScreenToClient( &pt );

	  CRect r; GetClientRect( r );
	  if( r.PtInRect(pt) == false ) 
	   {
         m_bFlAtCursor = false;
	     if( m_uiTimer )
		  {
            KillTimer( m_uiTimer );
	        m_uiTimer = 0;

			InvalidateRect( NULL );		 
		  }
	     //ReleaseCapture();		 	     
	   }
	}

   if( m_bFlNestedTimer == false )
     CButton::OnTimer( nIDEvent );
 }



void TMSItem::OnMouseMove( UINT nFlags, CPoint point )
 {
   CRect r; GetClientRect( r );

   if( m_bFlAtCursor == false )
    {
	  /*CWnd *pWnd = CWnd::GetFocus();
	  if( pWnd && pWnd->m_hWnd == AfxGetApp()->m_pMainWnd->m_hWnd ) 
	   {*/
         m_pPar->CheckOff( this );

         m_bFlAtCursor = true;
	     m_uiTimer = SetTimer( 1, 50, NULL );
	     //SetCapture();
	     InvalidateRect( NULL );
	   //}
    }
   else if( m_bFlAtCursor == true && r.PtInRect(point) == false )
    {
      m_bFlAtCursor = false;
	  if( m_uiTimer )
	   {
         KillTimer( m_uiTimer );
	     m_uiTimer = 0;
	   }
	  //ReleaseCapture();
	  InvalidateRect( NULL );
    }

   CButton::OnMouseMove( nFlags, point );
 }
/*
BOOL TMSItem::OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message )
 {
   //m_bFlAtCursor
   if( nHitTest == HTCLIENT )
	{
	  if( m_bFlAtCursor == false )
	   {
         m_bFlAtCursor = true;
		 SetCapture();
		 InvalidateRect( NULL );
	   }
	}
   else if( m_bFlAtCursor )
	{
      m_bFlAtCursor = false;
	  ReleaseCapture();
	  InvalidateRect( NULL );
	}

   return CButton::OnSetCursor( pWnd, nHitTest, message );
 }*/

void TMSItem::DrawItem( LPDRAWITEMSTRUCT lpDr )
 {   
   //int iID = IDtoIND(LOWORD(lpDr->itemID));

   //if( lpDr->itemAction & ODA_DRAWENTIRE )
	//{
      if( m_uiCommand == ID_SEPARATOR )
	   {
         CRect r;
		 GetClientRect( r );
		 CDC *pDc = CDC::FromHandle( lpDr->hDC );
         pDc->FillSolidRect( &r, GetSysColor(COLOR_BTNFACE) );

		 CPen penGray( PS_SOLID, 0, GetSysColor(COLOR_BTNSHADOW) );
		 CPen *pOld = (CPen *)pDc->SelectStockObject( WHITE_PEN );
		 int x = r.Width() / 2 - 2;
		 int y = r.Height();
		 pDc->MoveTo( x, 0 );
		 pDc->LineTo( x, y );

		 x++;
		 pDc->SelectObject( &penGray );
		 pDc->MoveTo( x, 0 );
		 pDc->LineTo( x, y );

		 x++;
		 pDc->SelectStockObject( WHITE_PEN );
		 pDc->MoveTo( x, 0 );
		 pDc->LineTo( x, y );

		 x++;
		 pDc->SelectObject( &penGray );
		 pDc->MoveTo( x, 0 );
		 pDc->LineTo( x, y );

		 pDc->SelectObject( pOld );

		 return;
	   }

      bool bTst = m_bPushBtn == false && IsChecked() == true; 

      if( m_bFlAtCursor == true && 
	      !((lpDr->itemState & (ODS_DISABLED|ODS_SELECTED)) || bTst==true) )
	   {
         Draw( lpDr, TMSItem::MS_Selected, m_pPar->GetFntPtr() );
		 return;
	   }

      if( lpDr->itemState & ODS_DISABLED ) Draw( lpDr, TMSItem::MS_Grayed, m_pPar->GetFntPtr() );
      else
	   {
	     if( bTst == true )
		  {		 
		    Draw( lpDr, TMSItem::MS_Pressed, m_pPar->GetFntPtr() );
		  }
		 else
		  {
	        if( lpDr->itemState & ODS_SELECTED ) Draw( lpDr, TMSItem::MS_Pressed, m_pPar->GetFntPtr() );
	        else if( lpDr->itemState & ODS_FOCUS ) Draw( lpDr, TMSItem::MS_Selected, m_pPar->GetFntPtr() );
	        else Draw( lpDr, TMSItem::MS_Normal, m_pPar->GetFntPtr() );
		  }
	   }
	//}   
 }

void TMSItem::Mask( CRect& rR, CDC *pDC )
 {
   int iOldROP = pDC->SetROP2( R2_MASKPEN );   
   m_MaskBrush.UnrealizeObject();
   CBrush *pBrushOld = pDC->SelectObject( &m_MaskBrush );
   CRgn rgn; rgn.CreateRectRgnIndirect( rR );
   pDC->PaintRgn( &rgn );

   pDC->SetROP2( R2_MERGEPEN );
   m_MaskBrush2.UnrealizeObject();
   pDC->SelectObject( &m_MaskBrush2 );
   //pDC->FillRect( rR, &m_MaskBrush2 );
   pDC->PaintRgn( &rgn );

   pDC->SetROP2( iOldROP );
   pDC->SelectObject( pBrushOld );
 }

void TMSItem::DrawInDC( CDC *pDc, TMSItem::TMS_Type msType )
 {
   //CBrush brBkGnd;//( GetSysColor(COLOR_GRAYTEXT) );
   //brBkGnd.CreateHatchBrush( HS_BDIAGONAL, RGB(255,255,255) );

   CRect r( 0, 0, sz.cx-1, sz.cy-1 );   
   /*if( msType == MS_Pressed )
     pDc->FillSolidRect( &r, RGB(200, 200, 200) );
   else*/
     pDc->FillSolidRect( &r, GetSysColor(COLOR_BTNFACE) );
   //LPARAM lpData, int nCount, int x, int y, int nWidth, int nHeight     

   TDrawData dta;
   dta.msType = msType;
   dta.pMSIt = this;
   

   if( msType == MS_Grayed )
	{	  
      //pDc->GrayString( &brBkGnd, MSDrawFunc, (LPARAM)&dta, -1, 0, 0, sz.cx, sz.cy );
	  MSDrawFunc( pDc->m_hDC, (LPARAM)&dta, 0 );	
	  Mask( r, pDc );
	  //
	}
   else
	{
      if( msType == MS_Normal )
	   {	 
         MSDrawFunc( pDc->m_hDC, (LPARAM)&dta, 0 );	
	   }
	  else if( msType == MS_Selected )
	   {
	     MSDrawFunc( pDc->m_hDC, (LPARAM)&dta, 0 );

		 //CRect r(0, 0, sz.cx - 1, sz.cy - 1 );
         //pDc->DrawEdge( r ,, BF_TOPLEFT |BF_BOTTOMRIGHT );

		 CPen *pOld = (CPen *)pDc->SelectStockObject( WHITE_PEN );
		 pDc->MoveTo( 0, 0 );
		 pDc->LineTo( sz.cx, 0 );

		 pDc->MoveTo( 0, 0 );
		 pDc->LineTo( 0, sz.cy );

		 CPen penGray( PS_SOLID, 0, GetSysColor(COLOR_BTNSHADOW) );
		 pDc->SelectObject( penGray );
		 pDc->MoveTo( sz.cx-2, sz.cy-2 );
		 pDc->LineTo( sz.cx-2, 0 );

		 pDc->MoveTo( sz.cx-2, sz.cy-2 );
		 pDc->LineTo( 0, sz.cy-2 );

		 pDc->SelectObject( pOld );
	   }
	  else
	   {
         MSDrawFunc( pDc->m_hDC, (LPARAM)&dta, 0 );		

		 CPen penGray( PS_SOLID, 0, GetSysColor(COLOR_BTNSHADOW) );
         CPen *pOld = pDc->SelectObject( &penGray );
		 
		 pDc->MoveTo( 0, 0 );
		 pDc->LineTo( sz.cx, 0 );

		 pDc->MoveTo( 0, 0 );
		 pDc->LineTo( 0, sz.cy );

		 pDc->SelectStockObject( WHITE_PEN );		 
		 pDc->MoveTo( sz.cx-2, sz.cy-2 );
		 pDc->LineTo( sz.cx-2, 0 );

		 pDc->MoveTo( sz.cx-2, sz.cy-2 );
		 pDc->LineTo( 0, sz.cy-2 );

		 pDc->SelectObject( pOld );
	   }
	}
   
 }

void TMSItem::Draw( LPDRAWITEMSTRUCT lpDr, TMSItem::TMS_Type msType, CFont *pFnt )
 {   
   CDC dcDC;   
   dcDC.Attach( lpDr->hDC );
     //dcDC.MoveTo( lpDr->rcItem.left, 0 ); dcDC.LineTo( lpDr->rcItem.left+25, lpDr->rcItem.top+25 );
 
   CRect rr;
	GetWindowRect(rr);
	m_pPar->ScreenToClient(rr);

   CDC dcMem;
   dcMem.CreateCompatibleDC( &dcDC );
 
   dcMem.SetMapMode( dcDC.GetMapMode() );
   dcMem.SetViewportOrg( dcDC.GetViewportOrg() );
   dcMem.SetWindowOrg( dcDC.GetWindowOrg() );

   if( m_bChaced == false )
	{
	  m_bmpNormal.CreateCompatibleBitmap( &dcDC, sz.cx, sz.cy );
	  m_bmpSelected.CreateCompatibleBitmap( &dcDC, sz.cx, sz.cy );
	  m_bmpGrayed.CreateCompatibleBitmap( &dcDC, sz.cx, sz.cy );	  
	  m_bmpPressed.CreateCompatibleBitmap( &dcDC, sz.cx, sz.cy );	  

	  CBitmap *pBmpOld = dcMem.SelectObject( &m_bmpNormal );
      CFont *pOldFnt = dcMem.SelectObject( pFnt );

	  DrawInDC( &dcMem, TMSItem::MS_Normal );
	  dcMem.SelectObject( &m_bmpSelected );
	  DrawInDC( &dcMem, TMSItem::MS_Selected );
	  dcMem.SelectObject( &m_bmpGrayed );
	  DrawInDC( &dcMem, TMSItem::MS_Grayed );
	  dcMem.SelectObject( &m_bmpPressed );
	  DrawInDC( &dcMem, TMSItem::MS_Pressed );

	  dcMem.SelectObject( pOldFnt );
	  dcMem.SelectObject( pBmpOld );
	  	      
      m_bChaced = true;
	}

   CBitmap *pBmp;
   if( msType == TMSItem::MS_Normal ) pBmp = &m_bmpNormal;
   else if( msType == TMSItem::MS_Selected ) pBmp = &m_bmpSelected;
   else if( msType == TMSItem::MS_Grayed) pBmp = &m_bmpGrayed;
   else pBmp = &m_bmpPressed;


   //pBmp = &m_bmpGrayed;

   CBitmap *pOldBmp = dcMem.SelectObject( pBmp );   

   CPoint pt(0, 0);
   dcMem.DPtoLP( &pt );
   BITMAP bm;
   int iRet = ::GetObject( (HBITMAP)*pBmp, sizeof(BITMAP), &bm );
   dcDC.BitBlt( lpDr->rcItem.left, lpDr->rcItem.top, bm.bmWidth, bm.bmHeight, &dcMem, pt.x, pt.y, SRCCOPY );

   //dcDC.MoveTo(0,10); dcDC.LineTo(bm.bmWidth-1,10);

   dcMem.SelectObject( pOldBmp );
   /*char cB[256]; sprintf(cB, "ffb%u.bmp", 
   CFile f( cB, CFile::modeReadWrite );
   CArchive*/
	//CArchive( CFile* pFile, UINT nMode, int nBufSize = 4096, void* lpBuf = NULL )


   /*TDrawData msd;
   msd.msType = msType;
   msd.pMSIt = this;
 
   MSDrawFunc( lpDr->hDC, (void*)&msd, 0 );*/

   dcMem.DeleteDC();
   dcDC.Detach();
 }

/*enum TMS_Type { MS_Normal, MS_Selected, MS_Grayed };


protected:
  CString m_str;
  CBitmap m_bmp;
  UINT m_uiCommand;    

  CSize sz;

private:
  CBitmap m_bmpNormal;
  CBitmap m_bmpSelected;
  CBitmap m_bmpGrayed;
  bool m_bChaced;*/

void TMSStyleTooolBar::OnUpdateCmdUI( CFrameWnd* pTarget, BOOL bDisableIfNoHndler )
 {
   UpdateDialogControls( pTarget, bDisableIfNoHndler );
 }

/*BOOL TMSStyleTooolBar::Create( CWnd *pPar, DWORD dwStyle, UINT uiId )
 {
   LPCSTR lpName = AfxRegisterWndClass( 0,
	  AfxGetApp()->LoadStandardCursor(IDC_ARROW),
	  (HBRUSH)GetStockObject(GRAY_BRUSH) );
   if( !lpName ) return FALSE;

   CRect r(0,0,300,100);
   return CControlBar::Create( lpName, "", 
	 dwStyle|WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN, r, pPar, uiId );
	//LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
 }*/

void TMSStyleTooolBar::CheckOff( TMSItem* pIt )
 {
   vector<TMSItem*>::iterator it( m_Dta.begin() );
   vector<TMSItem*>::iterator itEnd( m_Dta.end() );
   
   for( ; it != itEnd; ++it )
	{
      if( ((void*)(*it)) == (void*)pIt ) continue;

	  (*it)->m_bFlNestedTimer = true;
	  (*it)->OnTimer( 1 );
	  (*it)->m_bFlNestedTimer = false;

	  if( (*it)->GetState() & 0x8 )
	   {
	     if( !CWnd::GetCapture() )
           (*it)->WindowProc( WM_KILLFOCUS, (WPARAM)pIt->m_hWnd, 0 );
         //(*it)->InvalidateRect( NULL );
	   }
	}
 }

TMSStyleTooolBar::TMSStyleTooolBar( CWnd *pParent_ ): CToolBar()
 {
   pParent = pParent_;
 }

TMSStyleTooolBar::~TMSStyleTooolBar()
 {   
   int iS = m_Dta.size();
   for( int i = 0; i < iS; ++i )
	 if( m_Dta[i] )
	  {
	    //if( m_Dta[i]->m_uiCommand != ID_SEPARATOR )
	      //m_ToolTip.DelTool( m_Dta[i] );
        delete m_Dta[i]; m_Dta[i] = NULL;
	  }
 }



void TMSStyleTooolBar::Init( bool bFlEqualWidth, bool bFlEqualHigh )
 {   
   //CWindowDC dc( CWnd::GetDesktopWindow() );
   CWindowDC dc( pParent );
   CFont *pOldFnt = dc.SelectObject( m_arcFnt() );

   vector<TMSItem*>::iterator it( m_Dta.begin() );
   vector<TMSItem*>::iterator itEnd( m_Dta.end() );
   int iMaxW = 0, iMaxH = 0;   
   CSize szSub = dc.GetTextExtent( "~~" );
   for( ; it != itEnd; ++it )
	{
	  CSize szTxt;
	  BITMAP bm;
	  CSize szImg;

	  if( (*it)->m_uiCommand != ID_SEPARATOR )
	   {
	     szTxt = dc.GetTextExtent( (*it)->m_str );   		 
		 if( (*it)->m_str.Find('~') != -1 )
		   szTxt.cx -= szSub.cx; 		 
		  
	     ::GetObject( (*it)->m_bmp, sizeof(bm), &bm );
	     szImg.cx = bm.bmWidth; szImg.cy = bm.bmHeight;
	   }
	  else
	   {
         szImg.cx = szTxt.cx = 6;
		 szImg.cy = szTxt.cy = 8;
	   }

	  (*it)->sz.cx = max( szImg.cx, szTxt.cx );
	  (*it)->sz.cy = szImg.cy + szTxt.cy + TMSItem::iSkipBmpTOStr;

	  (*it)->sz.cx += 2.0*TMSItem::fFildH;
	  (*it)->sz.cy += 2.0*TMSItem::fFildV;

	  if( (*it)->sz.cx > iMaxW ) iMaxW = (*it)->sz.cx;
	  if( (*it)->sz.cy > iMaxH ) iMaxH = (*it)->sz.cy;
	}   

   if( bFlEqualWidth == true ) 
	{
      it = m_Dta.begin();
	  for( ; it != itEnd; ++it )
	    (*it)->sz.cx = iMaxW;
	}

   if( bFlEqualHigh == true ) 
	{
      it = m_Dta.begin();
	  for( ; it != itEnd; ++it )
	    (*it)->sz.cy = iMaxH+1;
	}

   m_iMaxW = iMaxW;
   m_iMaxH = iMaxH;

   dc.SelectObject( pOldFnt );
   
   RecalcPosAll();
 }

void TMSStyleTooolBar::RecalcPosAll()
 {
   vector<TMSItem*>::iterator it( m_Dta.begin() );
   vector<TMSItem*>::iterator itEnd( m_Dta.end() );

   CRect r( 0, m_cyTopBorder - 1, 0, 0 );
   
   for( ; it != itEnd; ++it )
	{
	  r.right = r.left + (*it)->sz.cx - 1;
      r.bottom = r.top + (*it)->sz.cy - 1;   

      (*it)->m_Rect = r;
	  int iW = r.Width();

	  r.left += iW + 1;
	  (*it)->m_bMoved = false;
	}

   m_iWidthToolbar = r.right;

   //GetToolBarCtrl().HideButton( 1000, TRUE );
   //ShiftRight();
 }

//void TMSItem::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags )
void TMSItem::OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags )
 {
   CButton::OnKeyUp( nChar, nRepCnt, nFlags );
   if( LOBYTE(nChar) == 27 )
	{
      CWnd *p = AfxGetApp()->m_pMainWnd;
	  if( p ) 
	   {
	     p->SendMessage( WM_ACTIVATE, 
		   MAKEWPARAM(WA_ACTIVE, p->IsIconic()), 
		   (LPARAM)m_hWnd );
	   }
	}
 }

void TMSStyleTooolBar::OnSize( UINT nType, int cx, int cy )
 {
   CToolBar::OnSize( nType, cx, cy ); 
   if( ShiftRight() == false ) return;

   vector<TMSItem*>::iterator it( m_Dta.begin() );
   vector<TMSItem*>::iterator itEnd( m_Dta.end() );
   
   for( ; it != itEnd; ++it )
	{
	  CRect r( (*it)->m_Rect );

      (*it)->SetWindowPos( NULL, r.left, r.top,
	                       r.Width(), r.Height(), SWP_NOACTIVATE | SWP_NOZORDER );	  

	}
 }

bool TMSStyleTooolBar::FindMoved() 
 {
   vector<TMSItem*>::iterator it( m_Dta.begin() );
   vector<TMSItem*>::iterator itEnd( m_Dta.end() );   
   for( ; it != itEnd; ++it )
	 if( (*it)->m_bMoved == true ) return true;

   return false;
 }

bool TMSStyleTooolBar::ShiftRight()
 {      
   CRect rT; GetClientRect( rT );
   //CRect r( rT.right - , m_cyTopBorder - 1, rT.right, 0 );
   int x = rT.right;
   if( x <=0 ) return false;   

   if( m_iWidthToolbar >= rT.Width() ) 
	{
	  if( FindMoved() == true ) 
	   {
	     RecalcPosAll();
	     return true;
	   }
	  else return false;
	}   
        
   for( int i = m_Dta.size() - 1; i; --i )
   	{
      if( m_Dta[i]->m_bRight == false ) break;      

	  int iW = m_Dta[i]->m_Rect.Width();
	  x -= iW + 1;
      m_Dta[i]->m_Rect.left = x; 
	  m_Dta[i]->m_Rect.right = x + iW; 
      m_Dta[i]->m_bMoved = true;
	}

  return true;
 }

int TMSStyleTooolBar::IDtoIND( int id ) 
 {     
   int iS = m_Dta.size();
   for( int i = 0; i < iS; ++i )
     if( m_Dta[i]->m_uiCommand == id ) return i;	

   return 0;
 }

BOOL TMSStyleTooolBar::PreCreateWindow( CREATESTRUCT& cs )
 {   
   if( !CToolBar::PreCreateWindow(cs) ) return FALSE;

   cs.style |= WS_CLIPCHILDREN;
   
   return TRUE;
 }


int TMSStyleTooolBar::OnCreate( LPCREATESTRUCT lpCreateStruct )
 {   
   if( CToolBar::OnCreate(lpCreateStruct) == -1 )
	  return -1;	   

   //m_ToolTip.Create( this );   

   int iS = m_Dta.size();
   TAutoMemoryRc<UINT*> arcBtn = new UINT[iS];
   for( int i = 0; i < iS; ++i )	
	{
	  if( m_Dta[i]->m_uiCommand != ID_SEPARATOR )
	   {
         m_Dta[i]->Create( m_Dta[i]->m_str, 
		  /*TBBS_BUTTON | */WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_OWNERDRAW | 
		  ((m_Dta[i]->m_bPushBtn==false) ? BS_AUTOCHECKBOX:BS_PUSHBUTTON), 
	                       m_Dta[i]->m_Rect,
	                       this, m_Dta[i]->m_uiCommand );	 
	
		 //m_ToolTip.AddTool( this, m_Dta[i]->m_uiCommand, m_Dta[i]->m_Rect, m_Dta[i]->m_uiCommand );		 
	   }
	  else
	    m_Dta[i]->Create( m_Dta[i]->m_str, 
	                      /*TBBS_SEPARATOR |*/ WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 
	                      m_Dta[i]->m_Rect,
	                      this, m_Dta[i]->m_uiCommand );
	  arcBtn()[i] = m_Dta[i]->m_uiCommand;	  
	}

   UINT ui = 1000; 
   CSize szBtn( 25, m_iMaxH ); CSize szImg( 16, 15 );
   SetSizes( szBtn, szImg );
   SetButtons( &ui, 1 );

   SetHeight( m_iMaxH + m_cyTopBorder + m_cyBottomBorder );  

   //GetToolBarCtrl().HideButton( ui, TRUE );
   //CWnd *pIt = GetDlgItem( ui );
   //if( pIt ) pIt->ModifyStyle( WS_VISIBLE, 0 );
   
   //GetToolBarCtrl().SetToolTips( &m_ToolTip );   
   //m_ToolTip.Activate( TRUE );
   //m_ToolTip.SetDelayTime( 50 );

   return 0;
 }



