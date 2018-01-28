#if !defined(_FDLG_)
#define _FDLG_


#include "stdafx.h"
#include "autorc.hpp"
#include "ncpane.hpp"
//#include "cfg.hpp"
#include "RC\\net.rh"
#include "fsrv.hpp" 


class TUserOfDrive_Cpy: public TUserOfDrive
 {
public:
  TUserOfDrive_Cpy( TDrive *pDrv, CEvent& revCancel, 
                    CEvent& revCancelPassed, CEvent& revRemoved ): 
    TUserOfDrive( pDrv ),
	m_revCancel( revCancel ),
	m_revCancelPassed( revCancelPassed ),
	m_revRemoved( revRemoved )
   {
     //m_pOwner = NULL;
   }  
  TUserOfDrive_Cpy( TUserOfDrive_NCPane& rUs, CEvent& revCancel, 
                    CEvent& revCancelPassed, CEvent& revRemoved ): 
    TUserOfDrive( rUs ),
	m_revCancel( revCancel ),
	m_revCancelPassed( revCancelPassed ),
	m_revRemoved( revRemoved )
	{
	}

  bool IsCallingOutside() const;   
  virtual bool NotifyUser( TUserOfDrive::TNotifyCode );

private:
  CEvent& m_revCancel;
  CEvent& m_revCancelPassed;
  CEvent& m_revRemoved;
 };



class TKBPassThread: public CWinThread
 {
public:
  TKBPassThread( AFX_THREADPROC pfnTh, CWnd* pResponse ): 
	CWinThread( pfnTh, (LPVOID)pResponse )
   {
     m_pResp = pResponse;
   }
  
  virtual BOOL PreTranslateMessage( MSG *pMsg );

private:
  CWnd *m_pResp;
 };




class TNCNotify;

class TCopyDlg: public CDialog
 {

friend UINT ThreadProc( LPVOID pObj );

public:
  TCopyDlg( CWnd *pPar, bool bFlErase, TUserOfDrive_NCPane *pUsSrc, 
            TUserOfDrive_NCPane *pUsDst, CEvent& evConnectBroken ):     
    CDialog( IDD_DLG_COPY, pPar ),
	m_evCancel( FALSE, TRUE ),
	m_evCancelPassed( FALSE, TRUE ),
	m_evConnectBroken( evConnectBroken ),
	m_evRemoved( FALSE, TRUE )
   {
     m_mPar = pPar;
	 m_pUsSrc = pUsSrc;	 
	 m_arcDupSrc = new TUserOfDrive_Cpy( *pUsSrc, m_evCancel, m_evCancelPassed, m_evRemoved );	 
	 *m_arcDupSrc()->GetDrive() + m_arcDupSrc();

	 m_pUsDst = pUsDst;
	 m_bFlErase = bFlErase; 
	 m_csKeyWord = bFlErase == true ? "Move":"Copy";
	 m_bFlBlue = false;

	 m_iOneItem = -1;
   }

  virtual ~TCopyDlg();

  afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );

  virtual BOOL OnInitDialog();
  virtual void DoDataExchange( CDataExchange* pDX );

  virtual void OnOK();  
  virtual void OnCancel();
  virtual BOOL DestroyWindow();

  void NotifyProc( TNotifyData *pNotifyData );

protected:
  void StartOp();

private:  
  CWnd* m_mPar;
  CEvent m_evCancel;
  CEvent m_evCancelPassed;
  CEvent m_evRemoved;

  TAutoMemoryRc<CWinThread*> m_arcTh;
  TAutoMemoryRc<TNCNotify*> m_arcNotify;

  CEvent& m_evConnectBroken;

  TAutoMemoryRc<TUserOfDrive_Cpy*> m_arcNewDst;
  TAutoMemoryRc<TUserOfDrive_Cpy*> m_arcDupSrc;

  TUserOfDrive_NCPane *m_pUsSrc;
  TUserOfDrive_NCPane *m_pUsDst;
  bool m_bFlErase;

  CString m_csKeyWord;
  bool m_bFlBlue;

  void SafetyKillTh();

  HWND m_hWndSrc, m_hWndDst, m_hWndProgr;
  unsigned long m_ulInkeySize;
  double m_dK;
  int m_iIndexInPath;

  int m_iOneItem;

  CFont m_fntDlg;
  CRect m_rDst, m_rSrc;

  DECLARE_MESSAGE_MAP()
 };


class TNCNotify: public TNotifyInstance
 {
public:
 TNCNotify( CWnd* pWndNotify, CEvent& rEvCancel, TCopyDlg& rDlg ): 
   m_rDlg( rDlg ),
   TNotifyInstance( pWndNotify, rEvCancel )
	{
	}

  virtual void Notinfy( TNotifyData *pNotifyData );

private:
  TCopyDlg& m_rDlg;
 };


class TNCNotifyE;

class TEraseDlg: public CDialog
 {

friend UINT ThreadProcE( LPVOID pObj );

public:
  TEraseDlg( CWnd *pPar, TUserOfDrive_NCPane *pUsSrc, CEvent& evConnectBroken ):     
    CDialog( IDD_DLG_ERASE, pPar ),
	m_evCancel( FALSE, TRUE ),
	m_evCancelPassed( FALSE, TRUE ),
	m_evConnectBroken( evConnectBroken ),
	m_evRemoved( FALSE, TRUE )
   {
     m_mPar = pPar;
	 m_pUsSrc = pUsSrc;	 
	 m_arcDupSrc = new TUserOfDrive_Cpy( *pUsSrc, m_evCancel, m_evCancelPassed, m_evRemoved );	 
	 *m_arcDupSrc()->GetDrive() + m_arcDupSrc();
	 m_bFlBlue = false;

	 m_iOneItem = -1;
   }

  virtual ~TEraseDlg();

  afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );

  virtual BOOL OnInitDialog();
  virtual void DoDataExchange( CDataExchange* pDX );

  virtual void OnOK();  
  virtual void OnCancel();
  virtual BOOL DestroyWindow();

  void NotifyProc( TNotifyData *pNotifyData );

protected:
  void StartOp();

private:  
  CWnd* m_mPar;
  CEvent m_evCancel;
  CEvent m_evCancelPassed;
  CEvent m_evRemoved;

  TAutoMemoryRc<CWinThread*> m_arcTh;
  TAutoMemoryRc<TNCNotifyE*> m_arcNotify;
  
  TAutoMemoryRc<TUserOfDrive_Cpy*> m_arcDupSrc;

  CEvent& m_evConnectBroken;

  TUserOfDrive_NCPane *m_pUsSrc;
  
  bool m_bFlBlue;

  void SafetyKillTh();

  HWND m_hWndSrc;

  int m_iIndexInPath;
  int m_iOneItem;

  CRect m_rSrc;

  CFont m_fntDlg;

  DECLARE_MESSAGE_MAP()
 };


class TNCNotifyE: public TNotifyInstance
 {
public:
 TNCNotifyE( CWnd* pWndNotify, CEvent& rEvCancel, TEraseDlg& rDlg ): 
   m_rDlg( rDlg ),
   TNotifyInstance( pWndNotify, rEvCancel )
	{
	}

  virtual void Notinfy( TNotifyData *pNotifyData );

private:
  TEraseDlg& m_rDlg;
 };


#endif
