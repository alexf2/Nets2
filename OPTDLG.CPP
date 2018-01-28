#include "optdlg.hpp"
#include "RC\net.rh"

#include <commctrl.h>


BEGIN_MESSAGE_MAP( TUsersDialog, TBWCCPropertySheet )  
  ON_WM_CREATE()
  ON_COMMAND( IDOK, OnOK )
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP( TPRopPageInfo, CPropertyPage )    
  ON_BN_CLICKED( IDC_RAD_COM1, OnCOM1 )
  ON_BN_CLICKED( IDC_RAD_COM2, OnCOM2 )
  ON_BN_CLICKED( IDC_RAD_COM3, OnCOM3 )
  ON_BN_CLICKED( IDC_RAD_COM4, OnCOM4 )
END_MESSAGE_MAP()


BEGIN_MESSAGE_MAP( TPRopPagePort, CPropertyPage )  
  ON_BN_CLICKED( IDC_RAD_COM1, OnCOM1 )
  ON_BN_CLICKED( IDC_RAD_COM2, OnCOM2 )
  ON_BN_CLICKED( IDC_RAD_COM3, OnCOM3 )
  ON_BN_CLICKED( IDC_RAD_COM4, OnCOM4 )
END_MESSAGE_MAP()


struct TBaudData
 {
   int m_iManemonic;
   char *cStr;
 };

LPCSTR GetBaudEq( int iMnem )
 {
   const int iNumbBPS = 20;

   static const TBaudData iBauds[iNumbBPS] =
	{
	 {BAUD_075,     "75 bps"},

	  {BAUD_110,    "110 bps"},
	  {BAUD_134_5,  "134.5 bps"},

	  {BAUD_150,    "150 bps"},
	  {BAUD_300,    "300 bps"},
	  {BAUD_600,    "600 bps"},
	  {BAUD_1200,   "1200 bps"},
	  {BAUD_1800,   "1800 bps"},
	  {BAUD_2400,   "2400 bps"},
	  {BAUD_4800,   "4800 bps"},
	  {BAUD_7200,   "7200 bps"},
	  {BAUD_9600,   "9600 bps"},
	  {BAUD_14400,  "14400 bps"},

	  {BAUD_19200,  "19200 bps"},
	  {BAUD_38400,  "38400 bps"},
	  {BAUD_56K,    "56K bps"},
	  {BAUD_57600,  "57600 bps"},

	  {BAUD_115200, "115200 bps"},
	  {BAUD_128K,   "128K bps"},
	  {BAUD_USER,   "Programmable baud rates available"},
	};

   for( int i = 0; i < iNumbBPS; i++ )
	 if( iBauds[i].m_iManemonic == iMnem ) return iBauds[i].cStr;

   return "USER bps";
 }


LPCSTR GetProvSubType( int iMnem )
 {
   const int iNumbTyps = 13;

   static const TBaudData iBauds[ iNumbTyps ] =
	{
	 {PST_FAX,             "FAX device"},	  
	 {PST_LAT,             "LAT protocol"},	  

	 {PST_MODEM,           "Modem device"},	  
	 {PST_NETWORK_BRIDGE,  "Unspecified network bridge"},	  
	 {PST_PARALLELPORT,    "Parallel port"},	  
	 {PST_RS232,           "RS-232 serial port"},	  
	 {PST_RS422,           "RS-422 port"},	  

	 {PST_RS423,           "RS-423 port"},	  
	 {PST_RS449,           "RS-449 port"},	  
	 {PST_SCANNER ,        "Scanner device"},	  
	 {PST_TCPIP_TELNET,    "TCP/IP Telnet® protocol"},	  

	 {PST_UNSPECIFIED,     "Unspecified"},	  
	 {PST_X25,             "X.25 standards"}
	};

   for( int i = 0; i < iNumbTyps; i++ )
	 if( iBauds[i].m_iManemonic == iMnem ) return iBauds[i].cStr;

   return "UNKNOWN";
 }

struct TBaudTable
 {
   int m_iMnem;
   int m_iId;
 };

const int iNumbBPS = 19;
const int iNumbStops = 3;
const int iNumbPorts = 4;



static const TBaudTable iPorts[ iNumbPorts ] =
 {
  {1,     IDC_RAD_COM1},
  {2,     IDC_RAD_COM2},
  {3,     IDC_RAD_COM3},
  {4,     IDC_RAD_COM4},
 };

static const TBaudTable iStops[ iNumbStops ] =
 {
  { STOPBITS_10,     IDC_RAD_B1 },
  { STOPBITS_15,     IDC_RAD_B15 },
  { STOPBITS_20,     IDC_RAD_B2 },
 };


static const TBaudTable iBauds[ iNumbBPS ] =
 {
  {BAUD_075,     IDC_RAD_75},

  {BAUD_110,    IDC_RAD_110},
  {BAUD_134_5,  IDC_RAD_134_5},

  {BAUD_150,    IDC_RAD_150},
  {BAUD_300,    IDC_RAD_300},
  {BAUD_600,    IDC_RAD_600},
  {BAUD_1200,   IDC_RAD_1200},
  {BAUD_1800,   IDC_RAD_1800},
  {BAUD_2400,   IDC_RAD_2400},
  {BAUD_4800,   IDC_RAD_4800},
  {BAUD_7200,   IDC_RAD_7200},
  {BAUD_9600,   IDC_RAD_9600},
  {BAUD_14400,  IDC_RAD_14400},

  {BAUD_19200,  IDC_RAD_19200},
  {BAUD_38400,  IDC_RAD_38400},
  {BAUD_56K,    IDC_RAD_56K},
  {BAUD_57600,  IDC_RAD_57600},

  {BAUD_115200, IDC_RAD_115200},
  {BAUD_128K,   IDC_RAD_128K}
 };


static int BaudId_ForMnem( int iMnem )
 {
   for( int i = 0; i < iNumbBPS; ++i )
	 if( iBauds[i].m_iMnem == iMnem ) return iBauds[i].m_iId;

   return 0;
 }
static int BaudMnem_ForId( int iId )
 {
   for( int i = 0; i < iNumbBPS; ++i )
	 if( iBauds[i].m_iId == iId ) return iBauds[i].m_iMnem;

   return 0;
 }

static int PortId_ForMnem( int iMnem )
 {
   for( int i = 0; i < iNumbPorts; ++i )
	 if( iPorts[i].m_iMnem == iMnem ) return iPorts[i].m_iId;

   return 0;
 }
static int PortMnem_ForId( int iId )
 {
   for( int i = 0; i < iNumbPorts; ++i )
	 if( iPorts[i].m_iId == iId ) return iPorts[i].m_iMnem;

   return 0;
 }

static int StopId_ForMnem( int iMnem )
 {
   for( int i = 0; i < iNumbStops; ++i )
	 if( iStops[i].m_iMnem == iMnem ) return iStops[i].m_iId;

   return 0;
 }
static int StopMnem_ForId( int iId )
 {
   for( int i = 0; i < iNumbStops; ++i )
	 if( iStops[i].m_iId == iId ) return iStops[i].m_iMnem;

   return 0;
 }


TPRopPagePort::~TPRopPagePort()
 {
 }

BOOL TPRopPagePort::OnInitDialog()
 {
   CPropertyPage::OnInitDialog();   

   	 ::EnableWindow( ::GetDlgItem(m_hWnd, IDC_RAD_COM1),
                    m_CP[0].m_bAval == true
				  );
	 ::EnableWindow( ::GetDlgItem(m_hWnd, IDC_RAD_COM2),
                    m_CP[1].m_bAval == true
				  );
	 ::EnableWindow( ::GetDlgItem(m_hWnd, IDC_RAD_COM3),
                    m_CP[2].m_bAval == true
				  );
	 ::EnableWindow( ::GetDlgItem(m_hWnd, IDC_RAD_COM4),
                    m_CP[3].m_bAval == true
				  );	  

	  int iCOM = PortId_ForMnem( m_rOpt.d.m_iCOMPort );


      SendDlgItemMessage( BaudId_ForMnem(m_rOpt.d.m_iBaud), BM_SETCHECK, BST_CHECKED, 0 );
      SendDlgItemMessage( iCOM, BM_SETCHECK, BST_CHECKED, 0 );  
      SendDlgItemMessage( StopId_ForMnem(m_rOpt.d.m_iStops), BM_SETCHECK, BST_CHECKED, 0 );  
	  
	  if( iCOM == IDC_RAD_COM1 ) iCOM = 0;
	  else if( iCOM == IDC_RAD_COM2 ) iCOM = 1;
	  else if( iCOM == IDC_RAD_COM3 ) iCOM = 2;
	  else if( iCOM == IDC_RAD_COM4 ) iCOM = 3;
	    
	  ChangeCOMM( iCOM );	  

   
   return FALSE;
 }

static int FindChk( HWND hw, const TBaudTable* pTbl, int iSz )
 {
   for( int i = 0; i < iSz; i++ )
	 if( SendMessage(GetDlgItem(hw, pTbl[i].m_iId), BM_GETCHECK, 0, 0) != BST_UNCHECKED )
	   return pTbl[i].m_iId;

   return 0;
 }

int TPRopPagePort::FindEnabled( const TBaudTable* pT, int iSz, int iSta )
 {
   for( int i = iSta; i > -1; --i )
	 if( ::IsWindowEnabled(::GetDlgItem(m_hWnd, pT[i].m_iId)) )
	   return i;

   for( i = iSta; i < iSz; ++i )
	 if( ::IsWindowEnabled(::GetDlgItem(m_hWnd, pT[i].m_iId)) )
	   return i;

   return -1;
 }

int TPRopPagePort::CheckForDisabledCheck( const TBaudTable* pT, int iSz )
 {
   int iRes = -1;
   for( int i = 0; i < iSz; i++ )
	{
	  HWND hw = ::GetDlgItem( m_hWnd, pT[i].m_iId );
	  BOOL bEnbl = ::IsWindowEnabled( hw );
	  int iChk = ::SendMessage( hw, BM_GETCHECK, 0, 0 );

	  if( !bEnbl && iChk )
	   {
	     ::SendMessage( hw, BM_SETCHECK, BST_UNCHECKED, 0 );
		 int iEn = FindEnabled( pT, iSz, i );
		 if( iEn != -1 )
		  {
		    ::SendMessage( ::GetDlgItem(m_hWnd, pT[iEn].m_iId), BM_SETCHECK, BST_CHECKED, 0 );
			return iEn;
		  }
		 else return -1;
	   }
	  else if( bEnbl && iChk ) return i;
	}

   return -1;
 }////////

void TPRopPagePort::ChangeCOMM( int iCOMM )
 {
   if( m_CP[ iCOMM ].m_bAval == false ) 
	{
      int iRes = CheckForDisabledCheck( iPorts, iNumbPorts );
      if( iRes != iCOMM && iRes != -1 ) ChangeCOMM( iRes );
	  return;
	}

   ::EnableWindow( ::GetDlgItem(m_hWnd, IDC_RAD_B1),
                    m_CP[iCOMM].m_comm.wSettableStopParity & STOPBITS_10
				  );
   ::EnableWindow( ::GetDlgItem(m_hWnd, IDC_RAD_B15),
                    m_CP[iCOMM].m_comm.wSettableStopParity & STOPBITS_15
				  );
   ::EnableWindow( ::GetDlgItem(m_hWnd, IDC_RAD_B2),
                    m_CP[iCOMM].m_comm.wSettableStopParity & STOPBITS_20
				  );

   //iBauds[ iNumbBPS
   //int iInd = 0;
   for( int i = 0; i < iNumbBPS; ++i )	 
	 ::EnableWindow( ::GetDlgItem(m_hWnd, iBauds[i].m_iId),
                     iBauds[i].m_iMnem & m_CP[iCOMM].m_comm.dwSettableBaud             
				   );

   CheckForDisabledCheck( iBauds, iNumbBPS );
   CheckForDisabledCheck( iStops, iNumbStops );      
 }

void TPRopPagePort::OnCOM1()
 {
   ChangeCOMM( 0 );
 }

void TPRopPagePort::OnCOM2()
 {
   ChangeCOMM( 1 );
 }

void TPRopPagePort::OnCOM3()
 {
   ChangeCOMM( 2 );
 }

void TPRopPagePort::OnCOM4()
 {
   ChangeCOMM( 3 );
 }


void TPRopPagePort::DoDataExchange( CDataExchange* pDX )
 {
   CPropertyPage::DoDataExchange( pDX );

   if( !pDX->m_bSaveAndValidate )
	{

	}
   else
	{
	  m_rOpt.d.m_iBaud    = BaudMnem_ForId( FindChk(m_hWnd, iBauds, iNumbBPS) ); 
	  m_rOpt.d.m_iCOMPort = PortMnem_ForId( FindChk(m_hWnd, iPorts, iNumbPorts) ); 
	  m_rOpt.d.m_iStops   = StopMnem_ForId( FindChk(m_hWnd, iStops, iNumbStops) ); 	  
	}
 }


TPRopPageProtocol::~TPRopPageProtocol()
 {
 }

BOOL TPRopPageProtocol::OnInitDialog()
 {
   CPropertyPage::OnInitDialog();

   return FALSE;
 }

void TPRopPageProtocol::DoDataExchange( CDataExchange* pDX )
 {
   CPropertyPage::DoDataExchange( pDX );

   DDX_Check( pDX, IDC_CHECK_POLYNOM, m_rOpt.d.m_iUsePoly );

   DDX_Check( pDX, IDC_CHECK_CPS, m_rOpt.d.m_bShowCPS );
   DDX_Check( pDX, IDC_CHECK_ERR, m_rOpt.d.m_bShowErr );

   DDX_Text( pDX, IDC_EDIT_REQ, m_rOpt.d.dwTOReqvest );
   DDX_Text( pDX, IDC_EDIT_LAREP, m_rOpt.d.dwTOWaitLARead );
   DDX_Text( pDX, IDC_EDIT_LAWAIT, m_rOpt.d.dwTOWaitLA );
   DDX_Text( pDX, IDC_EDIT_BROKEN, m_rOpt.d.dwTOBroken );
   DDX_Text( pDX, IDC_EDIT_BROKENSTOP, m_rOpt.d.m_dwTimeOutBrokenStop );

   DDV_MinMaxDWord( pDX, m_rOpt.d.dwTOReqvest, 10, ULONG_MAX );
   DDV_MinMaxDWord( pDX, m_rOpt.d.dwTOWaitLARead, 10, ULONG_MAX );
   DDV_MinMaxDWord( pDX, m_rOpt.d.dwTOWaitLA, 100, ULONG_MAX );
   DDV_MinMaxDWord( pDX, m_rOpt.d.dwTOBroken, 100, ULONG_MAX );
   DDV_MinMaxDWord( pDX, m_rOpt.d.m_dwTimeOutBrokenStop, 500, ULONG_MAX );		
 }

typedef void TSubItemProc( LPCSTR, TCOMMProp*, HTREEITEM&, CTreeCtrl&, HTREEITEM );

struct TItemsPrepare
 {
   LPCSTR m_lpName;
   TSubItemProc *m_pProc;
 };

void TV_PacketVer( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;   
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %lu - Версия блока конфигурации", (LPSTR)lp, 
	           (DWORD)pCOM->m_comm.wPacketVersion  );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }

void TV_ServMask( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;   
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %lu - Битовая маска расширенных сервисов производителя", (LPSTR)lp, 
	           (DWORD)pCOM->m_comm.dwServiceMask  );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }

void TV_MaxTx( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;   
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %lu(bytes)%s - Максимальная длина выходного буфера драйвера", (LPSTR)lp, 
	           (DWORD)pCOM->m_comm.dwMaxTxQueue,
			   pCOM->m_comm.dwMaxTxQueue ? "":"(не определена)" );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }

void TV_MaxRx( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;   
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %lu(bytes)%s - Максимальная длина входного буфера драйвера", (LPSTR)lp, 
	           (DWORD)pCOM->m_comm.dwMaxRxQueue,
			   pCOM->m_comm.dwMaxRxQueue ? "":"(не определена)" );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }

void TV_MaxBaud( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;   
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %s - Максимальная скорость передачи данных", (LPSTR)lp, 
	           (LPSTR)GetBaudEq( pCOM->m_comm.dwMaxBaud )  );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }

void TV_ProvSubType( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;   
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %s - Тип микросхемы порта", (LPSTR)lp, 
	           (LPSTR)GetProvSubType( pCOM->m_comm.dwProvSubType )  );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }

struct TPropDef
 {
   int     m_iMnem;
   LPCSTR  m_lpDescr;
 };

const int iNumb_ProvCap = 10;
static TPropDef pdProvCap[ iNumb_ProvCap ] = 
 {
  { PCF_16BITMODE,      "Special 16-bit mode"},
  { PCF_DTRDSR,         "DTR (data-terminal-ready)/DSR (data-set-ready)"},
  { PCF_INTTIMEOUTS,    "Interval time-outs"},
  { PCF_PARITY_CHECK,   "Parity checking"},

  { PCF_RLSD,           "RLSD (receive-line-signal-detect)"},
  { PCF_RTSCTS,         "RTS (request-to-send)/CTS (clear-to-send)"},
  { PCF_SETXCHAR,       "Settable XON/XOFF"},
  { PCF_SPECIALCHARS,   "Special character support provided"},
  { PCF_TOTALTIMEOUTS,  "Total (elapsed) time-outs"},
  { PCF_XONXOFF,        "XON/XOFF flow control"}
 };

const int iNumb_SetPar = 7;
static TPropDef pdSetPar[ iNumb_SetPar ] = 
 {
  { SP_BAUD,           "Baud rate" },

   { SP_DATABITS,      "Data bits" },
   { SP_HANDSHAKING,   "Handshaking (flow control)" },
   { SP_PARITY,        "Parity" },
   { SP_PARITY_CHECK,  "Parity checking" },
   { SP_RLSD,          "RLSD (receive-line-signal-detect)" },
   { SP_STOPBITS,      "Stop bits" }     
 };

const int iNumb_SetData = 6;
static TPropDef pdSetData[ iNumb_SetData ] = 
 {
  { DATABITS_5,    "5 data bits" },

   { DATABITS_6,   "6 data bits" },
   { DATABITS_7,   "7 data bits" },
   { DATABITS_8,   "8 data bits" },
   { DATABITS_16,  "16 data bits" },
   { DATABITS_16X, "Special wide path through serial hardware lines" },   
 };

const int iNumb_SetBaud = 20;
static TPropDef pdSetBaud[ iNumb_SetBaud ] = 
 {
  	 {BAUD_075,     "75 bps"},

	  {BAUD_110,    "110 bps"},
	  {BAUD_134_5,  "134.5 bps"},

	  {BAUD_150,    "150 bps"},
	  {BAUD_300,    "300 bps"},
	  {BAUD_600,    "600 bps"},
	  {BAUD_1200,   "1200 bps"},
	  {BAUD_1800,   "1800 bps"},
	  {BAUD_2400,   "2400 bps"},
	  {BAUD_4800,   "4800 bps"},
	  {BAUD_7200,   "7200 bps"},
	  {BAUD_9600,   "9600 bps"},
	  {BAUD_14400,  "14400 bps"},

	  {BAUD_19200,  "19200 bps"},
	  {BAUD_38400,  "38400 bps"},
	  {BAUD_56K,    "56K bps"},
	  {BAUD_57600,  "57600 bps"},

	  {BAUD_115200, "115200 bps"},
	  {BAUD_128K,   "128K bps"},
	  {BAUD_USER,   "Programmable baud rates available"},
 };

const int iNumb_SetParety = 8;
static TPropDef pdSetParety[ iNumb_SetParety ] = 
 {
  { STOPBITS_10,    "1 stop bit" },

   { STOPBITS_15,   "1.5 stop bits" },
   { STOPBITS_20,   "2 stop bits" },
   { PARITY_NONE,   "No parity" },
   { PARITY_ODD,  "Odd parity" },
   { PARITY_EVEN, "Even parity" },   

   { PARITY_MARK, "Mark parity" },   
   { PARITY_SPACE, "Space parity" }
 };


static void AddPropertys( DWORD dwProperty, TPropDef* pTbl, int iSz, TV_INSERTSTRUCT& rV, HTREEITEM hPar, CTreeCtrl& rTree )
 {
   rV.hInsertAfter = TVI_LAST;   
   rV.hParent = hPar;
   rV.item.state = TVIS_BOLD;
   rV.item.stateMask = TVIS_BOLD;

   CString str;
   for( int i = 0; i < iSz; i++ )
	{
	  int iTst = pTbl[i].m_iMnem & dwProperty;
      str.Format( "%s [%s]", (LPSTR)pTbl[i].m_lpDescr, iTst ? "Yes":"No" );

	  rV.item.mask = TVIF_TEXT | (iTst ? TVIF_STATE:0);
	  rV.item.pszText = (LPSTR)(LPCSTR)str;

	  rTree.InsertItem( &rV );      
	}
 }

void TV_ProvCap( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s - Поддерживаемые микросхемой функции", (LPSTR)lp );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 

   AddPropertys(  pCOM->m_comm.dwProvCapabilities, 
	              pdProvCap, iNumb_ProvCap, tv, ht, rTree );	
 }

void TV_SettableParams( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s - Параметры, которые можно изменять", (LPSTR)lp );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 

   AddPropertys(  pCOM->m_comm.dwSettableParams, 
	              pdSetPar, iNumb_SetPar, tv, ht, rTree );	   
 }

void TV_SettableBaud( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s - Поддержимаемые микросхемой скорости передачи", (LPSTR)lp );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 

   //static TPropDef pdSetBaud[ iNumb_SetBaud ] = 
   AddPropertys(  pCOM->m_comm.dwSettableBaud, 
	              pdSetBaud, iNumb_SetBaud, tv, ht, rTree );	   
 }

void TV_SettableData( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s - Поддержимаемые микросхемой числа бит данных", (LPSTR)lp );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
   
   AddPropertys(  pCOM->m_comm.wSettableData, 
	              pdSetData, iNumb_SetData, tv, ht, rTree );	   
 }

void TV_SettableParity( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s - Поддержимаемые микросхемой числа стоповых бит и контроль паритета", (LPSTR)lp );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 

   
   AddPropertys(  pCOM->m_comm.wSettableStopParity, 
	              pdSetParety, iNumb_SetParety, tv, ht, rTree );	   
 }

void TV_CurTx( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %lu(bytes) - Текущее значение длины выходной очереди драйвера", 
	           (LPSTR)lp, (DWORD)pCOM->m_comm.dwCurrentTxQueue );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }

void TV_CurRx( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %lu(bytes) - Текущее значение длины входной очереди драйвера", 
	           (LPSTR)lp, (DWORD)pCOM->m_comm.dwCurrentRxQueue );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }


static TItemsPrepare cRootsTxt[ iNumbRoots ] =
 {
  { "wPacketVersion",         TV_PacketVer },
  {	"dwServiceMask",          TV_ServMask },
  { "dwMaxTxQueue",           TV_MaxTx },
  { "dwMaxRxQueue",           TV_MaxRx },
  { "dwMaxBaud ",             TV_MaxBaud },
  { "dwProvSubType",          TV_ProvSubType },
  { "dwProvCapabilities...",  TV_ProvCap },
  { "dwSettableParams...",    TV_SettableParams },
  { "dwSettableBaud...",      TV_SettableBaud },
  { "wSettableData...",       TV_SettableData },
  { "wSettableStopParity...", TV_SettableParity },
  { "dwCurrentTxQueue",       TV_CurTx },
  { "dwCurrentRxQueue",	      TV_CurRx }
 };


static LPCSTR GetStrBaudDCB( int iBaud )
 {
   const int iNumbBPS = 15;

   static const TBaudData iBauds[iNumbBPS] =
	{
	 {CBR_110,      "110 bps"},

	  {CBR_300,     "300 bps"},
	  {CBR_600,     "600 bps"},

	  {CBR_1200,    "1200 bps"},
	  {CBR_2400,    "2400 bps"},
	  {CBR_4800,    "4800 bps"},
	  {CBR_9600,    "9600 bps"},
	  {CBR_14400,   "14400 bps"},
	  {CBR_19200,   "19200 bps"},
	  {CBR_38400,   "38400 bps"},
	  {CBR_56000,   "56000 bps"},
	  {CBR_57600,   "57600 bps"},
	  {CBR_115200,  "115200 bps"},

	  {CBR_128000,  "128000 bps"},	  
	  {CBR_256000,  "256000 bps"}	  
	};

   for( int i = 0; i < iNumbBPS; i++ )
	 if( iBauds[i].m_iManemonic == iBaud ) return iBauds[i].cStr;

   return "USER bps";
 }


void TV2_BaudRate( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %s", 
	           (LPSTR)lp, GetStrBaudDCB(pCOM->m_arcCfg()->dcb.BaudRate) );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }
void TV2_Bin( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %s - Двоичный режим передачи", 
	  (LPSTR)lp, pCOM->m_arcCfg()->dcb.fBinary ? "TRUE":"FALSE" );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }
void TV2_Par( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %s - Контроль паритета", 
	  (LPSTR)lp, pCOM->m_arcCfg()->dcb.fParity ? "TRUE":"FALSE" );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }
void TV2_CTSFlow( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %s - Использование CTS для управления потоком данных", 
	  (LPSTR)lp, pCOM->m_arcCfg()->dcb.fOutxCtsFlow ? "TRUE":"FALSE" );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }
void TV2_DSRFlow( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %s - Использование DSR для управления потоком данных", 
	  (LPSTR)lp, pCOM->m_arcCfg()->dcb.fOutxDsrFlow ? "TRUE":"FALSE" );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }
void TV2_DTRFlow( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;

   char *lpTT;
   if( pCOM->m_arcCfg()->dcb.fDtrControl == DTR_CONTROL_DISABLE )
	lpTT = "DTR_CONTROL_DISABLE";
   else if( pCOM->m_arcCfg()->dcb.fDtrControl == DTR_CONTROL_ENABLE )
	lpTT = "DTR_CONTROL_ENABLE";
   else lpTT = "DTR_CONTROL_HANDSHAKE";

   str.Format( "%s = %s - Использование DTR", 
	  (LPSTR)lp, lpTT );	  

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }
void TV2_Sensitiv( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %s - Проверка состояния DSR при получении данных", 
	  (LPSTR)lp, pCOM->m_arcCfg()->dcb.fDsrSensitivity ? "TRUE":"FALSE" );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }
void TV2_ContinueOnXOFF( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %s - Проверка символа XOFF при заполнении буфера", 
	  (LPSTR)lp, pCOM->m_arcCfg()->dcb.fTXContinueOnXoff ? "TRUE":"FALSE" );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }
void TV2_OutX( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %s - Останов передачи при получении символа XOFF", 
	  (LPSTR)lp, pCOM->m_arcCfg()->dcb.fOutX ? "TRUE":"FALSE" );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }
void TV2_InX( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %s - Контроль XON/XOFF на приёме", 
	  (LPSTR)lp, pCOM->m_arcCfg()->dcb.fInX ? "TRUE":"FALSE" );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }

void TV2_ErrorCh( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %s - Использование символа ошибки при ошибке паритета", 
	  (LPSTR)lp, pCOM->m_arcCfg()->dcb.fErrorChar ? "TRUE":"FALSE" );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }
void TV2_Null( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %s - Замещение нулевых символов", 
	  (LPSTR)lp, pCOM->m_arcCfg()->dcb.fNull ? "TRUE":"FALSE" );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }
void TV2_RTSComtrol( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;

   char *lpTT;
   if( pCOM->m_arcCfg()->dcb.fRtsControl == RTS_CONTROL_DISABLE )
	lpTT = "RTS_CONTROL_DISABLE";
   else if( pCOM->m_arcCfg()->dcb.fRtsControl == RTS_CONTROL_ENABLE  )
	lpTT = "RTS_CONTROL_ENABLE ";
   else if( pCOM->m_arcCfg()->dcb.fRtsControl == RTS_CONTROL_HANDSHAKE )
	lpTT = "RTS_CONTROL_HANDSHAKE";
   else if( pCOM->m_arcCfg()->dcb.fRtsControl == 0 )
	lpTT = "RTS_CONTROL_HANDSHAKE";
   else lpTT = "RTS_CONTROL_TOGGLE";

   str.Format( "%s = %s - Использование RTS", 
	  (LPSTR)lp, lpTT );	  

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv );       
 }
void TV2_Abort( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %s - Завершение ввода/вывода при ошибке", 
	  (LPSTR)lp, pCOM->m_arcCfg()->dcb.fAbortOnError ? "TRUE":"FALSE" );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }
void TV2_XONLim( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %lu - Минимальное число символов во входном буфере перед приходом XON", 
	  (LPSTR)lp, (DWORD)pCOM->m_arcCfg()->dcb.XonLim );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }
void TV2_XOFFLim( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %lu - Максимальное число символов во входном буфере перед приходом XOFF", 
	  (LPSTR)lp, (DWORD)pCOM->m_arcCfg()->dcb.XoffLim );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }
void TV2_ByteSize( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %lu - Число бит в байте", 
	  (LPSTR)lp, (DWORD)pCOM->m_arcCfg()->dcb.ByteSize );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }
void TV2_Parity( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;

   char *lpTT;
   if( pCOM->m_arcCfg()->dcb.Parity == EVENPARITY )
	lpTT = "EVENPARITY";
   else if( pCOM->m_arcCfg()->dcb.Parity == MARKPARITY  )
	lpTT = "MARKPARITY";
   else if( pCOM->m_arcCfg()->dcb.Parity == NOPARITY )
	lpTT = "NOPARITY";
   else if( pCOM->m_arcCfg()->dcb.Parity == ODDPARITY )
	lpTT = "ODDPARITY";
   else lpTT = "SPACEPARITY";

   str.Format( "%s = %s - Использование паритета", 
	  (LPSTR)lp, lpTT );	  

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv );          
 }
void TV2_StopBits( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;

   char *lpTT;
   if( pCOM->m_arcCfg()->dcb.StopBits == ONESTOPBIT )
	lpTT = "1 stop bit";
   else if( pCOM->m_arcCfg()->dcb.StopBits == ONE5STOPBITS )
	lpTT = "1.5 stop bits";   
   else lpTT = "2 stop bits";

   str.Format( "%s = %s - Использование стоповых битов", 
	  (LPSTR)lp, lpTT );	  

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv );          
 }
void TV2_XONChar( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %lu - Символ XON", 
	  (LPSTR)lp, (int)pCOM->m_arcCfg()->dcb.XonChar );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }
void TV2_XOFFChar( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %lu - Символ XOFF", 
	  (LPSTR)lp, (int)pCOM->m_arcCfg()->dcb.XoffChar );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }
void TV2_ErrorChar( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %lu - Символ EVT", 
	  (LPSTR)lp, (int)pCOM->m_arcCfg()->dcb.ErrorChar );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }
void TV2_EOFChar( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %lu - Символ ERROR", 
	  (LPSTR)lp, (int)pCOM->m_arcCfg()->dcb.EofChar );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }
void TV2_EVTChar( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %lu - Символ EOF", 
	  (LPSTR)lp, (int)pCOM->m_arcCfg()->dcb.EvtChar );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }

void TV2_RTimeOut( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %lu", 
	  (LPSTR)lp, (DWORD)pCOM->timOut.ReadIntervalTimeout );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }
void TV2_RTimeOutM( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %lu", 
	  (LPSTR)lp, (DWORD)pCOM->timOut.ReadTotalTimeoutMultiplier );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }

void TV2_RTimeOutC( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %lu", 
	  (LPSTR)lp, (DWORD)pCOM->timOut.ReadTotalTimeoutConstant );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }


void TV2_WTimeOutM( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %lu", 
	  (LPSTR)lp, (DWORD)pCOM->timOut.WriteTotalTimeoutMultiplier );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }

void TV2_WTimeOutC( LPCSTR lp, TCOMMProp* pCOM, HTREEITEM& ht, CTreeCtrl& rTree, HTREEITEM hPar )
 {
   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   tv.hParent = hPar;

   CString str;
   str.Format( "%s = %lu", 
	  (LPSTR)lp, (DWORD)pCOM->timOut.WriteTotalTimeoutConstant );

   tv.item.pszText = (LPSTR)(LPCSTR)str;
   ht = rTree.InsertItem( &tv ); 
 }

static TItemsPrepare cRootsTxt2[ iNumbRoots2 ] =
 {
  { "BaudRate",           TV2_BaudRate },  //1
  {	"fBinary",            TV2_Bin },//2
  { "fParity",            TV2_Par },//3
  { "fOutxCtsFlow",       TV2_CTSFlow },//4
  { "fOutxDsrFlow",       TV2_DSRFlow },//5
  { "fDtrControl",        TV2_DTRFlow },//6
  { "fDsrSensitivity",    TV2_Sensitiv },//7
  { "fTXContinueOnXoff ", TV2_ContinueOnXOFF },//8
  { "fOutX",              TV2_OutX },//9
  { "fInX",               TV2_InX },//10
  { "fErrorChar",         TV2_ErrorCh },//11
  { "fNull",              TV2_Null },//12
  { "fRtsControl",	      TV2_RTSComtrol },//13
  { "fAbortOnError",      TV2_Abort },//14
  { "XonLim",             TV2_XONLim },//15

  { "XoffLim",            TV2_XOFFLim },//16
  { "ByteSize",           TV2_ByteSize },//17
  { "Parity",             TV2_Parity },//18
  { "StopBits",           TV2_StopBits },//19

  { "XonChar",            TV2_XONChar },//20
  { "XoffChar",           TV2_XOFFChar },//21
  { "ErrorChar",          TV2_ErrorChar },//22

  { "EofChar",            TV2_EOFChar }, //23
  { "EvtChar",             TV2_EVTChar },//24

  { "ReadIntervalTimeout", TV2_RTimeOut },//25
  { "ReadTotalTimeoutMultiplier", TV2_RTimeOutM },//26
  { "ReadTotalTimeoutConstant", TV2_RTimeOutC },
  { "WriteTotalTimeoutMultiplier", TV2_WTimeOutM },
  { "WriteTotalTimeoutConstant", TV2_WTimeOutC },
 };


TPRopPageInfo::~TPRopPageInfo()
 {
 }

BOOL TPRopPageInfo::OnInitDialog()
 {
   CPropertyPage::OnInitDialog();  

   m_Tree.SubclassDlgItem( IDC_TREE, this );
   

   ::EnableWindow( ::GetDlgItem(m_hWnd, IDC_RAD_COM1),
                   m_CP[0].m_bAval == true
				 );
   ::EnableWindow( ::GetDlgItem(m_hWnd, IDC_RAD_COM2),
                   m_CP[1].m_bAval == true
				 );
   ::EnableWindow( ::GetDlgItem(m_hWnd, IDC_RAD_COM3),
                   m_CP[2].m_bAval == true
				 );
   ::EnableWindow( ::GetDlgItem(m_hWnd, IDC_RAD_COM4),
                   m_CP[3].m_bAval == true
				 );

   int rd[4] = {IDC_RAD_COM1, IDC_RAD_COM2, IDC_RAD_COM3, IDC_RAD_COM4};
   for( int i = 0; i < 4; ++i )
	 if( m_CP[i].m_bAval == true )
	  {
	    SendDlgItemMessage( rd[i], BM_SETCHECK, 
		                     BST_CHECKED, 0 );
        ChangeCOMM( i );
		break;
	  }

   return FALSE;
 }



void TPRopPageInfo::ChangeCOMM( int iCOM )
 {//CTreeView
   //CTreeCtrl& rTree = m_Tree.GetTreeCtrl();

   if( m_CP[iCOM].m_bAval == false ) return;

   m_Tree.DeleteAllItems();

   TV_INSERTSTRUCT tv; memset( &tv, 0, sizeof(TV_INSERTSTRUCT) );
   tv.hInsertAfter = TVI_LAST;
   tv.item.mask = TVIF_TEXT;     
   
   CString str;
   str.Format( "Port COM%d - возможности", iCOM + 1 );
   tv.item.pszText = (LPSTR)(LPCSTR)str;
   HTREEITEM hRoot = m_Tree.InsertItem( &tv ); 

   m_Tree.SetRedraw( FALSE );
   for( int i = 0; i < iNumbRoots; i++ )
	 cRootsTxt[i].m_pProc( cRootsTxt[i].m_lpName, m_CP + iCOM, 
	                       m_Root[i].m_h, m_Tree, hRoot );
   m_Tree.SetRedraw( TRUE );
   

   if( m_CP[iCOM].m_bAvalCfg == false ) return;
	

   str.Format( "Port COM%d - установки", iCOM + 1 );
   tv.item.pszText = (LPSTR)(LPCSTR)str;
   hRoot = m_Tree.InsertItem( &tv ); 

   m_Tree.SetRedraw( FALSE );
   for( i = 0; i < iNumbRoots2; i++ )
	 cRootsTxt2[i].m_pProc( cRootsTxt2[i].m_lpName, m_CP + iCOM, 
	                       m_Root2[i].m_h, m_Tree, hRoot );

   m_Tree.SetRedraw( TRUE );

   //m_Tree.Expand( hRoot, TVE_EXPAND );
 }

void TPRopPageInfo::OnCOM1()
 {
   ChangeCOMM( 0 );
 }

void TPRopPageInfo::OnCOM2()
 {
   ChangeCOMM( 1 );
 }

void TPRopPageInfo::OnCOM3()
 {
   ChangeCOMM( 2 );
 }

void TPRopPageInfo::OnCOM4()
 {
   ChangeCOMM( 3 );
 }



TUsersDialog::~TUsersDialog()
 {
 }

TUsersDialog::TUsersDialog( TMFOptions& rOpt, LPCTSTR pszCaption, CWnd *pParentWnd, UINT iSelectPage  ):
    TBWCCPropertySheet( pszCaption, pParentWnd, iSelectPage ),

    m_pagPort( IDD_DLG_PORTS, rOpt, m_CP ),
	m_pagProtocol( IDD_DLG_PROTOCOL, rOpt ),
	m_pagInfo( IDD_DLG_INFO, m_CP ),

	m_rOpt( rOpt )	
 {         
   m_pPar = pParentWnd;

   AddPage( &m_pagPort ); 
   AddPage( &m_pagProtocol ); 
   AddPage( &m_pagInfo );  

   for( int i = 0; i < 4; ++i )
	{
	  static char cBuf[] = "COM1";
	  cBuf[3] = '1' + i;
	  
	  HANDLE handl = CreateFile( cBuf, 
	    GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );

	  if( handl == INVALID_HANDLE_VALUE ) m_CP[i].m_bAval = false;
	  else
	   {
	     m_CP[i].m_comm.wPacketLength = sizeof(COMMPROP);
         if( GetCommProperties(handl, &(m_CP[i].m_comm)) )
		   m_CP[i].m_bAval = true;
		 else
		   m_CP[i].m_bAval = false;	    

		 GetCommTimeouts( handl, &m_CP[i].timOut );
		 		 
		 DWORD dwCfg = 0;
		 //m_CP[i].m_arcCfg = (COMMCONFIG*)new BYTE[ sizeof(COMMCONFIG) ];
		 m_CP[i].m_bAvalCfg = false;
		 
		 if( GetCommConfig(handl, NULL, &dwCfg) )
		  {
            m_CP[i].m_arcCfg = (COMMCONFIG*)new BYTE[ dwCfg ];
			m_CP[i].m_arcCfg()->dwSize = dwCfg;
			if( GetCommConfig(handl, m_CP[i].m_arcCfg(), &dwCfg) )
			  m_CP[i].m_bAvalCfg = true;
		  }
		 

		 CloseHandle( handl );
	   }
	}
 }
  
int TUsersDialog::OnCreate( LPCREATESTRUCT lpCreateStruct )
 {
   if( TBWCCPropertySheet::OnCreate(lpCreateStruct) == -1 )
	 return -1;

   CenterWindow( m_pPar );

   return 0;
 }


void TUsersDialog::OnOK()
 { 
   m_iPage = GetActiveIndex();

   Default();
   EndDialog( IDOK );
 }
