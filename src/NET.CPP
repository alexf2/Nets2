#include "net.hpp"
#include <locale.h>
#include "RC\\net.rh"

#include "nframe.hpp"
#include "autorc.hpp"
#include "..\\RUDAKOW\\SERVIS\\texcept.hpp"
//#include "mgraf.rh"

#include "..\\RUDAKOW\\SERVIS\\servis.hpp"
#include "..\\RUDAKOW\\SERVIS\\mydib.hpp"

#include <afxcmn.h>
#include <afxdisp.h>



BEGIN_MESSAGE_MAP( TNetApp, CWinApp )	
END_MESSAGE_MAP()


TNetApp::TNetApp(): CWinApp( "TNetApplication" ), bwcc( NULL )
 {

 }

BOOL TNetApp::OnIdle( LONG lCount )
 {
   //if( m_pMainWnd ) ((TMainFrame*)m_pMainWnd)->MyIdle( lCount );
   return CWinApp::OnIdle( lCount );
 }


BOOL TNetApp::InitInstance()
 {
   setlocale( LC_ALL, "" );

   LPSTR lpBuf = m_Startpath.GetBuffer( _MAX_PATH );
   ::GetModuleFileName( m_hInstance, lpBuf, _MAX_PATH );
   
    char drive[ _MAX_DRIVE ];
    char dir[ _MAX_DIR ];
    char fname[ _MAX_FNAME ];
    char ext[ _MAX_EXT ];

	_splitpath( lpBuf, drive, dir, fname, ext );
	_makepath( lpBuf, drive, dir, "", "" );	  	 

	m_Startpath.ReleaseBuffer();

   try {

     InitServisDLL();
	 InitMyDIBDLL();

     Enable3dControls();
	 //AfxEnableControlContainer();
     bwcc.EnableBWCC( m_hInstance, TRUE );     
	}
   catch( TExcept rExept )
    {
      rExept.ReportExcept();
	  rExept.Delete();

	  return FALSE;
	}
   catch( CException* pE )
	{
      pE->ReportError();
	  pE->Delete();

	  return FALSE;
	}
   catch(...)
    {
      AfxMessageBox( "Any exception", MB_OK ); 

	  return FALSE;
	}


   TAutoMemoryRc<TMainFrame*> arcFrame( new TMainFrame() );
   if( !(arcFrame())->LoadFrame(IDR_MAINFRAME) ) return FALSE;
   
   m_pMainWnd = arcFrame();
   arcFrame.ClearRc();

   m_pMainWnd->ShowWindow( m_nCmdShow );
   m_pMainWnd->UpdateWindow();

   return TRUE;
 }


TNetApp  myApp;
