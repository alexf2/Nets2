#include "ncpane.hpp"
#include "nframe.hpp"
#include "net.hpp"
#include "cfg.hpp"
#include "drv2dlg.hpp"
#include "RC\net.rh"

#include "..\\RUDAKOW\\SERVIS\\pathex.hpp"

#include <errno.h>
#include <direct.h>
#include <algorithm>
#include <typeinfo.h>
#include <afxpriv.h>

IMPLEMENT_DYNCREATE( TNortonLikePane, CFrameWnd )
IMPLEMENT_DYNAMIC( TNCSplitter, CSplitterWnd )

BEGIN_MESSAGE_MAP( TNCSplitter, CSplitterWnd )	
  ON_WM_ERASEBKGND()  
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP( THeaderCtrlNC, CHeaderCtrl )
  ON_WM_SIZE()

  ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONDOWN()

END_MESSAGE_MAP()

#define ID_PANE_LIST 2
BEGIN_MESSAGE_MAP( TNortonLikePane, CFrameWnd )	
  ON_WM_CREATE()
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
  ON_WM_SIZE()

  ON_WM_SETFOCUS()  
  //ON_WM_KILLFOCUS()  

  ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONDOWN()

  ON_NOTIFY( HDN_ENDTRACK, 1, OnEndTrack )

  ON_MESSAGE_VOID( WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI )

  ON_LBN_DBLCLK( ID_PANE_LIST, OnLb_DblClk )
  ON_LBN_SELCHANGE( ID_PANE_LIST, OnLb_SelChange )
END_MESSAGE_MAP() 


BEGIN_MESSAGE_MAP( CMyBox, CStatusBar )	
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
  ON_WM_CREATE()
  ON_WM_SIZE()

  ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP() 


BEGIN_MESSAGE_MAP( TNCList, CListBox )	
  ON_WM_CREATE()
  ON_WM_ERASEBKGND()

  ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONDOWN()

  ON_WM_KEYDOWN()

  //ON_WM_SETFOCUS()  
  //ON_WM_KILLFOCUS()  

END_MESSAGE_MAP()


TNCSplitter::~TNCSplitter()
 { 
 }  

TNortonLikePane* TNCSplitter::GetActivePane_Int()
 {
   TNortonLikePane *p1 = (*this)(0), *p2 = (*this)(1);

   if( p1 && p1->GetStateTTl() == false ) return p1;
   if( p2 && p2->GetStateTTl() == false ) return p2;

   return NULL;
 }

void TNCSplitter::RefocusList()
 {
   TNortonLikePane *p = (TNortonLikePane *)GetActivePane_Int();
   if( !p ) return;
   p->RefocusList();
 }

void TNCSplitter::StopTracking( BOOL bAccept )
 {
   CSplitterWnd::StopTracking( bAccept );

   if( bAccept == TRUE ) 
	{
      TMainFrame *pP = (TMainFrame *)GetParent();
	  if( pP ) pP->SplitterModfyNotify();
	}

   RefocusList();	 
 }

void TNCSplitter::SetFullMode( int iPane )
 {
   if( m_bModeFull == iPane ) return;
   TNortonLikePane *p1 = (*this)(0), *p2 = (*this)(1);
   if( !p1 || !p2 ) return;

   TNortonLikePane *p = NULL;
   if( iPane == 1 ) p = p1; 
   else if( iPane == 2 ) p = p2; 

   if( p && GetActivePane() != p ) SetActivePaneMy( p );

   m_bModeFull = iPane;   
   ((TMainFrame*)GetParent())->RecalcLayout( TRUE );
   RecalcLayout();
   //InvalidateRect( NULL );
 }

//TNortonLikePane* TNCSplitter::GetMyPane( int iPan )
TNortonLikePane* TNCSplitter::operator()( int iPane )
 {
   return (TNortonLikePane*)GetPane( 0, iPane ? 1:0 ); 
 }

LPCSTR TNCSplitter::GetPath( int iPane ) const
 {
   TNortonLikePane *p = (*(TNCSplitter*)this)( iPane );
   return p ? p->GetPath():NULL;
 }

void TNCSplitter::SetPath( int iPane, LPCSTR lp )
 {
   TNortonLikePane *p = (*this)( iPane );  
   if( p ) p->SetPath( lp );
 }

LPCSTR TNCSplitter::GetSelectMask( int iPane ) const
 {
   TNortonLikePane *p = (*(TNCSplitter*)this)( iPane );     
   if( !p ) return "*.*";

   return p->GetSelectMask();
 }

void TNCSplitter::SetSelectMask( int iPane, LPCSTR lp )
 {
   TNortonLikePane *p = (*this)( iPane );
   if( !p ) return;

   p->SetSelectMask( lp );
 }

void TNCSplitter::Refresh()
 {
   TNortonLikePane* p = (*this)( GetActivePaneMy() );
      
   if( !p ) return;

   p->NotifyUser( TUserOfDrive::NC_Updated );
 }

int TNCSplitter::GetActivePaneMy() const
 {
   TNortonLikePane *p1 = (*(TNCSplitter*)this)(0), *p2 = (*(TNCSplitter*)this)(1);

   return p1 ? (p1->IsActive() == true ? 0:1):0;
 }

void TNCSplitter::Select( int iPane, LPCSTR lpMask, bool bFlSel )
 {
   TNortonLikePane *p = (*this)(iPane);
   if( !p ) return;

   p->Select( lpMask, bFlSel );
 }

void TNCSplitter::SetActivePaneMy( TNortonLikePane* pPane )
 {
   TNortonLikePane *p1 = (*this)(0), *p2 = (*this)(1);

   if( pPane == GetActivePane() ) return;

   if( p1 == pPane )
	{
      p1->SetState(true); p2->SetState(false); SetActivePane(0,0);
	}
   else
	{
      p1->SetState(false); p2->SetState(true); SetActivePane(0,1);
	}
 }


void TNCSplitter::SetActivePaneMy( int iPane )
 {
   TNortonLikePane *p1 = (*this)(0), *p2 = (*this)(1);

   if( p1 && p2 )
	{
	  if( !iPane ) 
	  { p1->SetState(true); p2->SetState(false); SetActivePane(0,0); }
	  else { p1->SetState(false); p2->SetState(true); SetActivePane(0,1); }	  
	}
 }

int TNCSplitter::GetClmSize( int iPane ) const
 {
   TNortonLikePane *p = (*(TNCSplitter*)this)( iPane );
   return p ? p->GetClmSize():100;
 }

void TNCSplitter::SetClmSize( int iPane, int iSize )
 {
   TNortonLikePane *p = (*this)( iPane );
   if( p ) p->SetClmSize( iSize );	
 }


BOOL TNCSplitter::OnEraseBkgnd( CDC* pDC )
 {
   return TRUE;
 }

//void TNCSplitter::OnDrawSplitter( CDC* pDC, ESplitType nType, const CRect& rect )
void TNCSplitter::OnDrawSplitter( CDC* pDC, ESplitType nType, const CRect& rectArg )
 {
   if( pDC == NULL || nType != splitBar ) 
	{
	  CSplitterWnd::OnDrawSplitter( pDC, nType, rectArg );
	  return;
	}

   //if( m_bModeFull ) return;

   CPen *pcpWhite = CPen::FromHandle( (HPEN)::GetStockObject(WHITE_PEN) );
   CPen *pcpBlack = CPen::FromHandle( (HPEN)::GetStockObject(BLACK_PEN) );
   CPen cpGray( PS_SOLID, 0, ::GetSysColor(COLOR_BTNFACE) );
   CPen cpDkGray( PS_SOLID, 0, ::GetSysColor(COLOR_BTNSHADOW) );


   int iHigh = rectArg.Height();
   int iL = rectArg.left; 
   int iT = rectArg.top; 


   CPen *pOldPen = pDC->SelectObject( pcpWhite );
   pDC->MoveTo( iL, iT );
   pDC->LineTo( iL, iT+iHigh );
   iL++;   
   pDC->MoveTo( iL, iT );
   pDC->LineTo( iL, iT+iHigh );
   iL++;

   pDC->SelectObject( &cpGray );
   pDC->MoveTo( iL, iT );
   pDC->LineTo( iL, iT+iHigh );
   iL++;
   pDC->MoveTo( iL, iT );
   pDC->LineTo( iL, iT+iHigh );
   iL++;

   pDC->SelectObject( &cpDkGray );
   pDC->MoveTo( iL, iT );
   pDC->LineTo( iL, iT+iHigh );
   iL++;
   pDC->MoveTo( iL, iT );
   pDC->LineTo( iL, iT+iHigh );
   iL++;

   pDC->SelectObject( pcpBlack );
   pDC->MoveTo( iL, iT );
   pDC->LineTo( iL, iT+iHigh );
   iL++;

   pDC->SelectObject( pOldPen );
 }


TNortonLikePane::TNortonLikePane(): 
  CFrameWnd(), 
  m_hdrCtl(m_iSizeClm),
  m_nclLst( m_rvecFiles, m_iSizeClm, arcPenBorder, &TNortonLikePane::m_FntList ),
  m_usrUserOfSrv( NULL ),
  m_csSelMask( "*.*" )
 {
   m_usrUserOfSrv.SetParent( this );

   m_bFlActive = false;
   m_iSizeClm = 100;
   m_pOpt = NULL;
   m_pFSrv = NULL;
 }

TNortonLikePane::~TNortonLikePane()
 {
   ClearFList( m_rvecFiles );   
 }

//void TNortonLikePane::OnUpdateList( CCmdUI* )
void TNortonLikePane::OnIdleUpdateCmdUI()
 {
   CFrameWnd::OnIdleUpdateCmdUI();
   if( m_pFSrv )
	 m_pFSrv->FindNotifyFS();
 }

void TNortonLikePane::ClearFList( vector<TFileRecord*>& rLst )
 {
   vector<TFileRecord*>::iterator itSta( rLst.begin() );
   vector<TFileRecord*>::iterator itEnd( rLst.end() );

   for( ; itSta != itEnd; ++itSta )
	if( (*itSta) )
	 {
       delete *itSta; *itSta = NULL;
	 }

   rLst.clear();
 }


void GetErrorPath( CString& strTmp, LPSTR lp, CWnd *pPar )
 {
   char path_buffer[_MAX_PATH];
   char drive[_MAX_DRIVE];
   char dir[_MAX_DIR];
   char fname[_MAX_FNAME];
   char ext[_MAX_EXT];

   _splitpath( lp, drive, dir, fname, ext );
   _makepath( path_buffer, drive, "\\", "*", ".*" );

   while( 1 )
	{
      _finddata_t fnd;
      memset( &fnd, 0, sizeof(_finddata_t) );
      fnd.attrib = _A_ARCH | _A_SUBDIR | _A_SYSTEM | _A_HIDDEN | _A_RDONLY | _A_NORMAL;

      long lHandl = _findfirst( path_buffer, &fnd );
      if( lHandl != -1 )
	   {
	     _findclose( lHandl );

		 _splitpath( path_buffer, drive, dir, fname, ext );
		 if( *fname == '*' )
           _makepath( path_buffer, drive, dir, "", "" );
		 else
		   _makepath( path_buffer, drive, dir, fname, ext );

         strTmp = path_buffer;
	     return;
	   }

      TDriveDlg dlg( pPar, drive );
	  drive[1] = 0;
      dlg.DoModal();
	  drive[1] = ':'; drive[2] = 0;
	  //_makepath( path_buffer, drive, "\\", "*", ".*" );
	  _getdcwd( toupper(*drive) - 'A' + 1, path_buffer, _MAX_PATH );	  
	  _splitpath( path_buffer, drive, dir, fname, ext );
      _makepath( path_buffer, drive, dir, "*", ".*" );
	}
 }

void TNortonLikePane::SetPath( LPCSTR lp, bool bExtErr )
 {   
   CString strTmp;

   if( !lp || !*lp )
	{
      char buffer[ _MAX_PATH ];	  
	  if( _getcwd(buffer, _MAX_PATH) ) strTmp = buffer;
	  else strTmp = "";
	}
   else strTmp = lp;

   if( strTmp.IsEmpty() ) GetErrorPath( strTmp, (LPSTR)lp, this );

   bool bOK = false;
   bool byPass = bExtErr == true ? true:false;;
   do {

     if( byPass == true ) 
	   GetErrorPath( strTmp, (LPSTR)(LPCSTR)strTmp, this );
	 byPass = true;

     TDrive *pDrv;
	 while( !(pDrv = m_pFSrv->FindDrive(strTmp, false)) )
	    GetErrorPath( strTmp, (LPSTR)(LPCSTR)strTmp, this );     

     try {
        m_usrUserOfSrv.ChangeDrive( pDrv );
	    m_usrUserOfSrv.ChangePath( strTmp );
	    m_usrUserOfSrv.Directory( m_rvecFiles );
	    bOK = true;
	  }
     catch( TCannotChangeDrive )
	  {
        /*((TNetApp*)AfxGetApp())->bwcc.BWCCMessageBox( m_hWnd, 
	      "ќшибка при смене устройства", 
	      "ќшибка", MB_OK | MB_ICONSTOP );*/
	  }
	 catch( TBadUseDrive )
	  {
        ((TNetApp*)AfxGetApp())->bwcc.BWCCMessageBox( m_hWnd, 
	      "¬нутренн€€ ошибка", 
	      "ќшибка", MB_OK | MB_ICONSTOP );
	  }
     catch( CFileException* pE )
	  {
        if( pE->m_strFileName.IsEmpty() ) pE->m_strFileName = m_usrUserOfSrv.GetPath();
        //pE->ReportError( MB_OK | MB_ICONSTOP );
	    pE->Delete();  	        	  	  
	  }
     catch( CException* pE )
	  {	  
        //pE->ReportError( MB_OK | MB_ICONEXCLAMATION );
	    pE->Delete();  	  	  
	  } 

	} while( bOK == false );

   m_Ttl.SetTtl( m_usrUserOfSrv );
   m_nclLst.FillList();
 }

void TNortonLikePane::SetPath_DrOnly( LPCSTR lp )
 {   
   CString strTmp = lp;

   int iC = m_nclLst.GetCurSel();
   bool bOK = false;   
   //do {     

     try {     	    
	    m_usrUserOfSrv.ChangePath( strTmp );
	    m_usrUserOfSrv.Directory( m_rvecFiles );
	    bOK = true;
	  }
     catch( TCannotChangeDrive )
	  {
        /*((TNetApp*)AfxGetApp())->bwcc.BWCCMessageBox( m_hWnd, 
	      "ќшибка при смене устройства", 
	      "ќшибка", MB_OK | MB_ICONSTOP );*/

		//SetPath( lp );
	  }
	 catch( TBadUseDrive )
	  {
        ((TNetApp*)AfxGetApp())->bwcc.BWCCMessageBox( m_hWnd, 
	      "¬нутренн€€ ошибка", 
	      "ќшибка", MB_OK | MB_ICONSTOP );

		//SetPath( lp );	
	  }
     catch( CFileException* pE )
	  {
        if( pE->m_strFileName.IsEmpty() ) pE->m_strFileName = m_usrUserOfSrv.GetPath();
        //pE->ReportError( MB_OK | MB_ICONSTOP );
	    pE->Delete();  	        	  	  

		//SetPath( lp );		
	  }
     catch( CException* pE )
	  {	  
        //pE->ReportError( MB_OK | MB_ICONEXCLAMATION );
	    pE->Delete();  	  	  

		//SetPath( lp );	
	  } 

	//} while( bOK == false );

   if( bOK == false ) SetPath( lp, true );

   m_Ttl.SetTtl( m_usrUserOfSrv );
   m_nclLst.FillList();
      
   if( iC != -1 )
	{
	  m_nclLst.SetCurSel( 0 ); 
	  OnLb_SelChange();
	}
   else m_Stat.SetPaneText( 0, "" );
 }


void TNortonLikePane::SetClmSize( int iSz )
 {
   m_iSizeClm = iSz;
   int k = m_hdrCtl.GetItemCount();
   HD_ITEM hd; memset( &hd, 0, sizeof(HD_ITEM) );
   hd.mask = HDI_WIDTH; hd.cxy = iSz;

   m_hdrCtl.SetRedraw( FALSE );
   for( int i = 0; i < k; ++i )
	 m_hdrCtl.SetItem( i, &hd );	
   m_hdrCtl.SetRedraw( TRUE );

   m_nclLst.UpdateWidth();
 }

/*void TNortonLikePane::OnDraw( CDC* pDc )
 {
   //ValidateRect( NULL );
 }*/

/*void TNortonLikePane::OnActivate( UINT nState, CWnd* pWndOther, BOOL bMinimized )
 {
   if( nState != WA_INACTIVE )
	 ((TNCSplitter*)GetParent())->SetActivePaneMy( this );

   CFrameWnd::OnActivate( nState, pWndOther, bMinimized );
 }*/

void TNortonLikePane::SetActivePaneInstance()
 {
   ((TNCSplitter*)GetParent())->SetActivePaneMy( this );
 }

void TNortonLikePane::OnLButtonDown( UINT nFlags, CPoint point )
 {
   ((TNCSplitter*)GetParent())->SetActivePaneMy( this );
   CFrameWnd::OnLButtonDown( nFlags, point );
 }

void TNortonLikePane::OnRButtonDown( UINT nFlags, CPoint point )
 {
   ((TNCSplitter*)GetParent())->SetActivePaneMy( this );
   CFrameWnd::OnRButtonDown( nFlags, point );
 }

void TNortonLikePane::SetState( bool bSta )
 {
   if( m_bFlActive == bSta ) return;

   m_bFlActive = bSta;
   m_Ttl.m_bFlMode2 = !bSta;
   m_Ttl.InvalidateRect( NULL ); 

   if( bSta == true )
	{
      m_nclLst.SetCurSel( m_nclLst.m_iOldSel );
	  OnLb_SelChange();
	}
   else
	{
      m_nclLst.m_iOldSel = m_nclLst.GetCurSel();
      m_nclLst.SetCurSel( -1 );
	}
 }

BOOL TNortonLikePane::PreCreateWindow( CREATESTRUCT& cs )
 {
   if( !CFrameWnd::PreCreateWindow(cs) ) return FALSE;

   cs.style &= ~WS_BORDER;
   cs.style |= WS_CLIPCHILDREN;
   //cs.lpszName = "Window";

   return TRUE;
 }

static UINT indicatorsTtl[] =
 {
   1
 };

BOOL TNortonLikePane::OnEraseBkgnd( CDC* pDC )
 {
   CRect r;
   GetClientRect( r );
   pDC->FillSolidRect( r, RGB(0,0,0x80) );
   return 1;
 }

void TNortonLikePane::OnSize( UINT nType, int cx, int cy )
 {
   CFrameWnd::OnSize( nType, cx, cy );
   GetClientRect( m_rcClient );

   CRect rTtl, rStat;
   m_Ttl.GetWindowRect( rTtl ); 
   m_Stat.GetWindowRect( rStat ); 
   
   m_rcClient.top = rTtl.Height();
   m_rcClient.bottom -= rStat.Height();

   
   GetHdrRect( rTtl );
   m_hdrCtl.SetWindowPos( NULL, rTtl.left, rTtl.top, rTtl.Width(), rTtl.Height(), 
	                      SWP_NOZORDER | SWP_NOACTIVATE
					    );

   GetListRect( rTtl );
   m_nclLst.SetWindowPos( NULL, rTtl.left, rTtl.top, rTtl.Width(), rTtl.Height(), 
	                      SWP_NOZORDER | SWP_NOACTIVATE
					    );
 }

BOOL TNortonLikePane::ChDir( TNCList* pLst, bool& bFlUp )
 {
   bFlUp = false;

   int iC = pLst->GetCurSel();
   if( iC == -1 ) return FALSE;

   if( m_rvecFiles[iC]->m_csName == "." )
	{
	  char ch[10];
	  bFlUp = true;
      sprintf( ch, "%c:\\", char(*m_usrUserOfSrv.GetPath()) );
	  //SetPath( ch );
	  SetPath_DrOnly( ch );
	}
   else if( m_rvecFiles[iC]->m_csName == ".." )
	{
      bFlUp = true;

	  char path_buffer[_MAX_PATH];
      char drive[_MAX_DRIVE];
      char dir[_MAX_DIR];
      char fname[_MAX_FNAME];
      char ext[_MAX_EXT];

      _splitpath( m_usrUserOfSrv.GetPath(), drive, dir, fname, ext );      
	  _makepath( path_buffer, drive, dir, NULL, NULL );
	  int iL = strlen(path_buffer);
	  if( iL > 1 && path_buffer[iL-1] == '\\' && path_buffer[iL-2] != ':' ) path_buffer[ iL - 1 ] = 0;

	  //SetPath( path_buffer );
	  SetPath_DrOnly( path_buffer );
	}
   else if( m_rvecFiles[iC]->attrib & _A_SUBDIR )
	{
      int iL = strlen(m_usrUserOfSrv.GetPath()) + m_rvecFiles[iC]->m_csName.GetLength();
	  if( iL < _MAX_PATH )
	   {
         char path_buffer[_MAX_PATH];   

		 strcpy( path_buffer, m_usrUserOfSrv.GetPath() );
		 if( path_buffer[strlen(path_buffer) - 1] != '\\' )
		   strcat( path_buffer, "\\" );
		 strcat( path_buffer, m_rvecFiles[iC]->m_csName );         

	     //SetPath( path_buffer );
		 SetPath_DrOnly( path_buffer );
	   }
	}
   else return FALSE;

   return TRUE;
 }

void TNortonLikePane::OnPaint()
 {
   if( !arcPenBorder )
	{
      Default();
	  return;
	}

   CPaintDC dc( this );
   CPen *pOldPen = dc.SelectObject( arcPenBorder() );
   CBrush *pBrOld = (CBrush *)dc.SelectStockObject( NULL_BRUSH );

   CRect r( m_rcClient );
   //GetClientRect( r ); 
   
   r.DeflateRect( 1, 1 );
   dc.Rectangle( r );

   r.DeflateRect( 1, 1 );
   dc.Rectangle( r );

   r.DeflateRect( 3, 3 );
   dc.Rectangle( r );

   dc.SelectObject( pBrOld );
   dc.SelectObject( pOldPen );
 }


void TNortonLikePane::OnSetFocus( CWnd* pOldWnd )
 {
   CFrameWnd::OnSetFocus( pOldWnd );
   
   m_nclLst.SetFocus();
 }

CFont TNortonLikePane::m_FntTtl, 
             TNortonLikePane::m_FntStat, 
			 TNortonLikePane::m_FntList;
bool TNortonLikePane::m_bFlInitFont = false;

int TNortonLikePane::OnCreate( LPCREATESTRUCT lpCreateStruct )
 {
   if( CFrameWnd::OnCreate(lpCreateStruct) == -1 )
	  return -1;

   m_pOpt = ((TNCSplitter*)GetParent())->GetOpt();
   m_pFSrv = ((TNCSplitter*)GetParent())->GetFServer();

   arcPenBorder = new CPen( PS_SOLID, 0, RGB(0,0xFF,0xFF) );
   

   if( TNortonLikePane::m_bFlInitFont == false )
	{
      TNortonLikePane::m_bFlInitFont = true;

      TNortonLikePane::m_FntTtl.CreateFontIndirect( &m_pOpt->lfTtl );
      TNortonLikePane::m_FntStat.CreateFontIndirect( &m_pOpt->lfStatus );
      TNortonLikePane::m_FntList.CreateFontIndirect( &m_pOpt->lfList );
	}

   CClientDC dc( this );
   CFont *pFOld = dc.SelectObject( &TNortonLikePane::m_FntTtl );
   CSize sz = dc.GetTextExtent( "Str" );
      
   dc.SelectObject( &TNortonLikePane::m_FntStat );
   CSize sz2 = dc.GetTextExtent( "Str" );   

   dc.SelectObject( pFOld );
   //m_Ttl, m_Stat;
  //CFont m_FntTtl, m_FntBk;

   if( !m_Ttl.Create(this, WS_CHILD | WS_VISIBLE | CBRS_TOP, AFX_IDW_STATUS_BAR+1) ||
	   !m_Ttl.SetIndicators(indicatorsTtl, 1)	    
	 )
	{		
	    ((TNetApp*)AfxGetApp())->bwcc.BWCCMessageBox( m_hWnd, "ќшибка при создании заголовка окна", "ќшибка", MB_OK | MB_ICONSTOP );
		return -1;      
	}

   DWORD dw = m_Ttl.GetBarStyle();
   m_Ttl.SetBarStyle( dw & ~CBRS_BORDER_TOP );     

   if( !m_Stat.Create(this, WS_CHILD | WS_VISIBLE | CBRS_BOTTOM, AFX_IDW_STATUS_BAR+2) ||
	    !m_Stat.SetIndicators(indicatorsTtl, 1)	    
	 )
	{		
	    ((TNetApp*)AfxGetApp())->bwcc.BWCCMessageBox( m_hWnd, "ќшибка при создании окна", "ќшибка", MB_OK | MB_ICONSTOP );
		return -1;      
	}

   
   m_Ttl.Init( RGB(0,0,200), RGB(255,255,255), RGB(0x80,0x80,0x80), RGB(0xC0,0xC0,0xC0), 
	  &TNortonLikePane::m_FntTtl, sz.cy, true );
   //m_Stat.Init( RGB(200,200,200), RGB(0,0,0), &m_FntStat, sz2.cy );   

	
    UINT nID, nStyle;
	int cxWidth;
	m_Ttl.GetPaneInfo( 0, nID, nStyle, cxWidth );
	m_Ttl.SetPaneInfo( 0, nID, SBPS_STRETCH | SBPS_NORMAL, cxWidth );	
	m_Stat.GetPaneInfo( 0, nID, nStyle, cxWidth );
	m_Stat.SetPaneInfo( 0, nID, SBPS_STRETCH | SBPS_NORMAL, cxWidth );	

    //m_Ttl.SetPaneText( 0, "C:\\Txt2as\\asas\\sdsd\\sdsd\\sdaad\\sdasd" );
	//m_Stat.SetPaneText( 0, "Text" );

	//m_Ttl.SetTtl( "C:\\Txt2as\\asas\\sdsd\\sdsd\\sdaad\\sdasd" );

	CRect rH;
	GetHdrRect( rH );
	if( !m_hdrCtl.Create(WS_CHILD|WS_VISIBLE|HDS_BUTTONS|CCS_NOPARENTALIGN, rH, this, 1) )
	 {
        ((TNetApp*)AfxGetApp())->bwcc.BWCCMessageBox( m_hWnd, "ќшибка при создании заголовка списка", "ќшибка", MB_OK | MB_ICONSTOP );
		return -1;      
	 }

	CFont *pF = CFont::FromHandle( (HFONT)::GetStockObject(DEFAULT_GUI_FONT) );
	m_hdrCtl.SetFont( pF );


	GetListRect( rH );
	if( !m_nclLst.Create( WS_CHILD | WS_VISIBLE | WS_HSCROLL | LBS_MULTICOLUMN |
	                      LBS_NOINTEGRALHEIGHT |
						  LBS_NOTIFY | LBS_OWNERDRAWFIXED, rH, this, ID_PANE_LIST) )
	 {
        ((TNetApp*)AfxGetApp())->bwcc.BWCCMessageBox( m_hWnd, "ќшибка при создании списка", "ќшибка", MB_OK | MB_ICONSTOP );
		return -1;      
	 }
	 

   return 0;
 }

void TNortonLikePane::GetListRect( CRect& rC )
 {
   GetHdrRect( rC );
   rC.OffsetRect( 0, rC.Height() );

   CRect rT; m_Stat.GetWindowRect( rT );
   CRect r; GetClientRect( r );

   rC.bottom = r.Height() - rT.Height() - 6;
 }

void TNortonLikePane::GetHdrRect( CRect& rC )
 {
   CRect r; GetClientRect( r );
   CRect rT; m_Ttl.GetWindowRect( rT );

   rC.top = rT.Height() + 6;
   rC.left = 6;
   rC.right = r.Width() - 6;
   rC.bottom = rC.top + GetSystemMetrics( SM_CYMENU );
 }

void TNortonLikePane::OnEndTrack( NMHDR *pNMHDR, LRESULT *pRes )
 {
   *pRes = 0;
   //HDN_ENDTRACK

   HD_NOTIFY *pHd = (HD_NOTIFY *)pNMHDR;
   SetClmSize( pHd->pitem->cxy );

   ((TNCSplitter*)GetParent())->RefocusList();	 
 }

CMyBox::~CMyBox()
 {
 }
  
/*HBRUSH CMyBox::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
 {
   pDC->SetTextColor( m_cFnt );
   return (HBRUSH)(*arcBr());
 }*/

  

//void CMyBox::Init( COLORREF cBk, COLORREF cFnt, CFont *pFnt, int iCy, bool bOwnDraw )
void CMyBox::Init( COLORREF cBk, COLORREF cFnt, COLORREF cBk2, COLORREF cFnt2, CFont *pFnt, int iCy, bool bOwnDraw )
 {
   m_bOwnDraw = bOwnDraw;

   int nHorz, nVert, nSpacing;
   GetStatusBarCtrl().GetBorders( nHorz, nVert, nSpacing );
   //GetStatusBarCtrl().SetMinHeight( iCy + 2*nVert +50 );
   m_nMinHeight = iCy + 2*nVert;

   m_pFnt = pFnt; m_cBk = cBk; m_cFnt = cFnt;
   m_cBk2 = cBk2; m_cFnt2 = cFnt2;
   if( m_bOwnDraw == true ) 
	{
	  arcBr = new CBrush( m_cBk );   
	  arcBr2 = new CBrush( m_cBk2 );   

	  arcPenDark = new CPen( PS_SOLID, 0, RGB(0x80,0x80,0x80) );
	  arcPenLight = new CPen( PS_SOLID, 0, RGB(0xC0,0xC0,0xC0) );
	}

   //m_dwStyle &= ~CBRS_BORDER_TOP;
 }

int CMyBox::OnCreate( LPCREATESTRUCT lpCreateStruct )
 {
   //m_dwStyle &= ~CBRS_BORDER_TOP;

   if( CStatusBar::OnCreate(lpCreateStruct) == -1 )
	  return -1;

   //WindowProc( WM_SETFONT, (WPARAM)(HFONT)*m_pFnt, 0 );
   
   return 0;
 }

CMyBox::CMyBox(): CStatusBar() 
 {
   //m_nMinHeight = 50;
   m_pFnt = NULL;
   m_cFnt2 = m_cBk2 = m_cFnt = m_cBk = RGB(0,0,0); 
   m_bOwnDraw = false;
   m_bFlMode2 = true;
   //m_dwStyle &= ~CBRS_BORDER_TOP;
 }

void CMyBox::OnPaint()
 {   
   if( m_bOwnDraw == false )
	{
	  Default();
	  return;
	} 

   if( m_Str.IsEmpty() ) return;

   CRect r;
   GetClientRect( &r ); r.bottom -= 2;  
   CPaintDC dc( this );   
   CFont *pFnOld = dc.SelectObject( m_pFnt );
   dc.SetTextColor( m_bFlMode2==true ? m_cFnt2:m_cFnt );   
   dc.SetBkMode( TRANSPARENT );

   dc.DrawText( m_StrWork, r, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
      
   dc.SelectObject( pFnOld );
 }

void CMyBox::SetTtl( TUserOfDrive_NCPane& rUs )
 {
   m_Str.Format( "%s%s", 
	 rUs.GetDrive()->IsRemote() == true ? "Link:":"",
	 (LPSTR)(LPCSTR)rUs.GetPath() );
   RecalcWorkStr();
   InvalidateRect( NULL );
  //CString m_StrWork;
 }

void CMyBox::OnSize( UINT nType, int cx, int cy )
 {
   CStatusBar::OnSize( nType, cx, cy );
   RecalcWorkStr();
 }

void CMyBox::RecalcWorkStr()
 {
   if( m_Str.IsEmpty() ) return;

   CRect r; GetClientRect( &r );
   if( !r.Width() ) return;

   CClientDC dc( this );
   CFont *pFnOld = dc.SelectObject( m_pFnt );
   
   r.left += 2; r.right -= 2;
   m_StrWork = m_Str;   
   CompressPath( m_StrWork, dc, r );		

   dc.SelectObject( pFnOld );
 }

//( cStr, iLen, lp );


BOOL CMyBox::OnEraseBkgnd( CDC* pDC )
 {
   if( m_bOwnDraw == false ) return CStatusBar::OnEraseBkgnd( pDC );

   CRect r;
   GetClientRect( &r );   
   //m_cBk, m_cFnt;  
   pDC->FillRect( r, m_bFlMode2==true ? arcBr2():arcBr() );

   CPen *pPenOld = pDC->SelectObject( arcPenDark() );

   pDC->MoveTo( 0, r.bottom-1 );
   pDC->LineTo( r.right, r.bottom-1 );

   pDC->SelectObject( arcPenLight() );
   pDC->MoveTo( 0, r.bottom-2 );
   pDC->LineTo( r.right, r.bottom-2 );

   pDC->SelectObject( pPenOld );

   return 1;
 }

void CMyBox::OnLButtonDown( UINT nFlags, CPoint point )
 {
   ((TNortonLikePane*)GetParent())->SetActivePaneInstance();
   CStatusBar::OnLButtonDown( nFlags, point );
 }

void CMyBox::OnRButtonDown( UINT nFlags, CPoint point )
 {
   ((TNortonLikePane*)GetParent())->SetActivePaneInstance();
   CStatusBar::OnRButtonDown( nFlags, point );
 }


THeaderCtrlNC::~THeaderCtrlNC()
 {
 }

void THeaderCtrlNC::OnSize( UINT ui, int cx, int cy )
 {
   CHeaderCtrl::OnSize( ui, cx, cy ); 

   //if( !cx || !cy ) return;
	
   int iNumbHdr = GetSystemMetrics(SM_CXFULLSCREEN)/m_riSizeClm + 1;
   if( GetItemCount() >= iNumbHdr ) return;
	
   int iDelta = iNumbHdr - GetItemCount();
   int iSta = GetItemCount();
   int iEnd = iSta + iDelta;
   HD_ITEM hd; memset( &hd, 0, sizeof(HD_ITEM) );
   hd.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT;
   hd.pszText = "Name";
   hd.cxy = m_riSizeClm; 
   hd.fmt = HDF_CENTER | HDF_STRING;
   hd.cchTextMax = strlen( hd.pszText );

   SetRedraw( FALSE );
   for( int i = iSta; i < iEnd; ++i )
     InsertItem( i, &hd );
   SetRedraw( TRUE );
 }

void THeaderCtrlNC::OnLButtonDown( UINT nFlags, CPoint point )
 {
   ((TNortonLikePane*)GetParent())->SetActivePaneInstance();
   CHeaderCtrl::OnLButtonDown( nFlags, point );
 }

void THeaderCtrlNC::OnRButtonDown( UINT nFlags, CPoint point )
 {
   ((TNortonLikePane*)GetParent())->SetActivePaneInstance();
   CHeaderCtrl::OnRButtonDown( nFlags, point );
 }

void TNortonLikePane::OnLb_DblClk()
 {
   m_nclLst.OnKeyDown( VK_RETURN, 1, 0 );
 }

static LPSTR MyFormatSize( __int64 i64Size )
 {
   static char cBuf[128];

   float fDv = i64Size >= 1024*1024 ? 1000*1000:
	    (i64Size >= 1024 ? 1000:1);
   char *lp = i64Size >= 1024*1024 ? "MB":
	    (i64Size >= 1024 ? "KB":"Bytes");

   sprintf( cBuf, i64Size >= 1024 ? "%1.2f %s":"%1.0f %s",
	 float(i64Size)/fDv, lp );

   return cBuf;
 }

void TNortonLikePane::OnLb_SelChange()
 {
   char cBuf[ _MAX_PATH + 100 ];

   int iC = m_nclLst.GetCurSel();
   if( iC == -1 || iC > m_rvecFiles.size() ) 
	{
      m_Stat.SetPaneText( 0, "" );
	  return;
	}
   
   vector<TFileRecord*>::iterator itSta( m_rvecFiles.begin() );
   vector<TFileRecord*>::iterator itEnd( m_rvecFiles.end() );
   __int64 i64Size = 0;
   DWORD dwCountDir = 0, dwCountFil = 0;
   for( ; itSta != itEnd; ++itSta )
	 if( (*itSta)->m_flSelect == true ) 
	  {	    
		if( (*itSta)->attrib & _A_SUBDIR ) dwCountDir++;
		else
		 {
		   dwCountFil++;
		   i64Size += (*itSta)->size;
		 }
	  }

  TFileRecord *pRec = m_rvecFiles[ iC ];

  if( dwCountDir || dwCountFil )
     sprintf( cBuf, "%s  in %lu files and %lu directories", 
	          MyFormatSize( i64Size ),
			  dwCountFil,
			  dwCountDir
		    );
  else if( pRec->attrib & _A_SUBDIR )
     sprintf( cBuf, "%s  <SUB-DIR>  %s", 
	          (LPCSTR)pRec->m_csName, 
			  ctime( &(pRec->time_write) )   
		    );
   else
	{	  
	  sprintf( cBuf, "%s  %s  %s", 
	           (LPCSTR)pRec->m_csName, 
	 		   MyFormatSize( pRec->size ),			   
			   ctime( &(pRec->time_write) )  
			 );
	}

   m_Stat.SetPaneText( 0, cBuf );
 }

void TNCList::KeyCurDir()
 {   
   char drive[_MAX_DRIVE];
   char dir[_MAX_DIR];
   char fname[_MAX_FNAME];
   char ext[_MAX_EXT];

   _splitpath( ((TNortonLikePane*)GetParent())->GetPath(), 
	           drive, dir, fname, ext 
			 );

   LPSTR lp = m_strKeyDir.GetBuffer( _MAX_PATH );
   _makepath( lp, NULL, NULL, fname, ext );
   m_strKeyDir.ReleaseBuffer();
 }
  //CString m_strKeyDir;
void TNCList::SelectKeyedDir()
 {
   vector<TFileRecord*>* pV = ((TNortonLikePane*)GetParent())->GetSelectionVector();
   if( !pV || m_strKeyDir.IsEmpty() ) return;

   vector<TFileRecord*>::iterator itSta( pV->begin() );
   vector<TFileRecord*>::iterator itEnd( pV->end() );

   int i = 0;
   for( ; itSta != itEnd; ++itSta, ++i )	
	 if( ((*itSta)->attrib & _A_SUBDIR) && 
	     !(*itSta)->m_csName.CompareNoCase(m_strKeyDir) 
	   )
	  {
        SetCurSel( i );
		return;
	  }
 }

void TNCList::SetInSel()
 {
   vector<TFileRecord*>* pV = ((TNortonLikePane*)GetParent())->GetSelectionVector();
   if( !pV ) return;

   if( pV->size() >= 2 && !(*pV)[1]->m_csName.CompareNoCase("..") )	
     SetCurSel( 1 );
   else SetCurSel( 0 );	
 }

void TNCList::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
 {
   int iC = GetCurSel();
   
   if( iC != -1 )
	{
      if( nChar == VK_RETURN )
	   {
	     KeyCurDir();
		 bool bFl;
	     if( ((TNortonLikePane*)GetParent())->ChDir(this, bFl) )
		  {
		    //SetCurSel( 0 );
		    if( bFl == true ) SelectKeyedDir();
			else SetInSel();
		    ((TNortonLikePane*)GetParent())->OnLb_SelChange();
		  }
         return;
	   }
      else if( nChar == VK_INSERT )
	   {
	     if( m_rvevFiles[ iC ]->m_csName[0] != '.' )
		  {
            m_rvevFiles[ iC ]->InvertSelect();
  		    
			if( iC == m_rvevFiles.size() - 1 )
			 {
               CRect r;
		       GetItemRect( iC, r );
		       InvalidateRect( r, FALSE );
			 }
			else SetCurSel( iC + 1 );

            ((TNortonLikePane*)GetParent())->OnLb_SelChange();
		    return;
		  }
         SetCurSel( iC + 1 );

		 ((TNortonLikePane*)GetParent())->OnLb_SelChange();
		 return;
	   }
	}

   CListBox::OnKeyDown( nChar, nRepCnt, nFlags );
 }

int TNCList::CharToItem( UINT nKey, UINT nIndex )
 {      
   BYTE ch = toupper( nKey );
   for( int i = nIndex+1; i < m_rvevFiles.size(); ++i )
     if( ch == toupper(m_rvevFiles[i]->m_csName[0]) ) return i;

   if( !nIndex ) return -1;
   for( i = 0; i < nIndex-1; ++i )
     if( ch == toupper(m_rvevFiles[i]->m_csName[0]) ) return i;

   return -1;
 }

void TNCList::DrawItem( LPDRAWITEMSTRUCT lpD )
 {
   if( m_rvevFiles.size() < lpD->itemData ) return;

   TFileRecord& rRec = *m_rvevFiles[ lpD->itemData ];

   CString csPrefix( (rRec.attrib & _A_SYSTEM) ? '|':
                     ((rRec.attrib & (_A_RDONLY|_A_HIDDEN)) ? 'Ш':' ') );
   COLORREF cFnt = (rRec.m_flSelect == true) ? RGB(0xFF,0xFF,0):RGB(0,0xFF,0xFF);
   COLORREF cFon = (lpD->itemState & ODS_SELECTED) ? RGB(0,127,127):RGB(0,0,127);

   HFONT hfOld = (HFONT)::SelectObject( lpD->hDC, (HFONT)*m_pFnt );
   COLORREF cOld = ::SetTextColor( lpD->hDC, cFnt );
   COLORREF cBkOld = ::SetBkColor( lpD->hDC, cFon );

   CString strDraw( " " + csPrefix + rRec.m_csName );
   CRect rDra( lpD->rcItem ); rDra.DeflateRect( 1, 0 ); 
   ::DrawText( lpD->hDC, (LPCSTR)strDraw, strDraw.GetLength(), rDra, DT_LEFT|DT_SINGLELINE );
   SIZE sz;
   ::GetTextExtentPoint32( lpD->hDC, (LPCSTR)strDraw, strDraw.GetLength(), &sz );
   //::SetBkColor(m_hDC, clr);
   if( sz.cx < m_riSizeClm )
	{
      CRect r( rDra ); r.left += sz.cx; 

	  if( r.left < r.right )
        ::ExtTextOut( lpD->hDC, 0, 0, ETO_OPAQUE, r, NULL, 0, NULL );
	}

   ::SetBkColor( lpD->hDC, cBkOld );
   ::SetTextColor( lpD->hDC, cOld );
   SelectObject( lpD->hDC, hfOld );
 }

void TNCList::MeasureItem( LPMEASUREITEMSTRUCT lpM )
 {   
   CWindowDC dc( AfxGetApp()->m_pMainWnd );
   CFont *pFOld = dc.SelectObject( m_pFnt );

   //CString str = " " + m_rvevFiles[ lpM->itemData ]->m_csName;
   CString str( "Test" );
   CSize sz = dc.GetTextExtent( str );

   //lpM->itemWidth = sz.cx;
   lpM->itemWidth = m_riSizeClm;
   lpM->itemHeight = sz.cy;

   dc.SelectObject( pFOld );
 }

void TNCList::FillList()
 {
   ResetContent();
   if( !m_rvevFiles.size() ) return;
   InitStorage( m_rvevFiles.size(), sizeof(int) );	

   //vector<TFileRecord*>::iterator itSta( m_rvevFiles.begin() );
   //vector<TFileRecord*>::iterator itEnd( m_rvevFiles.end() );
   int iEnd = m_rvevFiles.size();
   SetRedraw( FALSE );
   for( int i = 0; i < iEnd; ++i )
    AddString( (LPSTR)i );
   SetRedraw( TRUE );
 }

int TNCList::OnCreate( LPCREATESTRUCT lpCreateStruct )
 {
   if( CListBox::OnCreate(lpCreateStruct) == -1 )
	  return -1;

   FillList();
   SetCurSel( 0 );
   ((TNortonLikePane*)GetParent())->OnLb_SelChange();

   return 0;
 }

BOOL TNCList::OnEraseBkgnd( CDC* pDC )
 {
   //return TRUE;
   CRect r;
   GetClientRect( r );
   pDC->FillSolidRect( r, RGB(0,0,128) );

   CPen *pOld = pDC->SelectObject( m_rAP() );
   int iEnd = r.Width() / m_riSizeClm + 1;
   int iY = r.Height();
   for( int i = 1; i < iEnd; ++i )
	{
      int iX = i * m_riSizeClm - 1;
	  pDC->MoveTo( iX, 0 );
	  pDC->LineTo( iX++, iY );
	  pDC->MoveTo( iX, 0 );
	  pDC->LineTo( iX, iY );
	}

   pDC->SelectObject( pOld );

   return TRUE;
 }

void TNCList::OnLButtonDown( UINT nFlags, CPoint point )
 {   
   ((TNortonLikePane*)GetParent())->SetActivePaneInstance();
   CListBox::OnLButtonDown( nFlags, point );
 }

void TNCList::OnRButtonDown( UINT nFlags, CPoint point )
 {   
   ((TNortonLikePane*)GetParent())->SetActivePaneInstance();
   CListBox::OnRButtonDown( nFlags, point );

   BOOL bFl;
   UINT uiInd = ItemFromPoint( point, bFl );
   if( bFl == FALSE )
	{
	  SetCurSel( uiInd );
      OnKeyDown( VK_INSERT, 1, 0 );
	}
 }

bool TNortonLikePane::NotifyUser( TUserOfDrive::TNotifyCode nfc )
 {
   bool bUpdate = false;
   int iC = m_nclLst.GetCurSel();

   try{
     if( nfc == TUserOfDrive::NC_Removed )
      {
        m_usrUserOfSrv.ChangeDrive( m_pFSrv->FindCurDrive() );
	    m_usrUserOfSrv.Directory( m_rvecFiles );
		m_Ttl.SetTtl( m_usrUserOfSrv );
		
	    m_nclLst.FillList();		                
      } 
     else if( nfc == TUserOfDrive::NC_Updated )
      {
        m_usrUserOfSrv.Directory( m_rvecFiles );
		
	    m_nclLst.FillList();	
      }
    }
   catch( CException *pE )
	{       
      pE->ReportError( MB_OK | MB_ICONSTOP );
	  pE->Delete();  	        	  	  

	  CString cs( GetPath()[0] ); cs += ":\\";
	  SetPath( (LPSTR)(LPCSTR)cs );
	  bUpdate = true;
	}
   catch(...)
	{
	  CString cs( GetPath()[0] ); cs += ":\\";
      SetPath( (LPSTR)(LPCSTR)cs );
	  bUpdate = true;
	}

   if( iC != -1 )
	{
      m_nclLst.SetCurSel( 0 ); 
	  OnLb_SelChange();
	}

   return bUpdate;
 }

void TNortonLikePane::SetDriveIndex( int iInd )
 {
   if( iInd < 0 || iInd >= m_pFSrv->GetDriveData().size() ) return;
   TDrive *pDrv = m_pFSrv->GetDriveData()[ iInd ];

   m_usrUserOfSrv.UpdateLastAcess();
   int iC = m_nclLst.GetCurSel();

   bool bOK = false;
   try {
        m_usrUserOfSrv.ChangeDrive( pDrv );	    
	    m_usrUserOfSrv.Directory( m_rvecFiles );	    

		m_Ttl.SetTtl( m_usrUserOfSrv );
        m_nclLst.FillList();
		bOK = true;
	  }
     catch( TCannotChangeDrive )
	  {
        /*((TNetApp*)AfxGetApp())->bwcc.BWCCMessageBox( m_hWnd, 
	      "ќшибка при смене устройства", 
	      "ќшибка", MB_OK | MB_ICONSTOP );*/
	  }
	 catch( TBadUseDrive )
	  {
        ((TNetApp*)AfxGetApp())->bwcc.BWCCMessageBox( m_hWnd, 
	      "¬нутренн€€ ошибка", 
	      "ќшибка", MB_OK | MB_ICONSTOP );
	  }
     catch( CFileException* pE )
	  {
        if( pE->m_strFileName.IsEmpty() ) pE->m_strFileName = m_usrUserOfSrv.GetPath();
        //pE->ReportError( MB_OK | MB_ICONSTOP );
	    pE->Delete();  	        	  	  
	  }
     catch( CException* pE )
	  {	  
        //pE->ReportError( MB_OK | MB_ICONEXCLAMATION );
	    pE->Delete();  	  	  
	  } 

	CString str( pDrv->GetPath() );
    if( bOK == false ) SetPath( (LPCSTR)str, true );

	if( iC != -1 )
	 {
	   m_nclLst.SetCurSel( 0 ); 
	   OnLb_SelChange();
	 }
	else m_Stat.SetPaneText( 0, "" );
 }

bool TUserOfDrive_NCPane::NotifyUser( TUserOfDrive::TNotifyCode nfc )
 {
   return m_pOwner->NotifyUser( nfc );
 }

bool TUserOfDrive_NCPane::IsCallingOutside() const
 {
   return false;
 }


void TNortonLikePane::Select( LPCSTR lpMask, bool bFlSel )
 {
   vector<TFileRecord*>::iterator itSta( m_rvecFiles.begin() );
   vector<TFileRecord*>::iterator itEnd( m_rvecFiles.end() );

   TMaskTemplate tmpl( lpMask );
   bool bFlFound = false;

   for( ; itSta != itEnd; ++itSta )	
     if( (!((*itSta)->attrib & _A_SUBDIR) || bFlSel==false) && 
	     tmpl.Check((*itSta)->m_csName) == true 
	   ) 
	  {
	    (*itSta)->m_flSelect = bFlSel;
		bFlFound = true;
	  }
     
   if( bFlFound == true )
	{
      m_nclLst.InvalidateRect( NULL );	 
      OnLb_SelChange();
	}
 }

LONG TNortonLikePane::CalcSelected( unsigned atr )
 {
   vector<TFileRecord*>::iterator itSta( m_rvecFiles.begin() );
   vector<TFileRecord*>::iterator itEnd( m_rvecFiles.end() );
   
   LONG lRes = 0;
   for( ; itSta != itEnd; ++itSta )	
	 if( (*itSta)->m_flSelect == true && 
	     ((*itSta)->attrib & atr)
	   )
	   lRes++;

   return lRes;
 }

LPSTR TNortonLikePane::GetFileAtCursor()
 {
   int iSel = m_nclLst.GetCurSel();
   if( iSel == -1 || iSel >= m_rvecFiles.size() ) return "";

   return  (LPSTR)(LPCSTR)m_rvecFiles[ iSel ]->m_csName; 
 }

TDrive* TNortonLikePane::GetDriveFor( CString& csPath, bool bRemote )
 {
   return m_pFSrv->FindDrive( csPath, bRemote );
 }

int TNortonLikePane::GetIndexAtCursor()
 {
   int iSel = m_nclLst.GetCurSel();
   if( iSel == -1 || iSel >= m_rvecFiles.size() ) return 0;

   return  iSel; 
 }


void TNortonLikePane::SelectIndex( int iInd, bool bFl )
 {
   if( iInd == -1 || iInd >= m_rvecFiles.size() ) return;
   m_rvecFiles[ iInd ]->m_flSelect = bFl;

   CRect r; m_nclLst.GetItemRect( iInd, r );
   m_nclLst.InvalidateRect( r );	 
   OnLb_SelChange();
 }
