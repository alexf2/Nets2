#if !defined(_MYFILE_SERVER_)
#define _MYFILE_SERVER_

#include "stdafx.h"
#include "autorc.hpp" 

//#include <afxcmn.h>
#include <io.h>
#include <afxmt.h>

#include <list>
#include <vector>
using namespace std;

class TCannotChangeDrive {}; //попытка изменить drive при измениении path
class TDriveNotExist {};
class TDuplicateUser {};
class TBadUseDrive {};
class TDuplicateDrive {};

class TUserCancel {};

struct TFileRecord
 {
   TFileRecord() { m_flSelect = false; }
   TFileRecord( _finddata_t& );

   void InvertSelect()
	{
      m_flSelect = !m_flSelect;
	}

   CString m_csName;

   unsigned attrib;
   __int64 size;
   time_t time_write;

   bool m_flSelect;
 };

struct TNotifyData
 { 
 };

class TDrive;
struct TNotifyDataCopy: public TNotifyData
 {    
   LPSTR m_lpFromShortPath;
   LPSTR m_lpToFullPath;
   bool m_bFlStartOp;
   bool m_bFlOpComplete;
   unsigned long m_ulSize;   
   TDrive *m_pSrc, *m_pDst;
 };

class TNotifyInstance
 {
public:
  TNotifyInstance( CWnd* pWndNotify, CEvent& rEvCancel ):
    m_rEvCancel( rEvCancel )
	 {
       m_pWndNotify = pWndNotify;
	   m_bFlAskReplace = true;
	   m_bFlAskErase = true;
	   m_bEnabled = true;
	 }
  virtual ~TNotifyInstance();

  virtual void Notinfy( TNotifyData *pNotifyData ) = 0;
  void IsCanceled();   

  CWnd* GetNotifyWnd() { return m_pWndNotify; }

  bool Enable( bool bFl = true ) 
   { 
     bool bKey = m_bEnabled;
     m_bEnabled = bFl; 
	 return bKey;
   }
  

  bool m_bFlAskReplace;
  bool m_bFlAskErase;

protected:
  CWnd*   m_pWndNotify; 
  CEvent& m_rEvCancel;

  bool m_bEnabled;
 };

struct TSecurity
 {
   TSecurity()
	{
	  pSec = 0;
	  dwAtr = 0;
	}
   ~TSecurity();

   PSECURITY_DESCRIPTOR pSec;
   DWORD dwAtr;
 };


class TVirtualFile
 {
public:
  TVirtualFile( LPCSTR lpName, UINT uiAtr ):      
	 m_csName( lpName )
   {
     m_uiAtr = uiAtr;
   }

  virtual ~TVirtualFile();

  virtual void MakeOpen() = 0;
  virtual DWORD GetSize() = 0;
  virtual DWORD Read( BYTE*, DWORD, TNotifyInstance* pNotify ) = 0;
  virtual void Write( BYTE*, DWORD, TNotifyInstance* pNotify ) = 0;
  virtual void Close() = 0;
  virtual bool CheckExist() = 0;  

  virtual void Erase( LPCSTR ) = 0;  

protected:
  CString m_csName;
  UINT m_uiAtr;
 };


class TVirtualSearch
 {
public:
  TVirtualSearch( LPCSTR lpPath );
  
  virtual ~TVirtualSearch();

  virtual _finddata_t *GetFirst() = 0;
  virtual _finddata_t *Next() = 0;

  DWORD GetErr() const
   {
     return m_dwLastErr;
   }

protected:
  CString m_csPath;
  _finddata_t m_fDta;
  DWORD m_dwLastErr;
 };


class TUserOfDrive;

class TDrive
 {

friend class TFileServer;
friend class TRemoteManeger;

public:
  TDrive(): m_mtxSHR( FALSE )
   {
     m_bDrive = 0;
	 m_uiType = 0;
	 m_bSheduledRemove = false;
   }
  virtual ~TDrive();
  
  virtual void Init( int iDr, UINT uiType ) = 0;

  TDrive& operator+( TUserOfDrive* );
  TDrive& operator-( TUserOfDrive* );

  virtual bool CmpDrive( TDrive& rD );
   

  LPCSTR GetPath() const { return m_strLastAcessPath; }  
  void ChangePath( LPCSTR lp );

  virtual bool IsRemote() const = 0;
  virtual DWORD GetBuffSize() const = 0;

  bool IsRemoved();
  void SheduleRemove();

  virtual void Directory( TUserOfDrive&, vector<TFileRecord*>& ) = 0;
  virtual void MkDir( LPCSTR lp ) = 0;
  virtual void RmEmptyDir( LPCSTR, bool bWarn ) = 0;

  virtual TSecurity* GetSecurityAndAtr( LPCSTR ) = 0;
  virtual void SetSecurityAndAtr( LPCSTR, TSecurity* ) = 0;

  void Copy( TUserOfDrive& src, TUserOfDrive& dst, 
             vector<TFileRecord*>*, 
			 bool bSubDir, bool bMove, 
			 TNotifyInstance* pNotify);
  void Erase( TUserOfDrive& src, vector<TFileRecord*>*, 
              bool bSubDir, TNotifyInstance* pNotify );

  UINT Type() const { return m_uiType; }
  BYTE Letter() const { return  m_bDrive; }

protected:    
  CString m_strLastAcessPath;
  BYTE m_bDrive;
  UINT m_uiType;

  bool m_bSheduledRemove;
  CMutex m_mtxSHR;

  list<TUserOfDrive*> m_lstUsers; //referenced

  void CopyFile( TUserOfDrive& src, TUserOfDrive& dst, 
				 LPCSTR lpName, TNotifyInstance* pNotify
			   );
					  
  void EraseFile( TUserOfDrive& src, LPSTR lpName, 
				  TNotifyInstance* pNotify
 				);

  virtual TVirtualFile* OpenFile( LPCSTR, UINT ) = 0;
  virtual TVirtualSearch* OpenSearch( LPCSTR ) = 0;
					 
 };


class TUserOfDrive
 {

friend class TLocalDrive;
friend class TRemoteDrive;
friend class TDrive;
friend class TFileServer;

public:
  TUserOfDrive( TDrive *pDrv )
   {
     m_pDrv = pDrv;	 
	 m_strCurPath = pDrv ? pDrv->GetPath():"";
	 m_bExist = pDrv ? true:false;
	 m_hNotify = NULL;

	 OpenNotify();
   }
  TUserOfDrive( TUserOfDrive& rUs )
   {
     m_pDrv = rUs.m_pDrv;	 
	 m_strCurPath = rUs.m_strCurPath;
	 m_bExist = rUs.m_bExist;
	 m_hNotify = NULL;
	 

	 OpenNotify();
   }
  virtual ~TUserOfDrive();
    

  enum TNotifyCode { NC_Removed, NC_Updated };

  virtual bool NotifyUser( TNotifyCode ) = 0;
  bool Notify( TNotifyCode );
  virtual bool IsCallingOutside() const = 0;

  void ChangeDrive( TDrive* );
  LPCSTR GetPath() const { return m_strCurPath; }
  void ChangePath( LPCSTR lp );

  void UpdateLastAcess();

  void Directory( vector<TFileRecord*>& );
  void Copy( TUserOfDrive& dst, vector<TFileRecord*>*, bool bSubDir, TNotifyInstance* pNotify );
  void Move( TUserOfDrive& dst, vector<TFileRecord*>*, bool bSubDir, TNotifyInstance* pNotify );
  void Erase( vector<TFileRecord*>*, bool bSubDir, TNotifyInstance* pNotify );

  TDrive *GetDrive() const { return m_pDrv; }

protected:
  TDrive *m_pDrv; //not owned
  CString m_strCurPath;
  bool m_bExist;

  HANDLE m_hNotify;

  void CheckDrive();
  void CloseNotify();
  void OpenNotify();
 };



#endif
