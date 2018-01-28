#include "drv2dlg.hpp"
#include "net.hpp"


static void KillSpaces( LPCSTR lpSource, LPSTR lpDst )
 {
   for( int i = strlen(lpSource); i; i--, lpSource++ )
	 if( *lpSource != ' ' ) *lpDst++ = *lpSource;

   *lpDst = 0;
 }


void TDriveDlg::DoDataExchange( CDataExchange* pDX )
 {
   CDialog::DoDataExchange( pDX );

   CString str( pB );
   DDX_Text( pDX, IDC_ED_DRIVE, str );
   strncpy( pB, (LPCSTR)str, 3 ); pB[2] = 0;
   DDV_MaxChars( pDX, pB, 1 );
   *pB = toupper( *pB );
   if( *pB < 'A' || *pB > 'Z' )
	{
	  ((TNetApp*)AfxGetApp())->bwcc.BWCCMessageBox( 
	     GetParent()->m_hWnd, 
		 "Неправильное устройство", 
		 "Ошибка", MB_OK | MB_ICONEXCLAMATION );
	  
	  HWND hw1 = ::GetDlgItem( m_hWnd, IDC_ED_DRIVE );
	  PostMessage( WM_NEXTDLGCTL, (WPARAM)hw1, 1 );	        

	  pDX->Fail();	  	  	
	}
 }

BOOL TDriveDlg::OnInitDialog()
 {
   CDialog::OnInitDialog();
   
   CString str;
   GetDlgItemText( IDC_TXT_DRIVE, str );
   str.SetAt( str.Find('#'), *pB );
   SetDlgItemText( IDC_TXT_DRIVE, str );

   m_Icon.SubclassDlgItem( IDC_MYICON, this );
   HICON hic = m_Icon.SetIcon( ::LoadIcon(NULL, IDI_ASTERISK) );
   if( hic ) DestroyIcon( hic );

   return FALSE;
 }

void TSelectDialog::DoDataExchange( CDataExchange* pDX )
 {
   CDialog::DoDataExchange( pDX );

   DDX_Text( pDX, IDC_ED_MASK, m_csMask );
   DDV_MaxChars( pDX, m_csMask, 260 );

   if( !pDX->m_bSaveAndValidate ) return;

   KillSpaces( m_csMask, (LPSTR)(LPCSTR)m_csMask );  
   
   char drive[ _MAX_DRIVE ];
   char dir[ _MAX_DIR ];
   char fname[ _MAX_FNAME ];
   char ext[ _MAX_EXT ];

   _splitpath( (LPCSTR)m_csMask, drive, dir, fname, ext );
   

   if( *drive || *dir || (!*fname && !*ext) )
	{
	  ((TNetApp*)AfxGetApp())->bwcc.BWCCMessageBox( 
	     GetParent()->m_hWnd, 
		 "Недопустимый формат маски", 
		 "Ошибка", MB_OK | MB_ICONEXCLAMATION );
	  
	  HWND hw1 = ::GetDlgItem( m_hWnd, IDC_ED_MASK );
	  PostMessage( WM_NEXTDLGCTL, (WPARAM)hw1, 1 );	        

	  pDX->Fail();	  	  	
	}
 }

BOOL TSelectDialog::OnInitDialog()
 {
   CDialog::OnInitDialog();

   SetDlgItemText( IDC_TXT_MASK, m_csHint );
   CenterWindow( m_mPar );

   return FALSE;
 }

TMaskTemplate::~TMaskTemplate()
 {
   m_csName.ReleaseBuffer();
   m_csExt.ReleaseBuffer();
 }

TMaskTemplate::TMaskTemplate( LPCSTR lp )
 {
   m_lpName = m_csName.GetBuffer( 257 );
   m_lpExt = m_csExt.GetBuffer( 257 );
   memset( m_lpName, 0, 257 );
   memset( m_lpExt, 0, 257 );

   char drive[ _MAX_DRIVE ];
   char dir[ _MAX_DIR ];   

   _splitpath( lp, drive, dir, m_lpName, m_lpExt );   
 }

bool TMaskTemplate::Check( LPCSTR lp )
 {
   char drive[ _MAX_DRIVE ];
   char dir[ _MAX_DIR ];
   char fname[ _MAX_FNAME ];
   char ext[ _MAX_EXT ];

   _splitpath( lp, drive, dir, fname, ext );
   if( !*ext ) strcpy( ext, "." );   

   return CmpTmpl( m_lpName, fname ) == true && 
	      CmpTmpl( m_lpExt, ext ) == true;
 }

bool TMaskTemplate::CmpTmpl( LPSTR lpMask, LPSTR lpTst )
 {
   if( !*lpMask ) return true;

   for( ; *lpMask; lpTst++, lpMask++ )
	{
      if( *lpMask == '*' ) return true;
	  if( !*lpTst ) return false;
	  if( *lpMask == '?' ) continue;
	  if( *lpMask != *lpTst ) return false;
	}

   return !bool( *lpTst );
 }
