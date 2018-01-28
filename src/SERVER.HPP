#if !defined(_SERVER_DRV_)
#define _SERVER_DRV_

#include "fsrv.hpp"


class TFileServer: public CWnd
 {
public:
  TFileServer(): CWnd(), m_mtxDrives( FALSE )
   {
   }
  virtual ~TFileServer();
  void DrivesClear();

  BOOL Create( CWnd* pPar, UINT id);

  afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
  void OnDeviceChange_My( UINT nEventType, DWORD dwData );
  void DrivesAssemble();

  static TDrive* GetCurDrive();
  TFileServer& operator+( TDrive* );
  TFileServer& operator-( TDrive* );


  void FindNotifyFS();

  void RemoveDrive( int iDr, bool bRemote );
  void AddLocalDrive( int iDr );

  TDrive* FindDrive( CString&, bool bRemote );
  TDrive* FindCurDrive();
  int FindCurDriveIndex( TDrive* );
  int FindCurDriveIndex_ForLetter( BYTE, bool );

  vector<TDrive*>& GetDriveData() { return m_vDrives; }
  CMutex m_mtxDrives;

protected:
  vector<TDrive*> m_vDrives; //owned  

  DECLARE_MESSAGE_MAP()
 };



#endif
