#if !defined(_CODING_AR_)
#define _CODING_AR_

#include "stdafx.h"
#include "autorc.hpp"
//#include <afxpriv.h>

class TCodingBAKMirrorFile: public CFile
 {
public:
  enum TCodingAlho { ALHO_XOR, ALHO_DES8 };
  TCodingBAKMirrorFile( bool bCoding = true, TCodingAlho alho = ALHO_XOR, LPCSTR lpKey = "PROTECT_" );

  virtual ~TCodingBAKMirrorFile();

  virtual void Close();
  virtual UINT Read( void* lpBuf, UINT nCount );
  virtual void Write( const void* lpBuf, UINT nCount );

  virtual BOOL Open( LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError = NULL );

protected:
  void CodeBuff( LPBYTE lpBuf, UINT nCount );
  void EncodeBuff( LPBYTE lpBuf, UINT nCount );

private:
  enum { SZ_SizeKey = 8 };
  bool m_bCoding;

  char m_cKey[SZ_SizeKey];
  int m_iKeyCnt;

  TCodingAlho m_alho;
 };

#endif
