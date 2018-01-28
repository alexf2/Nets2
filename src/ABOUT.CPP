#include "stdafx.h"

#include "about.hpp"
#include "autorc.hpp"

#include "..\\RUDAKOW\\SERVIS\\fullscr.hpp"
#include "..\\RUDAKOW\\SERVIS\\dibapi.h"
#include "..\\RUDAKOW\\SERVIS\\lzfile.hpp"


void AboutShow( CWnd* pParent, UINT uiRc )
 {

   HINSTANCE hHan;
   HRSRC hrc = FindResource( (hHan=AfxGetResourceHandle()), MAKEINTRESOURCE(uiRc), "BITMAP256" );
   if( !hrc ) AfxThrowResourceException();
	
   HGLOBAL hRch =  LoadResource( hHan, hrc );	
   if( !hRch ) AfxThrowResourceException();
   CString strBuf, strBuf2;
   LPSTR lp  = strBuf.GetBuffer(_MAX_PATH + 2);	     
   LPSTR lp2  = strBuf2.GetBuffer(_MAX_PATH + 2);	     
	
   ::GetTempPath( _MAX_PATH, lp );
   ::GetTempFileName( lp, "exp", 0, lp2 );   
   strBuf2.ReleaseBuffer(); lp2 = (LPSTR)(LPCSTR)strBuf2;
   strBuf.ReleaseBuffer();

   TFileKill frcHf( lp2, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive );

   LPVOID  lpRc = LockResource( hRch );
   DWORD dwSizeRc = SizeofResource( hHan, hrc );

   frcHf.Write( lpRc, dwSizeRc );
   frcHf.Close();

   TLZFile fLZ;
   CFileException e;
   if( !fLZ.Open(lp2,  CFile::modeRead | CFile::shareExclusive, &e) )
	 AfxThrowFileException( e.m_cause, e.m_lOsError, lp2 );

   TAutoMemoryRc_Win arcDIB( ReadDIBFile(fLZ) );
   fLZ.Close();

   frcHf.KillFile();

   //LPVOID lpL = arcDIB.Lock();
   //CFile f( "F:\\tst.bmp", CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive );
   //f.Write( lpL, GlobalSize(arcDIB()) );
  	

   TFullScrWindow *fswWin = 0;
   try {
	  HGLOBAL hgl = arcDIB(); arcDIB.ClearRc();
	  fswWin = new TFullScrWindow( pParent, (HDIB)hgl, hHan, 0 );
	}
   catch(...)
	{
      if( fswWin ) delete fswWin;
		throw;
	}
	
   fswWin->Create();
 }
