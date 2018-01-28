#include "stdafx.h"

#include "server.hpp"
#include "localus.hpp"

#include <algorithm>

#include <errno.h>
#include <direct.h>
#include <dbt.h>

//#include <afxpriv.h>


BEGIN_MESSAGE_MAP( TFileServer, CWnd )	
  ON_WM_CREATE()    
END_MESSAGE_MAP()


TFileServer::~TFileServer()
 {
   DrivesClear();
 }

void TFileServer::DrivesClear()
 {
   //CSingleLock lock( &m_mtxDrives, TRUE );

   vector<TDrive*>::iterator itSta( m_vDrives.begin() );
   vector<TDrive*>::iterator itEnd( m_vDrives.end() );

   for( ; itSta != itEnd; ++itSta )
	if( (*itSta) )
	 {
       delete *itSta; *itSta = NULL;
	 }

	m_vDrives.clear();
 }


BOOL TFileServer::Create( CWnd* pPar, UINT id )
 {
   LPCSTR lp = AfxRegisterWndClass( 0,
	  AfxGetApp()->LoadStandardCursor( IDC_ARROW ),
	  (HBRUSH)GetStockObject( GRAY_BRUSH ),
	  NULL );
   if( !lp ) return FALSE;

   CRect r;
   return CWnd::Create( lp, "", WS_OVERLAPPED, r, pPar, id );
 }

int TFileServer::OnCreate( LPCREATESTRUCT lpCreateStruct )
 {
   if( CWnd::OnCreate(lpCreateStruct) == -1 )
	  return -1;

   DrivesAssemble();   

   return 0;
 }

void TFileServer::DrivesAssemble()
 {
   CSingleLock lock( &m_mtxDrives, TRUE );

   char cDr[] = "A:\\";
   for( int i = 'A'; i <= 'Z'; ++i, ++(*cDr) )
	{
      UINT ui = GetDriveType( cDr );
	  if( ui == DRIVE_UNKNOWN || ui == DRIVE_NO_ROOT_DIR ) continue;

	  TAutoMemoryRc<TLocalDrive*> arc( new TLocalDrive() );
	  arc()->Init( i - 'A' + 1, ui );

	  m_vDrives.push_back( arc() );
	  arc.ClearRc();
	}   
 }

TFileServer& TFileServer::operator+( TDrive* pDr )
 {
   vector<TDrive*>::iterator itSta( m_vDrives.begin() );
   vector<TDrive*>::iterator itEnd( m_vDrives.end() );

   for( ; itSta != itEnd; ++itSta )
	if( (*itSta)->CmpDrive(*pDr) == true ) throw TDuplicateDrive();

   m_vDrives.push_back( pDr );

   return *this;
 }

TFileServer& TFileServer::operator-( TDrive* pDr )
 {  
   m_vDrives.erase( find(m_vDrives.begin(), m_vDrives.end(), pDr) );
   delete pDr;

   return *this;
 }


TDrive* TFileServer::GetCurDrive()
 {
   int iDR = _getdrive();   
   TAutoMemoryRc<TLocalDrive*> arc( new TLocalDrive() );
   CString str; str.Format( "%c:\\", char(iDR) );
   arc()->Init( iDR, GetDriveType((LPCSTR)str) );

   TDrive* pRet = arc();
   arc.ClearRc();

   return pRet;
 }

TDrive* TFileServer::FindCurDrive()
 {
   CSingleLock lock( &m_mtxDrives, TRUE );

   vector<TDrive*>::iterator itSta( m_vDrives.begin() );
   vector<TDrive*>::iterator itEnd( m_vDrives.end() );

   int iDR = _getdrive();
   for( ; itSta != itEnd; ++itSta )
     if( (*itSta)->m_bDrive == _getdrive() && (*itSta)->IsRemote() == false )
	    return *itSta;

   return NULL;
 }

void TFileServer::OnDeviceChange_My( UINT nEventType, DWORD dwData )
 {
   //BOOL bRes = CWnd::OnDeviceChange( nEventType, dwData );

   if( nEventType == DBT_DEVICEREMOVECOMPLETE )
	{
      DEV_BROADCAST_VOLUME *p = (DEV_BROADCAST_VOLUME*)dwData;
	  if( p->dbcv_devicetype == DBT_DEVTYP_VOLUME )
	   {
	     ULONG ul = 1;
	     for( int i = 'A'; i <= 'Z'; ++i, ul <<= 1 )		 
           if( p->dbcv_unitmask & ul ) RemoveDrive( i, false );		  
	   }
	}
   else if( nEventType == DBT_DEVICEARRIVAL )
	{
      DEV_BROADCAST_VOLUME *p = (DEV_BROADCAST_VOLUME*)dwData;
	  if( p->dbcv_devicetype == DBT_DEVTYP_VOLUME )
	   {
         ULONG ul = 1;
	     for( int i = 'A'; i <= 'Z'; ++i, ul <<= 1 )		 
           if( p->dbcv_unitmask & ul ) AddLocalDrive( i );
	   }
	}

 }

void TFileServer::RemoveDrive( int iDr, bool bRemote )
 {
   CSingleLock lock( &m_mtxDrives, TRUE );

   vector<TDrive*>::iterator itSta( m_vDrives.begin() );
   vector<TDrive*>::iterator itEnd( m_vDrives.end() );

   iDr -= 'A' - 1;
   for( ; itSta != itEnd; ++itSta )
     if( (*itSta)->m_bDrive == iDr && (*itSta)->IsRemote() == bRemote )
	  {
        *this - *itSta;
		return;
	  }
 }

void TFileServer::AddLocalDrive( int iDr )
 {
   CSingleLock lock( &m_mtxDrives, TRUE );

   TAutoMemoryRc<TLocalDrive*> arc( new TLocalDrive() );
   CString str; str.Format( "%c:\\", char(iDr) );
   iDr -= 'A' - 1;
   arc()->Init( iDr, GetDriveType((LPCSTR)str) );

   TDrive* pRet = arc();
   arc.ClearRc();

   *this + pRet;
 }

void TFileServer::FindNotifyFS()
 {
   CSingleLock lock( &m_mtxDrives, TRUE );

   vector<TDrive*>::iterator itSta( m_vDrives.begin() );
   vector<TDrive*>::iterator itEnd( m_vDrives.end() );

   for( ; itSta != itEnd; ++itSta )
    if( (*itSta)->IsRemoved() == true )
	 {
	   bool bFlRefreshList;
	   do {
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

   itSta = m_vDrives.begin();
   itEnd = m_vDrives.end();

   for( ; itSta != itEnd; ++itSta )
    if( (*itSta)->IsRemoved() == true )
	 {
       *this - *itSta;
	   itSta = m_vDrives.begin();
       itEnd = m_vDrives.end();
	 }

   itSta = m_vDrives.begin();
   itEnd = m_vDrives.end();

   for( ; itSta != itEnd; ++itSta )
	{      
	  bool bFlRefreshList;
	  do {
        bFlRefreshList = false;

        list<TUserOfDrive*>::iterator itSta2( (*itSta)->m_lstUsers.begin() );
        list<TUserOfDrive*>::iterator itEnd2( (*itSta)->m_lstUsers.end() );

	    for( ; itSta2 != itEnd2; ++itSta2 )
	     if( (*itSta2)->m_hNotify && 
		     (*itSta2)->m_hNotify != INVALID_HANDLE_VALUE &&
		     ::WaitForSingleObject((*itSta2)->m_hNotify,0) != WAIT_TIMEOUT 
		   )
		  {
		    bFlRefreshList = (*itSta2)->Notify( TUserOfDrive::NC_Updated );
			if( bFlRefreshList == true ) break;
		    ::FindNextChangeNotification( (*itSta2)->m_hNotify );
		  }		
	   } while( bFlRefreshList == true );
	}
 }

TDrive* TFileServer::FindDrive( CString& str, bool bRemote )
 {
   CSingleLock lock( &m_mtxDrives, TRUE );

   if( str.IsEmpty() ) return NULL;

   vector<TDrive*>::iterator itSta( m_vDrives.begin() );
   vector<TDrive*>::iterator itEnd( m_vDrives.end() );

   BYTE bDrv = toupper(str[0]) - 'A' + 1;
   for( ; itSta != itEnd; ++itSta )
	if( (*itSta)->m_bDrive == bDrv &&
	    (*itSta)->IsRemote() == bRemote
	  ) return *itSta;
	 
   return NULL;
 }

int TFileServer::FindCurDriveIndex( TDrive* pDrv )
 {
   CSingleLock lock( &m_mtxDrives, TRUE );

   vector<TDrive*>::iterator itSta( m_vDrives.begin() );
   vector<TDrive*>::iterator itEnd( m_vDrives.end() );

   int i = 0;
   for( ; itSta != itEnd; ++itSta, ++i )
	 if( (*itSta)->CmpDrive(*pDrv) == true ) return i;

   return -1;
 }

int TFileServer::FindCurDriveIndex_ForLetter( BYTE bDr, bool bRemote )
 {
   CSingleLock lock( &m_mtxDrives, TRUE );

   vector<TDrive*>::iterator itSta( m_vDrives.begin() );
   vector<TDrive*>::iterator itEnd( m_vDrives.end() );

   int i = 0; bDr = toupper(bDr) - 'A' + 1;
   for( ; itSta != itEnd; ++itSta, ++i )
	 if( (*itSta)->Letter() == bDr && (*itSta)->IsRemote() == bRemote ) 
	    return i;

   return -1;
 }
