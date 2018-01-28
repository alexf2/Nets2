#define TLZ_DLL
#include "lzfile.hpp"

#include <lzexpand.h>


TFileKill::~TFileKill()
 {
   KillFile();
 }

void TFileKill::KillFile()
 {
   if( m_FlKilled == false )
	{	  
      try {
	    CFile::Remove( (LPCSTR)m_Name );
	   }
	  catch( CFileException* pE )
	   {
         if( pE->m_cause != CFileException::fileNotFound ) throw pE;
		 pE->Delete();
	   }
      
      m_FlKilled = true;
	}
 }


/*TLZFile::TLZFile( LPCTSTR lpszFileName, UINT nOpenFlags ):
  m_LZ( 0 ),
  csName( lpszFileName ),
  CFile( lpszFileName, nOpenFlags )
 {   
 }*/

TLZFile::~TLZFile()
 {
 }

void TLZFile::Close()
 {
   if( m_LZ )
	{
      LZClose( m_LZ );
	  m_LZ = 0;
	}

   CFile::Close();
 }

UINT TLZFile::Read( void* lpBuf, UINT nCount )
 {
   UINT uiRes = LZRead( m_LZ, (LPSTR)lpBuf, nCount );
   if( uiRes == LZERROR_BADINHANDLE ||
	   uiRes == LZERROR_BADOUTHANDLE ||
	   uiRes == LZERROR_BADVALUE ||
	   uiRes == LZERROR_GLOBALLOC ||
	   uiRes == LZERROR_GLOBLOCK ||
	   uiRes == LZERROR_READ ||
	   uiRes == LZERROR_WRITE 	   
	 )
	 AfxThrowFileException( CFileException::accessDenied, 0, (LPCSTR)csName );

   return uiRes;
 }

BOOL TLZFile::Open( LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError )
 {
   if( !CFile::Open(lpszFileName, nOpenFlags, pError)  ) return FALSE;
   m_LZ = LZInit( m_hFile );
   if( m_LZ == LZERROR_BADINHANDLE || 
	   m_LZ == LZERROR_GLOBALLOC || 
	   m_LZ == LZERROR_GLOBLOCK ||
	   m_LZ == LZERROR_UNKNOWNALG
	 )
	{
      pError->m_cause = CFileException::generic;
	  pError->m_lOsError = 0;
	  pError->m_strFileName = lpszFileName;

	  return FALSE;
	}   

   return TRUE;
 }

LONG TLZFile::Seek( LONG lOff, UINT nFrom )
 {
   UINT uiF = 2;
   if( nFrom ==CFile::begin) uiF = 0;
   else if( nFrom ==CFile::current) uiF = 1;

   LONG lRes = LZSeek( m_LZ, lOff, uiF );
   if( lRes == LZERROR_BADINHANDLE || lRes == LZERROR_BADVALUE ||
	   lRes == LZERROR_GLOBLOCK
     )
     AfxThrowFileException( CFileException::badSeek, 0, (LPCSTR)csName );

   return lRes;
 }
