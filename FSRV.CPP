#include "fsrv.hpp"

#include "net.hpp"
#include "RC\net.rh"

#include "..\\RUDAKOW\\SERVIS\\pathex.hpp"

#include <errno.h>
//#include <direct.h>
#include <algorithm>
#include <typeinfo.h>




TFileRecord::TFileRecord( _finddata_t& rDta )
 {
   m_flSelect = false;

   m_csName = rDta.name;
   attrib = rDta.attrib;
   size = rDta.size;
   time_write = rDta.time_write;

   if( rDta.attrib & _A_SUBDIR ) m_csName.MakeUpper();
   else m_csName.MakeLower();
 }


TUserOfDrive::~TUserOfDrive()
 {
   if( m_bExist == true ) *m_pDrv - this;	
   CloseNotify();
 }

void TUserOfDrive::CloseNotify()
 {
   if( m_hNotify &&  m_hNotify != INVALID_HANDLE_VALUE )
	{
      FindCloseChangeNotification( m_hNotify );
	  m_hNotify = NULL;
	}
 }

void TUserOfDrive::OpenNotify()
 {
   if( m_pDrv && m_pDrv->IsRemote() == true )
	{
      m_hNotify = INVALID_HANDLE_VALUE;
	  return;
	}

   CloseNotify();

   if( m_strCurPath.IsEmpty() ) return;

   m_hNotify = FindFirstChangeNotification( 
	  (LPSTR)(LPCSTR)m_strCurPath, 
	  FALSE,
	  FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
	  FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
	  FILE_NOTIFY_CHANGE_LAST_WRITE 
	 );   
 }

bool TUserOfDrive::Notify( TNotifyCode nCode )
 {
   if( nCode == NC_Removed && IsCallingOutside() == false ) 
	{
      m_bExist = false; m_pDrv = NULL;
	  CloseNotify();
	}

   return NotifyUser( nCode );   
 }

void TUserOfDrive::ChangeDrive( TDrive* pDrv )
 {
   if( m_bExist == true ) *m_pDrv - this;

   m_pDrv = pDrv;
   m_bExist = true;
   *m_pDrv + this;

   m_strCurPath = pDrv->GetPath();
   OpenNotify();
 }

void TUserOfDrive::ChangePath( LPCSTR lp )
 {    
    char drive[ _MAX_DRIVE ];
    char dir[ _MAX_DIR ];
    char fname[ _MAX_FNAME ];
    char ext[ _MAX_EXT ];

	_splitpath( lp, drive, dir, fname, ext );
    if( toupper(int(*drive)) != toupper(int(m_strCurPath[0])) )
	  throw TCannotChangeDrive();

	CloseNotify();
	m_strCurPath = lp;
	OpenNotify();
 }

void TUserOfDrive::UpdateLastAcess()
 {
   CheckDrive();
   if( m_pDrv ) m_pDrv->ChangePath( m_strCurPath );
 }

void TUserOfDrive::Directory( vector<TFileRecord*>& rV )
 {
   CheckDrive();
   if( m_pDrv ) m_pDrv->Directory( *this, rV );
 }

void TUserOfDrive::Copy( TUserOfDrive& dst, 
						 vector<TFileRecord*>* rV, 
						 bool bSubDir, TNotifyInstance* pNotify )
 {
   CheckDrive();
   m_pDrv->Copy( *this, dst, rV, bSubDir, false, pNotify );	
 }

void TUserOfDrive::Move( TUserOfDrive& dst, vector<TFileRecord*>* rV, 
						  bool bSubDir, TNotifyInstance* pNotify )
 {
   CheckDrive();
   m_pDrv->Copy( *this, dst, rV, bSubDir, true, pNotify );	
 }

void TUserOfDrive::Erase( vector<TFileRecord*>* rV, bool bSubDir, TNotifyInstance* pNotify )
 {
   CheckDrive();
   m_pDrv->Erase( *this, rV, bSubDir, pNotify ); 	
 }

void TUserOfDrive::CheckDrive()
 {
   if( !m_pDrv ) throw TDriveNotExist();
 }

bool TDrive::IsRemoved()
 {
   CSingleLock lock( &m_mtxSHR, TRUE );
   return m_bSheduledRemove;
 }

void TDrive::SheduleRemove()
 {
   CSingleLock lock( &m_mtxSHR, TRUE );
   m_bSheduledRemove = true;
 }

TDrive& TDrive::operator+( TUserOfDrive* pUs )
 {
   if( pUs->GetDrive() != this ) throw TBadUseDrive();  

   if( find(m_lstUsers.begin(), m_lstUsers.end(), pUs) != m_lstUsers.end() )
	 throw TDuplicateUser();

   m_lstUsers.push_back( pUs );

   return *this;
 }

TDrive& TDrive::operator-( TUserOfDrive* pUs )
 {
   m_lstUsers.remove( pUs );

   return *this;
 }

void TDrive::ChangePath( LPCSTR lp )
 {    
    if( m_strLastAcessPath.IsEmpty() || 
	    toupper(int(*lp)) != toupper(int(m_strLastAcessPath[0])) 
	  )
	  throw TCannotChangeDrive();

	m_strLastAcessPath = lp;
 }

bool TDrive::CmpDrive( TDrive& rD )
 {
   return rD.IsRemote() == IsRemote() && rD.m_bDrive == m_bDrive &&
          rD.m_uiType == m_uiType;
 }

TDrive::~TDrive()
 {
   list<TUserOfDrive*>::iterator itSta( m_lstUsers.begin() );
   list<TUserOfDrive*>::iterator itEnd( m_lstUsers.end() );

   for( ; itSta != itEnd; ++itSta )
	(*itSta)->Notify( TUserOfDrive::NC_Removed );

   m_lstUsers.clear();
 }


class TDlgReplace: public CDialog
 {
public:
 TDlgReplace( CWnd *pPar, char *pStr ): 
	m_str(pStr), CDialog( IDD_DLG_REPLACE, pPar )
  {
  }

  virtual BOOL OnInitDialog();
  //virtual void DoDataExchange( CDataExchange* pDX );

  afx_msg void OnYes();
  afx_msg void OnNo();
  afx_msg void OnAll();

private:  
  CString m_str;

  DECLARE_MESSAGE_MAP()
 };

BEGIN_MESSAGE_MAP( TDlgReplace, CDialog )	
  ON_COMMAND( IDYES, OnYes )  
  ON_COMMAND( IDNO, OnNo )  
  ON_COMMAND( IDC_BTN_ALL, OnAll )  
END_MESSAGE_MAP()

void TDlgReplace::OnYes()
 {
   EndDialog( IDYES );
 }

void TDlgReplace::OnNo()
 {
   EndDialog( IDNO );
 }

void TDlgReplace::OnAll()
 {
   EndDialog( IDC_BTN_ALL );
 }


BOOL TDlgReplace::OnInitDialog()
 {
   CDialog::OnInitDialog();
   
   SetDlgItemText( IDC_TXT_MSG, m_str );

   return FALSE;
 }



void TDrive::CopyFile( TUserOfDrive& src, TUserOfDrive& dst, 
					   LPCSTR lpName, TNotifyInstance* pNotify
					  )
					  
 {   
   TDrive *pDrvSrc = src.GetDrive();
   TDrive *pDrvDst = dst.GetDrive();

   const DWORD dwBufSize = min( pDrvSrc->GetBuffSize(), pDrvDst->GetBuffSize() );

   CString csSrc( src.GetPath() );
   if( csSrc[csSrc.GetLength() - 1] != '\\' ) csSrc += "\\";
   csSrc += lpName;

   CString csDst( dst.GetPath() );
   if( csDst[csDst.GetLength() - 1] != '\\' ) csDst += "\\";
   csDst += lpName;

   TNotifyDataCopy nInst;
   nInst.m_lpFromShortPath = (LPSTR)(LPCSTR)csSrc;
   nInst.m_lpToFullPath = (LPSTR)(LPCSTR)csDst;
   nInst.m_bFlStartOp = true;
   nInst.m_pSrc = pDrvSrc;
   nInst.m_pDst = pDrvDst;
   


   TAutoMemoryRc<TVirtualFile*> arcFIn( 
      pDrvSrc->OpenFile(csSrc, CFile::modeRead|CFile::shareDenyWrite) );

   TAutoMemoryRc<TVirtualFile*> arcFOut( 
	 pDrvDst->OpenFile(csDst, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive) );

   if( pNotify->m_bFlAskReplace == true )
	{
	  if( arcFOut()->CheckExist() == true )
	   {
	     CString csMsg; 
		 csMsg.Format( "Файл: '%s' уже есть.\nПереписать ?", (LPCSTR)csDst );

         TDlgReplace dlg( pNotify->GetNotifyWnd(), (LPSTR)(LPCSTR)csMsg );
		 int iRes = dlg.DoModal();
		 
		 if( iRes == IDC_BTN_ALL )pNotify->m_bFlAskReplace = false;
		 else if( iRes != IDYES ) return;
	   }
	}

   arcFIn()->MakeOpen();
   arcFOut()->MakeOpen();

   TAutoMemoryRc<BYTE*> arcBuf( new BYTE[dwBufSize] );    

   nInst.m_ulSize = arcFIn()->GetSize();
   nInst.m_bFlOpComplete = false;
   pNotify->Notinfy( &nInst );
   nInst.m_bFlStartOp = false;

   DWORD dwRdnd = 0;
   pNotify->IsCanceled();
   while( 1 )
	{
      DWORD dwRead = 0;	  
	  try {
	    dwRead = arcFIn()->Read( arcBuf(), dwBufSize, pNotify );
	   }
	  catch( CFileException *pE )
	   {
	     if( pE->m_cause == CFileException::endOfFile )
		   pE->Delete();
		 else 
		  {
		    try {
			  arcFOut()->Close();
			 }
			catch( CFileException *pE )
			 {
			   pE->Delete();
			 }
			try {
			  pNotify->IsCanceled();
			  bool bKey = pNotify->Enable( false );
              EraseFile( dst, (LPSTR)(LPCSTR)csDst, pNotify );
			  pNotify->Enable( bKey );
			 }
			catch( CFileException *pE )
			 {
			   pE->Delete();
			 }
		   
		    throw pE;		  		  
		  }
	   }

	  //if( pNotify )
	   {
         nInst.m_ulSize = dwRdnd + dwRead/2;
		 pNotify->Notinfy( &nInst );
	   }

	  if( dwRead < 1 ) break;
	  pNotify->IsCanceled();

	  arcFOut()->Write( arcBuf(), dwRead, pNotify );
	  pNotify->IsCanceled();

	  dwRdnd += dwRead;
	  //if( pNotify )
	   {
         nInst.m_ulSize = dwRdnd; 
		 pNotify->Notinfy( &nInst );
	   }
	}

   arcFIn()->Close();
   arcFOut()->Close();

   //TAutoMemoryRc<TSecurity*> arcSecurity( arcFIn()->GetSecurityAndAtr() );
   //arcFOut()->SetSecurity( SetSecurityAndAtr() );

   TAutoMemoryRc<TSecurity*> arcSecurity( pDrvSrc->GetSecurityAndAtr(csSrc) );
   pDrvDst->SetSecurityAndAtr( csDst, arcSecurity() );

   nInst.m_bFlOpComplete = true;
   pNotify->Notinfy( &nInst );
 }

void TDrive::EraseFile( TUserOfDrive& src, LPSTR lpName, 
					    TNotifyInstance* pNotify
					  )
					 
 {
   TDrive *pDrvSrc = src.GetDrive();

   CString csSrc( src.GetPath() );
   if( csSrc[csSrc.GetLength() - 1] != '\\' ) csSrc += "\\";
   csSrc += lpName;

   TNotifyDataCopy nInst;
   nInst.m_lpFromShortPath = (LPSTR)(LPCSTR)csSrc;
   nInst.m_lpToFullPath = NULL;
   nInst.m_bFlStartOp = true;
   nInst.m_pSrc = pDrvSrc;
   nInst.m_pDst = NULL;
   
   pNotify->IsCanceled();

   if( pNotify->m_bFlAskErase == true )
	{
	  CString csMsg; 
	  csMsg.Format( "'%s'\nСтереть ?", (LPCSTR)csSrc );

      TDlgReplace dlg( pNotify->GetNotifyWnd(), (LPSTR)(LPCSTR)csMsg );
	  int iRes = dlg.DoModal();
		 
	  if( iRes == IDC_BTN_ALL )pNotify->m_bFlAskErase = false;
	  else if( iRes != IDYES ) return;	 
	}
   
   nInst.m_ulSize = 0;
   nInst.m_bFlOpComplete = false;
   pNotify->Notinfy( &nInst );
   nInst.m_bFlStartOp = false;

   TAutoMemoryRc<TVirtualFile*> arcFIn( 
      pDrvSrc->OpenFile(csSrc, CFile::modeRead|CFile::shareDenyWrite) );

   arcFIn()->Erase( csSrc );

   pNotify->IsCanceled();
   nInst.m_bFlOpComplete = true;
   pNotify->Notinfy( &nInst );
 }

static void PassException( CException *pE )
 {
   CString cs; 
   LPSTR lp = cs.GetBuffer( 1024 * 2 );
   if( pE )
	{
      pE->GetErrorMessage( lp, 1024*2 - 1 );			 
      pE->Delete();
	}
   else sprintf( lp, "Общая ошибка" );

   cs.ReleaseBuffer();
   cs.Format( "%s - продолжить ?", (LPSTR)(LPCSTR)cs );
   int iRes = AfxMessageBox( (LPSTR)(LPCSTR)cs, MB_YESNO|MB_ICONEXCLAMATION );
   if( iRes != IDYES ) throw TUserCancel();
 }

void TDrive::Copy( TUserOfDrive& src, TUserOfDrive& dst, 
			  	   vector<TFileRecord*>* pRec, 
			   	   bool bSubDir, bool bMove, TNotifyInstance* pNotify 
				 )
 {
   pNotify->IsCanceled();

   if( pRec )
	{
      vector<TFileRecord*>::iterator itSta( pRec->begin() );
      vector<TFileRecord*>::iterator itEnd( pRec->end() );

      for( ; itSta != itEnd; ++itSta )
	   {
         pNotify->IsCanceled();

		 try {
           if( (*itSta)->m_flSelect == false ) continue;
		   if( ((*itSta)->attrib & _A_SUBDIR) && bSubDir == true )
			{
		      int iKeyPos = src.m_strCurPath.GetLength();
			  int iKeyPos2 = dst.m_strCurPath.GetLength();

			  if( src.m_strCurPath[iKeyPos - 1] != '\\' )
			    src.m_strCurPath += "\\";
		      src.m_strCurPath += (*itSta)->m_csName;

			  if( dst.m_strCurPath[iKeyPos2 - 1] != '\\' )
			    dst.m_strCurPath += "\\";
		      dst.m_strCurPath += (*itSta)->m_csName;

			  try {
			    dst.GetDrive()->MkDir( dst.m_strCurPath );
			    TAutoMemoryRc<TSecurity*> arcSecurity( 
			      src.GetDrive()->GetSecurityAndAtr(src.m_strCurPath) );
                dst.GetDrive()->SetSecurityAndAtr( 
 			      dst.m_strCurPath, arcSecurity() );

			    TDrive::Copy( src, dst, NULL, bSubDir, bMove, pNotify );

			    if( bMove == true )
			      src.GetDrive()->RmEmptyDir( src.m_strCurPath, false );			
			  }
			  catch( CException *pE )
			   {
                 LPSTR lp = src.m_strCurPath.GetBufferSetLength( iKeyPos );
			     src.m_strCurPath.ReleaseBuffer( iKeyPos );

			     dst.m_strCurPath.GetBufferSetLength( iKeyPos2 );
			     dst.m_strCurPath.ReleaseBuffer( iKeyPos2 );

				 throw pE;
			   }
            
              LPSTR lp = src.m_strCurPath.GetBufferSetLength( iKeyPos );
			  src.m_strCurPath.ReleaseBuffer( iKeyPos );

			  dst.m_strCurPath.GetBufferSetLength( iKeyPos2 );
			  dst.m_strCurPath.ReleaseBuffer( iKeyPos2 );
			}
		   else
			{
		      CopyFile( src, dst, (*itSta)->m_csName, pNotify );
			  pNotify->IsCanceled();
              if( bMove == true )
			   {
			     bool bKey = pNotify->Enable( false );
			     EraseFile( src, (LPSTR)(LPCSTR)(*itSta)->m_csName, pNotify );
			     pNotify->Enable( bKey );
			   }
			}
		  
		   (*itSta)->m_flSelect = false;		 
		 }
		 catch( CException* pE )
		  {
		    PassException( pE );		    
		  }
		 /*catch(...)
		  {
            PassException( NULL );
		  }*/
	   }
	}
   else
	{
      TAutoMemoryRc<TVirtualSearch*> arcSh( 
	    src.GetDrive()->OpenSearch((LPCSTR)src.GetPath()) );

	  
      _finddata_t *pFnd = arcSh()->GetFirst();
	  while( pFnd )
	   {	     
         try {
           pNotify->IsCanceled();

		   if( (pFnd->attrib & _A_SUBDIR) && bSubDir == true )
			{
              if( strcmp(pFnd->name, ".") && strcmp(pFnd->name, "..") )
			   {			  
		         int iKeyPos = src.m_strCurPath.GetLength();
			     int iKeyPos2 = dst.m_strCurPath.GetLength();

			     if( src.m_strCurPath[iKeyPos - 1] != '\\' )
			       src.m_strCurPath += "\\";
		         src.m_strCurPath += pFnd->name;

			     if( dst.m_strCurPath[iKeyPos2 - 1] != '\\' )
			       dst.m_strCurPath += "\\";
		         dst.m_strCurPath += pFnd->name;

				 try {
			       dst.GetDrive()->MkDir( dst.m_strCurPath );
			       TAutoMemoryRc<TSecurity*> arcSecurity( 
			         src.GetDrive()->GetSecurityAndAtr(src.m_strCurPath) );
                   dst.GetDrive()->SetSecurityAndAtr( 
			         dst.m_strCurPath, arcSecurity() );

			       TDrive::Copy( src, dst, NULL, bSubDir, bMove, pNotify );                           

			       if( bMove == true )
			         src.GetDrive()->RmEmptyDir( src.m_strCurPath, false );			
				 }
				 catch( CException *pE )
				  {
                    LPSTR lp = src.m_strCurPath.GetBufferSetLength( iKeyPos );
			        src.m_strCurPath.ReleaseBuffer( iKeyPos );

			        dst.m_strCurPath.GetBufferSetLength( iKeyPos2 );
			        dst.m_strCurPath.ReleaseBuffer( iKeyPos2 );

					throw pE;
				  }


			     LPSTR lp = src.m_strCurPath.GetBufferSetLength( iKeyPos );
			     src.m_strCurPath.ReleaseBuffer( iKeyPos );

			     dst.m_strCurPath.GetBufferSetLength( iKeyPos2 );
			     dst.m_strCurPath.ReleaseBuffer( iKeyPos2 );
			   }
			}
		   else
			{
		      CopyFile( src, dst, pFnd->name, pNotify );
			  pNotify->IsCanceled();
              if( bMove == true )
			   {
			     bool bKey = pNotify->Enable( false );
			     EraseFile( src, pFnd->name, pNotify );
			     pNotify->Enable( bKey );
			   }
			}
		   }
	     catch( CException *pE )
		  {
            PassException( pE );
		  }
         /*catch(...)
		  {
            PassException( NULL );
		  }*/		 

		 pFnd = arcSh()->Next();		 
	   };	  
	  if( arcSh()->GetErr() != ENOENT ) 	   
	    CFileException::ThrowOsError( arcSh()->GetErr() );		  
	}
 }

void TDrive::Erase( TUserOfDrive& src, vector<TFileRecord*>* pRec, 
				    bool bSubDir, TNotifyInstance* pNotify )
 {
   pNotify->IsCanceled();

   if( pRec )
	{
      vector<TFileRecord*>::iterator itSta( pRec->begin() );
      vector<TFileRecord*>::iterator itEnd( pRec->end() );
  	  
      for( ; itSta != itEnd; ++itSta )
	   {
		 try {

           pNotify->IsCanceled();

           if( (*itSta)->m_flSelect == false ) continue;
		   if( ((*itSta)->attrib & _A_SUBDIR) && bSubDir == true )
			{
		      int iKeyPos = src.m_strCurPath.GetLength();

			  if( src.m_strCurPath[iKeyPos - 1] != '\\' )
			    src.m_strCurPath += "\\";
		      src.m_strCurPath += (*itSta)->m_csName;						

			  try {
			    TDrive::Erase( src, NULL, bSubDir, pNotify );			
			
			    src.GetDrive()->RmEmptyDir( src.m_strCurPath, pNotify->m_bFlAskErase );
			   }
			  catch( CException *pE )
			   {
                 LPSTR lp = src.m_strCurPath.GetBufferSetLength( iKeyPos );
			     src.m_strCurPath.ReleaseBuffer( iKeyPos );			

				 throw pE;
			   }
            
              LPSTR lp = src.m_strCurPath.GetBufferSetLength( iKeyPos );
			  src.m_strCurPath.ReleaseBuffer( iKeyPos );			
			}
		   else
			{		    
		      EraseFile( src, (LPSTR)(LPCSTR)(*itSta)->m_csName, pNotify );					  
			  //if( pNotify && pNotify->IsCanceled() == true ) return;            
			}
		  
		   (*itSta)->m_flSelect = false;

		   }
	      catch( CException *pE )
		   {
             PassException( pE );
		   }
		  /*catch(...)
		   {
             PassException( NULL );
		   }*/
	   }	   
	}
   else
	{
      TAutoMemoryRc<TVirtualSearch*> arcSh( 
	    src.GetDrive()->OpenSearch((LPCSTR)src.GetPath()) );
	  
      _finddata_t *pFnd = arcSh()->GetFirst();
	  while( pFnd )
	   {	     
		 try { 

           pNotify->IsCanceled();

		   if( (pFnd->attrib & _A_SUBDIR) && bSubDir == true )
			{
              if( strcmp(pFnd->name, ".") && strcmp(pFnd->name, "..") )
			   {			  
		         int iKeyPos = src.m_strCurPath.GetLength();
			
			     if( src.m_strCurPath[iKeyPos - 1] != '\\' )
			       src.m_strCurPath += "\\";	
				 src.m_strCurPath += pFnd->name;
			
				 try {
			       //TDrive::Copy( src, dst, NULL, bSubDir, bMove, pNotify );
			       TDrive::Erase( src, NULL, bSubDir, pNotify );
			       src.GetDrive()->RmEmptyDir( src.m_strCurPath, pNotify->m_bFlAskErase );
				 }
				 catch( CException *pE )
				  {
                    LPSTR lp = src.m_strCurPath.GetBufferSetLength( iKeyPos );
			        src.m_strCurPath.ReleaseBuffer( iKeyPos );			   			   

					throw pE;
				  }
            
                 LPSTR lp = src.m_strCurPath.GetBufferSetLength( iKeyPos );
			     src.m_strCurPath.ReleaseBuffer( iKeyPos );			   			   
			   }
			}
		   else
			{
		      //CopyFile( src, dst, pFnd->name, pNotify );			
		      EraseFile( src, pFnd->name, pNotify );
			}         
		 }
	    catch( CException *pE )
		 {
           PassException( pE );
		 }
		/*catch(...)
		 {
           PassException( NULL );
		 }*/

		pFnd = arcSh()->Next();

	   };
	   	  
	    if( arcSh()->GetErr() != ENOENT ) 	   
	      CFileException::ThrowOsError( arcSh()->GetErr() );		   
	}

 }

TVirtualSearch::TVirtualSearch( LPCSTR lpPath )
 {
   m_dwLastErr = 0;
   m_csPath = lpPath;
   if( m_csPath[m_csPath.GetLength() - 1] != '\\' ) m_csPath += "\\";
   m_csPath += "*.*";
 }

TVirtualSearch::~TVirtualSearch()
 {
 }



TSecurity::~TSecurity()
 {
   if( pSec ) 
	{
	  delete[] pSec;
	  pSec = NULL;
	}
 }


TVirtualFile::~TVirtualFile()
 {   
 }


TNotifyInstance::~TNotifyInstance()
 {
 }

void TNotifyInstance::IsCanceled()
 {
   if( ::WaitForSingleObject( m_rEvCancel, 0 ) != WAIT_TIMEOUT )
	 throw TUserCancel();
 }
