#include "coding.hpp"


TCodingBAKMirrorFile::TCodingBAKMirrorFile( bool bCoding, TCodingAlho alho, LPCSTR lpKey ): CFile()
 {
   static unsigned char cBXOR[ SZ_SizeKey+1 ] = { "CRFtPASS" };

   m_alho = alho;
   m_bCoding = bCoding;

   if( bCoding == false ) return;

   memset( m_cKey, 0, SZ_SizeKey );
   strncpy( m_cKey, lpKey, SZ_SizeKey );
   for( int i = 0; i < SZ_SizeKey; i++ ) m_cKey[i] ^= cBXOR[i];
 
   m_iKeyCnt = 0;

   //if( m_alho == ALHO_DES8 ) setkey( m_cKey );
 }

TCodingBAKMirrorFile::~TCodingBAKMirrorFile()
 {
 }

BOOL TCodingBAKMirrorFile::Open( LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError )
 {    
   if( nOpenFlags & (CFile::modeWrite|CFile::modeReadWrite|CFile::modeCreate) )
	{
      char path_buffer[ _MAX_PATH ];
      char drive[ _MAX_DRIVE ];
      char dir[ _MAX_DIR ];
      char fname[ _MAX_FNAME ];
      char ext[ _MAX_EXT ];

	  _splitpath( lpszFileName, drive, dir, fname, ext );
	  _makepath( path_buffer, drive, dir, fname, ".BAK" );	  	 

	  CFile fBAK;
	  CFileException exc;
	  if( fBAK.Open(path_buffer, CFile::shareExclusive | CFile::modeRead, &exc) )
	   {
         fBAK.Close();
		 CFile::Remove( path_buffer );
	   }
	  else if( exc.m_cause != CFileException::fileNotFound )
	   {
         pError->m_lOsError    = exc.m_lOsError;
         pError->m_cause       = exc.m_cause;
         pError->m_strFileName = path_buffer;

         return FALSE;
	   }

	  try {
	    CFile::Rename( lpszFileName, path_buffer );
	   }
	  catch( CFileException* pE )
	   {
         if( pE->m_cause != CFileException::fileNotFound ) throw pE;
		 pE->Delete();
	   }

	  if( nOpenFlags & CFile::modeCreate )
	    return CFile::Open( lpszFileName, nOpenFlags, pError );
	  
      if( !::CopyFile(path_buffer, lpszFileName, FALSE) )
	   {
	     pError->m_lOsError    = ::GetLastError();
         pError->m_cause       = CFileException::OsErrorToException( pError->m_lOsError );		 
         pError->m_strFileName = path_buffer;

         return FALSE;
	   }
	}
   
   return CFile::Open( lpszFileName, nOpenFlags, pError );
 }

void TCodingBAKMirrorFile::Close()
 {    
	CFile::Close();
 }

UINT TCodingBAKMirrorFile::Read( void* lpBuf, UINT nCount )
 {
   if( m_bCoding == false ) return CFile::Read( lpBuf, nCount );

   m_iKeyCnt = DWORD(CFile::GetPosition() % (DWORD)SZ_SizeKey);
   UINT uiRes = CFile::Read( lpBuf, nCount );
   
   EncodeBuff( (LPBYTE)lpBuf, uiRes );   
   return uiRes;
 }

void TCodingBAKMirrorFile::Write( const void* lpBuf, UINT nCount )
 {
   if( m_bCoding == false )
	{
	  CFile::Write( lpBuf, nCount );
	  return;
	}
   
   m_iKeyCnt = DWORD(CFile::GetPosition() % (DWORD)SZ_SizeKey);
   
   CodeBuff( (LPBYTE)lpBuf, nCount );
   CFile::Write( lpBuf, nCount );
 }

void TCodingBAKMirrorFile::CodeBuff( LPBYTE lpBuf, UINT nCount )
 {
   if( m_alho == ALHO_XOR )
	{
      for( ; nCount; nCount--, lpBuf++ )
	   {
         *lpBuf ^= m_cKey[ m_iKeyCnt++ ];
	     if( m_iKeyCnt >= SZ_SizeKey ) m_iKeyCnt = 0;
	   }
	}
 }

void TCodingBAKMirrorFile::EncodeBuff( LPBYTE lpBuf, UINT nCount )
 {
   if( m_alho == ALHO_XOR )
	{
      for( ; nCount; nCount--, lpBuf++ )
	   {
         *lpBuf ^= m_cKey[ m_iKeyCnt++ ];
	     if( m_iKeyCnt >= SZ_SizeKey ) m_iKeyCnt = 0;
	   }
	}
 }

