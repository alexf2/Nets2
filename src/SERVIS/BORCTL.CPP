#define TBORCTL_DLL
#include "borctl.hpp"

IMPLEMENT_DYNCREATE( TBorBtn, CButton )
IMPLEMENT_DYNCREATE( TBWCCPropertySheet, CPropertySheet )

BEGIN_MESSAGE_MAP( TBorBtn, CButton )
  ON_WM_CREATE()
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP( TBWCCPropertySheet, CPropertySheet )
  ON_WM_CREATE()
  ON_WM_DESTROY()
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP( CPropPageTooltips, CPropertyPage )
  ON_WM_CREATE()
END_MESSAGE_MAP()




BOOL TBorBtn::PreCreateWindow( CREATESTRUCT& cs )
 {
   static const char lpBorBtnName[] = "BorBtn";

   cs.lpszClass = lpBorBtnName;
   return CButton::PreCreateWindow( cs );
 }

TBWCCPropertySheet::TBWCCPropertySheet( LPCTSTR pszCaption, CWnd *pParentWnd, UINT iSelectPage ):
   CPropertySheet( pszCaption, pParentWnd, iSelectPage )
 {
   m_psh.dwFlags |= PSH_NOAPPLYNOW;
 }

TBWCCPropertySheet::~TBWCCPropertySheet()
 {
 }

void TBWCCPropertySheet::OnDestroy()
 {
   m_iActiveIndex = GetActiveIndex();   
   CPropertySheet::OnDestroy();
 }

BOOL TBWCCPropertySheet::PreCreateWindow( CREATESTRUCT& cs )
 {
   cs.style |= WS_SYSMENU;

   return CPropertySheet::PreCreateWindow( cs );
 }

/*int TBWCCPropertySheet::OnCreate( LPCREATESTRUCT lpC )
 {
   lpC->style |= WS_SYSMENU;

   return CPropertySheet::OnCreate( lpC );
 }*/

BOOL TBWCCPropertySheet::OnInitDialog()
 {
   const float fSpc = 1.0/5.0;
   
   const int iBWCCW_ = 43; 
   const int iBWCCH_ = 25;   


   CRect rDlg( 0, 0, iBWCCW_, iBWCCH_ );
   ::MapDialogRect( m_hWnd, rDlg );
   int iBWCCW = rDlg.right;
   int iBWCCH = rDlg.bottom;
   
   CPropertySheet::OnInitDialog();   

   HWND hwOK = ::GetDlgItem( m_hWnd, IDOK );
   HWND hwCancel = ::GetDlgItem( m_hWnd, IDCANCEL );
   HWND hwHelp = ::GetDlgItem( m_hWnd, IDHELP );

   CRect rBtn; ::GetWindowRect( hwOK, rBtn );  
   CRect rWnd; GetWindowRect( rWnd );

   ::DestroyWindow( hwOK );
   ::DestroyWindow( hwCancel );
   if( hwHelp ) ::DestroyWindow( hwHelp );

   
   SetWindowPos( NULL, 0, 0, 
	  rWnd.Width() + iBWCCW + 2.0*fSpc*float(iBWCCW), 
	  rWnd.Height() - rBtn.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE );

   GetClientRect( rWnd );
   CRect rBOK( rWnd.Width() - float(iBWCCW)*fSpc - iBWCCW,
	           (rWnd.Height() - 3*iBWCCH - iBWCCH) / 2,
			   0, 0 );
   rBOK.right = rBOK.left + iBWCCW;
   rBOK.bottom = rBOK.top + iBWCCH;
	           

   m_btnOK.Create( "&Да", 
	  BS_DEFPUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
	  rBOK, this, IDOK );

   rBOK.OffsetRect( 0, iBWCCH + iBWCCH/2 );
   m_btnCancel.Create( "&Отказ", 
	  WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
	  rBOK, this, IDCANCEL );

   rBOK.OffsetRect( 0, iBWCCH + iBWCCH/2 );
   m_btnHelp.Create( "&Помощь", 
	  WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
	  rBOK, this, IDHELP );

   HFONT hf = (HFONT)WindowProc( WM_GETFONT, 0, 0 );
   if( hf )
	{
      m_btnOK.SendMessage( WM_SETFONT, (WPARAM)hf, 0 );
	  m_btnCancel.SendMessage( WM_SETFONT, (WPARAM)hf, 0 );
	  m_btnHelp.SendMessage( WM_SETFONT, (WPARAM)hf, 0 );
	}
   
   /*
   //m_propSheet.m_psh.dwFlags &= ~PSH_MULTILINETABS;   
   m_propSheet.Create( this, WS_CHILD | WS_CLIPCHILDREN | WS_TABSTOP | WS_VISIBLE );

   CRect r;
   ::GetWindowRect( m_propSheet.m_hWnd, r );
   ScreenToClient( r );
   //m_propSheet.SetWindowPos( NULL, iLeftSpc, iTopSpc, r.Width(), r.Height(), SWP_NOZORDER | SWP_NOACTIVATE );
   CRect rOK;
   ::GetWindowRect( ::GetDlgItem(m_hWnd, IDOK), rOK );

   int iWidth = r.Width() + iLeftSpc + rOK.Width() + 4*iLeftSpc + 2*GetSystemMetrics(SM_CXBORDER);
   int iHigh = r.Height() + 2*iTopSpc  + 2*GetSystemMetrics(SM_CXBORDER) + GetSystemMetrics(SM_CYCAPTION);
   SetWindowPos( NULL, 0, 0, iWidth, iHigh, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE );

   CRect rCl;
   GetClientRect( rCl );
   int iXOK = r.Width() + iLeftSpc + (rCl.Width() - iLeftSpc - r.Width() - rOK.Width()) / 2;
   int iOKY = (rCl.Height() - 2*rOK.Height() - rOK.Height()/2.0) / 2.0;

   ::SetWindowPos( 
	  ::GetDlgItem(m_hWnd, IDOK), 
	  NULL, iXOK, iOKY, 0, 0, 
	  SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

   ::SetWindowPos( 
	  ::GetDlgItem(m_hWnd, IDCANCEL), 
	  NULL, iXOK, 
	  iOKY+rOK.Height()+rOK.Height()/2.0, 
	  0, 0, 
	  SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );


   m_propSheet.SetWindowPos( NULL, iLeftSpc, 
	  (rCl.Height()-r.Height())/2, 
	  r.Width(), 
	  r.Height(), 
	  SWP_NOZORDER | SWP_NOACTIVATE );
*/
   CenterWindow();
	//CWnd* pParentWnd = NULL, DWORD dwStyle = (DWORD)–1, DWORD dwExStyle = 0 );
   
   UpdateData( FALSE );

   return FALSE;
 }


CPropPageTooltips::~CPropPageTooltips()
 {
   list<TToolTipData *>::iterator itSta( m_vecToolTips.begin() );
   list<TToolTipData *>::iterator itEnd( m_vecToolTips.end() );

   for( ; itEnd != itSta; ++itSta )
     if( *itSta )
	  {
        delete *itSta;
		*itSta = NULL;
	  }	
 }

BOOL CPropPageTooltips::OnInitDialog()
 {
   CPropertyPage::OnInitDialog();

   EnableToolTips( TRUE );

   return TRUE;
 }

int CPropPageTooltips::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const
 {
   list<TToolTipData *>::iterator itSta( m_vecToolTips.begin() );
   list<TToolTipData *>::iterator itEnd( m_vecToolTips.end() );

   TToolTipData *pDta = NULL;
   CRect r;
   HWND hw;
   for( ; itEnd != itSta; ++itSta )
	{
	  hw = ::GetDlgItem( m_hWnd, (*itSta)->m_uiId );
	  if( !hw ) continue;
	  ::GetWindowRect( hw, &r );
	  ScreenToClient( &r );
	  if( r.PtInRect(point) == TRUE ) 
	   {
         pDta = *itSta;
		 break;
	   }
	}   

   if( !pDta ) return -1;

   //pTI->uFlags = TTF_IDISHWND | TTF_SUBCLASS;
   pTI->uFlags = TTF_IDISHWND;
   pTI->hwnd = m_hWnd;
   pTI->uId = (UINT)hw;
   pTI->rect = r;
   //pTI->hinst = AfxGetResourceHandle();
   pTI->hinst = 0;
   //pTI->lpszText = MAKEINTRESOURCE( m_uiCommand );
   pTI->lpszText = _strdup( (LPCSTR)(pDta->m_csTxt) );

   return 1;
 }
