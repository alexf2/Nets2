#include "cfg.hpp"
#include "nframe.hpp"
#include "coding.hpp"
#include "net.hpp"

#include <afxcmn.h>
#include <afxwin.h>
#include <typeinfo.h>

static LOGFONT lfStatus_INIT = { 8, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
      OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, 
	  VARIABLE_PITCH | FF_DONTCARE, "MS Sans Serif"
	};

static LOGFONT lfTtl_INIT = { 10, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
      OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, 
	  VARIABLE_PITCH | FF_DONTCARE, "MS Sans Serif"
	};

static LOGFONT lfList_INIT = { 10, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
      OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, 
	  VARIABLE_PITCH | FF_DONTCARE, "System"
	};


TMFOptions::TMFOptions()
 {
	d.m_bSaveMainFramePosOnExit = true;
	d.m_bRestoreMainFrame = true;
	
	//d.m_bLeftFull = d.m_bRightFull = false;
	d.m_iFullMode = 0;
	d.m_fDiv = 1.0;
	d.m_iActivePane = 0;
	d.m_iClmSize1 = d.m_iClmSize2 = 100;

	d.m_iBaud  = BAUD_2400;
    d.m_iStops = STOPBITS_10;
	d.m_iCOMPort = 1;

	d.m_iUsePoly = 0;

	d.m_dwTimeOutBrokenStop = 10000;	

    d.dwTOReqvest     = 2000;
    d.dwTOBroken      = 5000;
    d.dwTOWaitLARead  = 500;
    d.dwTOWaitLA      = 10000;

	d.m_bShowCPS = true;
    d.m_bShowErr = false;

	csSelMask1 = "*.*";
	csSelMask2 = "*.*";
	
	memset( &s, 0, sizeof(TScreenData) );
	
	dwCRC = 0;

    memcpy( &lfTtl, &lfTtl_INIT, sizeof(LOGFONT) );
	memcpy( &lfStatus, &lfStatus_INIT, sizeof(LOGFONT) );
	memcpy( &lfList, &lfList_INIT, sizeof(LOGFONT) );
 }


DWORD TMFOptions::CalcCRC()
 {
   DWORD dwKeep = dwCRC; dwCRC = 0;

   DWORD dwRes = 0;
   unsigned char *lp = (unsigned char *)&d;
   for( int i = sizeof(TOptData); i; --i, lp++ )
	 dwRes += *lp;

   dwCRC = dwKeep;
   return dwRes;
 }



void TMainFrame::SaveGlobalConfig()
 {
   TCodingBAKMirrorFile f;	  

   CString strPath( CString(((TNetApp*)AfxGetApp())->GetStartPath()) + CString(GLOBAL_CONFIG_FILENAME) );

   CFileException except;
   if( !f.Open((LPCSTR)strPath, CFile::shareExclusive | CFile::modeCreate | CFile::modeWrite, &except) )	      
	  AfxThrowFileException( except.m_cause, except.m_lOsError, (LPCSTR)strPath );	  
	   
   CArchive arch( &f, CArchive::store );

   arch << m_optOptions;	 
 }

void TMainFrame::LoadGlobalCongig()
 {
   TCodingBAKMirrorFile f;	  

   CString strPath( CString(((TNetApp*)AfxGetApp())->GetStartPath()) + CString(GLOBAL_CONFIG_FILENAME) );

   CFileException except;
   if( !f.Open((LPCSTR)strPath, CFile::shareExclusive | CFile::modeRead, &except) )
	{
      if( except.m_cause == CFileException::fileNotFound ) throw TExceptCfgNotFound();
	  AfxThrowFileException( except.m_cause, except.m_lOsError, (LPCSTR)strPath );	  
	}
	   
	CArchive arch( &f, CArchive::load );

	arch >> m_optOptions;	 
    if( m_optOptions.CheckCRC() == false ) throw TExceptCfgBadCRC();	
 }

void TMainFrame::CreateGlobalConfig()
 {
    TCodingBAKMirrorFile f;	  

    CString strPath( CString(((TNetApp*)AfxGetApp())->GetStartPath()) + CString(GLOBAL_CONFIG_FILENAME) );
	//LPSTR lpBuf = strPath.GetBuffer( 2*_MAX_PATH );
    //::GetTempFileName( ((TNetApp*)AfxGetApp())->GetStartPath(), "vsp", 0, lpBuf );
	//strPath.ReleaseBuffer();

	CFileException except;
    if( !f.Open((LPCSTR)strPath, CFile::shareExclusive | CFile::modeCreate | CFile::modeWrite, &except) )
	   AfxThrowFileException( except.m_cause, except.m_lOsError, (LPCSTR)strPath );
	   
	CArchive arch( &f, CArchive::store );

	arch << m_optOptions;	 
 }

void TMainFrame::KeepScreenConfig()
 {    
   m_optOptions.s.m_iScreenX = GetSystemMetrics( SM_CXSCREEN );
   m_optOptions.s.m_iScreenY = GetSystemMetrics( SM_CYSCREEN );
   GetWindowPlacement( &m_optOptions.s.m_wpFrame );
 }


void TMainFrame::LoadConfigOnStart()
 {
   m_bFlLoadingFrame = false;
   try {
     LoadGlobalCongig();
	 m_bFlLoadingFrame = true;
	}
   catch( TExceptCfgNotFound )
	{
      int iRes = ((TNetApp*)AfxGetApp())->bwcc.BWCCMessageBox( m_hWnd, 
	     "Файл конфигурации не найден. Создать его ?", 
		 "Вопрос", MB_YESNO | MB_ICONQUESTION );

	  if( iRes == IDYES )
	   {
	     try {
		    KeepScreenConfig();
            CreateGlobalConfig();
			//m_bFlLoadingFrame = true;
		  }
		 catch( CArchiveException* pE )
		  {	  		  
		    if( pE->m_strFileName.IsEmpty() ) pE->m_strFileName = GLOBAL_CONFIG_FILENAME;
            pE->ReportError( MB_OK | MB_ICONEXCLAMATION );
	        pE->Delete();  
		  }
		 catch( CFileException* pE )
		  {
            if( pE->m_strFileName.IsEmpty() ) pE->m_strFileName = GLOBAL_CONFIG_FILENAME;
            pE->ReportError( MB_OK | MB_ICONSTOP );
	        pE->Delete();  	        
		  }
	     catch( CException* pE )
		  {	  
            pE->ReportError( MB_OK | MB_ICONEXCLAMATION );
	        pE->Delete();  
		  } 
	   }
	}
   catch( TExceptCfgBadCRC )
	{
      ((TNetApp*)AfxGetApp())->bwcc.BWCCMessageBox( m_hWnd, 
	     "Файл конфигурации испорчен. Его можно стереть и перегрузить Visyal Expert.", 
		 "Предупреждение", MB_OK );
	}
   catch( CArchiveException* pE )
	{	  		  
	  if( pE->m_strFileName.IsEmpty() ) pE->m_strFileName = GLOBAL_CONFIG_FILENAME;
      pE->ReportError( MB_OK | MB_ICONEXCLAMATION );
	  pE->Delete();  
	}
   catch( CFileException* pE )
	{
      if( pE->m_strFileName.IsEmpty() ) pE->m_strFileName = GLOBAL_CONFIG_FILENAME;
      pE->ReportError( MB_OK | MB_ICONSTOP );
	  pE->Delete();  	  
	}
   catch( CException* pE )
	{	  
      pE->ReportError( MB_OK | MB_ICONEXCLAMATION );
	  pE->Delete();  
	} 
   
 }
