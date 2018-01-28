#if !defined(_CFG_)
#define _CFG_

#include "stdafx.h"
#include "autorc.hpp"


#define GLOBAL_CONFIG_FILENAME "net.cfg"

class TExceptCfgNotFound {};
class TExceptCfgBadCRC {};

struct TOptData
 {
   //*********** Protect ***********//   
   float m_fDiv;//Left:Right

   int m_iActivePane;
   int m_iFullMode;

   int m_iClmSize1, m_iClmSize2;

   int m_iBaud;
   int m_iStops;
   int m_iCOMPort;
   int m_iUsePoly;

   int m_bShowCPS;
   int m_bShowErr;

   DWORD m_dwTimeOutBrokenStop;

   DWORD dwTOReqvest;
   DWORD dwTOBroken;
   DWORD dwTOWaitLARead;
   DWORD dwTOWaitLA;

   //*********** User global interface ***********//   
   bool m_bSaveMainFramePosOnExit;
   bool m_bRestoreMainFrame;
 };

struct TScreenData
 {
   int m_iScreenX;
   int m_iScreenY;
   WINDOWPLACEMENT m_wpFrame;
 };

struct TMFOptions
 {

 friend CArchive& AFXAPI operator<<( CArchive& rA, TMFOptions& );
 friend CArchive& AFXAPI operator>>( CArchive& rA, TMFOptions& );

   TMFOptions();

   TMFOptions( const  TMFOptions& rOpt )
	{
	  operator=( rOpt );
	}

   TMFOptions& operator=( const TMFOptions& rOpt )
	{
      d = rOpt.d;
	  s = rOpt.s;

	  c_strPath1 = rOpt.c_strPath1;
	  c_strPath2 = rOpt.c_strPath2;

	  lfTtl = rOpt.lfTtl;
	  lfStatus = rOpt.lfStatus;
	  lfList = rOpt.lfList;

	  csSelMask1 = rOpt.csSelMask1;
	  csSelMask2 = rOpt.csSelMask2;

	  return *this;
	}

   bool CheckCRC() { return dwCRC == CalcCRC(); }//need after loading

   //*********** Global options ***********//
   TOptData d;
   CString c_strPath1, c_strPath2;

   LOGFONT lfTtl;  
   LOGFONT lfStatus;   
   LOGFONT lfList;
   
   CString csSelMask1, csSelMask2;

   //*********** MainFrame data ***********//
   TScreenData s;

private:
  DWORD dwCRC;

  DWORD CalcCRC();
 };


inline CArchive& AFXAPI operator<<( CArchive& rA, TMFOptions& rOpt )
 {
   rA.Write( &rOpt.d, sizeof(TOptData) );
   rA << rOpt.CalcCRC();
   rA.Write( &rOpt.s, sizeof(TScreenData) );
   rA << rOpt.c_strPath1 << rOpt.c_strPath2 << rOpt.csSelMask1 << rOpt.csSelMask2;

   rA.Write( &rOpt.lfTtl, sizeof(LOGFONT) );
   rA.Write( &rOpt.lfStatus, sizeof(LOGFONT) );
   rA.Write( &rOpt.lfList, sizeof(LOGFONT) );         

   return rA;
 }

inline CArchive& AFXAPI operator>>( CArchive& rA, TMFOptions& rOpt )
 {   
   rA.Read( &rOpt.d, sizeof(TOptData) );
   rA >> rOpt.dwCRC;
   rA.Read( &rOpt.s, sizeof(TScreenData) );
   rA >> rOpt.c_strPath1 >> rOpt.c_strPath2 >> rOpt.csSelMask1 >> rOpt.csSelMask2;

   rA.Read( &rOpt.lfTtl, sizeof(LOGFONT) );
   rA.Read( &rOpt.lfStatus, sizeof(LOGFONT) );
   rA.Read( &rOpt.lfList, sizeof(LOGFONT) );         
   	     
   return rA;
 }


#endif

