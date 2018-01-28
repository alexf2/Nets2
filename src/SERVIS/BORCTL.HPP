#if !defined(_TBORCTL_)
#define _TBORCTL_

#include "stdafx.h"
#include <list>
using namespace std;

#if defined(TBORCTL_DLL)
  #define EXPORT_TBORCTL_ __declspec(dllexport)
#else 
  #define EXPORT_TBORCTL_ __declspec(dllimport)
#endif


class EXPORT_TBORCTL_ TBorBtn: public CButton
 {
  DECLARE_DYNCREATE( TBorBtn )
public:
  TBorBtn(): CButton() {};

  virtual BOOL PreCreateWindow( CREATESTRUCT& cs );

protected:
  DECLARE_MESSAGE_MAP()
 };

class EXPORT_TBORCTL_ TBWCCPropertySheet: public CPropertySheet
 {
   DECLARE_DYNCREATE( TBWCCPropertySheet )
public:
  TBWCCPropertySheet(): CPropertySheet() {};
  TBWCCPropertySheet( LPCTSTR pszCaption, CWnd *pParentWnd = NULL, UINT iSelectPage = 0 );

  virtual ~TBWCCPropertySheet();

  virtual BOOL OnInitDialog();  
  
  //afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
  virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
  afx_msg void OnDestroy();

  int m_iActiveIndex;

private:
  TBorBtn m_btnOK, m_btnCancel, m_btnHelp;

  DECLARE_MESSAGE_MAP()
 };


struct EXPORT_TBORCTL_ TToolTipData
 {
   TToolTipData() { m_uiId = 0; }
   TToolTipData( UINT id, LPCSTR lp ): m_csTxt( lp )
	{
      m_uiId = id;
	}

   UINT m_uiId;
   CString m_csTxt;
 };

class EXPORT_TBORCTL_ CPropPageTooltips: public CPropertyPage
 {
public:
  CPropPageTooltips( UINT uiRc ): CPropertyPage( uiRc )
   {
   }
  virtual ~CPropPageTooltips();

  virtual BOOL OnInitDialog();  
  virtual int OnToolHitTest( CPoint point, TOOLINFO* pTI ) const;

  CPropPageTooltips& operator+( TToolTipData *pDta )
   {
     m_vecToolTips.push_back( pDta );
	 return *this;
   }

private:
  list<TToolTipData *> m_vecToolTips;

protected:
  DECLARE_MESSAGE_MAP()  
 };


#endif
