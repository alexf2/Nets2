#define TBWCC_DLL
#include "bwcc.hpp"

#define TEXCEPT_DLL
#include "texcept.hpp"

#define SERVIS_DLL
#include "servis.hpp"


#include "bwcc.rh"

  
TBWCC::TBWCC( LPCSTR lpPath )
 {
   m_Inst = NULL;
   bwccIntlInit = NULL; bwccRegister = NULL; bwccIntlTerm = NULL;
   if( !lpPath ) str = "bwcc32.dll"; 
   else str = lpPath;
 }

TBWCC::~TBWCC()
 {

   if( bwccIntlTerm )
	{
      bwccIntlTerm();
	  bwccIntlTerm = 0;
	}
 
   if( m_Inst )
    {
	  AfxFreeLibrary( m_Inst );
	  m_Inst = NULL;
	}
 }

void TBWCC::EnableBWCC( HINSTANCE hApp, BOOL bFl )
 {
   if( !m_Inst && bFl == TRUE )
    {
	  m_Inst = AfxLoadLibrary( (LPCSTR)str );
	  if( !m_Inst ) throw TExcept( FALSE, ID_ERR_BWCCCANNTLOAD, ID_ERR_BWCC );

	  (FARPROC&)bwccIntlInit   = GetProcAddress( m_Inst, "BWCCIntlInit" );
	  (FARPROC&)bwccRegister   = GetProcAddress( m_Inst, "BWCCRegister" );
	  (FARPROC&)bwccIntlTerm   = GetProcAddress( m_Inst, "BWCCIntlTerm" );
	  (FARPROC&)BWCCMessageBox = GetProcAddress( m_Inst, "BWCCMessageBox" );

	  if( !bwccIntlInit || !bwccRegister || !bwccIntlTerm || !BWCCMessageBox )
	    throw TExcept( FALSE, ID_ERR_BWCCINVALID, ID_ERR_BWCC );

	  if( !bwccIntlInit(0) )
        throw TExcept( FALSE, ID_ERR_BWCCINIT, ID_ERR_BWCC ); 
	  if( !bwccRegister(hApp) )
	    throw TExcept( FALSE, ID_ERR_BWCCREG, ID_ERR_BWCC ); 
	}
   else if( m_Inst && bFl == FALSE )
	{
	  if( bwccIntlTerm )
	   {
	     bwccIntlTerm();
	     bwccIntlTerm = 0;
	   }
	  AfxFreeLibrary( m_Inst );
	  m_Inst = NULL;
	}
 }

TBWCCLocalozeRC::TBWCCLocalozeRC()
 {
   m_hinstOldRC = NULL;
   m_bInit = false;
 }

TBWCCLocalozeRC::~TBWCCLocalozeRC()
 {
   Restore();
 }

void TBWCCLocalozeRC::Localize()
 {
   Restore();

   m_hinstOldRC = AfxGetResourceHandle();   
   AfxSetResourceHandle( GetDLL_BWCC_ServisModule() );
   m_bInit = true;
 }

void TBWCCLocalozeRC::Restore()
 {
   if( m_bInit == true )
	{
      AfxSetResourceHandle( m_hinstOldRC );   
	  m_bInit = false;
	}
 }
 
//CWinApp cwDllBWCC( "bwcc_.dll" );
