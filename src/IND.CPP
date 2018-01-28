#include "ind.hpp"

BEGIN_MESSAGE_MAP( TIndicator, CStatic )	
  ON_WM_ERASEBKGND()  
  ON_WM_PAINT()
  ON_WM_CREATE()
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP( TStatusWithLamp, CStatusBar )
  ON_WM_SIZE()
END_MESSAGE_MAP()


static const int iSkipLR = 2;
static const int iSkipInt = 2;


int TIndicator::GetW()
 {
   return 2*iSkipLR + iSkipInt + m_rTxt.Width() + m_rcRound.Width();
 }

void TIndicator::SetStOn( bool bFl, bool bUpdate )
 {
   bool bKey = m_bOn;

   m_bOn = bFl;
   if( m_bOn != bKey && bUpdate == true ) 
	{
	  /*HDC hdc = ::GetDC( m_hWnd );
	  DWORD dw;
	  ::SendMessageTimeout( m_hWnd, WM_PAINT, (WPARAM)hdc, 0,
	    SMTO_NORMAL, 500, &dw );
	  ::ReleaseDC( m_hWnd, hdc );*/
	  if( GetSafeHwnd() )
	   {
	     CClientDC dc( this );
	     IntPaint( dc );
	   }
	}
 }

void TIndicator::Signal( bool bFl )
 {
   bool bKey = m_bOn;
   m_bOn = bFl;
   
   if( bKey != m_bOn && GetSafeHwnd() )
	{
	  CClientDC dc( this );
	  IntPaint( dc );
	}
 }

void TIndicator::CalcRect( CRect& rRect )
 {   
   CClientDC dc( this );
   CFont *pOld = dc.SelectObject( &m_rFnt );
   CSize sz = dc.GetTextExtent( m_csHint );
   dc.SelectObject( pOld );

   rRect.left = 0;
   rRect.top = 0;
   rRect.right = 2*iSkipLR + sz.cx + iSkipInt + sz.cy;
   rRect.bottom = sz.cy;
      
   m_rTxt.left = iSkipLR;
   m_rTxt.top = 0;
   m_rTxt.right = m_rTxt.left + sz.cx;
   m_rTxt.bottom = sz.cy;

   m_rcRound.left = m_rTxt.right + iSkipInt;
   m_rcRound.top = 0;
   m_rcRound.right = m_rcRound.left + float(sz.cy)*0.8;
   m_rcRound.bottom = float(sz.cy)*0.8;

   m_rcRound.OffsetRect( 0, (sz.cy - m_rcRound.Height()) / 2 + 1 );

   m_pArc1.x = m_rcRound.left;
   m_pArc1.y = m_rcRound.bottom;

   m_pArc2.x = m_rcRound.right;
   m_pArc2.y = 0;
 }

void TIndicator::IntPaint( CDC& dc )
 {
   COLORREF coBk = ::GetSysColor(COLOR_BTNFACE);

   CPen penLP( PS_SOLID, 0, coBk );
   CPen penIN( PS_SOLID, 0, coBk );
   CPen penLight( PS_SOLID, 0, RGB(192,192,192) );
   CPen penDark( PS_SOLID, 0, RGB(128,128,128) );
   CBrush br( m_bOn == true ? RGB(255,0,0):RGB(128,0,0) );
   
   CFont *pOldFnt = dc.SelectObject( &m_rFnt );
   
   dc.SetBkMode( OPAQUE );
   dc.SetBkColor( coBk );
   dc.DrawText( m_csHint, m_rTxt, DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOCLIP );

   CBrush *pOldBr = dc.SelectObject( &br );
   dc.Ellipse( m_rcRound );
   dc.SelectObject( pOldBr );

   CPen *pOldPen = (CPen *)dc.SelectStockObject( WHITE_PEN );
   dc.Arc( m_rcRound, m_pArc1, m_pArc2 ); 
   dc.SelectObject( &penDark );
   dc.Arc( m_rcRound, m_pArc2, m_pArc1 ); 
   dc.SelectObject( pOldPen );

   CRect rR2( m_rcRound ); rR2.DeflateRect( 1, 1 );
   dc.SelectObject( &penLight );
   dc.Arc( rR2, m_pArc1, m_pArc2 ); 
   dc.SelectStockObject( BLACK_PEN );
   dc.Arc( rR2, m_pArc2, m_pArc1 ); 

   CRect r; GetClientRect( r );
   pOldPen = dc.SelectObject( &penLP );
   for( int i = iSkipLR; i; --i )
	{
      dc.MoveTo( i-1,0 );
      dc.LineTo( i-1,r.Height() );
	  dc.MoveTo( r.Width()+i-1,0 );
      dc.LineTo( r.Width()+i-1,r.Height() );
	}	
   
   dc.SelectObject( &penIN );
   for( i = iSkipInt; i; --i )
	{
      dc.MoveTo( m_rTxt.right+i-1,0 );
      dc.LineTo( m_rTxt.right+i-1,r.Height() );
	}

   dc.SelectObject( pOldPen );
   dc.SelectObject( pOldFnt );
 }

void TIndicator::OnPaint()
 {
   CPaintDC dc( this );
   IntPaint( dc );
 }

int TIndicator::OnCreate( LPCREATESTRUCT lpC )
 {
   if( CStatic::OnCreate(lpC) == -1 ) return -1; 

   CRect r;
   CalcRect( r );

   SetWindowPos( NULL, 0, 0, GetW(), m_rTxt.Height(), 
	 SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOREDRAW );

   return 0;
 }

BOOL TIndicator::OnEraseBkgnd( CDC* pDC )
 {
   return TRUE;
 }

TConnectToItem::~TConnectToItem()
 {
 }

TStatusWithLamp::~TStatusWithLamp()
 {
   list<TConnectToItem*>::iterator itSta( m_lstCnct.begin() );
   list<TConnectToItem*>::iterator itEnd( m_lstCnct.end() );

   for( ; itSta != itEnd; ++itSta )
	if( *itSta )
	 {
       delete *itSta;
	   *itSta = NULL;
	 }

	m_lstCnct.clear();
 }

void TStatusWithLamp::OnSize( UINT nType, int cx, int cy )
 {
   CStatusBar::OnSize( nType, cx, cy );
   //CRect rWnd; GetClientRect( rWnd );

   if( !m_lstCnct.size() ) return;

   list<TConnectToItem*>::iterator itSta( m_lstCnct.begin() );
   list<TConnectToItem*>::iterator itEnd( m_lstCnct.end() );
       

   for( ; itSta != itEnd; ++itSta )
	{
      list<TIndicator*>::iterator itSta2( (*itSta)->m_Lst.begin() );
      list<TIndicator*>::iterator itEnd2( (*itSta)->m_Lst.end() );
    
	  CRect r; GetItemRect( (*itSta)->m_iId, r );
	  //if( !r.Width() ) continue;
	  //if( r.right > rWnd.right ) continue;

	  CRect r2; 
      (*itSta2)->GetWindowRect( r2 );
      int iTop = (r.Height() - r2.Height()) / 2;   
	  
	  int iW = 0;
	  for( ; itSta2 != itEnd2; ++itSta2 )
	    iW += (*itSta2)->GetW() + 1;	   

	  itSta2 = (*itSta)->m_Lst.begin();
      itEnd2 = (*itSta)->m_Lst.end();

	  int iDelta;
	  r.left += (iDelta=(r.Width() - iW)) / 2;
	  if( iDelta < 0 ) continue;
      for( ; itSta2 != itEnd2; ++itSta2 )
	   {
	     (*itSta2)->SetWindowPos( NULL, r.left, r.top+iTop, 0, 0, 
		   SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOREDRAW );
		 CRect rr; (*itSta2)->GetWindowRect( rr );
		 r.OffsetRect( rr.Width()+1, 0 );
	   }
	}
 }
