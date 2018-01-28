#include "localus.hpp"

#include "net.hpp"
#include "RC\net.rh"

//#include "..\\RUDAKOW\\SERVIS\\pathex.hpp"

#include <errno.h>
#include <direct.h>
#include <algorithm>
#include <typeinfo.h>
#include <dbt.h>


int CmpRecords( TFileRecord *p1, TFileRecord *p2 )
 {
   if( p1->m_csName == "." ) return TRUE;
   if( p1->m_csName == ".." )
	{
      if( p2->m_csName == "." ) return FALSE;
	  return TRUE;
	}
   if( (p1->attrib & _A_SUBDIR) && !(p2->attrib & _A_SUBDIR) ) return TRUE;
   if( (p2->attrib & _A_SUBDIR) && !(p1->attrib & _A_SUBDIR) ) return FALSE;

   return p1->m_csName.CompareNoCase(p2->m_csName) < 0 ? TRUE:FALSE;
 }



TVFileLocal::~TVFileLocal()
 {
 }

void TVFileLocal::Erase( LPCSTR lp ) 
 {   
   if( !::DeleteFile(lp) ) 
	 CFileException::ThrowOsError( ::GetLastError(), lp );	
 }

bool TVFileLocal::CheckExist()
 {
   _finddata_t fnd;
   memset( &fnd, 0, sizeof(_finddata_t) );
   fnd.attrib = _A_ARCH | _A_SYSTEM | _A_HIDDEN | _A_RDONLY | _A_NORMAL;

   long lHandl = _findfirst( (LPSTR)(LPCSTR)m_csName, &fnd );
   if( lHandl != -1 ) 
	{
      _findclose( lHandl );
	  if( !(fnd.attrib & _A_SUBDIR) ) return true;
	}

   return false;
 }

void TVFileLocal::MakeOpen()
 {      
   CFileException except;
   if( !m_File.Open((LPCSTR)m_csName, m_uiAtr, &except) )	      
	  AfxThrowFileException( except.m_cause, except.m_lOsError, (LPCSTR)m_csName );	  
 }

DWORD TVFileLocal::GetSize()
 {
   return m_File.GetLength();
 }

DWORD TVFileLocal::Read( BYTE* lp, DWORD dw, TNotifyInstance* )
 {
   return m_File.Read( lp, dw );
 }

void TVFileLocal::Write( BYTE* lp, DWORD dw, TNotifyInstance* )
 {
   m_File.Write( lp, dw );
 }

void TVFileLocal::Close()
 {
   m_File.Close();
 }


TVSearchLocal::~TVSearchLocal()
 {
   if( lHandl != -1 ) 
	{
      _findclose( lHandl );
	  lHandl = -1;
	}
 }

_finddata_t* TVSearchLocal::GetFirst()
 {   
   memset( &m_fDta, 0, sizeof(_finddata_t) );
   m_fDta.attrib = _A_ARCH | _A_SUBDIR | _A_SYSTEM | _A_HIDDEN | _A_RDONLY | _A_NORMAL;

   lHandl = _findfirst( (LPCSTR)m_csPath, &m_fDta );
   DWORD dwE = ::GetLastError();
   if( lHandl == -1 && errno != ENOENT )
	 CFileException::ThrowOsError( dwE, (LPCSTR)m_csPath );	
   else if( lHandl == -1 && errno == ENOENT )
	{
      m_dwLastErr = errno;
	  return NULL;
	}

   return &m_fDta;
 }

_finddata_t* TVSearchLocal::Next()
 {
   if( !_findnext(lHandl, &m_fDta) ) return &m_fDta;

   if( errno != ENOENT )
	 CFileException::ThrowOsError( ::GetLastError(), (LPCSTR)m_csPath );	
   else 
	{
      m_dwLastErr = errno;
	  return NULL;
	}

   return NULL;
 }


void TLocalDrive::Init( int iDr, UINT uiType )
 { 
   char path_buffer[ _MAX_PATH ];
   if( !_getdcwd(iDr, path_buffer, _MAX_PATH) )
	  CFileException::ThrowOsError( ::GetLastError() );	

   m_strLastAcessPath = path_buffer; 
   m_bDrive = iDr;
   m_uiType = uiType;
 }

bool TLocalDrive::IsRemote() const
 {
   return false;
 }

void TLocalDrive::RmEmptyDir( LPCSTR lp, bool bWarn )
 {
   if( !::RemoveDirectory(lp) )
	{
	  DWORD dw = ::GetLastError();
	  if( dw == ERROR_DIR_NOT_EMPTY )
	   {
	     if( bWarn == false ) return;
		 CString cs; cs.Format( "Каталог '%s' не пуст - стереть нельзя", lp );
		 AfxMessageBox( (LPCSTR)cs, MB_OK|MB_ICONEXCLAMATION );
	   }
	  else
	    CFileException::ThrowOsError( dw, lp );	
	}	 
 }

void TLocalDrive::MkDir( LPCSTR lp )
 {
   _finddata_t fnd;
   memset( &fnd, 0, sizeof(_finddata_t) );
   fnd.attrib = _A_ARCH | _A_SUBDIR | _A_SYSTEM | _A_HIDDEN | _A_RDONLY | _A_NORMAL;

   long lHandl = _findfirst( lp, &fnd );
   if( lHandl != -1 ) 
	{
      _findclose( lHandl );
	  if( fnd.attrib & _A_SUBDIR ) return;
	}

   if( _mkdir( lp ) == -1 )
	 CFileException::ThrowOsError( ::GetLastError(), lp );	
 }

void TLocalDrive::FindFillTmp( LPSTR path_buffer )
 {
   ClearFList( m_rvecTmp );

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


void TLocalDrive::Directory( TUserOfDrive& rUs, vector<TFileRecord*>& rV )
 {
   char path_buffer[_MAX_PATH+50];
   char drive[_MAX_DRIVE];
   char dir[_MAX_DIR];
   char fname[_MAX_FNAME];
   char ext[_MAX_EXT];

   //_splitpath( (LPSTR)(LPCSTR)rUs.m_strCurPath, drive, dir, fname, ext );
   //if( !*dir || dir[strlen(dir) - 1] != '\\' ) strcpy( dir, "\\" );
   //_makepath( path_buffer, drive, dir, "*", ".*" );
   strcpy( path_buffer, rUs.m_strCurPath.IsEmpty() ? "":(LPCSTR)rUs.m_strCurPath );
   if( path_buffer[strlen(path_buffer) - 1] != '\\' )
     strcat( path_buffer, "\\*.*" );
   else strcat( path_buffer, "*.*" );

   try {
     FindFillTmp( path_buffer );
	}
   catch( CFileException* pE )
	{
      if( pE->m_strFileName.IsEmpty() ) pE->m_strFileName = path_buffer;
      //pE->ReportError( MB_OK | MB_ICONSTOP );
	  //pE->Delete();  	        
	  ClearFList( m_rvecTmp );
	  throw pE;
	}
   catch( CException* pE )
	{	  
      //pE->ReportError( MB_OK | MB_ICONEXCLAMATION );
	  //pE->Delete();  
	  ClearFList( m_rvecTmp );
	  throw pE;
	}      

   ClearFList( rV );

   vector<TFileRecord*>::iterator itSta( m_rvecTmp.begin() );
   vector<TFileRecord*>::iterator itEnd( m_rvecTmp.end() );

   rV.assign( m_rvecTmp.size() );
   for( int i = 0; itSta != itEnd; ++itSta, ++i )
	{
      TAutoMemoryRc<TFileRecord*> arcRec( *itSta );
	  *itSta = NULL;
	  rV[ i ] = arcRec();
	  arcRec.ClearRc();
	}

   ClearFList( m_rvecTmp );
 }

void TLocalDrive::ClearFList( vector<TFileRecord*>& rV )
 {
   vector<TFileRecord*>::iterator itSta( rV.begin() );
   vector<TFileRecord*>::iterator itEnd( rV.end() );

   for( ; itSta != itEnd; ++itSta )
	if( (*itSta) )
	 {
       delete *itSta; *itSta = NULL;
	 }

	rV.clear();
 }

TLocalDrive::~TLocalDrive()
 {
   ClearFList( m_rvecTmp );
 }

TVirtualFile* TLocalDrive::OpenFile( LPCSTR lp, UINT atr )
 {
   return new TVFileLocal( lp, atr );
 }

TVirtualSearch* TLocalDrive::OpenSearch( LPCSTR lp )
 {
   return new TVSearchLocal( lp );
 }

TSecurity* TLocalDrive::GetSecurityAndAtr( LPCSTR lp )
 {
   DWORD dwLength = 0;
   PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
   TAutoMemoryRc<TSecurity*> arc( new TSecurity() );

   if( GetFileSecurity( lp, DACL_SECURITY_INFORMATION,
			            NULL, dwLength, &dwLength )
	 )
	{
      //pSecurityDescriptor = (PSECURITY_DESCRIPTOR) new BYTE[ dwLength ];
	  TAutoMemoryRc_AR<PSECURITY_DESCRIPTOR> arcS( new BYTE[ dwLength ] );
	  if( ::GetFileSecurity( lp, DACL_SECURITY_INFORMATION,
				             arcS(), dwLength, &dwLength )
        )	   
	   { 	     	     
		 arc()->pSec = arcS();
		 arcS.ClearRc();		 
	   }	  	
	}

   arc()->dwAtr = GetFileAttributes( lp );
   if( arc()->dwAtr == 0xFFFFFFFF ) return NULL;
   TSecurity* pRet = arc();
   arc.ClearRc();

   return pRet;
 }


void TLocalDrive::SetSecurityAndAtr( LPCSTR lp, TSecurity* pSec )
 {  
   if( pSec )
	{
      SetFileSecurity( lp, DACL_SECURITY_INFORMATION, pSec->pSec );
	  SetFileAttributes( lp, pSec->dwAtr );
	}
 }

DWORD TLocalDrive::GetBuffSize() const
 {
   return 1024*128;
 }