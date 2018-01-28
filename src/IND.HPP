#if !defined(_IND_)
#define _IND_

#include "stdafx.h"
#include "autorc.hpp"

#include <list>
//#include <afxcmn.h>

using namespace std ;


class TIndicator: public CStatic
 {
public:
  TIndicator( LPCSTR lpHint, CFont& rFnt ):
    CStatic(),
	m_rFnt( rFnt ),
	m_csHint( lpHint )
	 {
	   m_bOn = false;
	   dwTicks = 0;
	 }

	bool GetStOn() const { return m_bOn; }
	void SetStOn( bool bFl, bool bUpdate = true );
	void Signal( bool );

	void CalcRect( CRect& );
	int GetW();
	 

	afx_msg void OnPaint();
	afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg BOOL OnEraseBkgnd( CDC* pDC );

	void IntPaint( CDC& dc );

private:
  CString m_csHint;
  CFont& m_rFnt;

  CRect m_rTxt;
  CRect m_rcRound;
  CPoint m_pArc1, m_pArc2;

  bool m_bOn;

  DWORD dwTicks;

  DECLARE_MESSAGE_MAP()
 };

class TConnectToItem
 {

friend class TStatusWithLamp; 

public:   
   TConnectToItem( int id ) { m_iId = id; };
   virtual ~TConnectToItem();

   TConnectToItem& operator+( TIndicator *pW )
	{
      m_Lst.push_back( pW );
	  return *this;
	}

protected:
  list<TIndicator*> m_Lst;
  int m_iId;
 };

class TSignalHold
 {
public:
  TSignalHold( TIndicator* pInd, bool bFl )
   {
     m_pInd = pInd;
	 pInd->Signal( bFl );
	 m_bFl = !bFl;
   }
  ~TSignalHold()
   {
     m_pInd->Signal( m_bFl );
   }

private:
  TIndicator* m_pInd;
  bool m_bFl;
 };

class TStatusWithLamp: public CStatusBar
 {
public:
  TStatusWithLamp(): CStatusBar()
   {
   }
 
  virtual ~TStatusWithLamp();

  TStatusWithLamp& operator+( TConnectToItem *pW )
   {
     m_lstCnct.push_back( pW );
	 return *this;
   }

  afx_msg void OnSize( UINT nType, int cx, int cy );

private:
  list<TConnectToItem*> m_lstCnct;

  DECLARE_MESSAGE_MAP()
 };

#endif
