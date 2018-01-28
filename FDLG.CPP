#include "fdlg.hpp"
#include "net.hpp"

#include "..\\RUDAKOW\\SERVIS\\pathex.hpp"

bool TUserOfDrive_Cpy::NotifyUser( TUserOfDrive::TNotifyCode nfc )
 {
   if( nfc == TUserOfDrive::NC_Removed )
	{
	  m_revCancel.SetEvent();
	  m_revRemoved.SetEvent();

	  //CSingleLock lock( &m_revCancelPassed, TRUE );      
	}

   return false;
 }

bool TUserOfDrive_Cpy::IsCallingOutside() const
 {
   return true;
 }


BOOL TKBPassThread::PreTranslateMessage( MSG *pMsg )
 {
   if( pMsg->message == WM_CHAR && pMsg->wParam == VK_ESCAPE )
	 ::PostMessage( m_pResp->m_hWnd, WM_CHAR, pMsg->wParam, pMsg->lParam );

   return CWinThread::PreTranslateMessage( pMsg );
 }

BEGIN_MESSAGE_MAP( TCopyDlg, CDialog )	
  ON_WM_CTLCOLOR()  
END_MESSAGE_MAP()


TCopyDlg::~TCopyDlg()
 {
   SafetyKillTh();   
   m_evCancelPassed.SetEvent();
 }


HBRUSH TCopyDlg::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
 {
   HBRUSH hbr = CDialog::OnCtlColor( pDC, pWnd, nCtlColor );

   if( m_bFlBlue == true && nCtlColor == CTLCOLOR_STATIC )
	{
	  int id = pWnd->GetDlgCtrlID();
	  if( id == IDC_TXT_FILE )
	    pDC->SetTextColor( RGB(0,0,255) );
	}

   return hbr;
 }

BOOL TCopyDlg::DestroyWindow()
 {
   BOOL bCanc = (WaitForSingleObject( m_evRemoved, 0 ) != WAIT_TIMEOUT);

   SafetyKillTh();
   m_fntDlg.Detach();
   
   BOOL bRes = CDialog::DestroyWindow();
   if( !bCanc ) m_pUsSrc->InvalidateList();

   m_evCancelPassed.SetEvent();

   return bRes;
 }

void TCopyDlg::SafetyKillTh()
 {
   if( !m_arcTh() ) return;

   DWORD dwExitCode;
   if( GetExitCodeThread(m_arcTh()->m_hThread, &dwExitCode) && dwExitCode == STILL_ACTIVE )
	{
	  DWORD dwRes = m_arcTh()->ResumeThread();
	  if( dwRes != 0xFFFFFFFF && dwRes > 1 )
	   {
         while( dwRes > 1 )
		   dwRes = m_arcTh()->ResumeThread();
	   }

      m_evCancel.SetEvent();	  	  	  	  
	   	  
	  while( WaitForSingleObject(m_arcTh()->m_hThread, 0) == WAIT_TIMEOUT );
	}

   m_arcTh.FreeRc();
 }

void TNCNotify::Notinfy( TNotifyData *pNotifyData )
 {
   if( m_bEnabled == true )
     m_rDlg.NotifyProc( pNotifyData );
 }         

void TCopyDlg::NotifyProc( TNotifyData *pNotifyData_ )
 {                  
   TNotifyDataCopy *pNotifyData = (TNotifyDataCopy *)pNotifyData_;

   if( pNotifyData->m_bFlStartOp == true )
	{      
      CString str;
	  str.Format( "%s%s", pNotifyData->m_pSrc->IsRemote() == true ? "Link:":"",
	    pNotifyData->m_lpFromShortPath + m_iIndexInPath );
      
	  CString str2;
	  str2.Format( "%s%s", pNotifyData->m_pDst->IsRemote() == true ? "Link:":"",
	    pNotifyData->m_lpToFullPath );      

      CClientDC dc( this );
      CFont *pFnOld = dc.SelectObject( &m_fntDlg );	           
      CompressPath( str, dc, m_rSrc );		
	  CompressPath( str2, dc, m_rDst );		
      dc.SelectObject( pFnOld );

	  DWORD dw;   //IDC_TXT_FILE
      if( !::SendMessageTimeout(m_hWndSrc, WM_SETTEXT, 0, (LPARAM)(LPCSTR)str,
	    SMTO_NORMAL, 200, &dw) ) return;

	  if( !::SendMessageTimeout(m_hWndDst, WM_SETTEXT, 0, (LPARAM)(LPCSTR)str2,
	    SMTO_NORMAL, 200, &dw) ) return;

      m_ulInkeySize = pNotifyData->m_ulSize;
	  if( m_ulInkeySize )
	    m_dK = 1000.0 / double( m_ulInkeySize );
	  else m_dK = -1;

	  if( !::SendMessageTimeout(m_hWndProgr, PBM_SETRANGE, 0, MAKELPARAM(0,1000),
	    SMTO_NORMAL, 200, &dw) ) return;
	  if( !::SendMessageTimeout(m_hWndProgr, PBM_SETPOS, 0, 0,
	    SMTO_NORMAL, 200, &dw) ) return;
	}
   else if( pNotifyData->m_bFlOpComplete == true )
	{
	  DWORD dw;
      if( !::SendMessageTimeout(m_hWndProgr, PBM_SETPOS, 1000, 0,
	    SMTO_NORMAL, 200, &dw) ) return;
	}
   else
	{
	  DWORD dw;
      if( m_dK == -1 )
	   {
	     if( !::SendMessageTimeout(m_hWndProgr, PBM_SETPOS, 1000, 0,
	       SMTO_NORMAL, 200, &dw) ) return;
	   }
	  else
	   {
         if( !::SendMessageTimeout(m_hWndProgr, PBM_SETPOS, 
		    (WPARAM)(double(pNotifyData->m_ulSize)*m_dK), 0,
	       SMTO_NORMAL, 200, &dw) ) return;
	   }
	}
/*
struct TNotifyDataCopy: public TNotifyData
 { 
   LPSTR m_lpFromShortPath;
   LPSTR m_lpToFullPath;
   bool m_bFlStartOp;
   unsigned long m_ulSize;
   TDrive *m_pSrc, *m_pDst;
 };
*/
 }

static UINT ThreadProc( LPVOID pObj )
 {
   ((TCopyDlg*)pObj)->StartOp();
   return 0;
 }

void TCopyDlg::StartOp()
 {   
   try {
     m_iIndexInPath = strlen( m_arcDupSrc()->GetPath() );

     if( m_bFlErase == true )
       m_arcDupSrc()->Move( *m_arcNewDst(), m_pUsSrc->GetSelectionVector(), true, m_arcNotify() );
     else
	   m_arcDupSrc()->Copy( *m_arcNewDst(), m_pUsSrc->GetSelectionVector(), true, m_arcNotify() );
	}
   catch( CFileException* pE )
	{
      //if( pE->m_strFileName.IsEmpty() ) pE->m_strFileName = m_usrUserOfSrv.GetPath();
      pE->ReportError( MB_OK | MB_ICONSTOP );
	  pE->Delete();  	        	  	  
	}
   catch( CException* pE )
	{	  
      pE->ReportError( MB_OK | MB_ICONEXCLAMATION );
	  pE->Delete();  	  	  
	} 
   catch( TUserCancel )
	{
	}
   catch(...)
	{
	}

   ::PostMessage( m_hWnd, WM_COMMAND, 
	  MAKEWPARAM(IDCANCEL, BN_CLICKED), 
	  (LPARAM)::GetDlgItem(m_hWnd, IDCANCEL) );
 }

void TCopyDlg::OnOK()
 {
   //CDialog::OnOK();
   try {   
     CString csPath;
     GetDlgItemText( IDC_ED_PATH, csPath );
     TDrive *pDrv = m_pUsSrc->GetDriveFor( csPath, m_pUsDst->GetDrive()->IsRemote() );
     if( !pDrv )
	  {
        ((TNetApp*)AfxGetApp())->bwcc.BWCCMessageBox( m_hWnd, 
	      "Логическое устройство не найдено", "Ошибка", MB_OK | MB_ICONSTOP );
	    return;
	  }

     m_arcNewDst = new TUserOfDrive_Cpy( pDrv, m_evCancel, m_evCancelPassed, m_evRemoved );	 
	 *m_arcNewDst()->GetDrive() + m_arcNewDst();
     m_arcNewDst()->ChangePath( (LPCSTR)csPath );	 
	}
   catch(TCannotChangeDrive)
	{
	  ((TNetApp*)AfxGetApp())->bwcc.BWCCMessageBox( m_hWnd, 
	      "Ошибка доступа к логическому устройству", "Ошибка", MB_OK | MB_ICONSTOP );
	   return;
	}
   catch(...)
	{
	  ((TNetApp*)AfxGetApp())->bwcc.BWCCMessageBox( m_hWnd, 
	      "Ошибка доступа к логическому устройству", "Ошибка", MB_OK | MB_ICONSTOP );
	  return;
	}

   m_arcNotify = new TNCNotify( this, m_evCancel, *this );

   GetDlgItem(IDOK)->EnableWindow( FALSE );
   GotoDlgCtrl( GetDlgItem(IDCANCEL) );
   GetDlgItem(IDC_ED_PATH)->EnableWindow( FALSE );
   GetDlgItem(IDC_TXT_FILE)->ModifyStyle( 0, SS_CENTER ); 
   m_bFlBlue = true;
	//DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0 );

   m_arcTh = new TKBPassThread( ThreadProc, this );   
   m_arcTh()->m_bAutoDelete = FALSE;
   m_arcTh()->CreateThread();   
 }


void TCopyDlg::OnCancel()
 {
   m_evCancel.SetEvent();
   SafetyKillTh();

   if( m_iOneItem != -1  ) m_pUsSrc->SelectIndex( m_iOneItem, false );

   CDialog::OnCancel();
 }

void TCopyDlg::DoDataExchange( CDataExchange* pDX )
 {
   CDialog::DoDataExchange( pDX );
 }

BOOL TCopyDlg::OnInitDialog()
 {
   CDialog::OnInitDialog();
       
   CString csSrc;
   csSrc.Format( "%s file(s)/directory(s)", (LPCSTR)m_csKeyWord );
   SetWindowText( (LPCSTR)csSrc );
   
   LONG lNSel = m_pUsSrc->CalcSelected();
   if( lNSel )
	{
      LONG lDir =   m_pUsSrc->CalcSelected( _A_SUBDIR );
	  LONG lFile =  lNSel - lDir;

	  if( lDir && lFile )
	    csSrc.Format( "%s %lu files and %lu directories to:",
		  (LPCSTR)m_csKeyWord, lFile, lDir );
	  else if( lDir )
	    csSrc.Format( "%s %lu directories to:",
		  (LPCSTR)m_csKeyWord, lDir );
	  else
	   csSrc.Format( "%s %lu files to:",
		  (LPCSTR)m_csKeyWord, lFile );
	}
   else
	{
	  csSrc.Format( "%s '%s' to:",
		   (LPCSTR)m_csKeyWord, m_pUsSrc->GetFileAtCursor() );
	  m_iOneItem = m_pUsSrc->GetIndexAtCursor();
	  m_pUsSrc->SelectIndex( m_iOneItem, true );
	}

   CString scDst( m_pUsDst->GetPath() );
   if( scDst[scDst.GetLength() - 1] != '\\' ) scDst += "\\";

   CClientDC dc( this );
   CFont *pFnt = dc.SelectObject( GetFont() );   
   CSize szSrc = dc.GetTextExtent( csSrc );
   CSize szDst = dc.GetTextExtent( scDst );   
   CSize szExtra = dc.GetTextExtent( "W" );
   dc.SelectObject( pFnt );
   //szExtra.cx /= 2; szExtra.cy /= 2;

   int iCx;

   int iLeftWnd;
   int iLeftPane;
   int iTopWnd;
   int iTopPane;

   int iHProgr;
   int iSkipTxt;

   int iWndWP;

   CWnd *pOK;
   CRect r;

   int iWndWMin;
   int iWindowWidth;
   int iWindowHigh;

   int iHPane;
   int iHTxt;
   int iHEd;

   while( 1 )
	{
      pOK = GetDlgItem( IDOK );
	  pOK->GetWindowRect( r ); ScreenToClient( r );
	  iWndWMin = 5 * r.Width() + 2*GetSystemMetrics( SM_CXDLGFRAME );

	  iCx = max( szSrc.cx, szDst.cx ) + 2*szExtra.cx;

	  if( iCx <= iWndWMin ) iCx = float(iCx) * 1.3;   

	  iHTxt = szExtra.cy + 2;
	  iHEd = szExtra.cy + 2*GetSystemMetrics(SM_CYBORDER) + 2;
	
	  iLeftWnd = szExtra.cx/2; 
	  iLeftPane = szExtra.cx/2; 
	  iTopWnd = szExtra.cy / 2; 
	  iTopPane = szExtra.cy / 2;
	
	  iHProgr = szExtra.cy*1.5;
	  iSkipTxt = 1;
	
	  iWndWP = 2*GetSystemMetrics(SM_CXDLGFRAME) + 
	   2*iLeftWnd + 
  	   2*iLeftPane + iCx;
		  
	
      iHPane = 3*iTopPane + iHEd + iHProgr + iSkipTxt + iHTxt;

	  iWindowWidth = max( iWndWMin, iWndWP );
	  iCx = iWindowWidth - 2*iLeftWnd - 
	    2*GetSystemMetrics(SM_CXDLGFRAME) - 2*iLeftPane;

	  iWindowHigh = 2*GetSystemMetrics( SM_CYDLGFRAME ) +	  
	  GetSystemMetrics( SM_CYCAPTION ) +
	    r.Height() +
	  3*iTopWnd + iHPane;
	
	  if( GetSystemMetrics(SM_CXSCREEN)-20 < iWindowWidth )
	   {
	     const float fMul = 0.8;
	     //iCx = float(iCx) * fMul;
		 szSrc.cx = szSrc.cx * 0.8; 
		 szDst.cx = szDst.cx * 0.8; 
	   }
	  else break;
	};   

   SetWindowPos( NULL, 0, 0, iWindowWidth, iWindowHigh, SWP_NOZORDER|SWP_NOREDRAW|SWP_NOMOVE|SWP_NOACTIVATE );

   CWnd *pPane = GetDlgItem( IDC_GRP_GRP );
   CWnd *pTxt = GetDlgItem( IDC_TXT_FILE );
   CWnd *pEd = GetDlgItem( IDC_ED_PATH );
   CWnd *pProgr = GetDlgItem( IDC_PROGR_BAR );
   CWnd *pCancel = GetDlgItem( IDCANCEL );
   
   pPane->SetWindowPos( NULL, iLeftWnd, iTopWnd, iCx+2*iLeftPane, 
	  iHPane, 
	  SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE 
	);

   pTxt->SetWindowPos( NULL, iLeftWnd + iLeftPane, 
	  iTopWnd + iTopPane, iCx, 
	  iHTxt, 
	  SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE 
	);

   pEd->SetWindowPos( NULL, iLeftWnd + iLeftPane, 
	  iTopWnd + iTopPane + iHTxt + iSkipTxt, iCx, 
	  iHEd, 
	  SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE 
	);

   pProgr->SetWindowPos( NULL, iLeftWnd + iLeftPane, 
	  iTopWnd + iTopPane + iHTxt + iSkipTxt + iTopPane + iHEd, iCx, 
	  iHProgr, 
	  SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE 
	);

   CRect r2; GetClientRect( r2 );
   int iX = (r2.Width() - 3*r.Width()) / 2;
   CRect r3; pPane->GetWindowRect( r3 ); ScreenToClient( r3 );

   pOK->SetWindowPos( NULL, iX, 
	  r3.bottom + (r2.Height()-r3.bottom-r.Height())/2, 0, 
	  0, 
	  SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE|SWP_NOSIZE
	);   

   pCancel->SetWindowPos( NULL, iX + 2*r.Width(), 
	  r3.bottom + (r2.Height()-r3.bottom-r.Height())/2, 0, 
	  0, 
	  SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE|SWP_NOSIZE
	);

   //pTxt->SetWindowText( csSrc );
   pEd->SetWindowText( scDst );

   CenterWindow( m_mPar );

   m_hWndSrc   = pTxt->m_hWnd;
   m_hWndDst   = pEd->m_hWnd;
   m_hWndProgr = pProgr->m_hWnd;

   m_fntDlg.Attach( (HFONT)SendMessage(WM_GETFONT, 0, 0) );
   pEd->GetClientRect( m_rDst );
   pTxt->GetClientRect( m_rSrc );

   CClientDC dc2( this );
   CFont *pFnOld = dc.SelectObject( &m_fntDlg );	           
   CompressPath( csSrc, dc, m_rSrc );		
   dc2.SelectObject( pFnOld );

   pTxt->SetWindowText( csSrc );


   return FALSE;
 }

 BEGIN_MESSAGE_MAP( TEraseDlg, CDialog )	
  ON_WM_CTLCOLOR()  
END_MESSAGE_MAP()


TEraseDlg::~TEraseDlg()
 {
   SafetyKillTh();   
   m_evCancelPassed.SetEvent();
 }


HBRUSH TEraseDlg::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
 {
   HBRUSH hbr = CDialog::OnCtlColor( pDC, pWnd, nCtlColor );

   if( m_bFlBlue == true && nCtlColor == CTLCOLOR_STATIC )
	{
	  int id = pWnd->GetDlgCtrlID();
	  if( id == IDC_TXT_FILE )
	    pDC->SetTextColor( RGB(0,0,255) );
	}

   return hbr;
 }

BOOL TEraseDlg::DestroyWindow()
 {
   BOOL bCanc = (WaitForSingleObject( m_evRemoved, 0 ) != WAIT_TIMEOUT);

   SafetyKillTh();
   m_fntDlg.Detach();
   
   BOOL bRes = CDialog::DestroyWindow();   
   if( !bCanc ) m_pUsSrc->InvalidateList();

   m_evCancelPassed.SetEvent();

   return bRes;
 }

void TEraseDlg::SafetyKillTh()
 {
   if( !m_arcTh() ) return;

   DWORD dwExitCode;
   if( GetExitCodeThread(m_arcTh()->m_hThread, &dwExitCode) && dwExitCode == STILL_ACTIVE )
	{
	  DWORD dwRes = m_arcTh()->ResumeThread();
	  if( dwRes != 0xFFFFFFFF && dwRes > 1 )
	   {
         while( dwRes > 1 )
		   dwRes = m_arcTh()->ResumeThread();
	   }

      m_evCancel.SetEvent();	  	  	  	  
	   	  
	  while( WaitForSingleObject(m_arcTh()->m_hThread, 0) == WAIT_TIMEOUT );
	}

   m_arcTh.FreeRc();
 }

void TNCNotifyE::Notinfy( TNotifyData *pNotifyData )
 {
   if( m_bEnabled == true )
     m_rDlg.NotifyProc( pNotifyData );
 }         

void TEraseDlg::NotifyProc( TNotifyData *pNotifyData_ )
 {                  
   TNotifyDataCopy *pNotifyData = (TNotifyDataCopy *)pNotifyData_;

   if( pNotifyData->m_bFlStartOp == true )
	{      
      CString str;
	  str.Format( "%s%s", pNotifyData->m_pSrc->IsRemote() == true ? "Link:":"",
	    pNotifyData->m_lpFromShortPath + m_iIndexInPath );

	  CClientDC dc( this );
      CFont *pFnOld = dc.SelectObject( &m_fntDlg );	           
      CompressPath( str, dc, m_rSrc );		
      dc.SelectObject( pFnOld );

      DWORD dw;   //IDC_TXT_FILE
      if( !::SendMessageTimeout(m_hWndSrc, WM_SETTEXT, 0, (LPARAM)(LPCSTR)str,
	    SMTO_NORMAL, 200, &dw) ) return;      	  
	}  
 }

static UINT ThreadProcE( LPVOID pObj )
 {
   ((TEraseDlg*)pObj)->StartOp();
   return 0;
 }

void TEraseDlg::StartOp()
 {   
   try {
     m_iIndexInPath = strlen( m_arcDupSrc()->GetPath() );

     m_arcDupSrc()->Erase( m_pUsSrc->GetSelectionVector(), true, m_arcNotify() );	 
	}
   catch( CFileException* pE )
	{
      //if( pE->m_strFileName.IsEmpty() ) pE->m_strFileName = m_usrUserOfSrv.GetPath();
      pE->ReportError( MB_OK | MB_ICONSTOP );
	  pE->Delete();  	        	  	  
	}
   catch( CException* pE )
	{	  
      pE->ReportError( MB_OK | MB_ICONEXCLAMATION );
	  pE->Delete();  	  	  
	} 
   catch( TUserCancel )
	{
	}
   catch(...)
	{
	}

   ::PostMessage( m_hWnd, WM_COMMAND, 
	  MAKEWPARAM(IDCANCEL, BN_CLICKED), 
	  (LPARAM)::GetDlgItem(m_hWnd, IDCANCEL) );
 }

void TEraseDlg::OnOK()
 {        
   m_arcNotify = new TNCNotifyE( this, m_evCancel, *this );

   GetDlgItem(IDOK)->EnableWindow( FALSE );
   GotoDlgCtrl( GetDlgItem(IDCANCEL) );   
   GetDlgItem(IDC_TXT_FILE)->ModifyStyle( 0, SS_CENTER ); 
   m_bFlBlue = true;

   m_arcTh = new TKBPassThread( ThreadProcE, this );   
   m_arcTh()->m_bAutoDelete = FALSE;
   m_arcTh()->CreateThread();   
 }

void TEraseDlg::OnCancel()
 {
   m_evCancel.SetEvent();
   SafetyKillTh();

   if( m_iOneItem != -1  ) m_pUsSrc->SelectIndex( m_iOneItem, false );

   CDialog::OnCancel();
 }

void TEraseDlg::DoDataExchange( CDataExchange* pDX )
 {
   CDialog::DoDataExchange( pDX );
 }

BOOL TEraseDlg::OnInitDialog()
 {
   CDialog::OnInitDialog();
       
   CString csSrc;   
   SetWindowText( "Erase file(s)/rirctory(s)" );
   
   LONG lNSel = m_pUsSrc->CalcSelected();
   if( lNSel )
	{
      LONG lDir =   m_pUsSrc->CalcSelected( _A_SUBDIR );
	  LONG lFile =  lNSel - lDir;

	  if( lDir && lFile )
	    csSrc.Format( "%s %lu files and %lu directories",
		  "Erase", lFile, lDir );
	  else if( lDir )
	    csSrc.Format( "%s %lu directories",
		  "Erase", lDir );
	  else
	   csSrc.Format( "%s %lu files",
		  "Erase", lFile );
	}
   else
	{
	  csSrc.Format( "%s '%s'",
		   "Erase", m_pUsSrc->GetFileAtCursor() );
	  m_iOneItem = m_pUsSrc->GetIndexAtCursor();
	  m_pUsSrc->SelectIndex( m_iOneItem, true );
	}
   
   CClientDC dc( this );
   CFont *pFnt = dc.SelectObject( GetFont() );   
   CSize szSrc = dc.GetTextExtent( csSrc );
   CSize szExtra = dc.GetTextExtent( "W" );   
   dc.SelectObject( pFnt );
   //szExtra.cx /= 2; szExtra.cy /= 2;

   int iCx;

   int iLeftWnd;
   int iLeftPane;
   int iTopWnd;
   int iTopPane;


   int iWndWP;

   CWnd *pOK;
   CRect r;

   int iWndWMin;
   int iWindowWidth;
   int iWindowHigh;

   int iHPane;
   int iHTxt;

   while( 1 )
	{
      pOK = GetDlgItem( IDOK );
	  pOK->GetWindowRect( r ); ScreenToClient( r );
	  iWndWMin = 5 * r.Width() + 2*GetSystemMetrics( SM_CXDLGFRAME );

	  iCx = szSrc.cx + 2*szExtra.cx;

	  if( iCx <= iWndWMin ) iCx = float(iCx) * 1.3;   

	  iHTxt = szExtra.cy + 2;	  
	
	  iLeftWnd = szExtra.cx/2; 
	  iLeftPane = szExtra.cx/2; 
	  iTopWnd = szExtra.cy / 2; 
	  iTopPane = szExtra.cy / 2;	  
	
	  iWndWP = 2*GetSystemMetrics(SM_CXDLGFRAME) + 
	   2*iLeftWnd + 
  	   2*iLeftPane + iCx;
		  
	
      iHPane = 2*iTopPane + iHTxt;

	  iWindowWidth = max( iWndWMin, iWndWP );
	  iCx = iWindowWidth - 2*iLeftWnd - 
	    2*GetSystemMetrics(SM_CXDLGFRAME) - 2*iLeftPane;

	  iWindowHigh = 2*GetSystemMetrics( SM_CYDLGFRAME ) +	  
	  GetSystemMetrics( SM_CYCAPTION ) +
	    r.Height() +
	  3*iTopWnd + iHPane;
	
	  if( GetSystemMetrics(SM_CXSCREEN)-20 < iWindowWidth )
	   {
	     const float fMul = 0.8;
	     //iCx = float(iCx) * fMul;
		 szSrc.cx = szSrc.cx * 0.8; 
	   }
	  else break;
	};   

   SetWindowPos( NULL, 0, 0, iWindowWidth, iWindowHigh, SWP_NOZORDER|SWP_NOREDRAW|SWP_NOMOVE|SWP_NOACTIVATE );

   CWnd *pPane = GetDlgItem( IDC_GRP_GRP );
   CWnd *pTxt = GetDlgItem( IDC_TXT_FILE );   
   CWnd *pCancel = GetDlgItem( IDCANCEL );
   
   pPane->SetWindowPos( NULL, iLeftWnd, iTopWnd, iCx+2*iLeftPane, 
	  iHPane, 
	  SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE 
	);

   pTxt->SetWindowPos( NULL, iLeftWnd + iLeftPane, 
	  iTopWnd + iTopPane, iCx, 
	  iHTxt, 
	  SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE 
	);
   
   CRect r2; GetClientRect( r2 );
   int iX = (r2.Width() - 3*r.Width()) / 2;
   CRect r3; pPane->GetWindowRect( r3 ); ScreenToClient( r3 );

   pOK->SetWindowPos( NULL, iX, 
	  r3.bottom + (r2.Height()-r3.bottom-r.Height())/2, 0, 
	  0, 
	  SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE|SWP_NOSIZE
	);   

   pCancel->SetWindowPos( NULL, iX + 2*r.Width(), 
	  r3.bottom + (r2.Height()-r3.bottom-r.Height())/2, 0, 
	  0, 
	  SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE|SWP_NOSIZE
	);   

   CenterWindow( m_mPar );

   m_hWndSrc   = pTxt->m_hWnd;
   
   m_fntDlg.Attach( (HFONT)SendMessage(WM_GETFONT, 0, 0) );
   pTxt->GetClientRect( m_rSrc );

   CClientDC dc2( this );
   CFont *pFnOld = dc.SelectObject( &m_fntDlg );	           
   CompressPath( csSrc, dc, m_rSrc );		
   dc2.SelectObject( pFnOld );

   pTxt->SetWindowText( csSrc );

   return FALSE;
 }
