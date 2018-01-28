#if !defined(_OPT_DLG_)
#define _OPT_DLG_

#include "stdafx.h"
#include "autorc.hpp" 
#include "RC\\net.rh"
#include "cfg.hpp" 

#include "cfg.hpp"

#include "..\\RUDAKOW\\SERVIS\\borctl.hpp"

#include <afxcmn.h>
#include <afxcview.h>


LPCSTR GetBaudEq( int iMnem );

struct TCOMMProp
 {
   COMMPROP     m_comm;
   COMMTIMEOUTS timOut;
   TAutoMemoryRc_AR<COMMCONFIG*>  m_arcCfg;
   bool        m_bAval;
   bool        m_bAvalCfg;
 };

struct TBaudTable;

class TPRopPagePort: public CPropertyPage
 {
public:
  
  TPRopPagePort( UINT uiRc, TMFOptions& rOpt, TCOMMProp* CP ): 
    CPropertyPage( uiRc ),
	m_rOpt( rOpt )
	 {
	   m_CP = CP;
	 };

  virtual ~TPRopPagePort();

  virtual BOOL OnInitDialog();  
  virtual void DoDataExchange( CDataExchange* pDX );

  void ChangeCOMM( int );
  int FindEnabled( const TBaudTable* pT, int iSz, int iSta );
  int CheckForDisabledCheck( const TBaudTable* pT, int iSz );

  afx_msg void OnCOM1();
  afx_msg void OnCOM2();
  afx_msg void OnCOM3();
  afx_msg void OnCOM4();
  
  //afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
    
protected:
  DECLARE_MESSAGE_MAP()

  TCOMMProp* m_CP;
  TMFOptions& m_rOpt;
 };

class TPRopPageProtocol: public CPropertyPage
 {
public:
  
  TPRopPageProtocol( UINT uiRc, TMFOptions& rOpt ): 
    CPropertyPage( uiRc ),
	m_rOpt( rOpt )
	 {};

  virtual ~TPRopPageProtocol();

  virtual BOOL OnInitDialog();  
  virtual void DoDataExchange( CDataExchange* pDX );
  
  //afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
    
protected:
  //DECLARE_MESSAGE_MAP()

  TMFOptions& m_rOpt;
 };

struct TTreeItem
 {
   TTreeItem() { m_h = NULL; }

   HTREEITEM m_h;
   //CString m_csStr;
 };
const int iNumbRoots = 13;
const int iNumbRoots2 = 29;

class TPRopPageInfo: public CPropertyPage
 {
public:
  
  TPRopPageInfo( UINT uiRc, TCOMMProp* CP ): 
    CPropertyPage( uiRc )	
	 { m_CP = CP; };

  virtual ~TPRopPageInfo();

  virtual BOOL OnInitDialog();  

  void ChangeCOMM( int );

  afx_msg void OnCOM1();
  afx_msg void OnCOM2();
  afx_msg void OnCOM3();
  afx_msg void OnCOM4();
          
protected:
  TCOMMProp* m_CP;
  CTreeCtrl m_Tree;

  TTreeItem m_Root[ iNumbRoots ];
  TTreeItem m_Root2[ iNumbRoots2 ];

  DECLARE_MESSAGE_MAP()  
 };


class TUsersDialog: public TBWCCPropertySheet
 {

public:

  TUsersDialog( TMFOptions& rOpt, LPCTSTR pszCaption, CWnd *pParentWnd, UINT iSelectPage = 0 );
   

  virtual ~TUsersDialog();

  afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
  afx_msg void OnOK();

  int m_iPage;

private:  
  CWnd *m_pPar;
  TPRopPagePort     m_pagPort;    
  TPRopPageProtocol m_pagProtocol;
  TPRopPageInfo     m_pagInfo;   

  TCOMMProp m_CP[4];

  TMFOptions& m_rOpt;    

  BOOL UpdateDataMy( CPropertyPage& rP, BOOL bFl );

  DECLARE_MESSAGE_MAP()
 };


#endif
