#include "nframe.hpp"

#include "RC\net.rh"
#include "net.hpp"
#include "autorc.hpp"

#include <afxcmn.h>
#include <afxwin.h>
#include <typeinfo.h>
#include <afxmt.h>

#include "about.hpp"
#include "ncpane.hpp"
#include "drvsel.hpp"
#include "drv2dlg.hpp"
#include "fdlg.hpp"
#include "optdlg.hpp"

#include <strstream>
#include <iomanip>

#include "..\\RUDAKOW\\SERVIS\\pathex.hpp"


IMPLEMENT_DYNCREATE( TMainFrame, CFrameWnd )


BEGIN_MESSAGE_MAP( TMainFrame, CFrameWnd )
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_WM_ERASEBKGND()

	ON_WM_ACTIVATE()

	ON_UPDATE_COMMAND_UI( AFX_IDW_TOOLBAR, CFrameWnd::OnUpdateControlBarMenu )
	ON_COMMAND_EX( AFX_IDW_TOOLBAR, CFrameWnd::OnBarCheck )
	
	
	ON_UPDATE_COMMAND_UI( ID_DRIVE, OnUpdateOp )
	ON_UPDATE_COMMAND_UI( ID_COPY, OnUpdateOpSingl )
	ON_UPDATE_COMMAND_UI( ID_RENAME, OnUpdateOpSingl )
	ON_UPDATE_COMMAND_UI( ID_ERASE, OnUpdateOp )
	

	ON_UPDATE_COMMAND_UI( ID_CONNECT, OnAlways )
	ON_UPDATE_COMMAND_UI( ID_OPTIONS, OnUpdateOpt )	  
		
	ON_COMMAND( ID_APP_ABOUT, OnAbout )

	ON_COMMAND( ID_DRIVE, OnDrive )
	ON_COMMAND( ID_DRIVE1, OnDrive1 )
	ON_COMMAND( ID_DRIVE2, OnDrive2 )
	ON_COMMAND( ID_COPY, OnCopy )
	ON_COMMAND( ID_RENAME, OnRename )
	ON_COMMAND( ID_ERASE, OnErase )

	ON_COMMAND( ID_FULLPANE1, OnFullPane1 )
	ON_COMMAND( ID_FULLPANE2, OnFullPane2 )

	ON_COMMAND( ID_CONNECT, OnConnect )
	ON_COMMAND( ID_OPTIONS, OnOptions )

	ON_COMMAND( ID_CHPANE, OnChPane )
	ON_COMMAND( ID_ZOOM, OnZoom )

	ON_COMMAND( ID_MYMENU, OnIDMyMenu )

	ON_COMMAND( ID_SELECT, OnSelectF )
	ON_COMMAND( ID_UNSELECT, OnUnSelectF )

	ON_COMMAND( ID_REFRESH, OnRefresh )

	ON_WM_DEVICECHANGE()

END_MESSAGE_MAP()


void TMainFrame::OnActivate( UINT nState, CWnd* pWndOther, BOOL bMinimized )
 {
   CFrameWnd::OnActivate( nState, pWndOther, bMinimized );
   
   if( m_wndSplit.GetSafeHwnd() ) m_wndSplit.SetActivePaneMy( m_wndSplit.GetActivePaneMy() );
 }

void TMainFrame::OnAlways( CCmdUI* pCmdUI )
 {
   pCmdUI->Enable( TRUE );
 }

void TMainFrame::OnUpdateOpt( CCmdUI* pCmdUI )
 {
   BOOL bRes = ::WaitForSingleObject(m_evEnableConnect, 0)==WAIT_TIMEOUT;
   pCmdUI->Enable( bRes );
   if( bRes && m_pmsOpt->GetCheckMy() == true )
	 m_pmsOpt->SetCheckMy( false );
 }

void TMainFrame::OnUpdateOpSingl( CCmdUI* pCmdUI )
 {
   //pCmdUI->Enable( !m_wndSplit.GetFullMode() );   
   pCmdUI->Enable( TRUE );
 }

void TMainFrame::OnUpdateOp( CCmdUI* pCmdUI )
 {
   pCmdUI->Enable( TRUE );
 }


TMainFrame::TMainFrame():
 m_evEnableConnect( FALSE, TRUE ),
 m_evCancelConnect( TRUE, TRUE ),
 m_evConnectOK( FALSE, TRUE ),
 m_evConnectBroken( TRUE, TRUE )
 {
   m_parrMenuBmp = NULL;
   m_bFlLoadingFrame = false;
   m_bFlShow = false;   
   m_bSplitCreated = false;

   m_iPrivCps = m_iPrivErr = 0;

   iPrivPage = 0;
 }

TMainFrame::~TMainFrame()
 {
   if( m_parrMenuBmp )
	{
      delete[] m_parrMenuBmp;
	  m_parrMenuBmp = NULL;
	}
 }

void TMainFrame::SplitterModfyNotify()
 {
   if( m_bSplitCreated == false ) return;

   //if( m_optOptions.d.m_bLeftFull == true || m_optOptions.d.m_bRightFull == true ) return;
   if( m_optOptions.d.m_iFullMode ) return;

   int iCur, iMin;
   m_wndSplit.GetColumnInfo( 0, iCur, iMin );
   int iCur2, iMin2;
   m_wndSplit.GetColumnInfo( 1, iCur2, iMin2 );
   m_optOptions.d.m_fDiv = float(iCur) / float(iCur2);
 }

void TMainFrame::RecalcLayout( BOOL bNotify )
 {   
   if( m_bSplitCreated == true ) 
	{
   CRect r; GetClientRect( &r );
   /*if( m_optOptions.d.m_iFullMode == 1 )
	{
	  m_wndSplit.SetColumnInfo( 0, r.Width(), 0 );
	}
   else if( m_optOptions.d.m_iFullMode == 2 )
	{
	  m_wndSplit.SetColumnInfo( 1, r.Width(), 0 );
	}
   else
	{*/
	  int iCur, iMin;
	  int cx, cy;
 
	  int iFull;
      if( (iFull=m_wndSplit.GetFullMode()) )
	   {
         if( iFull == 1 )
		  {
		    m_wndSplit.SetColumnInfo( 0, r.Width(), 0 );
			if( m_wndSplit(1)->IsWindowVisible() )
			  m_wndSplit(1)->ShowWindow( SW_HIDE );			
		  }
		 else
		  {
		    m_wndSplit.SetColumnInfo( 0, 0, 0 );
		    m_wndSplit.SetColumnInfo( 1, r.Width(), 0 );
			if( m_wndSplit(0)->IsWindowVisible() )
			  m_wndSplit(0)->ShowWindow( SW_HIDE );			
		  }
	   } 
	  else
	   {
	     m_wndSplit.Get_Amount( cx, cy );
	     r.right -= cx;
	     m_wndSplit.GetColumnInfo( 0, iCur, iMin );
	     m_wndSplit.SetColumnInfo( 0, 
	       float(r.Width())/(1 + 1/m_optOptions.d.m_fDiv), iMin );	  	

		 if( !m_wndSplit(0)->IsWindowVisible() ) m_wndSplit(0)->ShowWindow( SW_SHOW );
	     if( !m_wndSplit(1)->IsWindowVisible() ) m_wndSplit(1)->ShowWindow( SW_SHOW );
	   }

	//}

	//m_wndSplit.RecalcLayout();
	}

   CFrameWnd::RecalcLayout( bNotify ); 
 }



void TMainFrame::DriveChange( int iDr )
 {
   CWinApp *pApp = AfxGetApp();
   int iPrior = pApp->GetThreadPriority();
   pApp->SetThreadPriority( THREAD_PRIORITY_HIGHEST );

   try{
     int iSelDrive = m_wndSplit(iDr)->GetDriveIndex();
     TDriveSelDlg dlg( m_wndSplit(iDr), m_FSrvForMe, 
	   iSelDrive, iDr ? "Choose right Drive":"Choose left Drive" );
     if( dlg.DoModal() == IDOK && iSelDrive != -1 )
	  {
        m_wndSplit(iDr)->SetDriveIndex( iSelDrive );
	  }
	}
   catch( CException *pE )
	{
      pApp->SetThreadPriority( iPrior );
	  throw pE;
	}

   pApp->SetThreadPriority( iPrior );
 }

void TMainFrame::OnDrive()
 {
   DriveChange( m_wndSplit.GetActivePaneMy() );
 }
void TMainFrame::OnDrive1()
 {
   DriveChange( 0 );
 }
void TMainFrame::OnDrive2()
 {
   DriveChange( 1 );
 }

void TMainFrame::OnFullPane1()
 {
   m_optOptions.d.m_iFullMode = m_wndSplit.GetFullMode() == 1 ? 0:1;
   m_wndSplit.SetFullMode( m_optOptions.d.m_iFullMode );
 }

void TMainFrame::OnFullPane2()
 {
   m_optOptions.d.m_iFullMode = m_wndSplit.GetFullMode() == 2 ? 0:2;
   m_wndSplit.SetFullMode( m_optOptions.d.m_iFullMode );
 }



void TMainFrame::MakeCopy( bool bFlErase )
 {
   TNortonLikePane *pPane = m_wndSplit.GetActivePane_Int();
   if( !pPane ) return;
   TUserOfDrive_NCPane *pUsSrc = pPane->GetUser(); 

   LPSTR lp = pUsSrc->GetFileAtCursor();
   if( (!lp || !*lp || *lp == '.') && !pUsSrc->CalcSelected() ) return;
   
   TNortonLikePane *pPaneDst; 
   
   if( m_wndSplit.GetFullMode() ) 
	pPaneDst = m_wndSplit( m_wndSplit.GetActivePaneMy() );
   else
	pPaneDst = m_wndSplit( m_wndSplit.GetActivePaneMy() ? 0:1 );

   if( !pPaneDst ) return;

   TUserOfDrive_NCPane *pUsDst = pPaneDst->GetUser(); 

   TCopyDlg dlg( this, bFlErase, pUsSrc, pUsDst, m_evConnectBroken );
   dlg.DoModal();
 }


void TMainFrame::MakeErase()
 {
   TNortonLikePane *pPane = m_wndSplit.GetActivePane_Int();
   if( !pPane ) return;
   TUserOfDrive_NCPane *pUsSrc = pPane->GetUser(); 

   LPSTR lp = pUsSrc->GetFileAtCursor();
   if( (!lp || !*lp || *lp == '.') && !pUsSrc->CalcSelected() ) return;
   
   TEraseDlg dlg( this, pUsSrc, m_evConnectBroken );
   dlg.DoModal();
 }


void TMainFrame::OnCopy()
 {
   MakeCopy( false );
 }

void TMainFrame::OnRename()
 {
   MakeCopy( true );
 }

void TMainFrame::OnErase()
 {
   MakeErase();
 }

void TMainFrame::OnConnect()
 {
   if( ::WaitForSingleObject(m_evEnableConnect, 0) == WAIT_TIMEOUT )
	{
      m_evEnableConnect.SetEvent();	  
	  m_evCancelConnect.ResetEvent();
	  m_pmsOpt->SetCheckMy( true );
	}
   else 
	{ 
	  m_evEnableConnect.ResetEvent();
	  m_evCancelConnect.SetEvent();
	  m_pmsOpt->SetCheckMy( false );
	}
 }

void TMainFrame::OnOptions()
 {
   TMFOptions tmp( m_optOptions );

   TUsersDialog dlg( tmp, "Настройка", this, iPrivPage );
   if( dlg.DoModal() == IDOK )
	{
	  m_optOptions = tmp;
      DispBCE( m_optOptions.d.m_iBaud, 0, 0 );	   
	  iPrivPage = dlg.m_iPage;
	}   
 }

void TMainFrame::OnChPane()
 {
   if( m_wndSplit.GetFullMode() ) return;
   m_wndSplit.SetActivePaneMy( m_wndSplit.GetActivePaneMy() ? 0:1 );
 }

void TMainFrame::OnIDMyMenu()
 {
   if( m_arcTool() ) 
	{
      m_arcTool()->SetFocus();
	  CWnd *pW = m_arcTool()->GetWindow( GW_CHILD );
	  if( pW ) pW->SetFocus();
	}
 }

void TMainFrame::SelectInstance( bool bFlSelect )
 {
   CWinApp *pApp = AfxGetApp();
   int iPrior = pApp->GetThreadPriority();
   pApp->SetThreadPriority( THREAD_PRIORITY_HIGHEST );

   try{
      TSelectDialog dlg( m_wndSplit.GetActivePane_Int(), 
	    (LPSTR)m_wndSplit.GetSelectMask( m_wndSplit.GetActivePaneMy() ), 
	    (LPSTR)(bFlSelect == true ? "Select mask:":"Unselect mask:") );
      int iRes = dlg.DoModal();
      if( iRes == IDOK ) 
	   {
	     m_wndSplit.SetSelectMask( m_wndSplit.GetActivePaneMy(), dlg.m_csMask );
	     m_wndSplit.Select( m_wndSplit.GetActivePaneMy(), dlg.m_csMask, bFlSelect );	
	   }
	}
   catch( CException *pE )
	{
      pApp->SetThreadPriority( iPrior );
	  throw pE;
	}

   pApp->SetThreadPriority( iPrior );
 }

void TMainFrame::OnSelectF()
 {
   SelectInstance( true );   
 }

void TMainFrame::OnUnSelectF()
 {
   SelectInstance( false );   
 }

void TMainFrame::OnRefresh()
 {
   m_wndSplit.Refresh();
 }

void TMainFrame::OnZoom()
 {
   if( IsZoomed() ) ShowWindow( SW_RESTORE );
   else if( !IsIconic() ) ShowWindow( SW_SHOWMAXIMIZED );
 }

void TMainFrame::OnAbout()
 {
   try {
     AboutShow( this, IDB_DIB_ABOUT256 );
	}
   catch( CException *pE )
	{	  
	  pE->ReportError( MB_OK | MB_ICONSTOP );
	  pE->Delete();  	
	  return;
	}   
 }


static char* const lpButtonsText[] =
 {  
    "Drive/~D~",
	"Copy/~F5~",
	"Rename/~F6~",
	"Erase/~F8~",

	"",

	"Connect/~C~",
	"Options/~O~",

	"",
	
    "Exit/~X~",
	
	"",

	"About"    
 };

struct TBtnDescr
 {
   UINT uCmd;
   bool bRight;
   bool bPush;
 };

static TBtnDescr buttons[] =
 {
  { ID_DRIVE, false, true },
  { ID_COPY, false, true },
  { ID_RENAME, false, true },
  { ID_ERASE, false, true },  

	{ ID_SEPARATOR,	false, true },

  { ID_CONNECT, false, false },  
  { ID_OPTIONS, false, true },  

  	{ ID_SEPARATOR,	false, true },  

  { ID_APP_EXIT, false, true },  

	{ ID_SEPARATOR,	false, true },

  { ID_APP_ABOUT, true, true }
 };

static UINT buttonsRC[] =
 {   
   IDB_DRIVE,
   IDB_COPY,
   IDB_MOVE,
   IDB_ERASE,
    0,
   IDB_CONNECT,
   IDB_OPTIONS,
    0,
   IDB_EXIT,
    0,
   IDB_ABOUT
 };



static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator	    
	ID_LAMP,
	ID_CFG,
};


BOOL TMainFrame::PreCreateWindow( CREATESTRUCT& cs )
 {   
   //cs.style |= WS_HSCROLL | WS_HSCROLL;
   //cs.dwExStyle |= 

    //if( !cs.hMenu ) return CMDIFrameWnd::PreCreateWindow( cs );

    BOOL bRes = CFrameWnd::PreCreateWindow( cs );
    if( !bRes ) return bRes;

	cs.lpszClass = AfxRegisterWndClass( 0,
	  AfxGetApp()->LoadStandardCursor(IDC_ARROW),
	  (HBRUSH)GetStockObject(GRAY_BRUSH),
	  AfxGetApp()->LoadIcon(IDR_MAINFRAME) );
	if( !cs.lpszClass ) return FALSE;

	cs.style |= WS_CLIPCHILDREN;
    
    return bRes;
 }


void TMainFrame::OnClose()
 {
   if( m_optOptions.d.m_bSaveMainFramePosOnExit == true )
	{  
	  try{
	    KeepScreenConfig();
		KeepPanes();
        SaveGlobalConfig();		
	   }
	  catch( CArchiveException* pE )
	   {	  		  
	     if( pE->m_strFileName.IsEmpty() ) pE->m_strFileName = GLOBAL_CONFIG_FILENAME;
         pE->ReportError( MB_OK | MB_ICONEXCLAMATION );
	     pE->Delete();  
	   }
	  catch( CException* pE )
	   {	  
         pE->ReportError( MB_OK | MB_ICONEXCLAMATION );
	     pE->Delete();  
	   } 	   	   
	}
 
   CFrameWnd::OnClose();
 }


void TMainFrame::OnDestroy()
 {
   //m_arcTool()->Detach();

   CFrameWnd::OnDestroy();
 }

BOOL TMainFrame::DestroyWindow()
 {
   m_evEnableConnect.ResetEvent();
   m_evConnectOK.ResetEvent();
   m_evCancelConnect.SetEvent();   
   m_evConnectBroken.SetEvent();

   m_arcRM1.FreeRc();
   m_arcRM2.FreeRc();

   m_arcPhysLvl.FreeRc();   

   return CFrameWnd::DestroyWindow();
 }

int TMainFrame::OnCreate( LPCREATESTRUCT lpCreateStruct )
 {
   if( lpCreateStruct->hMenu ) 
	 {
	   DestroyMenu( lpCreateStruct->hMenu );
	   lpCreateStruct->hMenu = NULL;
	   m_hMenuDefault = NULL;
	 }


    LoadConfigOnStart();
   

	if( CFrameWnd::OnCreate(lpCreateStruct) == -1 )
	  return -1;	


	m_arcTool = new TMSStyleTooolBar( this );
		
	for( int i = 0; i < sizeof(buttons) / sizeof(TBtnDescr); i++ )
	 {
       
	   TMSItem *pMsIt;
	   pMsIt = new TMSItem(); 
	   pMsIt->Init( lpButtonsText[i], buttonsRC[i], buttons[i].uCmd, buttons[i].bRight, buttons[i].bPush );
	   *m_arcTool(), pMsIt;

	   if( buttons[i].bPush == false ) 
		 m_pmsOpt = pMsIt;
	 }
	
    LOGFONT lf = { 8, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
      OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, 
	  VARIABLE_PITCH | FF_DONTCARE, "MS Sans Serif"
	};

	CFont *pFnt = new CFont();
	pFnt->CreateFontIndirect( &lf );
    m_arcTool()->SetFont( pFnt );
	
    m_arcTool()->Init( false ); 

    if( !m_arcTool()->Create(this, WS_CHILD | WS_VISIBLE |
		 	CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY,  AFX_IDW_TOOLBAR ) 
		//!m_arcTool()->LoadBitmap(IDR_MAINFRAME) 		
	  )
	{
		//AfxMessageBox( "Failed to create toolbar\n", MB_OK | MB_ICONSTOP );
	    ((TNetApp*)AfxGetApp())->bwcc.BWCCMessageBox( m_hWnd, "Ошибка при создании тулбара", "Ошибка", MB_OK | MB_ICONSTOP );
		return -1;      
	}

			 	    
	if( !m_wndStatusBar.Create(this, WS_CHILD | WS_VISIBLE | CBRS_BOTTOM , AFX_IDW_STATUS_BAR) ||
	    !m_wndStatusBar.SetIndicators(indicators,
	    sizeof(indicators)/sizeof(UINT))
	  )
	{
		//TRACE0("Failed to create status bar\n");
	    ((TNetApp*)AfxGetApp())->bwcc.BWCCMessageBox( m_hWnd, "Ошибка при создании строки статуса", "Ошибка", MB_OK | MB_ICONSTOP );
		return -1;      
	}

    //m_indOnLine, m_indIn, m_indOut;
    m_fInd.Attach( (HFONT)*m_wndStatusBar.GetFont() );

	m_arcIndOnLine = new TIndicator( "OnLine", m_fInd );
	m_arcIndIn     = new TIndicator( "In", m_fInd ); 
	m_arcIndOut    = new TIndicator( "Out", m_fInd );	

	

	CRect rLamp( 0, 0, 10, 10 );
	m_arcIndOnLine()->Create( NULL, SS_USERITEM|WS_CHILD|WS_VISIBLE, rLamp, &m_wndStatusBar, 
	                    ID_IND_ONLINE );
	m_arcIndIn()->Create( NULL, SS_USERITEM|WS_CHILD|WS_VISIBLE, rLamp, &m_wndStatusBar, 
	                ID_IND_IN );
	m_arcIndOut()->Create( NULL, SS_USERITEM|WS_CHILD|WS_VISIBLE, rLamp, &m_wndStatusBar, 
	                 ID_IND_OUT );

	TAutoMemoryRc<TConnectToItem*> arcIt;
	arcIt = new TConnectToItem( 1 );
	*(arcIt()) + m_arcIndOnLine() + m_arcIndIn() + m_arcIndOut();
	TConnectToItem* pItem = arcIt();
	arcIt.ClearRc();
	m_wndStatusBar + pItem;

	int iLampW = m_arcIndOnLine()->GetW() + 
	             m_arcIndIn()->GetW() + 
				 m_arcIndOut()->GetW() + 2;
	
	UINT nID, nStyle;
	int cxWidth;
	m_wndStatusBar.GetPaneInfo( 0, nID, nStyle, cxWidth );
	m_wndStatusBar.SetPaneInfo( 0, nID, SBPS_STRETCH | SBPS_NORMAL, cxWidth );	

	m_wndStatusBar.SetPaneText( ID_LAMP, "" );
	m_wndStatusBar.GetPaneInfo( ID_LAMP, nID, nStyle, cxWidth );
	m_wndStatusBar.SetPaneInfo( ID_LAMP, nID, 
	                            SBPS_NOBORDERS|SBPS_POPOUT, iLampW );	

	char cBuf[100];
    sprintf( cBuf, "Baud:%10s, CPS:%5u, Bad packet:%7u",
	 GetBaudEq( BAUD_115200 ), 10000, 10000 );	
	
	//BAUD_115200
    CClientDC dc( &m_wndStatusBar );
	CFont *pFOld = dc.SelectObject( m_wndStatusBar.GetFont() );
	CSize szTx = dc.GetTextExtent( cBuf, strlen(cBuf) );
	dc.SelectObject( pFOld );

	sprintf( cBuf, "Baud:%10s, CPS:%5u, Bad packet:%7u",
	 GetBaudEq( m_optOptions.d.m_iBaud ), 0, 0 );	
	
	//DispBCE( m_optOptions.d.m_iBaud, 0, 0 );
	m_wndStatusBar.SetPaneText( ID_CFG, cBuf );		
	m_wndStatusBar.GetPaneInfo( ID_CFG, nID, nStyle, cxWidth );
	m_wndStatusBar.SetPaneInfo( ID_CFG, nID, SBPS_NORMAL, szTx.cx );	
	//ID_CFG,
	//ID_LAMP

	m_arcPhysLvl = new TPhysicalLevel( m_optOptions, 
	  m_evConnectOK,
	  m_evEnableConnect,
	  m_evCancelConnect,
	  m_evConnectBroken,
	  *m_arcIndOnLine(),
	  *m_arcIndIn(),
	  *m_arcIndOut(), m_wndStatusBar );
	

	m_arcChLocal  = new TChannel( NUMB_CHANNEL_SRVLOCAL, *m_arcPhysLvl(), m_evConnectOK );
	m_arcChRemote = new TChannel( NUMB_CHANNEL_SRVREMOTE, *m_arcPhysLvl(), m_evConnectOK );

	m_arcRM1 = new TRemoteManeger( &m_FSrvForMe, m_arcChLocal(), m_arcChRemote(), m_evConnectOK, false );	
	m_arcRM2 = new TRemoteManeger( &m_FSrvForYou, m_arcChLocal(), m_arcChRemote(), m_evConnectOK, true );	
    
	m_arcPhysLvl()->StartWorkCycleOut();
    m_arcPhysLvl()->StartWorkCycleIn();  

	m_arcRM1()->StartWorkCycle();
	m_arcRM2()->StartWorkCycle();
	
	return 0;
 }


void TMainFrame::DispBCE( int iBaud, int iCPS, int iErr )
 {
   char cBuf[100];
   sprintf( cBuf, "Baud:%10s, CPS:%5u, Bad packet:%7u",
	 GetBaudEq( iBaud ),
	 iCPS, iErr
	);

   /*DefWindowProc(SB_SETTEXT, ((WORD)pSBP->nStyle)|nIndex,
		(pSBP->nStyle & SBPS_DISABLED) ? NULL :
		(LPARAM)(LPCTSTR)pSBP->strText);*/   
   //m_wndStatusBar.SetPaneText( ID_CFG, cBuf );
   DWORD dw;
   ::SendMessageTimeout( m_wndStatusBar.GetSafeHwnd(), SB_SETTEXT, 
	  (WPARAM)ID_CFG, (LPARAM)cBuf,
	  SMTO_NORMAL, 500, &dw );	  
 }

BOOL TMainFrame::OnCreateClient( LPCREATESTRUCT lpcs, CCreateContext* pContext )
 {
   if( !m_FSrvForMe.Create(this,1) || !m_FSrvForYou.Create(this,2) )
	{
      ((TNetApp*)AfxGetApp())->bwcc.BWCCMessageBox( m_hWnd, "Ошибка при создании файловых серверов", "Ошибка", MB_OK | MB_ICONSTOP );
	  return FALSE;
	}      

   m_wndSplit.AssFServer( &m_FSrvForMe );
   m_wndSplit.SetCfg( &m_optOptions );

   if( !m_wndSplit.CreateStatic(this, 1, 2) ) return FALSE;

   int cx, cy;
   //m_wndSplit.Get_SizeBar( cx, cy );
   //m_wndSplit.Set_SizeBar( 25, 25 );

   //m_wndSplit.Get_Space( cx, cy );
   //m_wndSplit.Set_Space( -1, -1 );   

   //m_wndSplit.Get_Amount( cx, cy );
   //m_wndSplit.Set_Amount( 25, 25 );   

   //m_wndSplit.Get_Border( cx, cy );
   //m_wndSplit.Set_Border( 25, 25 );   
   

   CRect r; GetClientRect( &r );
   //CSize sz( r.Width() / 2, r.Height() );
   CSize sz( 0, 0 );
   

   if( !m_wndSplit.CreateView(0, 0, RUNTIME_CLASS(TNortonLikePane), 
	 sz, pContext) ) return FALSE;
   
   if( !m_wndSplit.CreateView(0, 1, RUNTIME_CLASS(TNortonLikePane), 
	 sz, pContext) ) return FALSE;
   

   m_bSplitCreated = true;

   SetupPanes();

   return TRUE;
 }

void TMainFrame::SetupPanes()
 {
   m_wndSplit.SetFullMode( m_optOptions.d.m_iFullMode );   

   m_wndSplit.SetClmSize( 0, m_optOptions.d.m_iClmSize1 );
   m_wndSplit.SetClmSize( 1, m_optOptions.d.m_iClmSize2 );

   m_wndSplit.SetPath( 0, m_optOptions.c_strPath1 );
   m_wndSplit.SetPath( 1, m_optOptions.c_strPath2 );

   m_wndSplit.SetActivePaneMy( m_optOptions.d.m_iActivePane );

   m_wndSplit.SetSelectMask( 0, m_optOptions.csSelMask1 );
   m_wndSplit.SetSelectMask( 1, m_optOptions.csSelMask2 );
 }

void TMainFrame::KeepPanes()
 {
   m_optOptions.d.m_iFullMode = m_wndSplit.GetFullMode();
   m_optOptions.d.m_iActivePane = m_wndSplit.GetActivePaneMy();

   m_optOptions.d.m_iClmSize1 = m_wndSplit.GetClmSize( 0 );
   m_optOptions.d.m_iClmSize2 = m_wndSplit.GetClmSize( 1 );

   m_optOptions.c_strPath1 = m_wndSplit.GetPath( 0 );
   m_optOptions.c_strPath2 = m_wndSplit.GetPath( 1 );

   m_optOptions.csSelMask1 = m_wndSplit.GetSelectMask( 0 );
   m_optOptions.csSelMask2 = m_wndSplit.GetSelectMask( 1 );
 }

void TMainFrame::OnShowWindow( BOOL bShow, UINT nStatus )
 {
   CFrameWnd::OnShowWindow( bShow, nStatus );

   if( m_bFlShow == false ) 
	{	 
      m_bFlShow = true;

	  if( m_bFlLoadingFrame == true && m_optOptions.d.m_bRestoreMainFrame == true )
	   {
         int cx = GetSystemMetrics( SM_CXSCREEN );
	     int cy = GetSystemMetrics( SM_CYSCREEN );

	     if( GetSystemMetrics(SM_CXSCREEN) != m_optOptions.s.m_iScreenX ||
		     GetSystemMetrics(SM_CYSCREEN) != m_optOptions.s.m_iScreenY
		   ) 
	      DefaultPos();
	     else				  
	      SetWindowPlacement( &m_optOptions.s.m_wpFrame );	      				  		  		
	   }	
	  else
	    DefaultPos();
	}
 }

void TMainFrame::DefaultPos()
 {
   TMSStyleTooolBar	*pBar = (TMSStyleTooolBar*)GetDescendantWindow( AFX_IDW_TOOLBAR );
   if( !pBar || typeid(*pBar) != typeid(TMSStyleTooolBar) ) return;

   int iWidth = pBar->GetReqvWidth() + 20 + 2*GetSystemMetrics( SM_CXBORDER );
   int iScrW = GetSystemMetrics( SM_CXSCREEN );
   int iScrH = GetSystemMetrics( SM_CYSCREEN );
   int iWOpt = float(iScrW) * (2.0/3.0);
   int iHOpt = float(iScrH) * (2.0/3.0);

   int iW = min( max(iWidth, iWOpt), iScrW);

   m_optOptions.d.m_fDiv = 1;
   //m_optOptions.d.m_bLeftFull = false;
   //m_optOptions.d.m_bRightFull = false;
   m_optOptions.d.m_iFullMode = 0;

   SetWindowPos( NULL, (iScrW - iW)/2, (iScrH - iHOpt)/2, iW, iHOpt, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW );
 }

BOOL TMainFrame::OnEraseBkgnd( CDC* pDC )
 {
   return TRUE;
   //return CFrameWnd::OnEraseBkgnd( pDC );
 }


BOOL TMainFrame::OnDeviceChange( UINT nEventType, DWORD dwData )
 {
   m_FSrvForMe.OnDeviceChange_My( nEventType, dwData );
   m_FSrvForYou.OnDeviceChange_My( nEventType, dwData );

   return CFrameWnd::OnDeviceChange( nEventType, dwData );
 }
