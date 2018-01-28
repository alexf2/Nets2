#define TEXCEPT_DLL
#include "texcept.hpp"

#include "stdafx.h"

#define SERVIS_DLL
#include "servis.hpp"

#define TBWCC_DLL
#include "bwcc.hpp"

#include "bwcc.rh"

TExcept::TExcept( bool bShDel ):
  m_bShDel( bShDel )
 {
   m_uiRcID  = ID_EXCEPT_GENERIC;
   m_uiTtlID = ID_EXCEPT_TTL_EXCEPT;
 }

TExcept::TExcept( bool bShDel, unsigned int uiRcID, unsigned int uiTtlID ):
  m_bShDel( bShDel )
 {
   m_uiRcID = uiRcID;
   m_uiTtlID = uiTtlID;
 }

TExcept::TExcept( bool bShDel, LPCSTR lpMsg, unsigned int uiTtlID ):
  m_bShDel( bShDel ),
  m_cstrMsg( lpMsg )
 {
   m_uiRcID = 0;   
   m_uiTtlID = uiTtlID;
 }

TExcept::TExcept( const TExcept& rE )
 {
   *this = rE;
 }

TExcept& TExcept::operator=( const TExcept& rE )
 {
   if( &rE != this )
	{
      m_bShDel = false;
	  m_uiRcID = rE.m_uiRcID;
	  m_uiTtlID = rE.m_uiTtlID;
	  m_cstrMsg = rE.m_cstrMsg;
	}

   return *this;
 }

TExcept::~TExcept()
 {
 }

void TExcept::Delete()
 {
   if( m_bShDel == true ) 
	{
	  m_bShDel = false;
	  delete this;
	}
 }

void TExcept::ReportExcept()
 {
   CString cstrHdr, cstrMsg;

   TBWCCLocalozeRC local;
   local.Localize();
   //HINSTANCE hInstRc = AfxGetResourceHandle();   
   //AfxSetResourceHandle( GetDLL_BWCC_ServisModule() );
   
	try {
     if( !cstrHdr.LoadString(m_uiTtlID) || !cstrMsg.LoadString(m_uiRcID) )
	  {
	    //AfxSetResourceHandle( hInstRc );
		local.Restore();
	    if( !cstrHdr.LoadString(m_uiTtlID) || !cstrMsg.LoadString(m_uiRcID) )
	      throw TExcept( FALSE );
	  }
	 }
	catch(...)
	 {
	   AfxMessageBox( "Произошло двойное исключение" );		

	   //AfxSetResourceHandle( hInstRc );
	   local.Restore();
	   return;
	 }

	::MessageBox( NULL, (LPCSTR)cstrMsg, (LPCSTR)cstrHdr, MB_OK | MB_ICONSTOP );

     //AfxSetResourceHandle( hInstRc );   
 }


