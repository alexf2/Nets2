#if !defined(_NFRAME_)
#define _NFRAME_


#include "stdafx.h"
#include "autorc.hpp"
#include "about.hpp"
#include "ncpane.hpp"
#include "cfg.hpp"
#include "RC\\net.rh"
#include "fsrv.hpp" 
#include "ind.hpp" 
#include "phus.hpp" 
#include "ruser.hpp" 


#include "..\\RUDAKOW\\SERVIS\\mymenu.hpp"

#define ID_LAMP  1
#define ID_CFG   2

#define ID_IND_ONLINE  101
#define ID_IND_IN      102 
#define ID_IND_OUT     103


class TMainFrame: public CFrameWnd
 {

    friend class TNCSplitter;

protected: 	
	DECLARE_DYNCREATE( TMainFrame )

public:	
    TMainFrame();
	virtual ~TMainFrame();

    virtual BOOL PreCreateWindow( CREATESTRUCT& cs );

	void SetupMenu( HMENU hMenu );

    void SplitterModfyNotify();


	void TMainFrame::DispCPS( int cps )
	 {
	   m_iPrivCps = cps;
       DispBCE( m_optOptions.d.m_iBaud, cps, m_iPrivErr );	   
	 }
    void TMainFrame::DispErr( int err )
	 {
	   m_iPrivErr = err;
       DispBCE( m_optOptions.d.m_iBaud, m_iPrivCps, err );	   
	 }

    void DispBCE( int iBaud, int iCPS, int iErr );

	CEvent m_evEnableConnect;
	CEvent m_evCancelConnect;
	CEvent m_evConnectOK;
	CEvent m_evConnectBroken;

protected:
	afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct) ;
	afx_msg void OnClose();	
	afx_msg void OnDestroy();
	
	afx_msg void OnAbout();			

    afx_msg void OnDrive(); 
	afx_msg void OnDrive1(); 
	afx_msg void OnDrive2(); 
	afx_msg void OnCopy(); 
	afx_msg void OnRename(); 
    afx_msg void OnErase(); 
	afx_msg void OnConnect(); 
	afx_msg void OnOptions(); 

	afx_msg void OnFullPane1();
	afx_msg void OnFullPane2();

	afx_msg void OnChPane();
	afx_msg void OnZoom();
	afx_msg void OnIDMyMenu();

	afx_msg void OnSelectF();
	afx_msg void OnUnSelectF();
	afx_msg void OnRefresh();
	void SelectInstance( bool bFlSelect );

    afx_msg void OnShowWindow( BOOL bShow, UINT nStatus );
	afx_msg BOOL OnEraseBkgnd( CDC* pDC );

	afx_msg void OnActivate( UINT nState, CWnd* pWndOther, BOOL bMinimized );

	afx_msg BOOL OnDeviceChange( UINT nEventType, DWORD dwData );

	virtual BOOL OnCreateClient( LPCREATESTRUCT lpcs, CCreateContext* pContext );
	virtual void RecalcLayout( BOOL bNotify );

	void SetupPanes();
    void KeepPanes();
	
	void OnAlways( CCmdUI* pCmdUI );	
    void OnUpdateOp( CCmdUI* pCmdUI );
	void OnUpdateOpSingl( CCmdUI* pCmdUI );
	void OnUpdateOpt( CCmdUI* pCmdUI );

	void MakeCopy( bool bFlErase );
	void MakeErase();

	void DefaultPos();

	virtual BOOL DestroyWindow();
	
private:  
	TStatusWithLamp  m_wndStatusBar;
    TNCSplitter      m_wndSplit;

	TFileServer  m_FSrvForMe, m_FSrvForYou;

	TAutoMemoryRc<TMSStyleTooolBar*> m_arcTool;
	CBitmap* m_parrMenuBmp;

    bool m_bSplitCreated;

	TMFOptions m_optOptions;
	bool m_bFlLoadingFrame;
	bool m_bFlShow;

	int iPrivPage;

	int m_iPrivCps, m_iPrivErr;

	TAutoMemoryRc<TIndicator*> m_arcIndOnLine, m_arcIndIn, m_arcIndOut;
	CFont m_fInd;

	TMSItem* m_pmsOpt;

	
	TAutoMemoryRc<TPhysicalLevel*> m_arcPhysLvl;
	TAutoMemoryRc<TChannel*> m_arcChLocal;
	TAutoMemoryRc<TChannel*> m_arcChRemote;;
	
	void CreateGlobalConfig();
    void LoadGlobalCongig();    
	void LoadConfigOnStart();

	void SaveGlobalConfig();
	void KeepScreenConfig();

	void DriveChange( int iDr );

	TAutoMemoryRc<TRemoteManeger*> m_arcRM1;
	TAutoMemoryRc<TRemoteManeger*> m_arcRM2;

	DECLARE_MESSAGE_MAP()
};


#endif



