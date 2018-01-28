#if !defined(_REMOTE_USER_)
#define _REMOTE_USER_

#include "stdafx.h"
#include "autorc.hpp" 
#include "fsrv.hpp" 
#include "server.hpp" 
#include "phus.hpp" 

#include <vector>
#include <list>
using namespace std;


//#include <afxcmn.h>
//#include <io.h>

//#include <vector>
//using namespace std;

enum CR_Types { CR_Result = 0, CR_Erase = 1, CR_CheckExist = 2 ,
                CR_Open = 3, CR_GetSize = 4, CR_Close = 5,
                CR_Read = 6, CR_Write = 7, CR_FindFirst = 8,
				CR_FindNext = 9, CR_FindClose = 10,

				CR_DriveInit = 11, CR_RmDir = 12, CR_MkDir = 13,
				CR_GetSecurity = 14, CR_SetSecurity = 15, CR_Dir = 16,

				CR_GetDrives = 17 };

struct TCommonReq
 {
   TCommonReq( BYTE bT ) { m_bType = bT; }

   BYTE m_bType;
 };


struct TCh_DriveInit: public TCommonReq
 {
   TCh_DriveInit():TCommonReq( CR_DriveInit ) {};
   int iDr;
   UINT uiType ;
 };

struct TCh_SizeData: public TCommonReq
 {
   TCh_SizeData():TCommonReq( CR_GetSize ) {};
   HFILE mHandle;
 };

struct TCh_ReadData: public TCommonReq
 {
   TCh_ReadData():TCommonReq( CR_Read ) {};
   HFILE mHandle;
   DWORD dwSize;
 };

struct TCh_EraseData: public TCommonReq
 {
   TCh_EraseData():TCommonReq( CR_Erase ) {};

   static  TCh_EraseData* Create( LPSTR lp );
   char m_Name[1];

   int SZ() { return sizeof(TCh_EraseData) + strlen(m_Name); }
 };

struct TCh_OpenData: public TCommonReq
 {
   TCh_OpenData():TCommonReq( CR_Open ) {};

   static  TCh_OpenData* Create( LPSTR lp, UINT atr );
   DWORD uiAtr;
   char m_Name[1];

   int SZ() { return sizeof(TCh_OpenData) + strlen(m_Name); }
 };


struct TCh_Result: public TCommonReq
 {
   TCh_Result(): TCommonReq( CR_Result )
	{ 	  
	  m_dwRes = m_dwErr = m_dwExtErr = 0;
	}   
   
   TCh_Result& operator=( TCh_Result& rU )
	{
	  m_bType    = rU.m_bType;
      m_dwRes    = rU.m_dwRes;
      m_dwErr    = rU.m_dwErr;
      m_dwExtErr = rU.m_dwExtErr;

	  return *this;
	}

   DWORD m_dwRes;
   DWORD m_dwErr;
   DWORD m_dwExtErr;
 };

struct TMyFindData
 {  
   unsigned attrib;
   __int64 size;
   time_t time_write;
 };


/*class TRemoteUser: public TUserOfDrive
 {
public:
  TRemoteUser( TDrive *pDrv ): 
    TUserOfDrive( pDrv )
   {
     m_pOwner = NULL;
	 m_pPar = NULL;
   }
  
  virtual bool NotifyUser( TUserOfDrive::TNotifyCode );

  TDrive* GetDriveFor( CString& csPath, bool bRemote );
  void SetPar( TRemoteManeger *pPar )
   {
     m_pPar = pPar;
   }  
   

private:
  TRemoteManeger *m_pPar;
 };*/



class TVFileRemote: public TVirtualFile
 {
public:
  TVFileRemote( LPCSTR lpName, UINT uiAtr, TChannel* pCh ):  
    m_pCh( pCh ),
	TVirtualFile( lpName, uiAtr )	
	 {	
	   m_Handle = (HFILE)INVALID_HANDLE_VALUE;
	 }
  virtual ~TVFileRemote();

  virtual void MakeOpen();
  virtual DWORD GetSize();
  virtual DWORD Read( BYTE*, DWORD, TNotifyInstance* pNotify );
  virtual void Write( BYTE*, DWORD, TNotifyInstance* pNotify );
  virtual void Close();
  virtual bool CheckExist();

  virtual void Erase( LPCSTR );
  
private:  
  TChannel* m_pCh;
  HFILE m_Handle;
 };

class TVSearchRemote: public TVirtualSearch
 {
public:
  TVSearchRemote( LPCSTR lp, TChannel* pCh ): TVirtualSearch( lp ),
   m_pCh( pCh )
   {
     lHandl = -1;
   }

  virtual ~TVSearchRemote();

  virtual _finddata_t *GetFirst();
  virtual _finddata_t *Next();

private:
  long lHandl;
  TChannel* m_pCh;
 };

class TRemoteDrive: public TDrive
 {
public:
  TRemoteDrive( TChannel* pCh ): TDrive(), m_pCh( pCh ) {};
  virtual ~TRemoteDrive();

  virtual void Init( int iDr, UINT uiType );
  virtual bool IsRemote() const;
  virtual DWORD GetBuffSize() const;
  
  virtual void Directory( TUserOfDrive&, vector<TFileRecord*>& );
  virtual void MkDir( LPCSTR lp );
  virtual void RmEmptyDir( LPCSTR, bool bWarn );

  virtual TSecurity* GetSecurityAndAtr( LPCSTR );
  virtual void SetSecurityAndAtr( LPCSTR, TSecurity* );

  /*virtual void Copy( TUserOfDrive& src, TUserOfDrive& dst, 
                     vector<TFileRecord*>*, 
					 bool bSubDir, bool bMove, TNotifyInstance* pNotify );
  virtual void Erase( TUserOfDrive& src, vector<TFileRecord*>*, 
                      bool bSubDir, TNotifyInstance* pNotify );*/

  void ClearFList( vector<TFileRecord*>& rV );
  void FindFillTmp( LPSTR path_buffer );

private:
  TChannel* m_pCh;
  vector<TFileRecord*> m_rvecTmp;

  virtual TVirtualFile* OpenFile( LPCSTR, UINT );
  virtual TVirtualSearch* OpenSearch( LPCSTR );
  
 };


struct TFndData
 {
   LONG m_Handl;
   _finddata_t m_Fnd;
 };

class TRemoteManeger
 {
public:
  TRemoteManeger( TFileServer *pFSrv, TChannel* pCh1, TChannel* pCh2, CEvent& evConnectOK, bool bRemote ):
   //m_usrUserOfSrv( NULL ),
   m_evCancel( FALSE, TRUE ),
   m_evConnectOK( evConnectOK )
    {
      m_pFSrv = pFSrv;
	  m_pCh = NULL;
	  m_pCh_1 = pCh1;
	  m_pCh_2 = pCh2;
	  m_bRemote = bRemote;
	  //m_usrUserOfSrv.SetPar( this );
    }

   virtual ~TRemoteManeger();

  void StartWorkCycle();
  void WorkCycle();

    
private:
  //TRemoteUser m_usrUserOfSrv;
  TChannel *m_pCh, *m_pCh_1, *m_pCh_2;
  TFileServer *m_pFSrv;

  bool m_bRemote; 

  void SafetyKillTh();  
  TAutoMemoryRc<CWinThread*> m_arcTh;
  CEvent m_evCancel;

  CEvent& m_evConnectOK;

  void DispatchReq( BYTE* pBlock );
  DWORD SendResult( TCh_Result& res );  
  void MakeErase( TCommonReq *pCommon );
  void MakeOpen( TCommonReq *pCommon );
  void MakeCheckExist( TCommonReq *pCommon );
  void MakeGetSize( TCommonReq *pCommon );

  void MakeRead( TCommonReq *pCommon );
  void MakeWrite( TCommonReq *pCommon );

  void MakeClose( TCommonReq *pCommon );
  void MakeFindFirst( TCommonReq *pCommon );  
  void MakeFindNext( TCommonReq *pCommon );
  void MakeFindClose( TCommonReq *pCommon );

  void MakeGetDir( TCommonReq *pCommon );
  void MakeGetSecurity( TCommonReq *pCommon );
  void MakeSetSecurity( TCommonReq *pCommon );

  void GetDrives( TCommonReq *pCommon );
  void MakeRmDir( TCommonReq *pCommon );
  void MakeMkDir( TCommonReq *pCommon );

  void MakeDriveInit( TCommonReq *pCommon );

  void ConnectProc();
  void DisconnectProc();

  list<CFile*> m_lstFiles;
  void ClearFileList();
  CFile* GetFile( HFILE mHandle );

  list<TFndData*> m_lstFind;
  void ClearFindList();  
  TFndData* GetFnd( LONG mHandle );

  void AddRemoteDrives();
  void RemoveRemoteDrives();
  
  void FindFillTmp( LPSTR path_buffer, vector<TFileRecord*>& m_rvecTmp );
 };


#endif
