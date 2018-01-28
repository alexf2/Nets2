#include "drvsel.hpp"
#include "server.hpp"

BEGIN_MESSAGE_MAP( CMyListBox, CListBox )	
  ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP( TDriveSelDlg, CDialog )	
  ON_LBN_DBLCLK( IDC_LST_DRIVES, OnLb_DblClk )
END_MESSAGE_MAP()



BOOL CMyListBox::OnEraseBkgnd( CDC* pDC )
 {
   CRect r; GetClientRect( r );
   pDC->FillSolidRect( r, RGB(0xFF,0xFF,0xFF) );

   return TRUE;
 }

int CMyListBox::CharToItem( UINT nKey, UINT nIndex )
 {         
   int iInd = m_rSrv.FindCurDriveIndex_ForLetter( nKey, false );
   if( iInd != -1 ) 
	{
      SetCurSel( iInd );
	  GetParent()->PostMessage( WM_COMMAND, MAKEWPARAM(IDOK,BN_CLICKED), (LPARAM)m_hWnd );
	  //((CDialog*)GetParent())->OnOK();
	  return iInd;
	}

   iInd = m_rSrv.FindCurDriveIndex_ForLetter( nKey, true );   
   if( iInd != -1 ) 
	{
      SetCurSel( iInd );
	  GetParent()->PostMessage( WM_COMMAND, MAKEWPARAM(IDOK,BN_CLICKED), (LPARAM)m_hWnd );
	  return iInd;
	  //((CDialog*)GetParent())->OnOK();
	}

   return iInd;
 }


bool CMyListBox::m_bInit = false;
int CMyListBox::m_CX = 0, CMyListBox::m_CY = 0;
int CMyListBox::m_cxFild = 0;
CImageList CMyListBox::m_img;
CFont CMyListBox::m_Fnt;

int CMyListBox::IndexOfType( int ind )
 {
   if( m_rSrv.GetDriveData().size() <= ind ) return 0;

   if( m_rSrv.GetDriveData()[ind]->IsRemote() == true ) return 4;

   switch( m_rSrv.GetDriveData()[ind]->Type() )
	{
	  case DRIVE_REMOVABLE:
	    return 0;

	  case DRIVE_FIXED:
	    return 1;

	  case DRIVE_REMOTE:
	    return 2;

	  case DRIVE_CDROM:
	    return 3;

	  case DRIVE_RAMDISK:
	    return 0;

	  default:
	    return 0;
	};
 }

LPSTR CMyListBox::TextOf( int ind )
 {
   static char cBuf[25];

   if( m_rSrv.GetDriveData().size() <= ind ) return "";
   if( m_rSrv.GetDriveData()[ind]->IsRemote() == true ) 
	 sprintf( cBuf, "%c:/L", (char)(m_rSrv.GetDriveData()[ind]->Letter() + 'A' - 1) );
   else sprintf( cBuf, "%c:", (char)(m_rSrv.GetDriveData()[ind]->Letter() + 'A' - 1) );

   return cBuf;
 }

void CMyListBox::DrawItem( LPDRAWITEMSTRUCT lpDIS )
 {   
   const int iCL = 5;

   CDC dc; dc.Attach( lpDIS->hDC );   

   if( lpDIS->itemAction & (ODA_DRAWENTIRE | ODA_SELECT) )
	{
	  //CBrush br( (lpDIS->itemState & ODS_SELECTED) ? RGB(0,0,128):RGB(255,255,255) );
	  dc.FillSolidRect( &lpDIS->rcItem, 
	    (lpDIS->itemState & ODS_SELECTED) ? RGB(0,0,128):RGB(0xFF,0xFF,0xFF) );

	  CRect r( lpDIS->rcItem );
	  CPoint pt( r.Width() - iCL - m_CX, lpDIS->rcItem.top + 1 );
	  m_img.Draw( &dc, IndexOfType(lpDIS->itemData), pt, ILD_NORMAL );	  

	  CFont *pF = dc.SelectObject( &m_Fnt );

      LPSTR lp = TextOf( lpDIS->itemData );
	  r.left += iCL; r.right = r.left+m_cxFild; r.top++; r.bottom--; 
	  dc.SetTextColor( (lpDIS->itemState & ODS_SELECTED) ? RGB(255,255,255):RGB(0,0,0) );
	  dc.DrawText( lp, strlen(lp), r, DT_VCENTER | DT_LEFT | DT_NOCLIP | DT_SINGLELINE );
	   // LPCTSTR lpszString, int nCount, LPRECT lpRect, UINT nFormat 

      dc.SelectObject( pF );
	}

	if ((lpDIS->itemState & ODS_FOCUS) &&
		(lpDIS->itemAction & (ODA_FOCUS | ODA_DRAWENTIRE)))
	 {
		// item has been selected - hilite frame
       LOGBRUSH brush;
	   brush.lbStyle = BS_SOLID;
	   brush.lbColor = RGB( 0, 255, 0 );
	   brush.lbHatch = HS_BDIAGONAL;
	   CPen pen( PS_GEOMETRIC, 0, &brush );	
	   //CPen( int nPenStyle, int nWidth, const LOGBRUSH* pLogBrush, int nStyleCount = 0, const DWORD* lpStyle = NULL );
	   CPen *pOld = dc.SelectObject( &pen );
	   CBrush *pBr = (CBrush *)dc.SelectStockObject( NULL_BRUSH );
       
	   dc.SetBkColor( RGB(0,0,0) );
       dc.Rectangle( &lpDIS->rcItem );

       dc.SelectObject( pBr ); 
  	   dc.SelectObject( pOld );
	 }

	if (!(lpDIS->itemState & ODS_FOCUS) &&
		(lpDIS->itemAction & ODA_FOCUS))
	 {
		// Item has been de-selected -- remove frame		        
	   CPen *pOld = (CPen*)dc.SelectStockObject( WHITE_PEN );
	   CBrush *pBr = (CBrush *)dc.SelectStockObject( NULL_BRUSH );

	   dc.Rectangle( &lpDIS->rcItem );

	   dc.SelectObject( pBr );
	   dc.SelectObject( pOld );
	 }

	dc.Detach();
 }

/*void CMyListBox::MeasureItem( LPMEASUREITEMSTRUCT lpM )
 {
 }*/

int CMyListBox::Init()
 {
   static LOGFONT lf = { 16, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
      OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, 
	  VARIABLE_PITCH | FF_DONTCARE, "MS Sans Serif"
	};

   if( m_bInit == false )
	{ 
	  CBitmap bmRemovable, bmFixed, bmRemote, bmCD, bmDirect;
	  if( !bmRemovable.LoadBitmap(IDB_REMOVABLE) ) return -1;
	  if( !bmFixed.LoadBitmap(IDB_FIXED) ) return -1;
	  if( !bmRemote.LoadBitmap(IDB_REMOTE) ) return -1;
	  if( !bmCD.LoadBitmap(IDB_CDROM) ) return -1;
	  if( !bmDirect.LoadBitmap(IDB_DIRECT) ) return -1;
	  
	  BITMAP bm;
      ::GetObject( (HBITMAP)bmRemovable, sizeof(bm), &bm );

	  m_img.Create( (m_CX=bm.bmWidth), (m_CY=bm.bmHeight), 
	                ILC_MASK | ILC_COLOR4, 5, 1 );
	  m_img.Add( &bmRemovable, RGB(0,0,128) );
	  m_img.Add( &bmFixed, RGB(0,0,128) );
	  m_img.Add( &bmRemote, RGB(0,0,128) );
	  m_img.Add( &bmCD, RGB(0,0,128) );
	  m_img.Add( &bmDirect, RGB(0,0,128) );

	  m_img.SetBkColor( CLR_NONE );

      m_Fnt.CreateFontIndirect( &lf );

	  m_bInit = true;
	}   


   CWindowDC dc( AfxGetApp()->m_pMainWnd );
   CFont *pFOld = dc.SelectObject( &m_Fnt );

   CSize sz = dc.GetTextExtent( "W/WWW" );
   m_cxFild = sz.cx;

   m_itemWidth  = sz.cx + m_CX + 2;
   m_itemHeight = max( sz.cy, m_CY ) + 2;

   dc.SelectObject( pFOld );
   SetItemHeight( 0, m_itemHeight );
   SetColumnWidth( m_itemWidth );   

   return 0;
 }


void TDriveSelDlg::OnLb_DblClk()
 {
   if( m_lstBox.GetCurSel() != -1 ) OnOK();
 }

void TDriveSelDlg::OnOK()
 {
   m_iSel = m_lstBox.GetCurSel();
   CDialog::OnOK();
 }

void TDriveSelDlg::DoDataExchange( CDataExchange* pDX )
 {
   CDialog::DoDataExchange( pDX );
  
   //DDX_Control( pDX, IDC_LST_DRIVES, m_lstBox );  
 }

BOOL TDriveSelDlg::OnInitDialog()
 {                 
   CDialog::OnInitDialog(); 

   m_lstBox.SubclassDlgItem( IDC_LST_DRIVES, this );
   m_lstBox.Init();

   int iL = m_rSrv.GetDriveData().size();

   int iLW = m_lstBox.m_itemWidth + //3*GetSystemMetrics( SM_CXEDGE ) +
	         2*GetSystemMetrics( SM_CXBORDER ) +
			 2*GetSystemMetrics( SM_CXFIXEDFRAME );
	         //GetSystemMetrics( SM_CXVSCROLL );

   int iLH = iL * m_lstBox.m_itemHeight + 
	         //2*GetSystemMetrics( SM_CYEDGE ) + 
			 2*GetSystemMetrics( SM_CYBORDER ) + 
			 2*GetSystemMetrics( SM_CYFIXEDFRAME ) + 2;

   int iMaxClientH = GetSystemMetrics(SM_CYSCREEN) - 
     GetSystemMetrics(SM_CYCAPTION) - 5*GetSystemMetrics(SM_CYDLGFRAME) -
     2*GetSystemMetrics(SM_CYBORDER) - 0.1*float(GetSystemMetrics(SM_CYSCREEN));

   if( iLH > iMaxClientH ) 
	{
	  iLH = iMaxClientH;
	  iLW += GetSystemMetrics( SM_CXVSCROLL );
	}

   CRect rBtn; GetDlgItem(IDOK)->GetWindowRect( rBtn );
   int iWP1 = iLW + 4*GetSystemMetrics( SM_CXDLGFRAME );
   int iHP1 = iLH + 4*GetSystemMetrics( SM_CYDLGFRAME );
   iHP1 = max( iHP1, 3*rBtn.Height() );

   int iWP2 = rBtn.Width() * 1.2;   

   int iSpcX = GetSystemMetrics( SM_CXDLGFRAME );
   int iSpcY = GetSystemMetrics( SM_CYDLGFRAME );
   
   int iWWnd = 2*GetSystemMetrics(SM_CXDLGFRAME) + 3*iSpcX + iWP1 + 
	           iWP2;
   int iHWnd = GetSystemMetrics(SM_CYCAPTION) + 
	           2*GetSystemMetrics(SM_CYDLGFRAME) + 2*iSpcY + iHP1;

   SetWindowPos( NULL, 0, 0, iWWnd, iHWnd, SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOREDRAW );

   CWnd *pPan1 = GetDlgItem( IDC_GRP_LST );
   pPan1->SetWindowPos( NULL, iSpcX, iSpcY, iWP1, iHP1, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW );

   m_lstBox.SetWindowPos( NULL, 0, 0, iLW, iLH, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW|SWP_NOMOVE );
   m_lstBox.CenterWindow( pPan1 );

   CWnd *pPan2 = GetDlgItem( IDC_GRP_BTN );
   pPan2->SetWindowPos( NULL, 2*iSpcX+iWP1, iSpcY, iWP2, iHP1, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW );

   CRect r2; pPan2->GetWindowRect( r2 ); ScreenToClient( r2 );
   CWnd *pOK = GetDlgItem( IDOK );
   CWnd *pCancel = GetDlgItem( IDCANCEL );
   int iX = r2.left + (r2.Width() - rBtn.Width()) / 2;
   int iY = r2.top + (r2.Height()-2*rBtn.Height()-rBtn.Height()/3) / 2;

   pOK->SetWindowPos( NULL, iX, iY, 0, 0, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW|SWP_NOSIZE );
   pCancel->SetWindowPos( NULL, iX, iY+rBtn.Height()+rBtn.Height()/3, 
	 0, 0, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW|SWP_NOSIZE );
	//( const CWnd* pWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags );


   SetWindowText( (LPCSTR)m_sTtl );
   //CWnd *pWnd = GetParent();
   CenterWindow( m_pPar );
   
   m_lstBox.InitStorage( iL, sizeof(int) );
   for( int i = 0; i < iL; ++i )
     m_lstBox.AddString( (LPSTR)i );

   if( m_iSel < 0 ) m_iSel = 0;
   m_lstBox.SetCurSel( m_iSel );
   

   return FALSE;
 }
