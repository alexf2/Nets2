#if !defined(_LZ_FILE_)
#define  _LZ_FILE_

#include "stdafx.h"

#if defined(TLZ_DLL)
  #define EXPORT_TLZ_ __declspec(dllexport)
#else 
  #define EXPORT_TLZ_ __declspec(dllimport)
#endif


class EXPORT_TLZ_ TFileKill: public CFile
 {
public:
  TFileKill( LPCSTR lpName, UINT uiAtr ): 
    CFile( lpName, uiAtr ),
    m_Name( lpName )
	 {
	   m_FlKilled = false;
	 };

  virtual ~TFileKill();
  void KillFile();

private:
  CString m_Name;
  bool m_FlKilled;
 };

class EXPORT_TLZ_ TLZFile: public CFile
 {
public:
  //TLZFile( LPCTSTR lpszFileName, UINT nOpenFlags );
  TLZFile(): CFile() { m_LZ = 0; }
  virtual ~TLZFile();

  virtual UINT Read( void* lpBuf, UINT nCount );

  virtual BOOL Open( LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError = NULL );
  virtual LONG Seek( LONG lOff, UINT nFrom );

  virtual void Close();

private:
  INT m_LZ;
  CString csName;
 };

#endif
