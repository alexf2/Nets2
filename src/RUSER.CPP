#include "ruser.hpp"
#include "localus.hpp"

#include <lmerr.h>
#include <errno.h>
#include <direct.h>
#include <algorithm>
//#include <typeinfo.h>
//#include <dbt.h>

static void Quant()
 {
   int res;
   do {
     MSG msg;
     if( (res = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) == TRUE )      
       DispatchMessage( &msg );
    } while( res == TRUE );
 }


static void MyReportErr( CException *pE )
 {
   CWinApp *pApp = AfxGetApp();
   char cBuf[ 1024 ];
   pE->GetErrorMessage( cBuf, 1024 );
   MessageBox( pApp->m_pMainWnd ? pApp->m_pMainWnd->GetSafeHwnd():NULL,
	 cBuf, "Error", MB_OK|MB_ICONSTOP );
 }


TCh_EraseData* TCh_EraseData::Create( LPSTR lp )
 {
   TCh_EraseData* p = (TCh_EraseData*)new BYTE[ sizeof(TCh_EraseData) + strlen(lp) ];
   p->m_bType = CR_Erase;
   strcpy( p->m_Name, lp );

   return p;
 }

TCh_OpenData* TCh_OpenData::Create( LPSTR lp, UINT atr )
 {
   TCh_OpenData* p = (TCh_OpenData*)new BYTE[ sizeof(TCh_OpenData) + strlen(lp) ];
   p->m_bType = CR_Open;
   strcpy( p->m_Name, lp );
   p->uiAtr = atr;

   return p;
 }

void TRemoteManeger::SafetyKillTh()
 {
   if( !m_arcTh() ) return;

   DWORD dwExitCode;
   if( GetExitCodeThread(m_arcTh()->m_hThread, &dwExitCode) && dwExitCode == STILL_ACTIVE )
	{	  
      m_evCancel.SetEvent();	  	  	  	  
	   	  
	  WaitForSingleObject( m_arcTh()->m_hThread, INFINITE );
	}

   m_arcTh.FreeRc();
 }

TRemoteManeger::~TRemoteManeger()
 {
   SafetyKillTh();  
   ClearFileList();
   ClearFindList();
 }


void TRemoteManeger::ClearFindList()
 {
   list<TFndData*>::iterator itSta( m_lstFind.begin() );
   list<TFndData*>::iterator itEnd( m_lstFind.end() );

   for( ; itSta != itEnd; ++itSta )	
	{
      _findclose( (*itSta)->m_Handl );
	  delete *itSta;
	}

   m_lstFind.clear();	
 }

void TRemoteManeger::ClearFileList()
 {
   list<CFile*>::iterator itSta( m_lstFiles.begin() );
   list<CFile*>::iterator itEnd( m_lstFiles.end() );

   for( ; itSta != itEnd; ++itSta )	
     delete *itSta;

   m_lstFiles.clear();	
 }


static UINT ThreadProc_RM( LPVOID pObj )
 {
   ((TRemoteManeger*)pObj)->WorkCycle();
   return 0;
 }

void TRemoteManeger::StartWorkCycle()
 {
   m_arcTh = new CWinThread( ThreadProc_RM, (LPVOID)this );
   m_arcTh()->m_bAutoDelete = FALSE;
   m_arcTh()->CreateThread();    
 }

DWORD TRemoteManeger::SendResult( TCh_Result& res )
 {
   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( (BYTE*)&res, sizeof(TCh_Result), &evEnd, dwRes );   
	
   CSingleLock lock( &evEnd, TRUE );

   return dwRes;
 }


void TRemoteManeger::MakeRmDir( TCommonReq *pCommon )
 {
   TCh_EraseData *pD = (TCh_EraseData*)pCommon;

   TCh_Result res; 
   if( !::RemoveDirectory(pD->m_Name) ) 
	{
	  res.m_dwRes = 0;
	  res.m_dwErr = ::GetLastError();
	 }
   else res.m_dwRes = 1;

   SendResult( res );   
 }

void TRemoteManeger::MakeGetSecurity( TCommonReq *pCommon )
 {
   TCh_EraseData *pD = (TCh_EraseData*)pCommon;

   TCh_Result res; 
   
   res.m_dwRes = 1;
   res.m_dwErr = GetFileAttributes( pD->m_Name );	 

   SendResult( res );
 }

void TRemoteManeger::MakeSetSecurity( TCommonReq *pCommon )
 {   
   TCh_OpenData *pD = (TCh_OpenData*)pCommon;

   TCh_Result res; 
   
   res.m_dwRes = SetFileAttributes( pD->m_Name, pD->uiAtr );
   res.m_dwErr = 0;
   
   SendResult( res );
 }


void TRemoteManeger::MakeMkDir( TCommonReq *pCommon )
 {
   TCh_EraseData *pD = (TCh_EraseData*)pCommon;

   TCh_Result res; 

   _finddata_t fnd;
   memset( &fnd, 0, sizeof(_finddata_t) );
   fnd.attrib = _A_ARCH | _A_SUBDIR | _A_SYSTEM | _A_HIDDEN | _A_RDONLY | _A_NORMAL;

   long lHandl = _findfirst( pD->m_Name, &fnd );
   if( lHandl != -1 ) 
	{
      _findclose( lHandl );
	  if( fnd.attrib & _A_SUBDIR ) 
	   {
         res.m_dwRes = 1;
		 SendResult( res );

		 return;
	   }
	}

   if( _mkdir( pD->m_Name ) == -1 )
	{
      res.m_dwRes = 0;
	  res.m_dwErr = ::GetLastError();
	}
   else res.m_dwRes = 1;
	 
   SendResult( res );
 }


void TRemoteManeger::MakeErase( TCommonReq *pCommon )
 {
   TCh_EraseData *pD = (TCh_EraseData*)pCommon;

   TCh_Result res; 
   if( !::DeleteFile(pD->m_Name) ) 
	{
	  res.m_dwRes = 0;
	  res.m_dwErr = ::GetLastError();
	 }
   else res.m_dwRes = 1;

   SendResult( res );
 }

void TRemoteManeger::MakeOpen( TCommonReq *pCommon )
 {
   TCh_OpenData *pD = (TCh_OpenData*)pCommon;

   TCh_Result res; 

   CFileException except;
   CFile* pFile = new CFile();
   if( !pFile->Open(pD->m_Name, pD->uiAtr, &except) )	      
	{
	  res.m_dwRes = 0;
	  res.m_dwErr = except.m_lOsError;
	  delete pFile;
	}
   else 
	{
	  res.m_dwRes = 1;
	  res.m_dwErr = pFile->m_hFile;
	  m_lstFiles.push_back( pFile );
	}

   SendResult( res );
 }

CFile* TRemoteManeger::GetFile( HFILE mHandle )
 {
   list<CFile*>::iterator itSta( m_lstFiles.begin() );
   list<CFile*>::iterator itEnd( m_lstFiles.end() );

   for( ; itSta != itEnd; ++itSta )	
	 if( (*itSta)->m_hFile == mHandle ) return *itSta;

   return NULL;
 }

void TRemoteManeger::MakeGetSize( TCommonReq *pCommon )
 {
   TCh_SizeData *pD = (TCh_SizeData*)pCommon;

   TCh_Result res; 

   CFile *pF = GetFile( pD->mHandle );
   if( !pF )
	{
      res.m_dwRes = 0;
	  res.m_dwErr = ERROR_INVALID_HANDLE;
	}
   else
	{
	  try {
        res.m_dwErr = pF->GetLength();
		res.m_dwRes = 1;
	   }
	  catch( CFileException *pE )
	   {
         res.m_dwRes = 0;
	     res.m_dwErr = pE->m_lOsError;
		 pE->Delete();
	   }
	}

   SendResult( res );
 }



void TRemoteManeger::MakeClose( TCommonReq *pCommon )
 {
   TCh_SizeData *pD = (TCh_SizeData*)pCommon;

   TCh_Result res; 

   CFile *pF = GetFile( pD->mHandle );
   if( !pF )
	{
      res.m_dwRes = 0;
	  res.m_dwErr = ERROR_INVALID_HANDLE;
	}
   else
	{
	  try {
        pF->Close();
		res.m_dwErr = 0;
		res.m_dwRes = 1;
        m_lstFiles.remove( pF ); 
		delete pF;
	   }
	  catch( CFileException *pE )
	   {
         res.m_dwRes = 0;
	     res.m_dwErr = pE->m_lOsError;
		 pE->Delete();
	   }
	}

   SendResult( res );
 }

void TRemoteManeger::MakeWrite( TCommonReq *pCommon )
 {
   TCh_ReadData *pD = (TCh_ReadData*)pCommon;

   TCh_Result res; 

   CFile *pF = GetFile( pD->mHandle );
   if( !pF )
	{
      res.m_dwRes = 0;
	  res.m_dwErr = ERROR_INVALID_HANDLE;
	}
   else
	{
	  try {
        pF->Write( ((BYTE*)pD) + sizeof(TCh_ReadData), pD->dwSize );
		res.m_dwRes = 1;        
	   }
	  catch( CFileException *pE )
	   {
         res.m_dwRes = 0;
	     res.m_dwErr = pE->m_lOsError;
		 pE->Delete();
	   }
	}

   SendResult( res );
 }


void TRemoteManeger::MakeRead( TCommonReq *pCommon )
 {
   TCh_ReadData *pD = (TCh_ReadData*)pCommon;

   TCh_Result res; 

   TAutoMemoryRc_AR<BYTE*> arc( new BYTE[pD->dwSize + sizeof(TCh_Result)] );
   CFile *pF = GetFile( pD->mHandle );
   if( !pF )
	{
      res.m_dwRes = 0;
	  res.m_dwErr = ERROR_INVALID_HANDLE;
	}
   else
	{	  
	  try {
        res.m_dwErr = pF->Read( arc()+sizeof(TCh_Result), pD->dwSize );
		res.m_dwRes = 1;        
	   }
	  catch( CFileException *pE )
	   {
         res.m_dwRes = 0;
	     res.m_dwErr = pE->m_lOsError;
		 pE->Delete();

		 SendResult( res );
		 return;
	   }
	}

   TCh_Result *pRes = (TCh_Result*)arc();
   *pRes = res;
   //pRes->m_dwRes = res.m_dwRes;
   //pRes->m_dwErr = res.m_dwErr;

   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( arc(), pD->dwSize + sizeof(TCh_Result), &evEnd, dwRes );   
	
   CSingleLock lock( &evEnd, TRUE );   
 }
   

void TRemoteManeger::MakeFindFirst( TCommonReq *pCommon )
 {
   TCh_EraseData *pD = (TCh_EraseData*)pCommon;

   TCh_Result res; 
   _finddata_t fDta;

   memset( &fDta, 0, sizeof(_finddata_t) );
   fDta.attrib = _A_ARCH | _A_SUBDIR | _A_SYSTEM | _A_HIDDEN | _A_RDONLY | _A_NORMAL;

   TAutoMemoryRc_AR<BYTE*> arc( new BYTE[sizeof(TCh_Result) + sizeof(_finddata_t)] );

   LONG lHandl = _findfirst( pD->m_Name, &fDta );
   DWORD dwE = ::GetLastError();
   if( lHandl == -1 && errno != ENOENT )
	{
	  //CFileException::ThrowOsError( dwE, (LPCSTR)m_csPath );	
	  res.m_dwRes = 0;
	  res.m_dwErr = dwE;
	  res.m_dwExtErr = 0;	  
	}
   else if( lHandl == -1 && errno == ENOENT )
	{
      //m_dwLastErr = errno;
	  //return NULL;
	  res.m_dwRes = 0;
	  res.m_dwErr = errno;
	  res.m_dwExtErr = 1;	 
	}
   else
	{
      res.m_dwRes = 1;
	  res.m_dwErr = lHandl;
	  res.m_dwExtErr = 2;

      TFndData *pF = new TFndData();
	  pF->m_Handl = lHandl;
	  memcpy( &pF->m_Fnd, &fDta, sizeof(_finddata_t) );
	  m_lstFind.push_back( pF );
	}
      
   TCh_Result *pRes = (TCh_Result*)arc();
   *pRes = res;
   //pRes->m_dwRes = res.m_dwRes;
   //pRes->m_dwErr = res.m_dwErr;
   //pRes->m_dwExtErr = res.m_dwExtErr;
   memcpy( ((BYTE*)pRes) + sizeof(TCh_Result), &fDta, sizeof(_finddata_t) );

   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( arc(), sizeof(_finddata_t) + sizeof(TCh_Result), &evEnd, dwRes );   
	
   CSingleLock lock( &evEnd, TRUE );   
 }


void TRemoteManeger::MakeCheckExist( TCommonReq *pCommon )
 {
   TCh_EraseData *pD = (TCh_EraseData*)pCommon;
   TCh_Result res; 

   _finddata_t fnd;
   memset( &fnd, 0, sizeof(_finddata_t) );
   fnd.attrib = _A_ARCH | _A_SYSTEM | _A_HIDDEN | _A_RDONLY | _A_NORMAL;

   long lHandl = _findfirst( pD->m_Name, &fnd );
   if( lHandl != -1 ) 
	{
      _findclose( lHandl );
	  if( !(fnd.attrib & _A_SUBDIR) ) res.m_dwRes = true;
	  SendResult( res );
	  return;
	}

   res.m_dwRes = false;
   SendResult( res );
 }

/*if( !_findnext(lHandl, &m_fDta) ) return &m_fDta;

   if( errno != ENOENT )
	 CFileException::ThrowOsError( ::GetLastError(), (LPCSTR)m_csPath );	
   else 
	{
      m_dwLastErr = errno;
	  return NULL;
	}

   return NULL;*/

  
//CFile* TRemoteManeger::GetFile( HANDLE mHandle )
TFndData* TRemoteManeger::GetFnd( LONG mHandle )
 {
   list<TFndData*>::iterator itSta( m_lstFind.begin() );
   list<TFndData*>::iterator itEnd( m_lstFind.end() );

   for( ; itSta != itEnd; ++itSta )	
	 if( (*itSta)->m_Handl == mHandle ) return *itSta;

   return NULL;
 }


void TRemoteManeger::MakeFindNext( TCommonReq *pCommon )
 {
   TCh_SizeData *pD = (TCh_SizeData*)pCommon;

   TCh_Result res; 

   TFndData* pFnd = GetFnd( pD->mHandle );

   _finddata_t fnd;
      
   if( !pFnd )
	{
      res.m_dwRes = 0;
	  res.m_dwErr = ERROR_INVALID_HANDLE;
	  res.m_dwExtErr = 0;	        
	}
   else
	{
      if( !_findnext(pD->mHandle, &fnd) ) 
	   {
         res.m_dwRes = 1;
	     res.m_dwErr = 0;
	     res.m_dwExtErr = 2;
         
	     memcpy( &pFnd->m_Fnd, &fnd, sizeof(_finddata_t) );	     
	   }
	  else
	   {
	     if( errno != ENOENT )
		  {
		    res.m_dwRes = 0;
	        res.m_dwErr = ::GetLastError();
	        res.m_dwExtErr = 0;	  
		  }
		 else
		  {
            res.m_dwRes = 0;
	        res.m_dwErr = errno;
	        res.m_dwExtErr = 1;	 
		  }
	   }
	}

   TAutoMemoryRc_AR<BYTE*> arc( new BYTE[sizeof(TCh_Result) + sizeof(_finddata_t)] );
 
   TCh_Result *pRes = (TCh_Result*)arc();
   *pRes = res;
   //pRes->m_dwRes = res.m_dwRes;
   //pRes->m_dwErr = res.m_dwErr;
   //pRes->m_dwExtErr = res.m_dwExtErr;
   memcpy( ((BYTE*)pRes) + sizeof(TCh_Result), &fnd, sizeof(_finddata_t) );

   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( arc(), sizeof(_finddata_t) + sizeof(TCh_Result), &evEnd, dwRes );   
	
   CSingleLock lock( &evEnd, TRUE );   
 }

   /*if( lHandl != -1 ) 
	{
      _findclose( lHandl );
	  lHandl = -1;
	}*/
//MakeFindClose

void TRemoteManeger::MakeFindClose( TCommonReq *pCommon )
 {
   TCh_SizeData *pD = (TCh_SizeData*)pCommon;

   TCh_Result res; 
   
   TFndData* pFnd = GetFnd( pD->mHandle );
   if( !pFnd )
	{
      res.m_dwRes = 0;
	  res.m_dwErr = ERROR_INVALID_HANDLE;
	}
   else
	{	  
      res.m_dwErr = 0;
	  res.m_dwRes = 1;
      m_lstFind.remove( pFnd ); 
	  delete pFnd;	   
	}

   SendResult( res );
 }

/*char path_buffer[ _MAX_PATH ];
   if( !_getdcwd(iDr, path_buffer, _MAX_PATH) )
	  CFileException::ThrowOsError( ::GetLastError() );	

   m_strLastAcessPath = path_buffer; 
   m_bDrive = iDr;
   m_uiType = uiType;*/

void TRemoteManeger::MakeDriveInit( TCommonReq *pCommon )
 {
   TCh_DriveInit *pD = (TCh_DriveInit*)pCommon;

   //MessageBeep( -1 );

   TCh_Result res; 

   TAutoMemoryRc_AR<BYTE*> arc( new BYTE[_MAX_PATH + sizeof(TCh_Result)] );
   LPSTR lp = (LPSTR)(arc() + sizeof(TCh_Result));
   int iDSz = 0;
   if( !_getdcwd(pD->iDr, lp, _MAX_PATH) )
	{
      res.m_dwRes = 0;
	  res.m_dwErr = ::GetLastError();	 
	}
   else
	{
	  iDSz = strlen( lp ) + 1;
      res.m_dwRes = 1;	  
	}

   /*CFile f( "F:\\dri.ini", CFile::modeCreate|CFile::modeReadWrite|CFile::modeNoTruncate );
   f.Seek( 0, CFile::end );

   char cBuf[500];
	  sprintf( cBuf, "\tDrive: %c\n"
                     "\tPath: %s\n",					 
					 (char)(pD->iDr+'A'-1),					 
					 lp );
   f.Write( cBuf, strlen(cBuf) );*/
   
   
   TCh_Result *pRes = (TCh_Result*)arc();
   *pRes = res;
   //pRes->m_dwRes = res.m_dwRes;
   //pRes->m_dwErr = res.m_dwErr;

   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( arc(), iDSz + sizeof(TCh_Result), &evEnd, dwRes );   
	
   CSingleLock lock( &evEnd, TRUE );   
 }

static void ClearTmp( vector<TFileRecord*>& m_rvecTmp )
 {
   vector<TFileRecord*>::iterator itSta( m_rvecTmp.begin() );
   vector<TFileRecord*>::iterator itEnd( m_rvecTmp.end() );

   for( ; itSta != itEnd; ++itSta )	
	 if( *itSta ) 
	  {
        delete *itSta;
	  }

   m_rvecTmp.clear();
 }


void TRemoteManeger::FindFillTmp( LPSTR path_buffer, vector<TFileRecord*>& m_rvecTmp )
 {
   ClearTmp( m_rvecTmp );

   _finddata_t fnd;
   memset( &fnd, 0, sizeof(_finddata_t) );
   fnd.attrib = _A_ARCH | _A_SUBDIR | _A_SYSTEM | _A_HIDDEN | _A_RDONLY | _A_NORMAL;

   long lHandl = _findfirst( path_buffer, &fnd );
   if( lHandl == -1 )
	 CFileException::ThrowOsError( ::GetLastError(), path_buffer );	
     
   int iRes;
   do {
	  TAutoMemoryRc<TFileRecord*> arcRec( new TFileRecord(fnd) );
	  m_rvecTmp.push_back( arcRec() );
	  arcRec.ClearRc();

      iRes = _findnext( lHandl, &fnd );	  

	} while( !iRes );

   DWORD dw = ::GetLastError();
   _findclose( lHandl );
   if( errno != ENOENT ) CFileException::ThrowOsError( dw, path_buffer );

   sort( m_rvecTmp.begin(), m_rvecTmp.end(), CmpRecords );
 }


void TRemoteManeger::MakeGetDir( TCommonReq *pCommon )
 {
   TCh_EraseData *pD = (TCh_EraseData*)pCommon;

   TCh_Result res; 
   
   //res.m_dwRes = 1;
   //res.m_dwErr = GetFileAttributes( pD->m_Name );	 
   vector<TFileRecord*> m_rvecTmp;

   try {
     FindFillTmp( pD->m_Name, m_rvecTmp );
	}
   catch( CFileException* pE )
	{
	  ClearTmp( m_rvecTmp );
	  res.m_dwRes = 0;
	  res.m_dwErr = pE->m_lOsError;
	  SendResult( res );

	  return;
   	}

   vector<TFileRecord*>::iterator itSta( m_rvecTmp.begin() );
   vector<TFileRecord*>::iterator itEnd( m_rvecTmp.end() );

   DWORD dwSize = 0;
   for( ; itSta != itEnd; ++itSta )	
	 dwSize += (*itSta)->m_csName.GetLength() + 1 + sizeof(TMyFindData);

   TAutoMemoryRc_AR<BYTE*> arc( new BYTE[dwSize + sizeof(TCh_Result)] );
   TMyFindData* pDta = (TMyFindData*)(arc() + sizeof(TCh_Result));

   itSta = m_rvecTmp.begin();
   itEnd = m_rvecTmp.end();
   for( ; itSta != itEnd; ++itSta )	
	{
      pDta->attrib = (*itSta)->attrib;
	  pDta->size = (*itSta)->size;
	  pDta->time_write = (*itSta)->time_write;

	  pDta++; strcpy( (LPSTR)pDta, (LPSTR)(LPCSTR)(*itSta)->m_csName );      

	  pDta = (TMyFindData*)(((BYTE*)pDta) + 
	         (*itSta)->m_csName.GetLength() + 1);
	}
   

   TCh_Result *pRes = (TCh_Result*)arc();
   *pRes = res;
   pRes->m_dwRes = 1;
   pRes->m_dwErr = m_rvecTmp.size();

   ClearTmp( m_rvecTmp );

   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( arc(), dwSize + sizeof(TCh_Result), &evEnd, dwRes );
	
   CSingleLock lock( &evEnd, TRUE );      
 }

struct TRemoteDRive
 {
   BYTE  bLetter;
   UINT  uiType;
 };

void TRemoteManeger::GetDrives( TCommonReq *pCommon )
 {
   TCh_SizeData *pD = (TCh_SizeData*)pCommon;   

   TCh_Result res; 

   TAutoMemoryRc_AR<BYTE*> arc( new BYTE[sizeof(TRemoteDRive)*int('Z'-'A'+1) + sizeof(TCh_Result)] );
   TRemoteDRive* pDta = (TRemoteDRive*)(arc() + sizeof(TCh_Result));

   char cDr[] = "A:\\";
   int iCount = 0;
   for( int i = 'A'; i <= 'Z'; ++i, ++(*cDr) )
	{
      UINT ui = GetDriveType( cDr );
	  if( ui == DRIVE_UNKNOWN || ui == DRIVE_NO_ROOT_DIR ) continue;

	  pDta->bLetter = i - 'A' + 1;
	  pDta->uiType = ui;

	  pDta++;
	  iCount++;
	}   

   TCh_Result *pRes = (TCh_Result*)arc();
   *pRes = res;
   pRes->m_dwRes = 1;
   pRes->m_dwErr = iCount;

   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( arc(), sizeof(TRemoteDRive)*iCount + sizeof(TCh_Result), &evEnd, dwRes );
	
   CSingleLock lock( &evEnd, TRUE );      
 }

void TRemoteManeger::DispatchReq( BYTE* pBlock )
 {
   if( ((TGenericBlock*)pBlock)->m_64Size < sizeof(TCommonReq) ) return;
   TCommonReq *pCommon = (TCommonReq *)(pBlock + sizeof(TGenericBlock));

   /*CFile f( "F:\\dri2.ini", CFile::modeCreate|CFile::modeReadWrite|CFile::modeNoTruncate );
   f.Seek( 0, CFile::end );

   char cBuf[500];*/
	  					    

   switch( pCommon->m_bType )
	{
	   case CR_Result:
		 break;

	   case CR_Erase:		 
		 MakeErase( pCommon );
		 break;

	   case CR_CheckExist:		
		 MakeCheckExist( pCommon );
		 break;

	   case CR_Open:
		 MakeOpen( pCommon );
		 break;

	   case CR_GetSize:
		MakeGetSize( pCommon );
	    break;

	   case CR_Close:
		MakeClose( pCommon );
	    break;

	   case CR_Read:
        MakeRead( pCommon );
	    break;

	   case CR_Write:
		MakeWrite( pCommon );
	    break;

	   case CR_FindFirst:
		MakeFindFirst( pCommon );
	    break;

	   case CR_FindNext:
		MakeFindNext( pCommon );
	    break;

	   case CR_FindClose:
		MakeFindClose( pCommon );
	    break;

	   case CR_DriveInit:
		/*sprintf( cBuf, "\tDispatch: %d\n"
                     "\tType: %s\n"
					 "\tLbl: %c\n",
					 (int)((TGenericBlock*)pBlock)->m_64Size, "CR_DriveInit", 
                     *(((TPacket*)pBlock)->m_Data+5) );
		f.Write( cBuf, strlen(cBuf) );*/

		MakeDriveInit( pCommon );
	    break;

	   case CR_RmDir:
        MakeRmDir( pCommon );
	    break;

	   case CR_MkDir:
		MakeMkDir( pCommon );
		break;

	   case CR_GetSecurity:
		 MakeGetSecurity( pCommon );
		 break;

	   case CR_SetSecurity:
		 MakeSetSecurity( pCommon );
		 break;

	   case CR_Dir:
		MakeGetDir( pCommon );
		break;
		
	   case CR_GetDrives:
		GetDrives( pCommon );
		break;

	   default:
		break;
	};
 }

void TRemoteManeger::ConnectProc()
 {
   if( m_pCh_1->GetCnctIndex() == 1 )
	 m_pCh = (m_bRemote == false) ? m_pCh_1:m_pCh_2;	
   else 
	 m_pCh = (m_bRemote == false) ? m_pCh_2:m_pCh_1;

   //Sleep( 1000 );

   try {
    if( m_bRemote == false ) AddRemoteDrives();
	}
   catch( CException *pE )
	{
      //pE->ReportError();	
	  MyReportErr( pE );
	  pE->Delete();
	}
 }

void TRemoteManeger::DisconnectProc()
 {   
   try {
    RemoveRemoteDrives();
	}
   catch( CException *pE )
	{
      //pE->ReportError();
      MyReportErr( pE );
	  pE->Delete();
	}
 }

void TRemoteManeger::WorkCycle()
 {       
   CSyncObject* syncEx[2] = { &m_evCancel, &m_evConnectOK };
   CMultiLock mLockEx( syncEx, 2 );
   
   while(1)
	{
	   DWORD dwResEx = mLockEx.Lock( INFINITE, FALSE );

	   if( dwResEx >= WAIT_OBJECT_0 && dwResEx <= WAIT_OBJECT_0 + 1 )
         dwResEx -= WAIT_OBJECT_0;
	   else if( dwResEx >= WAIT_ABANDONED_0 && dwResEx <= WAIT_ABANDONED_0 + 1 )
         dwResEx = -1;

      if( dwResEx == -1 || dwResEx == 0 ) return;


	  CSyncObject* syncL[2] = { &m_evCancel, m_pCh_1->GetEvLetsActive() };
      CMultiLock mLockL( syncL, 2 );
	  dwResEx = mLockL.Lock( INFINITE, FALSE );

	  if( dwResEx >= WAIT_OBJECT_0 && dwResEx <= WAIT_OBJECT_0 + 1 )
         dwResEx -= WAIT_OBJECT_0;
	   else if( dwResEx >= WAIT_ABANDONED_0 && dwResEx <= WAIT_ABANDONED_0 + 1 )
         dwResEx = -1;

      if( dwResEx == -1 || dwResEx == 0 ) return;


	  //Sleep( 500 );
	  ConnectProc();

	  int iL = m_bRemote == false ? 2:3;
	  CSyncObject* sync[3] = { &m_evCancel, m_pCh->GetBrokenEv(), m_pCh->GetBlockEv() };
      CMultiLock mLock( sync, iL );
	  iL--;

      //CSingleLock mSLOck( m_pCh->GetPckEv() );

      while( 1 )
	   {
         DWORD dwRes = mLock.Lock( INFINITE, FALSE );

	     if( dwRes >= WAIT_OBJECT_0 && dwRes <= WAIT_OBJECT_0 + iL )
           dwRes -= WAIT_OBJECT_0;
	     else if( dwRes >= WAIT_ABANDONED_0 && dwRes <= WAIT_ABANDONED_0 + iL )
           dwRes = -1;

	     if( dwRes == -1 || dwRes == 0 ) return;
	     if( dwRes == 1 ) break;

		 //CSingleLock lockRd( m_pCh->GetReadLock(), TRUE );
	     //if( mSLOck.IsLocked() ) continue; !!!!!!!!!!!

		 if( m_bRemote == false )
		  {
		   int a =1;
		  }
		  

	     DWORD dwSz = 0;
         TAutoMemoryRc_AR<BYTE*> arcBl( m_pCh->ReadBlock( dwSz, dwRes ) );

         if( dwRes < 0 ) continue;
	     DispatchReq( arcBl() );
	   }

	  DisconnectProc();
	}
 }

void TRemoteManeger::RemoveRemoteDrives()
 {
   CSingleLock lock( &m_pFSrv->m_mtxDrives, TRUE );
   
   vector<TDrive*>::iterator itSta( m_pFSrv->GetDriveData().begin() );
   vector<TDrive*>::iterator itEnd( m_pFSrv->GetDriveData().end() );

    for( ; itSta != itEnd; ++itSta )	   	     
	  if( (*itSta)->IsRemote() == true ) 
	   {
	     (*itSta)->SheduleRemove();		 	   	

		 bool bFlRefreshList;
	    do 
		 {
           bFlRefreshList = false;

           list<TUserOfDrive*>::iterator itSta2( (*itSta)->m_lstUsers.begin() );
           list<TUserOfDrive*>::iterator itEnd2( (*itSta)->m_lstUsers.end() );

	       for( ; itSta2 != itEnd2; ++itSta2 )	     
			{
               if( (*itSta2)->IsCallingOutside() == false ) continue;

		       bFlRefreshList = (*itSta2)->Notify( TUserOfDrive::NC_Removed );
			   if( bFlRefreshList == true ) break;		    
			}		
		 } while( bFlRefreshList == true );

	   }
 }

void TRemoteManeger::AddRemoteDrives()
 {
   //CSingleLock lockRd( m_pCh->GetReadLock(), TRUE );

   TAutoMemoryRc<TCh_SizeData*> arc( new TCh_SizeData() );
   arc()->mHandle = 0;
   arc()->m_bType = CR_GetDrives;
   
   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( (BYTE*)arc(), sizeof(TCh_SizeData), &evEnd, dwRes );
   	
   CSingleLock lock( &evEnd, TRUE );
   if( dwRes != 1 ) 
	 CFileException::ThrowOsError( NERR_BadControlRecv, NULL );	

   DWORD dwSz = 0;
   TAutoMemoryRc_AR<BYTE*> arcBl( m_pCh->ReadBlock( dwSz, dwRes ) );

   if( dwRes < 0 )
	 CFileException::ThrowOsError( NERR_DevNotRedirected, NULL );	

   if( dwSz < sizeof(TCh_Result)  )
	 CFileException::ThrowOsError( NERR_BadRecipient, NULL  );	

   
   TCh_Result* pRes = (TCh_Result*)(arcBl() + sizeof(TGenericBlock));
   if( pRes->m_bType != CR_Result )
	 CFileException::ThrowOsError( NERR_BadEventName, NULL );	
   if( !pRes->m_dwRes )
	 CFileException::ThrowOsError( pRes->m_dwErr, NULL );	

   CSingleLock lock2( &m_pFSrv->m_mtxDrives, TRUE );

   //lockRd.Unlock();

   TRemoteDRive *pDta = (TRemoteDRive*)(((BYTE*)pRes) + sizeof(TCh_Result));
   for( int i = 0; i < pRes->m_dwErr; ++i, ++pDta )
	{
      TAutoMemoryRc<TRemoteDrive*> arc( new TRemoteDrive(m_pCh) );

	  arc()->Init( pDta->bLetter, pDta->uiType );
	  m_pFSrv->GetDriveData().push_back( arc() );
	  arc.ClearRc();   
	}
 }



TVFileRemote::~TVFileRemote()
 {
   try {
     Close();
	}
   catch( CException *pE )
	{
      pE->Delete();
	}
 }

void TVFileRemote::Erase( LPCSTR lp ) 
 {   
   //if( !::DeleteFile(lp) ) 
	 //CFileException::ThrowOsError( ::GetLastError(), lp );	
   
   //CSingleLock lockRd( m_pCh->GetReadLock(), TRUE );
   
   TAutoMemoryRc_AR<BYTE*> arc( (BYTE*)TCh_EraseData::Create((LPSTR)lp) );
   TCh_EraseData* pErase = (TCh_EraseData*)arc();

   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( arc(), pErase->SZ(), &evEnd, dwRes );   
	
   CSingleLock lock( &evEnd, TRUE );
   if( dwRes != 1 ) 
	 CFileException::ThrowOsError( NERR_BadControlRecv, lp );	

   DWORD dwSz = 0;
   TAutoMemoryRc_AR<BYTE*> arcBl( m_pCh->ReadBlock( dwSz, dwRes ) );

   if( dwRes < 0 )
	 CFileException::ThrowOsError( NERR_DevNotRedirected, lp );	

   if( dwSz != sizeof(TCh_Result)  )
	 CFileException::ThrowOsError( NERR_BadRecipient, lp );	

   

   TCh_Result* pRes = (TCh_Result*)(arcBl() + sizeof(TGenericBlock));
   if( pRes->m_bType != CR_Result )
	 CFileException::ThrowOsError( NERR_BadEventName, lp );	

   if( !pRes->m_dwRes )
	 CFileException::ThrowOsError( pRes->m_dwErr, lp );	
 }


bool TVFileRemote::CheckExist()
 {
   //CSingleLock lockRd( m_pCh->GetReadLock(), TRUE );

   TAutoMemoryRc_AR<BYTE*> arc( (BYTE*)TCh_EraseData::Create((LPSTR)(LPCSTR)m_csName) );
   TCh_EraseData* pErase = (TCh_EraseData*)arc();
   pErase->m_bType = CR_CheckExist;

   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( arc(), pErase->SZ(), &evEnd, dwRes );   
	
   CSingleLock lock( &evEnd, TRUE );
   if( dwRes != 1 ) 
	 CFileException::ThrowOsError( NERR_BadControlRecv, (LPSTR)(LPCSTR)m_csName );	

   DWORD dwSz = 0;
   TAutoMemoryRc_AR<BYTE*> arcBl( m_pCh->ReadBlock( dwSz, dwRes ) );

   if( dwRes < 0 )
	 CFileException::ThrowOsError( NERR_DevNotRedirected, (LPSTR)(LPCSTR)m_csName );	

   if( dwSz != sizeof(TCh_Result)  )
	 CFileException::ThrowOsError( NERR_BadRecipient, (LPSTR)(LPCSTR)m_csName );	

   
   TCh_Result* pRes = (TCh_Result*)(arcBl() + sizeof(TGenericBlock));
   if( pRes->m_bType != CR_Result )
	 CFileException::ThrowOsError( NERR_BadEventName, (LPSTR)(LPCSTR)m_csName );	

   //if( pRes->m_dwRes != 0 )
	 //CFileException::ThrowOsError( pRes->m_dwRes, lp );	

   return pRes->m_dwRes;
 }

void TVFileRemote::MakeOpen()
 {     
   //CSingleLock lockRd( m_pCh->GetReadLock(), TRUE );

   TAutoMemoryRc_AR<BYTE*> arc( (BYTE*)TCh_OpenData::Create((LPSTR)(LPCSTR)m_csName, m_uiAtr) );
   TCh_OpenData* pOpen = (TCh_OpenData*)arc();
   
   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( arc(), pOpen->SZ(), &evEnd, dwRes );   
	
   CSingleLock lock( &evEnd, TRUE );
   if( dwRes != 1 ) 
	 CFileException::ThrowOsError( NERR_BadControlRecv, (LPSTR)(LPCSTR)m_csName );	

   DWORD dwSz = 0;
   TAutoMemoryRc_AR<BYTE*> arcBl( m_pCh->ReadBlock( dwSz, dwRes ) );

   if( dwRes < 0 )
	 CFileException::ThrowOsError( NERR_DevNotRedirected, (LPSTR)(LPCSTR)m_csName );	

   if( dwSz != sizeof(TCh_Result)  )
	 CFileException::ThrowOsError( NERR_BadRecipient, (LPSTR)(LPCSTR)m_csName );	

   
   TCh_Result* pRes = (TCh_Result*)(arcBl() + sizeof(TGenericBlock));
   if( pRes->m_bType != CR_Result )
	 CFileException::ThrowOsError( NERR_BadEventName, (LPSTR)(LPCSTR)m_csName );	
   if( !pRes->m_dwRes )
	 CFileException::ThrowOsError( pRes->m_dwErr, (LPSTR)(LPCSTR)m_csName );	

   m_Handle = (HFILE)pRes->m_dwErr;
 }

DWORD TVFileRemote::GetSize()
 {
   //CSingleLock lockRd( m_pCh->GetReadLock(), TRUE );

   TAutoMemoryRc<TCh_SizeData*> arc( new TCh_SizeData() );
   arc()->mHandle = m_Handle;
   
   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( (BYTE*)arc(), sizeof(TCh_SizeData), &evEnd, dwRes );   
	
   CSingleLock lock( &evEnd, TRUE );
   if( dwRes != 1 ) 
	 CFileException::ThrowOsError( NERR_BadControlRecv, (LPSTR)(LPCSTR)m_csName );	

   DWORD dwSz = 0;
   TAutoMemoryRc_AR<BYTE*> arcBl( m_pCh->ReadBlock( dwSz, dwRes ) );

   if( dwRes < 0 )
	 CFileException::ThrowOsError( NERR_DevNotRedirected, (LPSTR)(LPCSTR)m_csName );	

   if( dwSz != sizeof(TCh_Result)  )
	 CFileException::ThrowOsError( NERR_BadRecipient, (LPSTR)(LPCSTR)m_csName );	

   
   TCh_Result* pRes = (TCh_Result*)(arcBl() + sizeof(TGenericBlock));
   if( pRes->m_bType != CR_Result )
	 CFileException::ThrowOsError( NERR_BadEventName, (LPSTR)(LPCSTR)m_csName );	
   if( !pRes->m_dwRes )
	 CFileException::ThrowOsError( pRes->m_dwErr, (LPSTR)(LPCSTR)m_csName );	
   
   return pRes->m_dwErr;
 }

DWORD TVFileRemote::Read( BYTE* lp, DWORD dw, TNotifyInstance* )
 {//TCh_ReadData
   //CSingleLock lockRd( m_pCh->GetReadLock(), TRUE );

   TAutoMemoryRc<TCh_ReadData*> arc( new TCh_ReadData() );
   arc()->mHandle = m_Handle;
   arc()->dwSize = dw;
   
   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( (BYTE*)arc(), sizeof(TCh_ReadData), &evEnd, dwRes );   
	
   CSingleLock lock( &evEnd, TRUE );
   if( dwRes != 1 ) 
	 CFileException::ThrowOsError( NERR_BadControlRecv, (LPSTR)(LPCSTR)m_csName );	

   DWORD dwSz = 0;
   TAutoMemoryRc_AR<BYTE*> arcBl( m_pCh->ReadBlock( dwSz, dwRes ) );

   if( dwRes < 0 )
	 CFileException::ThrowOsError( NERR_DevNotRedirected, (LPSTR)(LPCSTR)m_csName );	

   if( dwSz < sizeof(TCh_Result)  )
	 CFileException::ThrowOsError( NERR_BadRecipient, (LPSTR)(LPCSTR)m_csName );	

   
   TCh_Result* pRes = (TCh_Result*)(arcBl() + sizeof(TGenericBlock));
   if( pRes->m_bType != CR_Result )
	 CFileException::ThrowOsError( NERR_BadEventName, (LPSTR)(LPCSTR)m_csName );	
   if( !pRes->m_dwRes )
	 CFileException::ThrowOsError( pRes->m_dwErr, (LPSTR)(LPCSTR)m_csName );	
   
   memcpy( lp, ((BYTE*)pRes) + sizeof(TCh_Result), pRes->m_dwErr );
   return pRes->m_dwErr;
 }

void TVFileRemote::Write( BYTE* lp, DWORD dw, TNotifyInstance* )
 {
   //CSingleLock lockRd( m_pCh->GetReadLock(), TRUE );

   TAutoMemoryRc_AR<BYTE*> arc( new BYTE[sizeof(TCh_ReadData) + dw] );
   TCh_ReadData *pWr = (TCh_ReadData*)arc();
   pWr->mHandle = m_Handle;
   pWr->dwSize = dw;
   pWr->m_bType = CR_Write;
   memcpy( arc() + sizeof(TCh_ReadData), lp, dw );
   
   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( arc(), sizeof(TCh_ReadData) + dw, &evEnd, dwRes );   
	
   CSingleLock lock( &evEnd, TRUE );
   if( dwRes != 1 ) 
	 CFileException::ThrowOsError( NERR_BadControlRecv, (LPSTR)(LPCSTR)m_csName );	

   DWORD dwSz = 0;
   TAutoMemoryRc_AR<BYTE*> arcBl( m_pCh->ReadBlock( dwSz, dwRes ) );

   if( dwRes < 0 )
	 CFileException::ThrowOsError( NERR_DevNotRedirected, (LPSTR)(LPCSTR)m_csName );	

   if( dwSz != sizeof(TCh_Result)  )
	 CFileException::ThrowOsError( NERR_BadRecipient, (LPSTR)(LPCSTR)m_csName );	

   
   TCh_Result* pRes = (TCh_Result*)(arcBl() + sizeof(TGenericBlock));
   if( pRes->m_bType != CR_Result )
	 CFileException::ThrowOsError( NERR_BadEventName, (LPSTR)(LPCSTR)m_csName );	
   if( !pRes->m_dwRes )
	 CFileException::ThrowOsError( pRes->m_dwErr, (LPSTR)(LPCSTR)m_csName );	
      
 }

void TVFileRemote::Close()
 {
   if( m_Handle == (HFILE)INVALID_HANDLE_VALUE ) return;

   //CSingleLock lockRd( m_pCh->GetReadLock(), TRUE );

   TAutoMemoryRc<TCh_SizeData*> arc( new TCh_SizeData() );
   arc()->mHandle = m_Handle;
   arc()->m_bType = CR_Close;
   
   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( (BYTE*)arc(), sizeof(TCh_SizeData), &evEnd, dwRes );   
	
   CSingleLock lock( &evEnd, TRUE );
   if( dwRes != 1 ) 
	 //CFileException::ThrowOsError( NERR_BadControlRecv, (LPSTR)(LPCSTR)m_csName );	
	 return;

   DWORD dwSz = 0;
   TAutoMemoryRc_AR<BYTE*> arcBl( m_pCh->ReadBlock( dwSz, dwRes ) );

   if( dwRes < 0 )
	 //CFileException::ThrowOsError( NERR_DevNotRedirected, (LPSTR)(LPCSTR)m_csName );	
	 return;

   if( dwSz != sizeof(TCh_Result)  )
	 //CFileException::ThrowOsError( NERR_BadRecipient, (LPSTR)(LPCSTR)m_csName );	
	 return;
   

   TCh_Result* pRes = (TCh_Result*)(arcBl() + sizeof(TGenericBlock));
   if( pRes->m_bType != CR_Result )
	 //CFileException::ThrowOsError( NERR_BadEventName, (LPSTR)(LPCSTR)m_csName );	
	 return;

   m_Handle = (HFILE)INVALID_HANDLE_VALUE;

   if( !pRes->m_dwRes )
	 //CFileException::ThrowOsError( pRes->m_dwErr, (LPSTR)(LPCSTR)m_csName );	      
	 return;
 }


TVSearchRemote::~TVSearchRemote()
 {
   if( lHandl == -1 ) return;

   //CSingleLock lockRd( m_pCh->GetReadLock(), TRUE );

   TAutoMemoryRc<TCh_SizeData*> arc( new TCh_SizeData() );
   arc()->mHandle = lHandl;
   arc()->m_bType = CR_FindClose;
   
   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( (BYTE*)arc(), sizeof(TCh_SizeData), &evEnd, dwRes );   
	
   CSingleLock lock( &evEnd, TRUE );
   if( dwRes != 1 ) 
	 //CFileException::ThrowOsError( NERR_BadControlRecv, (LPSTR)(LPCSTR)m_csPath );	
	 return;

   DWORD dwSz = 0;
   TAutoMemoryRc_AR<BYTE*> arcBl( m_pCh->ReadBlock( dwSz, dwRes ) );

   if( dwRes < 0 )
	 //CFileException::ThrowOsError( NERR_DevNotRedirected, (LPSTR)(LPCSTR)m_csPath );	
	 return;

   if( dwSz != sizeof(TCh_Result)  )
	 //CFileException::ThrowOsError( NERR_BadRecipient, (LPSTR)(LPCSTR)m_csPath );	
	 return;
   

   TCh_Result* pRes = (TCh_Result*)(arcBl() + sizeof(TGenericBlock));
   if( pRes->m_bType != CR_Result )
	 //CFileException::ThrowOsError( NERR_BadEventName, (LPSTR)(LPCSTR)m_csPath );	
	 return;

   lHandl = -1;   
 }

_finddata_t* TVSearchRemote::GetFirst()
 {   
   //CSingleLock lockRd( m_pCh->GetReadLock(), TRUE );
   
   TAutoMemoryRc_AR<BYTE*> arc( (BYTE*)TCh_EraseData::Create((LPSTR)(LPCSTR)m_csPath) );
   TCh_EraseData* pErase = (TCh_EraseData*)arc();
   pErase->m_bType = CR_FindFirst;

   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( arc(), pErase->SZ(), &evEnd, dwRes );   
	
   CSingleLock lock( &evEnd, TRUE );
   if( dwRes != 1 ) 
	 CFileException::ThrowOsError( NERR_BadControlRecv, (LPSTR)(LPCSTR)m_csPath );	

   DWORD dwSz = 0;
   TAutoMemoryRc_AR<BYTE*> arcBl( m_pCh->ReadBlock( dwSz, dwRes ) );

   if( dwRes < 0 )
	 CFileException::ThrowOsError( NERR_DevNotRedirected, (LPSTR)(LPCSTR)m_csPath );	

   if( dwSz != sizeof(TCh_Result) + sizeof(_finddata_t) )
	 CFileException::ThrowOsError( NERR_BadRecipient, (LPSTR)(LPCSTR)m_csPath );	

   

   TCh_Result* pRes = (TCh_Result*)(arcBl() + sizeof(TGenericBlock));
   if( pRes->m_bType != CR_Result )
	 CFileException::ThrowOsError( NERR_BadEventName, (LPSTR)(LPCSTR)m_csPath );	
   
   lHandl = -1;
   if( !pRes->m_dwExtErr )
	 CFileException::ThrowOsError( pRes->m_dwErr, (LPCSTR)m_csPath );	
   else if( pRes->m_dwExtErr == 1 )   
	{
	  m_dwLastErr = pRes->m_dwErr;
	  return NULL;
	}

   lHandl = pRes->m_dwErr;
   memcpy( &m_fDta, ((BYTE*)pRes) + sizeof(TCh_Result), sizeof(_finddata_t) );
   
   return &m_fDta;
 }

_finddata_t* TVSearchRemote::Next()
 {
   //CSingleLock lockRd( m_pCh->GetReadLock(), TRUE );      

   TAutoMemoryRc<TCh_SizeData*> arc( new TCh_SizeData() );
   arc()->mHandle = lHandl;
   arc()->m_bType = CR_FindNext;

   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( (BYTE*)arc(), sizeof(TCh_SizeData), &evEnd, dwRes );   
	
   CSingleLock lock( &evEnd, TRUE );
   if( dwRes != 1 ) 
	 CFileException::ThrowOsError( NERR_BadControlRecv, (LPSTR)(LPCSTR)m_csPath );	

   DWORD dwSz = 0;
   TAutoMemoryRc_AR<BYTE*> arcBl( m_pCh->ReadBlock( dwSz, dwRes ) );

   if( dwRes < 0 )
	 CFileException::ThrowOsError( NERR_DevNotRedirected, (LPSTR)(LPCSTR)m_csPath );	

   if( dwSz != sizeof(TCh_Result) + sizeof(_finddata_t) )
	 CFileException::ThrowOsError( NERR_BadRecipient, (LPSTR)(LPCSTR)m_csPath );	

   

   TCh_Result* pRes = (TCh_Result*)(arcBl() + sizeof(TGenericBlock));
   if( pRes->m_bType != CR_Result )
	 CFileException::ThrowOsError( NERR_BadEventName, (LPSTR)(LPCSTR)m_csPath );	
   
   //lHandl = -1;
   if( !pRes->m_dwExtErr )
	 CFileException::ThrowOsError( pRes->m_dwErr, (LPCSTR)m_csPath );	
   else if( pRes->m_dwExtErr == 1 )   
	{
	  m_dwLastErr = pRes->m_dwErr;
	  return NULL;
	}

   //lHandl = pRes->m_dwErr;
   memcpy( &m_fDta, ((BYTE*)pRes) + sizeof(TCh_Result), sizeof(_finddata_t) );
   
   return &m_fDta;
   
 }




void TRemoteDrive::Init( int iDr, UINT uiType )
 { //TCh_DriveInit 
   //CSingleLock lockRd( m_pCh->GetReadLock(), TRUE );

   TAutoMemoryRc<TCh_DriveInit*> arc( new TCh_DriveInit() );
   arc()->iDr = iDr;
   arc()->uiType = uiType;
   
   
   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( (BYTE*)arc(), sizeof(TCh_DriveInit), &evEnd, dwRes );   
	
   CSingleLock lock( &evEnd, TRUE );
   if( dwRes != 1 ) 
	 CFileException::ThrowOsError( NERR_BadControlRecv );	

   DWORD dwSz = 0;
   TAutoMemoryRc_AR<BYTE*> arcBl( m_pCh->ReadBlock( dwSz, dwRes ) );

   if( dwRes < 0 )
	 CFileException::ThrowOsError( NERR_DevNotRedirected );	

   if( dwSz < sizeof(TCh_Result)  )
	 CFileException::ThrowOsError( NERR_BadRecipient );	

   
   TCh_Result* pRes = (TCh_Result*)(arcBl() + sizeof(TGenericBlock));
   if( pRes->m_bType != CR_Result )
	 CFileException::ThrowOsError( NERR_BadEventName );	
   if( !pRes->m_dwRes )
	 CFileException::ThrowOsError( pRes->m_dwErr );	
   

   m_bDrive = iDr;
   m_uiType = uiType;
   m_strLastAcessPath = (LPSTR)(((BYTE*)pRes) + sizeof(TCh_Result));
//?????      
 }

bool TRemoteDrive::IsRemote() const
 {
   return true;
 }

void TRemoteDrive::RmEmptyDir( LPCSTR lp, bool bWarn )
 {
   //CSingleLock lockRd( m_pCh->GetReadLock(), TRUE );
   
   TAutoMemoryRc_AR<BYTE*> arc( (BYTE*)TCh_EraseData::Create((LPSTR)lp) );
   TCh_EraseData* pErase = (TCh_EraseData*)arc();
   pErase->m_bType = CR_RmDir;

   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( arc(), pErase->SZ(), &evEnd, dwRes );   
	
   CSingleLock lock( &evEnd, TRUE );
   if( dwRes != 1 ) 
	 CFileException::ThrowOsError( NERR_BadControlRecv, lp );	

   DWORD dwSz = 0;
   TAutoMemoryRc_AR<BYTE*> arcBl( m_pCh->ReadBlock( dwSz, dwRes ) );

   if( dwRes < 0 )
	 CFileException::ThrowOsError( NERR_DevNotRedirected, lp );	

   if( dwSz != sizeof(TCh_Result)  )
	 CFileException::ThrowOsError( NERR_BadRecipient, lp );	

   

   TCh_Result* pRes = (TCh_Result*)(arcBl() + sizeof(TGenericBlock));
   if( pRes->m_bType != CR_Result )
	 CFileException::ThrowOsError( NERR_BadEventName, lp );	

   if( !pRes->m_dwRes )
	{
	  if( pRes->m_dwErr == ERROR_DIR_NOT_EMPTY )
	   {
	     if( bWarn == false ) return;
		 CString cs; cs.Format( "Каталог '%s' не пуст - стереть нельзя", lp );
		 AfxMessageBox( (LPCSTR)cs, MB_OK|MB_ICONEXCLAMATION );
	   }
	  else
	    CFileException::ThrowOsError( pRes->m_dwErr, lp );	
	}
 }

void TRemoteDrive::MkDir( LPCSTR lp )
 {
   //CSingleLock lockRd( m_pCh->GetReadLock(), TRUE );
   
   TAutoMemoryRc_AR<BYTE*> arc( (BYTE*)TCh_EraseData::Create((LPSTR)lp) );
   TCh_EraseData* pErase = (TCh_EraseData*)arc();
   pErase->m_bType = CR_MkDir;

   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( arc(), pErase->SZ(), &evEnd, dwRes );   
	
   CSingleLock lock( &evEnd, TRUE );
   if( dwRes != 1 ) 
	 CFileException::ThrowOsError( NERR_BadControlRecv, lp );	

   DWORD dwSz = 0;
   TAutoMemoryRc_AR<BYTE*> arcBl( m_pCh->ReadBlock( dwSz, dwRes ) );

   if( dwRes < 0 )
	 CFileException::ThrowOsError( NERR_DevNotRedirected, lp );	

   if( dwSz != sizeof(TCh_Result)  )
	 CFileException::ThrowOsError( NERR_BadRecipient, lp );	

   

   TCh_Result* pRes = (TCh_Result*)(arcBl() + sizeof(TGenericBlock));
   if( pRes->m_bType != CR_Result )
	 CFileException::ThrowOsError( NERR_BadEventName, lp );	

   if( !pRes->m_dwRes )
	  CFileException::ThrowOsError( pRes->m_dwErr, lp );	
 }



void TRemoteDrive::Directory( TUserOfDrive& rUs, vector<TFileRecord*>& rV )
 {
   char path_buffer[_MAX_PATH+50];
   char drive[_MAX_DRIVE];
   char dir[_MAX_DIR];
   char fname[_MAX_FNAME];
   char ext[_MAX_EXT];

   Quant();

   strcpy( path_buffer, rUs.m_strCurPath.IsEmpty() ? "":(LPCSTR)rUs.m_strCurPath );
   if( path_buffer[strlen(path_buffer) - 1] != '\\' )
     strcat( path_buffer, "\\*.*" );
   else strcat( path_buffer, "*.*" );


   //CSingleLock lockRd( m_pCh->GetReadLock(), TRUE );
   
   TAutoMemoryRc_AR<BYTE*> arc( (BYTE*)TCh_EraseData::Create(path_buffer) );
   TCh_EraseData* pErase = (TCh_EraseData*)arc();
   pErase->m_bType = CR_Dir;

   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( arc(), pErase->SZ(), &evEnd, dwRes );
   	
   CSingleLock lock( &evEnd, TRUE );
   if( dwRes != 1 ) 
	 CFileException::ThrowOsError( NERR_BadControlRecv, path_buffer );	

   DWORD dwSz = 0;
   TAutoMemoryRc_AR<BYTE*> arcBl( m_pCh->ReadBlock( dwSz, dwRes ) );

   if( dwRes < 0 )
	 CFileException::ThrowOsError( NERR_DevNotRedirected, path_buffer );	

   if( dwSz < sizeof(TCh_Result)  )
	 CFileException::ThrowOsError( NERR_BadRecipient, path_buffer );	

   

   TCh_Result* pRes = (TCh_Result*)(arcBl() + sizeof(TGenericBlock));
   if( pRes->m_bType != CR_Result )
	 CFileException::ThrowOsError( NERR_BadEventName, path_buffer );	

   if( !pRes->m_dwRes )
	  CFileException::ThrowOsError( pRes->m_dwErr, path_buffer );	


   ClearFList( rV );

   //vector<TFileRecord*>::iterator itSta( m_rvecTmp.begin() );
   //vector<TFileRecord*>::iterator itEnd( m_rvecTmp.end() );

   TMyFindData *pDta = (TMyFindData*)(((BYTE*)pRes) + sizeof(TCh_Result));
   rV.assign( pRes->m_dwErr );

   for( int i = 0; i < pRes->m_dwErr; ++i )
	{
      TAutoMemoryRc<TFileRecord*> arcRec( new TFileRecord() );
	  arcRec()->attrib = pDta->attrib;
	  arcRec()->size = pDta->size;
	  arcRec()->time_write = pDta->time_write;
	  arcRec()->m_csName = ((BYTE*)pDta) + sizeof(TMyFindData);

	  pDta = (TMyFindData*)(((BYTE*)pDta) + sizeof(TMyFindData) + 
	         arcRec()->m_csName.GetLength() + 1);

	  rV[ i ] = arcRec();
	  arcRec.ClearRc();
	}

   //ClearFList( m_rvecTmp );
 }

void TRemoteDrive::ClearFList( vector<TFileRecord*>& rV )
 {
   vector<TFileRecord*>::iterator itSta( rV.begin() );
   vector<TFileRecord*>::iterator itEnd( rV.end() );

   for( ; itSta != itEnd; ++itSta )
	 if( (*itSta) ) delete *itSta;	 

	rV.clear();
 }

TRemoteDrive::~TRemoteDrive()
 {
   ClearFList( m_rvecTmp );
 }

TVirtualFile* TRemoteDrive::OpenFile( LPCSTR lp, UINT atr )
 {
   return new TVFileRemote( lp, atr, m_pCh );
 }

TVirtualSearch* TRemoteDrive::OpenSearch( LPCSTR lp )
 {
   return new TVSearchRemote( lp, m_pCh );
 }

TSecurity* TRemoteDrive::GetSecurityAndAtr( LPCSTR lp )
 {
   //CSingleLock lockRd( m_pCh->GetReadLock(), TRUE );

   TAutoMemoryRc_AR<BYTE*> arc( (BYTE*)TCh_EraseData::Create((LPSTR)lp) );
   TCh_EraseData* pErase = (TCh_EraseData*)arc();
   pErase->m_bType = CR_GetSecurity;

   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( arc(), pErase->SZ(), &evEnd, dwRes );      
	
   CSingleLock lock( &evEnd, TRUE );
   if( dwRes != 1 ) 
	 CFileException::ThrowOsError( NERR_BadControlRecv, lp );	

   DWORD dwSz = 0;
   TAutoMemoryRc_AR<BYTE*> arcBl( m_pCh->ReadBlock( dwSz, dwRes ) );

   if( dwRes < 0 )
	 CFileException::ThrowOsError( NERR_DevNotRedirected, lp );	

   if( dwSz != sizeof(TCh_Result)  )
	 CFileException::ThrowOsError( NERR_BadRecipient, lp );	

   
   TCh_Result* pRes = (TCh_Result*)(arcBl() + sizeof(TGenericBlock));
   if( pRes->m_bType != CR_Result )
	 CFileException::ThrowOsError( NERR_BadEventName, lp );	

   if( pRes->m_dwErr == 0xFFFFFFFF ) return NULL;	 
   
   TSecurity* pS = new TSecurity();
   memset( pS, 0, sizeof(TSecurity) );
   pS->dwAtr = pRes->m_dwErr;

   return pS;   
 }


void TRemoteDrive::SetSecurityAndAtr( LPCSTR lp, TSecurity* pSec )
 {  
   //CSingleLock lockRd( m_pCh->GetReadLock(), TRUE );

   TAutoMemoryRc_AR<BYTE*> arc( (BYTE*)TCh_OpenData::Create((LPSTR)lp, pSec->dwAtr) );
   TCh_OpenData* pOpen = (TCh_OpenData*)arc();
   pOpen->m_bType = CR_SetSecurity;
   
   CEvent evEnd( FALSE, TRUE );
   DWORD dwRes;
   m_pCh->WriteBlock( arc(), pOpen->SZ(), &evEnd, dwRes );
   	
   CSingleLock lock( &evEnd, TRUE );
   if( dwRes != 1 ) 
	 CFileException::ThrowOsError( NERR_BadControlRecv, lp );	

   DWORD dwSz = 0;
   TAutoMemoryRc_AR<BYTE*> arcBl( m_pCh->ReadBlock( dwSz, dwRes ) );

   if( dwRes < 0 )
	 CFileException::ThrowOsError( NERR_DevNotRedirected, lp );	

   if( dwSz != sizeof(TCh_Result)  )
	 CFileException::ThrowOsError( NERR_BadRecipient, lp );	

   
   TCh_Result* pRes = (TCh_Result*)(arcBl() + sizeof(TGenericBlock));
   if( pRes->m_bType != CR_Result )
	 CFileException::ThrowOsError( NERR_BadEventName, lp );	      
 }

DWORD TRemoteDrive::GetBuffSize() const
 {
   return 1024*2;
 }