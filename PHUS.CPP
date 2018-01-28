#include "phus.hpp"
#include "nframe.hpp"
#include "net.hpp"

#include "polinom.h"
#include <math.h>
#include <winbase.h>

#include "RC\net.rh"

class TOHold //m_rOpt.d.dwTOBroken
 {
public:
 TOHold( DWORD& dwHold, DWORD dwNewVal ):
   m_dwHold( dwHold )
   {
     m_dwKey = dwHold;
	 dwHold = dwNewVal;
	} 

  ~TOHold()
   {
     m_dwHold = m_dwKey;
   }

private:
  DWORD& m_dwHold;
  DWORD m_dwKey;
 };


TEvHold::TEvHold( CEvent* pEv, bool bFl )
 {
   m_bFl = bFl;
   m_pEv = pEv;
   if( bFl == true ) pEv->SetEvent();
   else pEv->ResetEvent();
 }

TEvHold::~TEvHold()
 {     
   if( m_bFl == true ) m_pEv->ResetEvent();	  
   else m_pEv->SetEvent();
 }

static BYTE CalcCRCAll( TPacket* pPck, BYTE& bCRCOld, int dwSize )
 {
   bCRCOld = pPck->m_CRCAll; pPck->m_CRCAll = 0;
   BYTE bKey = pPck->m_CRCHdr; pPck->m_CRCHdr = 0;

   BYTE bCRC = 0;
   BYTE *lp1 = (BYTE*)pPck;
   BYTE *lp2 = (BYTE*)pPck; lp2++; 

   for( ; dwSize > 1; dwSize--, lp1++, lp2++ )
	 bCRC += *lp1 ^ *lp2;

   pPck->m_CRCHdr = bKey;
   return bCRC;
 }

static BYTE CalcCRCHdr( TPacket* pPck, BYTE& bCRCOld )
 {
   bCRCOld = pPck->m_CRCHdr; pPck->m_CRCHdr = 0;
   BYTE bKey = pPck->m_CRCAll; pPck->m_CRCAll = 0;

   BYTE bCRC = 0;
   BYTE *lp1 = (BYTE*)pPck;
   BYTE *lp2 = (BYTE*)pPck; lp2++;    

   for( int dwSize = sizeof(TPacket) - 1; dwSize > 1; dwSize--, lp1++, lp2++ )
	 bCRC += *lp1 ^ *lp2;
              

   pPck->m_CRCAll = bKey;
   return bCRC;
 }

BYTE* TPhysicalLevel::CrePck( BYTE bSizeData, TPacket*& pPck, BYTE*& pData )
 {
   UINT iSize = m_rOpt.d.m_iUsePoly ? 516:260;

   //int iSz = sizeof(TPacket) + bSizeData;
   //BYTE *pB = new BYTE[ iSz ];
   BYTE *pB = new BYTE[ iSize ];

   memset( pB, 0, iSize );
   pPck = (TPacket*)pB;
   pData = pPck->m_Data;

   pPck->m_Size = bSizeData;
   
   return pB;
 }

static void MyReportErr( CException *pE )
 {
   CWinApp *pApp = AfxGetApp();
   char cBuf[ 1024 ];

   pE->GetErrorMessage( cBuf, 1024 );
   MessageBox( pApp->m_pMainWnd ? pApp->m_pMainWnd->GetSafeHwnd():NULL,
	 cBuf, "Error", MB_OK|MB_ICONSTOP|MB_TASKMODAL );   
 }

enum TM_Why { BR_Break = 0, BR_TimOut = 1 };

static void MessaBoxBroken( int iWhy )
 {
   static char *cWhy[] =
	{
      "Разрыв связи :(",
	  "Разрыв связи по истечении тайм-аута :(("
	};

   CWinApp *pApp = AfxGetApp();

   //MessageBox( pApp->m_pMainWnd ? pApp->m_pMainWnd->GetSafeHwnd():NULL,
	 //cWhy[iWhy], "Error", MB_OK|MB_ICONSTOP );

   ((TNetApp*)pApp)->bwcc.BWCCMessageBox( pApp->m_pMainWnd ? pApp->m_pMainWnd->GetSafeHwnd():NULL,
	  cWhy[iWhy], "Error", MB_OK|MB_ICONSTOP|MB_TASKMODAL );
 }

TChannel::TChannel( BYTE bNumb, TPhysicalLevel& rPh, CEvent& rConnectOK ):
  m_rPh( rPh ),
  m_evInQueueHavePacket( FALSE, TRUE ),
  m_evCancel( FALSE, TRUE ),
  m_mtxIOBusy( FALSE ),
  m_evHaveBlocks( FALSE, TRUE ),  
  m_mtxReadUnLocked( FALSE ),
  m_evConnectOK( rConnectOK )
   {
	 m_bNumb = bNumb;

	 rPh + *this;
   }

CEvent* TChannel::GetBrokenEv() { return &m_rPh.GetEvCnctBroken(); }
CEvent* TChannel::GetEvLetsActive() { return &m_rPh.GetEvLetsActive(); }

TPackedFrame::~TPackedFrame()
 {
   if( m_FlOwnPack == true && m_pPacked )
	{
      delete[] (BYTE*)m_pPacked; m_pPacked = NULL;
	}   
 }

TPackedFrame::TPackedFrame( TPacket* pPacked, DWORD& dwRes, CEvent *pevSended ):
    m_dwRes( dwRes )
  {
    m_pPacked = pPacked;
	m_pevSended = pevSended;
	dwRes = 0;
	m_FlOwnPack = true;

	if( pPacked->m_Type == TT_DataEnd ) m_dwTimeStart = ::GetTickCount();	
    m_dwSizeBlock = 0;
  }


TChannel::~TChannel()
 {
   SavetyKill();

   FreeRetrList();
   FreeBlocksList();
 }

void TChannel::FreeBlocksList()
 {
   list<TGenericBlock*>::iterator itSta( m_lstBlocks.begin() );
   list<TGenericBlock*>::iterator itEnd( m_lstBlocks.end() );
                                          
   for( ; itSta != itEnd; ++itSta )
	 if( *itSta )
	  {
        delete[] (BYTE*)*itSta; //*itSta = NULL;
	  }

   m_lstBlocks.clear();
 }

void TChannel::FreeRetrList()
 {//  list<TPacket*> m_lstRetrieved;
   list<TPacket*>::iterator itSta( m_lstRetrieved.begin() );
   list<TPacket*>::iterator itEnd( m_lstRetrieved.end() );

   for( ; itSta != itEnd; ++itSta )
	 if( *itSta )
	  {
        delete *itSta; //*itSta = NULL;
	  }

   m_lstRetrieved.clear();
 }

int TChannel::GetCnctIndex() const
 {
   return m_rPh.m_iCnctIndex;
 }

void TChannel::SavetyKill()
 {
   if( !m_arcThChannel() ) return;

   DWORD dwExitCode;
   if( GetExitCodeThread(m_arcThChannel()->m_hThread, &dwExitCode) && dwExitCode == STILL_ACTIVE )
	{
	  /*DWORD dwRes = m_arcTh()->ResumeThread();
	  if( dwRes != 0xFFFFFFFF && dwRes > 1 )
	   {
         while( dwRes > 1 )
		   dwRes = m_arcTh()->ResumeThread();
	   }*/

      m_evCancel.SetEvent();	  	  	  	  
	   	  
	  WaitForSingleObject( m_arcThChannel()->m_hThread, INFINITE );
	}

   m_arcThChannel.FreeRc();
 }

static UINT ThreadProc_TChannel( LPVOID pObj )
 {
   ((TChannel*)pObj)->WorkCycle();
   return 0;
 }

void TChannel::StartWorkCycle()
 {
   FreeBlocksList();
   FreeRetrList();

   m_evInQueueHavePacket.ResetEvent();
   m_evCancel.ResetEvent();

   m_mtxIOBusy.Unlock();
   m_evHaveBlocks.ResetEvent();

   m_arcThChannel = new CWinThread( ThreadProc_TChannel, (LPVOID)this );
   m_arcThChannel()->m_bAutoDelete = FALSE;
   m_arcThChannel()->CreateThread();    
 }

void TChannel::WorkCycle()
 {    
   CSyncObject* sync[3] = { &m_evInQueueHavePacket, &m_evCancel, &m_rPh.GetEvCnctBroken() };
   CMultiLock mLock( sync, 3 );
   
   while( 1 )
	{
      DWORD dwRes = mLock.Lock( INFINITE, FALSE );

	  if( dwRes >= WAIT_OBJECT_0 && dwRes <= WAIT_OBJECT_0 + 2 )
        dwRes -= WAIT_OBJECT_0;
	  else if( dwRes >= WAIT_ABANDONED_0 && dwRes <= WAIT_ABANDONED_0 + 2 )
        dwRes = -1;

	  if( dwRes == -1 || dwRes == 1 || dwRes == 2 ) return;
	  if( !dwRes ) GetPacket();
	}
 }

void TChannel::GetPacket()
 {
   CSingleLock lock( &m_mtxIOBusy, TRUE );
   
   TPacket* pPck = m_rPh.GetInPacket( this );

   //TGenericBlock *pGen = (TGenericBlock *)pPck->m_Data;
   if( pPck->m_Type == TT_DataSta || pPck->m_Type == TT_Data )
     m_lstRetrieved.push_back( pPck );
   else
	{

      m_lstRetrieved.push_back( pPck );

	  list<TPacket*>::iterator itSta( m_lstRetrieved.begin() );
      list<TPacket*>::iterator itEnd( m_lstRetrieved.end() );

	  __int64 iSize = 0;
      for( ; itSta != itEnd; ++itSta )
	    iSize += (*itSta)->m_Size;

	  TAutoMemoryRc_AR<BYTE*> 
	    arc( new BYTE[iSize + sizeof(TGenericBlock)] );
	  TGenericBlock* pGen = (TGenericBlock*)arc();

	  itSta = m_lstRetrieved.begin();
      itEnd = m_lstRetrieved.end();
	  LPBYTE lp = (LPBYTE)arc(); pGen->m_64Size = iSize;
	  lp += sizeof( TGenericBlock );
	  for( ; itSta != itEnd; ++itSta )
	   {
         memcpy( lp, (*itSta)->m_Data, (*itSta)->m_Size );
		 lp += (*itSta)->m_Size;
	   }

	  FreeRetrList();

	  arc.ClearRc();
	  m_lstBlocks.push_back( pGen );
	  if( m_lstBlocks.size() )  m_evHaveBlocks.SetEvent();
	}   

   lock.Unlock();
 }

//return is generic block
LPBYTE TChannel::ReadBlock( DWORD& dwSize, DWORD& dwRes_ )
 {   
   /*CSyncObject* syncW[3] = { &m_rPh.GetEvCnctBroken(), &m_evCancel, &m_evConnectOK };
   CMultiLock mLockW( syncW, 3 );
   
   DWORD dwResW = mLockW.Lock( INFINITE, FALSE );

   if( dwResW >= WAIT_OBJECT_0 && dwResW <= WAIT_OBJECT_0 + 2 )
     dwResW -= WAIT_OBJECT_0;
   else if( dwResW >= WAIT_ABANDONED_0 && dwResW <= WAIT_ABANDONED_0 + 2 )
     dwResW = -1;

   if( dwResW < 2 ) 
	{
      dwRes_ = -1; dwSize = 0; return NULL;
	}*/


   CSyncObject* sync[3] = { &m_rPh.GetEvCnctBroken(), &m_evCancel, &m_evHaveBlocks };
   CMultiLock mLock( sync, 3 );

   //TOHold holdTO( m_rPh.dwTORead, m_rPh.m_rOpt.d.dwTOBroken );
   
   while( 1 )
	{
      DWORD dwRes = mLock.Lock( INFINITE, FALSE );

	  if( dwRes >= WAIT_OBJECT_0 && dwRes <= WAIT_OBJECT_0 + 2 )
        dwRes -= WAIT_OBJECT_0;
	  else if( dwRes >= WAIT_ABANDONED_0 && dwRes <= WAIT_ABANDONED_0 + 2 )
        dwRes = -1;

	  if( dwRes == 1 || dwRes == -1 ) 
	   {
         dwSize = 0; dwRes_ = -1; //cancel
		 return NULL;
	   }

	  if( dwRes == 0 ) 
	   {
         dwSize = 0; dwRes_ = -2; //broken
		 return NULL;
	   }

      
	  TGenericBlock* p = PopBlock();
	  if( p )
	   {
	     dwSize = p->m_64Size;
		 dwRes_ = 1;
	     return (LPBYTE)p;
	   }

	  dwSize = 0; dwRes_ = 1; 
	  return NULL;
	}
 }

TGenericBlock* TChannel::PopBlock()
 {
   CSingleLock lock( &m_mtxIOBusy, TRUE );

   if( !m_lstBlocks.size() ) return NULL;

   if( m_lstBlocks.size() == 1 ) m_evHaveBlocks.ResetEvent();
   TGenericBlock* p = *m_lstBlocks.begin();
   m_lstBlocks.pop_front();

   return p;
 }

//writed is non generic block
void TChannel::WriteBlock( LPBYTE lp, DWORD dwSize, CEvent* pEvEnd, DWORD& dwRes )
 {  
   CSyncObject* sync[3] = { &m_rPh.GetEvCnctBroken(), &m_evCancel, &m_evConnectOK };
   CMultiLock mLockW( sync, 3 );
   
   DWORD dwResW = mLockW.Lock( INFINITE, FALSE );

   if( dwResW >= WAIT_OBJECT_0 && dwResW <= WAIT_OBJECT_0 + 2 )
     dwResW -= WAIT_OBJECT_0;
   else if( dwResW >= WAIT_ABANDONED_0 && dwResW <= WAIT_ABANDONED_0 + 2 )
     dwResW = -1;

   if( dwResW < 2 )
	{
      if( pEvEnd )  
	   {
	     dwRes = -1;
	     pEvEnd->SetEvent();
	   }
	  return;
	}


   CSingleLock lock( &m_rPh.GetOutQueueLock(), TRUE ); 

   int iType = TT_DataSta;
	//TT_DataSta = 13, 		
   //TT_Data = 14, TT_DataEnd 
   UINT iMul = (m_rPh.PckDataSize() == MAX_PACKETDATA_SIZE) ? 1:2;
   DWORD dwSizeKey = dwSize;
   for( int iCount = 0; true; iCount++ )
	{
	  //if( dwSize <= m_rPh.PckDataSize() - sizeof(TGenericBlock) )
	  if( dwSize <= m_rPh.PckDataSize() )
	   {
         TAutoMemoryRc_AR<BYTE*> arc( 
		   new BYTE[sizeof(TPacket) + dwSize*iMul] );
         TPacket* pPck = (TPacket*)arc();
		 memset( pPck, 0, sizeof(TPacket) + dwSize*iMul );

		 pPck->m_Type = TT_DataEnd;
		 pPck->m_NumbChannel = m_bNumb;
		 pPck->m_Size = dwSize;
		 memcpy( pPck->m_Data, lp, dwSize );

		 arc.ClearRc();
		 TPackedFrame *pF = new TPackedFrame( pPck, dwRes, pEvEnd );
		 pF->m_dwSizeBlock = dwSizeKey + sizeof(TPacket)*(iCount+1);

		 
		 m_rPh.AddOutPacket( pF );
		 return;
	   }

	  TAutoMemoryRc_AR<BYTE*> arc( 
		   new BYTE[sizeof(TPacket) + MAX_PACKETDATA_SIZE ] );
      TPacket* pPck = (TPacket*)arc();
	  memset( pPck, 0, sizeof(TPacket) + MAX_PACKETDATA_SIZE  );

	  pPck->m_Type = iCount ? TT_Data:TT_DataSta;
	  pPck->m_NumbChannel = m_bNumb;
	  pPck->m_Size = m_rPh.PckDataSize();
	  memcpy( pPck->m_Data, lp, m_rPh.PckDataSize() );

	  lp += m_rPh.PckDataSize();
	  dwSize -= m_rPh.PckDataSize();

	  arc.ClearRc();
	  TPackedFrame *pF = new TPackedFrame( pPck, dwRes, NULL );
	  m_rPh.AddOutPacket( pF );
	}
 }




TPhysicalLevel::TPhysicalLevel( TMFOptions& rOpt,
							    CEvent& rEvCnctOK, 
							    CEvent& rEvEable, CEvent& rEvCancelCnct,
								CEvent& evConnectBroken,
								TIndicator& rIndCnct, 
								TIndicator& rIndIn, TIndicator& rIndOut,
								TStatusWithLamp& rStatus ):
	m_rIndCnct( rIndCnct ), 
	m_rIndIn( rIndIn ),
	m_rIndOut( rIndOut ),
	m_evEnableConnect( rEvEable ),
	m_evCancelConnect( rEvCancelCnct ),
	m_evConnectOK( rEvCnctOK ),
	m_evConnectBroken( evConnectBroken ),
	m_evLetsActive( FALSE, TRUE ),
	m_evConnectOKInt( FALSE, TRUE ),
	m_evConnectOK2( FALSE, TRUE ),
	m_rOpt( rOpt ),	
	m_mtxInQ( FALSE ), m_mtxOutQ( FALSE ), m_mtxCycleOut( FALSE ),
	m_mtxTransmit( FALSE ),	
	m_evCancel( FALSE, TRUE ),
	m_evHasOutpack( FALSE, TRUE ),
	m_evPortInitComplete( FALSE, TRUE ),

	m_evOV( FALSE, TRUE ),
	m_evOVw( FALSE, TRUE ),
	
    m_rStatus( rStatus ),

	m_waitR( PURGE_RXABORT ),
	m_waitW( PURGE_TXABORT )

	 {
	   m_iNChannelSeqvence = 1;
       m_iCnctIndex = 0; //number of used channel

	   dwTORead = dwTOWrite = INFINITE;

	   m_dwCPS = 0;
	   m_pMFWnd = NULL;       
       m_dwErrCount = 0;

	   m_fFilePort = INVALID_HANDLE_VALUE;
	   m_bInNumber = m_bOutNumber = 0;
	   dwCfgSz = 0;

	   m_vecQCycleOut.assign( 256 );
       for( int i = 0; i < 256; ++i )
		 m_vecQCycleOut[i] = NULL;
	 }

TPhysicalLevel::~TPhysicalLevel()
 {
   SavetyKill();

   //DumpTEST( m_lst_TESTIn, "F:\\dump.in" );
   //DumpTEST( m_lst_TESTOut, "F:\\dump.out" );

   ClearPackets();
   ClearInQ();
   ClearOutQ();

   CloseCOMM();
 }

void TPhysicalLevel::DumpTEST( list<TPacket*>& rLst, LPSTR lp )
 {
  //m_lst_TESTIn, m_lst_TESTOut;
   list<TPacket*>::iterator itSta( rLst.begin() );
   list<TPacket*>::iterator itEnd( rLst.end() );

   CFile cf( lp, CFile::modeCreate|CFile::modeReadWrite );
   char msg1[] = "In:\n";
   cf.Write( msg1, strlen(msg1) );

   for( ; itSta != itEnd; ++itSta )
	{
      char msg1[] = "\n-----------------\n";
      cf.Write( msg1, strlen(msg1) );

	  char cBuf[500];
	  sprintf( cBuf, "\tNumb: %d\n"
                     "\tNumbCh: %d\n"
					 "\tType: %d\n"
					 "\tSize: %d\n"
					 "\tCS: %d\n"
					 "\tDATA:\n",
					 (int)(*itSta)->m_Number,
					 (int)(*itSta)->m_NumbChannel,
					 (int)(*itSta)->m_Type,
					 (int)(*itSta)->m_Size,
					 (int)(*itSta)->m_CountSend );
	  cf.Write( cBuf, strlen(cBuf) );
	  cf.Write( (*itSta)->m_Data, (*itSta)->m_Size );
	  char msg2[] = "\n*****************\n";
      cf.Write( msg2, strlen(msg1) );					 
	}

   itSta = rLst.begin();
   itEnd = rLst.end();
   for( ; itSta != itEnd; ++itSta )
	 delete *itSta;
   rLst.clear();
 }

void TPhysicalLevel::ClearPackets()
 {
   for( int i = 0; i < 256; i++ )
	 if( m_vecQCycleOut[i] )
	  {
        delete[] (BYTE*)m_vecQCycleOut[i];
		m_vecQCycleOut[i] = NULL;
	  }
 }
void TPhysicalLevel::ClearInQ()
 {
   if( !m_lstInQueue.size() ) return;

   list<TPacket*>::iterator itSta( m_lstInQueue.begin() );
   list<TPacket*>::iterator itEnd( m_lstInQueue.end() );

   for( ; itSta != itEnd; ++itSta )
	if( *itSta )
	  {
        delete[] (BYTE*)*itSta; //*itSta = NULL;
	  }

   m_lstInQueue.clear();
 }


void TPhysicalLevel::ClearInQSys()
 {
   if( !m_lstInQueue.size() ) return;
   
   //m_lstInQueue.remove_if( PrSys );
   
   list<TPacket*>::iterator itSta( m_lstInQueue.begin() );
   list<TPacket*>::iterator itEnd( m_lstInQueue.end() );

   
   for( ; itSta != itEnd; ++itSta )
	if( *itSta && (*itSta)->m_NumbChannel == NUMB_CHANNEL_SYS )
	  {
	    BYTE* p = (BYTE*)*itSta; 
        delete[] (BYTE*)*itSta; //*itSta = NULL;
        m_lstInQueue.erase( itSta ); 
		
		itSta = m_lstInQueue.begin();
        itEnd = m_lstInQueue.end();
	  }
 }

void TPhysicalLevel::ClearOutQ()
 {
   if( !m_lstOutQueue.size() ) return;

   list<TPackedFrame*>::iterator itSta( m_lstOutQueue.begin() );
   list<TPackedFrame*>::iterator itEnd( m_lstOutQueue.end() );

   for( ; itSta != itEnd; ++itSta )
	if( *itSta )
	  {
        if( (*itSta)->m_pevSended )
		 {
		   (*itSta)->m_dwRes = -1;
	       (*itSta)->m_pevSended->SetEvent();	       
		 }

        delete *itSta; //*itSta = NULL;
	  }

   m_lstOutQueue.clear();
 }

void TPhysicalLevel::SavetyKill()
 {
   m_evEnableConnect.ResetEvent();
   m_evConnectOK.ResetEvent();
   m_evConnectBroken.SetEvent();
   m_evCancelConnect.SetEvent();
      
   Sleep( 200 );

   m_evCancel.SetEvent();
   CloseCOMM();
//m_arcThIn;
  //TAutoMemoryRc<CWinThread*> m_arcThOut
   if( m_arcThIn() )
	{
      m_evCancel.SetEvent();
	  m_evCancelConnect.SetEvent();
	  m_evEnableConnect.ResetEvent();

      DWORD dwExitCode;
      if( GetExitCodeThread(m_arcThIn()->m_hThread, &dwExitCode) && dwExitCode == STILL_ACTIVE )	          	   	  
	    WaitForSingleObject( m_arcThIn()->m_hThread, INFINITE );
	  m_arcThIn.FreeRc();
	}

   if( m_arcThOut() )
	{
	  m_evCancel.SetEvent();
	  m_evCancelConnect.SetEvent();
	  m_evEnableConnect.ResetEvent();

      DWORD dwExitCode;
      if( GetExitCodeThread(m_arcThOut()->m_hThread, &dwExitCode) && dwExitCode == STILL_ACTIVE )	          	   	  
	    WaitForSingleObject( m_arcThOut()->m_hThread, INFINITE );
	  m_arcThOut.FreeRc();
	}   
 }

TPhysicalLevel& TPhysicalLevel::operator+( TChannel& rCh )
 {
   CSingleLock lock( &m_mtxLstChannels, TRUE );

   m_lstChannels.push_back( &rCh );

   return *this;
 }

TPhysicalLevel& TPhysicalLevel::operator-( TChannel& rCh )
 {
   CSingleLock lock( &m_mtxLstChannels, TRUE );

   m_lstChannels.remove( &rCh );

   return *this;
 }

bool TPhysicalLevel::CheckPackets( TChannel* pCh )
 {
   list<TPacket*>::iterator itSta( m_lstInQueue.begin() );
   list<TPacket*>::iterator itEnd( m_lstInQueue.end() );

   for( ; itSta != itEnd; ++itSta )
	if( (*itSta)->m_NumbChannel == pCh->GetNumb() ) return true;

   return false;
 }

TPacket* TPhysicalLevel::GetInPacket( TChannel* pCh )
 {
   CSingleLock lock( &m_mtxInQ, TRUE );

   list<TPacket*>::iterator itSta( m_lstInQueue.begin() );
   list<TPacket*>::iterator itEnd( m_lstInQueue.end() );

   for( ; itSta != itEnd; ++itSta )
	 if( (*itSta)->m_NumbChannel == pCh->GetNumb() )
	  {
        TPacket* pRes = *itSta;
		m_lstInQueue.remove( pRes );
		if( CheckPackets(pCh) == false ) pCh->m_evInQueueHavePacket.ResetEvent();

		return pRes;
	  }

   pCh->m_evInQueueHavePacket.ResetEvent();
   return NULL;
 }

void TPhysicalLevel::AddOutPacket( TPackedFrame* pF, bool bFlFront )//external lock
 {   
   if( bFlFront == false ) m_lstOutQueue.push_back( pF );
   else m_lstOutQueue.push_front( pF );

   /*TPacket* pPckTST = (TPacket*)new BYTE[ SZ_P(pF->m_pPacked) ];
   memcpy( pPckTST, pF->m_pPacked, SZ_P(pF->m_pPacked) );
   if( bFlFront == false ) m_lst_TESTOut.push_back( pPckTST );
   else m_lst_TESTOut.push_front( pPckTST );*/


   if( m_lstOutQueue.size() == 1 ) m_evHasOutpack.SetEvent();
   //m_evHasOutpack.SetEvent();
 }


static UINT ThreadProc_TPhysIn( LPVOID pObj )
 {
   ((TPhysicalLevel*)pObj)->WorkCycleIn();
   ((TPhysicalLevel*)pObj)->OffLamps();   
   ((TPhysicalLevel*)pObj)->ResetEvents(); 
   return 0;
 }

static UINT ThreadProc_TPhysOut( LPVOID pObj )
 {
   ((TPhysicalLevel*)pObj)->WorkCycleOut();
   ((TPhysicalLevel*)pObj)->OffLamps();
   ((TPhysicalLevel*)pObj)->ResetEvents(); 
   return 0;
 }


void TPhysicalLevel::StartWorkCycleOut()
 {   
   m_evConnectBroken.SetEvent();
   m_evConnectOK.ResetEvent();
   m_evConnectOKInt.ResetEvent();
   m_evConnectOK2.ResetEvent();
   m_evPortInitComplete.ResetEvent();
   m_evCancel.ResetEvent();

   m_arcThOut = new CWinThread( ThreadProc_TPhysOut, (LPVOID)this );
   m_arcThOut()->m_bAutoDelete = FALSE;
   m_arcThOut()->CreateThread();    
 }

void TPhysicalLevel::StartWorkCycleIn()
 {
   m_arcThIn = new CWinThread( ThreadProc_TPhysIn, (LPVOID)this );
   m_arcThIn()->m_bAutoDelete = FALSE;
   m_arcThIn()->CreateThread();    
 }

int TPhysicalLevel::WaitEnableCnct( CMultiLock& mLock )
 {////CSyncObject* sync[2] = { &m_evPortInitComplete, &m_evCancel };
   CloseCOMM();

   DWORD dwRes = mLock.Lock( INFINITE, FALSE );   
   if( dwRes >= WAIT_OBJECT_0 && dwRes <= WAIT_OBJECT_0 + 1 )
    dwRes -= WAIT_OBJECT_0;
   else if( dwRes >= WAIT_ABANDONED_0 && dwRes <= WAIT_ABANDONED_0 + 1 )
    dwRes = -1;
 
   if( dwRes == -1 || dwRes == 1 ) return -1;
 
   try {
    OpenCOMM();
	}
   catch( CException* pE )
	{
      MyReportErr( pE );
      pE->Delete();      
	  m_evEnableConnect.ResetEvent();
	  m_evCancelConnect.SetEvent();
      return 0;
	}

   return 1;
 }


int TPhysicalLevel::SendPacket( TPacket* pPck, bool bDSR  )
 {    //////return 1;
   if( m_rOpt.d.m_iUsePoly ) CodePacket( pPck );

   OVERLAPPED ov; memset( &ov, 0, sizeof(OVERLAPPED) );   
   ov.hEvent = m_evOVw;
   m_evOVw.ResetEvent();

   /*evCOMM_WR.ResetEvent();
   OVERLAPPED ovEV; memset( &ovEV, 0, sizeof(OVERLAPPED) );   
   ovEV.hEvent = evCOMM_WR;   
   DWORD dwEvMask;
   if( bDSR == true ) WaitCommEvent( m_fFilePort, &dwEvMask, &ovEV );*/
   
   //int iWaitRes = m_waitW();
   //if( bDSR == true && iWaitRes ) return 4;
   //int iInd = (bDSR == true) ? 4:3;
   int iInd = 3;

   BYTE bOld;
   BYTE bCRCH = CalcCRCHdr( pPck, bOld );
   BYTE bCRCA = CalcCRCAll( pPck, bOld, SZ_P(pPck) );

   pPck->m_CRCHdr = bCRCH;
   pPck->m_CRCAll = bCRCA;
	//TPacket* pPck, BYTE& bCRCOld )

   TSignalHold holder( &m_rIndOut, true );

   CSyncObject* sync[4] = { &m_evCancelConnect, &m_evCancel, &m_evOVw, m_waitW.EV() };
   CMultiLock mLock( sync, iInd );
   iInd--;


   while(1)
	{
      BYTE bStaPck = CHAR_BEGINPACKET;
	  DWORD dwRd;	  
      WriteFile( m_fFilePort, &bStaPck, 1, &dwRd, &ov );	  	
     
	  DWORD dwRes = mLock.Lock( INFINITE, FALSE );   
      if( dwRes >= WAIT_OBJECT_0 && dwRes <= WAIT_OBJECT_0 + iInd )
        dwRes -= WAIT_OBJECT_0;
      else if( dwRes >= WAIT_ABANDONED_0 && dwRes <= WAIT_ABANDONED_0 + iInd )
        dwRes = -1;
	  
	  if( dwRes == 3 ) 
	   {
	     PurgeComm(m_fFilePort, PURGE_TXABORT);

	     //if( bDSR == false ) continue; 
		 m_waitW();

		 //CancelIo(m_fFilePort); 
		 //DWORD dwR;
		 //GetCommModemStatus( m_fFilePort, &dwR );
		 //if( dwR & MS_RLSD_ON ) continue;

		 //PurgeComm(m_fFilePort, PURGE_TXABORT);
		 return 4;
	   }
       
	  if( dwRes == -1 || dwRes == 1 ) 
	   {
	     PurgeComm(m_fFilePort, PURGE_TXABORT); 
		 return -1;
	   }
      if( !dwRes ) 
	   {
	     PurgeComm(m_fFilePort, PURGE_TXABORT); 
		 return 0;
	   }

	  BOOL bRDRes = GetOverlappedResult( m_fFilePort, &ov, &dwRd, FALSE );
	  if( !bRDRes ) 
	   {
	     DWORD err = ::GetLastError();
		 PurgeComm(m_fFilePort, PURGE_TXABORT); 
	     if( err == ERROR_IO_INCOMPLETE ) continue;		  

	     CFileException::ThrowOsError( err, NULL );
	   }

	  //if( dwRd != 1 )
	    //continue;

	  memset( &ov, 0, sizeof(OVERLAPPED) );   
      ov.hEvent = m_evOVw;
      m_evOVw.ResetEvent();

      
      WriteFile( m_fFilePort, pPck, SZ_P(pPck), &dwRd, &ov );	  
	  

      dwRes = mLock.Lock( INFINITE, FALSE );   
      if( dwRes >= WAIT_OBJECT_0 && dwRes <= WAIT_OBJECT_0 + iInd )
        dwRes -= WAIT_OBJECT_0;
      else if( dwRes >= WAIT_ABANDONED_0 && dwRes <= WAIT_ABANDONED_0 + iInd )
        dwRes = -1;
	  
	  if( dwRes == 3 ) 
	   {
         PurgeComm(m_fFilePort, PURGE_TXABORT);

	     //if( bDSR == false ) continue; 
		 m_waitW();

		 //CancelIo(m_fFilePort); 
		 //DWORD dwR;
		 //GetCommModemStatus( m_fFilePort, &dwR );
		 //if( dwR & MS_RLSD_ON ) continue;

		 //PurgeComm(m_fFilePort, PURGE_TXABORT);
		 return 4;
	   }
       
	  if( dwRes == -1 || dwRes == 1 ) 
	   {
	     PurgeComm(m_fFilePort, PURGE_TXABORT); 
		 return -1;
	   }
      if( !dwRes ) 
	   {
	     PurgeComm(m_fFilePort, PURGE_TXABORT); 
		 return 0;
	   }

	  bRDRes = GetOverlappedResult( m_fFilePort, &ov, &dwRd, FALSE );
	  if( !bRDRes ) 
	   {
	     DWORD err = ::GetLastError();
		 PurgeComm(m_fFilePort, PURGE_TXABORT); 
	     if( err == ERROR_IO_INCOMPLETE ) continue;		  

	     CFileException::ThrowOsError( err, NULL );
	   }

	  //if( dwRd != SZ_P(pPck) )
	    //continue;
	

	  /*TPacket* pPckTST = (TPacket*)new BYTE[ SZ_P(pPck) ];
      memcpy( pPckTST, pPck, SZ_P(pPck) );
      m_lst_TESTOut.push_back( pPckTST );*/

	  return 1;
	}
 }


void TPhysicalLevel::SendReqvestPacket()
 {
   TPacket *pPck;
   TConnectReq *pReq;
   TAutoMemoryRc_AR<BYTE*> arc( CrePck(sizeof(TConnectReq), pPck, (BYTE*&)pReq) );

   pPck->m_NumbChannel = NUMB_CHANNEL_SYS;
   pPck->m_Type = TT_ConnectReqvest;   
   pPck->m_CountSend = 0;

   pReq->m_iBoud   = m_iBaud;  
   pReq->m_iStops  = m_iStops;
   
   SendPacket( pPck, false );   
     //CFileException::ThrowOsError( ::GetLastError(), NULL );	
 }

struct TBaudData
 {
   int m_iManemonic;
   char *cStr;
 };

static void Confirm( int& iBoud, int& iStops, HANDLE fFilePort )
 {
   COMMPROP cmpr;
   cmpr.wPacketLength = sizeof(COMMPROP);
   GetCommProperties( fFilePort, &cmpr );
   if( !(iStops & cmpr.wSettableStopParity) )
	 iStops = STOPBITS_10;
   if( !(iBoud & cmpr.dwSettableBaud) )
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

   int k = 0;
   for( int i = 0; i < iNumbBPS; i++ )
	 if( iBauds[i].m_iManemonic == iBoud ) 
	  {
        k = i;
		break;
	  }
   iBoud = -1;
   for( i = k-1; i >= 0 ; i-- )
	 if( iBauds[i].m_iManemonic & cmpr.dwSettableBaud )
	   iBoud = iBauds[i].m_iManemonic;

   if( iBoud == -1 ) iBoud = BAUD_075;  
	}
 }

struct TBaudData2
 {
   int m_iManemonic;
   int m_iManemonic2;
 };


static int TranslateBaud( int iB )
 {
   const int iNumbBPS = 20;
   static const TBaudData2 iBauds[iNumbBPS] =
	{
	 {BAUD_075,     CBR_110},

	  {BAUD_110,    CBR_110},
	  {BAUD_134_5,  CBR_110},

	  {BAUD_150,    CBR_300},
	  {BAUD_300,    CBR_300},
	  {BAUD_600,    CBR_600},
	  {BAUD_1200,   CBR_1200},
	  {BAUD_1800,   CBR_1200},
	  {BAUD_2400,   CBR_2400},
	  {BAUD_4800,   CBR_4800},
	  {BAUD_7200,   CBR_9600},
	  {BAUD_9600,   CBR_9600},
	  {BAUD_14400,  CBR_14400},

	  {BAUD_19200,  CBR_19200},
	  {BAUD_38400,  CBR_38400},
	  {BAUD_56K,    CBR_56000},
	  {BAUD_57600,  CBR_57600},

	  {BAUD_115200, CBR_115200},
	  {BAUD_128K,   CBR_128000},
	  {BAUD_USER,   CBR_256000},
	};

   for( int i = 0; i < iNumbBPS; i++ )
     if( iBauds[i].m_iManemonic == iB ) return iBauds[i].m_iManemonic2;

   return CBR_110;
 }
static int TranslateStop( int iS )
 {
   if( iS == STOPBITS_10 ) return ONESTOPBIT;
   if( iS == STOPBITS_15 ) return ONE5STOPBITS;

   return TWOSTOPBITS;
 }


void TPhysicalLevel::SendOKPacket( bool bOK2 )
 {
   TPacket *pPck;
   TConnectReq *pReq;
   TAutoMemoryRc_AR<BYTE*> arc( CrePck(sizeof(TConnectReq), pPck, (BYTE*&)pReq) );   
      

/////////GetInPacket
   CSingleLock lock( &m_mtxInQ, TRUE );

   list<TPacket*>::iterator itSta( m_lstInQueue.begin() );
   TAutoMemoryRc_AR<BYTE*> arcInPck( (BYTE*)*itSta );
   //m_lstInQueue.pop_front();   
   m_lstInQueue.remove( *itSta );
   ClearInQ();

   BYTE szKey;
   memcpy( pPck, arcInPck(), (szKey=SZ_P(pPck)) );

   pPck->m_Number = 0;
   pPck->m_NumbChannel = NUMB_CHANNEL_SYS;
   pPck->m_Type = bOK2 == true ? TT_ConnectOK2:TT_ConnectOK;   
   pPck->m_Size = szKey;

   if( bOK2 == false ) 
	{	
	  Confirm( pReq->m_iBoud, pReq->m_iStops, m_fFilePort );   
      m_iBaud   = pReq->m_iBoud;
      m_iStops  = pReq->m_iStops;

      SendPacket( pPck, false );      
	}
   
   DCB dcb; GetCommState( m_fFilePort, &dcb );
   int iBaud = TranslateBaud( m_iBaud );
   int iStop = TranslateStop( m_iStops );

   bool bSleep = false;
   if( dcb.BaudRate != iBaud || dcb.StopBits != iStop )
	{
	  /*bSleep = true;      
	  Sleep( 200 );

      PurgeComm( m_fFilePort, PURGE_RXCLEAR );
      ClearInQ();
	  
      dcb.BaudRate = iBaud;
      dcb.StopBits = iStop;
   
      SetCommState( m_fFilePort, &dcb );
      SetupTimeouts( m_fFilePort, dcb.BaudRate );

	  m_rOpt.d.m_iBaud  = m_iBaud;
      m_rOpt.d.m_iStops = m_iStops;
	  TMainFrame* pFr = (TMainFrame*)AfxGetApp()->m_pMainWnd;
	  if( pFr ) pFr->DispBCE( m_iBaud, 0, 0 );	  */
	}

   if( bOK2 == true ) 
	{ 
	  /*if( bSleep == false ) 
	   {
	     Sleep( 200 ); 
		 PurgeComm( m_fFilePort, PURGE_RXCLEAR );
         ClearInQ();
	   }*/
	  SendPacket( pPck, false ); //??????
	}
 }

int TPhysicalLevel::WaitReqvestCnct( CMultiLock& mLock )
 {
 //SyncObject* sync2[3] = { &m_evCancelConnect, &m_evCancel, &m_evConnectReqvest, &m_evConnectOK2, &m_evConnectOK  };
   while(1)
	{
      DWORD dwRes = mLock.Lock( m_rOpt.d.dwTOReqvest, FALSE );   
      if( dwRes >= WAIT_OBJECT_0 && dwRes <= WAIT_OBJECT_0 + 4 )
       dwRes -= WAIT_OBJECT_0;
      else if( dwRes >= WAIT_ABANDONED_0 && dwRes <= WAIT_ABANDONED_0 + 4 )
       dwRes = -1;
      else if( dwRes == WAIT_TIMEOUT )
	   {
	     try {
		   SendReqvestPacket();   
		  }
		 catch( CException *pE )
		  {
		    //pE->ReportError();
		    MyReportErr( pE );
			pE->Delete();
			
			m_evEnableConnect.ResetEvent();
			m_evCancelConnect.SetEvent();
			m_evPortInitComplete.ResetEvent();
			CloseCOMM();
			return 0;
		  }
		 continue;
	   }
 
      if( dwRes == -1 || dwRes == 1 ) return -1;
      if( !dwRes ) return 0;

	  if( dwRes == 2 ) //ReqvestCnct
	   {
	     m_evConnectReqvest.ResetEvent();
	   	 try {
		   SendOKPacket( false );
		  }
		 catch( CException *pE )
		  {
		    //pE->ReportError();
		    MyReportErr( pE );
			pE->Delete();
			
			m_evEnableConnect.ResetEvent();
			m_evCancelConnect.SetEvent();
			m_evPortInitComplete.ResetEvent();
			CloseCOMM();
			return 0;
		  }
		 continue;
	   }
	  else if( dwRes == 3 ) //3 ConnectOK2
	   {	     
         m_evConnectOK2.ResetEvent();		 		 
		 return 1;
	   }
	  else //4 ConnectOK
	   {
	     m_evConnectOKInt.ResetEvent();
	     try {
		   SendOKPacket( true );////////
		  }
		 catch( CException *pE )
		  {
		    //pE->ReportError();
		    MyReportErr( pE );
			pE->Delete();
			
			m_evEnableConnect.ResetEvent();
			m_evCancelConnect.SetEvent();
			m_evPortInitComplete.ResetEvent();
			CloseCOMM();
			return 0;
		  }
		 //continue;		 
		 return 1;
	   }
	}    
 }

void TPhysicalLevel::WorkCycleOut()
 {//m_evPortInitComplete     
   CSyncObject* sync[2] = { &m_evEnableConnect, &m_evCancel };
   CMultiLock mLock( sync, 2 );
   

   while(1)
	{
      while(1)
	   { 
	     m_evLetsActive.ResetEvent();

	     m_evPortInitComplete.ResetEvent();
		 m_evConnectBroken.SetEvent();
		 m_evConnectOK.ResetEvent();

		 int iRes1 = WaitEnableCnct( mLock );
		 if( iRes1 == -1 ) return;
		 if( !iRes1 ) continue;

		 m_pMFWnd = (TMainFrame*)AfxGetApp()->m_pMainWnd;

		 m_iBaud = m_rOpt.d.m_iBaud;
         m_iStops = m_rOpt.d.m_iStops;   

		 m_evPortInitComplete.SetEvent();

		 m_evConnectReqvest.ResetEvent();
		 m_evConnectOKInt.ResetEvent();
		 m_evConnectOK2.ResetEvent();
		 CSyncObject* sync2[5] = { &m_evCancelConnect, &m_evCancel, 
		                           &m_evConnectReqvest, &m_evConnectOK2, 
								   &m_evConnectOKInt };
         CMultiLock mLock2( sync2, 5 );

		 iRes1 = WaitReqvestCnct( mLock2 );
		 if( iRes1 == -1 ) return;
		 if( !iRes1 ) continue;        

		 CSingleLock lockOut( &m_mtxOutQ, TRUE );
	     ClearOutQ();
	     lockOut.Unlock();

		 CSingleLock lockOutPck( &m_mtxCycleOut, TRUE );
		 ClearPackets();
		 lockOutPck.Unlock();

		 m_evConnectBroken.ResetEvent();
		 m_evConnectOKInt.SetEvent();		 
		 m_evConnectOK.SetEvent();     //???????

		 m_rIndCnct.SetStOn( true, true );		 
		 StartChannels();
		 

		 CSyncObject* sync3[4] = { &m_evCancelConnect, &m_evCancel, 
		                           &m_evHasOutpack, &m_evConnectBroken };
		  
         CMultiLock mLock3( sync3, 4 );
		 m_bOutNumber = 0;
		 
		 while(1)
		  {
            DWORD dwRes = mLock3.Lock( INFINITE, FALSE );   
            if( dwRes >= WAIT_OBJECT_0 && dwRes <= WAIT_OBJECT_0 + 3 )
              dwRes -= WAIT_OBJECT_0;
            else if( dwRes >= WAIT_ABANDONED_0 && dwRes <= WAIT_ABANDONED_0 + 3 )
              dwRes = -1;

			if( dwRes == -1 || dwRes == 1 ) 
			 {
               CloseCOMM(); return;
			 }
			if( !dwRes || dwRes == 3 ) break;
			
			dwRes = TransmitQPacket();			
			if( dwRes == 4 )
			 {
               MessaBoxBroken( BR_Break );
               break;
			 }
			
			if( dwRes == -1 )
			 {
               CloseCOMM(); return;
			 }
			if( !dwRes ) break;
		  }
	   }
	}//while no CANCEL and wait ENABLE CONNECT
                 
 }

static void NextSeq( int& riVal, int iLow, int iHigh )
 {
   riVal++;
   if( riVal > iHigh ) riVal = iLow;
 }

TPackedFrame* TPhysicalLevel::GetPrioretyPck()
 {
   list<TPackedFrame*>::iterator itSta( m_lstOutQueue.begin() );
   list<TPackedFrame*>::iterator itEnd( m_lstOutQueue.end() );

   bool bFlFound = false;
   for( ; itSta != itEnd; ++itSta )
	{      
	  if( (*itSta)->m_pPacked->m_NumbChannel == NUMB_CHANNEL_SYS )
	   {
	     bFlFound = true;
	     break;
	   }

	  if( (*itSta)->m_pPacked->m_CountSend )
	   {
	     bFlFound = true;
	     break;
	   }

	  if( (*itSta)->m_pPacked->m_NumbChannel == m_iNChannelSeqvence )
	   {
         NextSeq( m_iNChannelSeqvence, 1, m_lstChannels.size() );
		 bFlFound = true;
		 break;
	   }
	}

   list<TPackedFrame*>::iterator itRet = (bFlFound == true) ? 
                                 itSta:m_lstOutQueue.begin();

   TPackedFrame* p = *itRet;
   m_lstOutQueue.erase( itRet );

   return p;
 }


int TPhysicalLevel::TransmitQPacket()
 {
   CSingleLock lockTr( &m_mtxTransmit, TRUE );
   CSingleLock lock( &m_mtxOutQ, TRUE );

   
   //TOHold holdTO( dwTOWrite, m_rOpt.d.dwTOBroken );


   if( !m_lstOutQueue.size() ) return 1;

   TAutoMemoryRc<TPackedFrame*> arcF( GetPrioretyPck() );
   //m_lstOutQueue.pop_front();
  
   if( !m_lstOutQueue.size() )  m_evHasOutpack.ResetEvent();

   arcF()->m_pPacked->m_CountSend++;
   if( arcF()->m_pPacked->m_CountSend == 1 )	
     arcF()->m_pPacked->m_Number = m_bOutNumber;
	  	

   int iRes = SendPacket( arcF()->m_pPacked, true );

   if( m_rOpt.d.m_bShowCPS  &&
	  arcF()->m_pPacked->m_Type == TT_DataEnd && 
	  arcF()->m_dwSizeBlock )
	{
	  m_dwCPS = 1000.0 * double(arcF()->m_dwSizeBlock) / 
	            fabs(::GetTickCount() - arcF()->m_dwTimeStart);	  
	  if( m_pMFWnd ) m_pMFWnd->DispBCE( m_iBaud, m_dwCPS, m_dwErrCount );
	}
   if( arcF()->m_pevSended ) 
	{
	  arcF()->m_dwRes = (iRes == 4) ? 0:iRes;
	  arcF()->m_pevSended->SetEvent();	  
	}

   if( iRes == -1 ) return -1; 
   if( !iRes ) return 0;
   if( iRes == 4 ) return 4;

   if( arcF()->m_pPacked->m_CountSend > 1 ) 
	{
      m_vecQCycleOut[ arcF()->m_pPacked->m_Number ]->m_CountSend++;
	  return 1;
	}

   CSingleLock lock2( &m_mtxCycleOut, TRUE );

   if( m_vecQCycleOut[m_bOutNumber] ) delete m_vecQCycleOut[ m_bOutNumber ];
   m_vecQCycleOut[ m_bOutNumber ] = arcF()->m_pPacked;
   arcF()->m_FlOwnPack = false;

   m_bOutNumber++;

   return 1;
 }

void TPhysicalLevel::StartChannels()
 {
   CSingleLock lock( &m_mtxLstChannels, TRUE );

   list<TChannel*>::iterator itSta( m_lstChannels.begin() );
   list<TChannel*>::iterator itEnd( m_lstChannels.end() );
                                          
   for( ; itSta != itEnd; ++itSta )
	 (*itSta)->StartWorkCycle();
 }

int TPhysicalLevel::WaitReady( CMultiLock& mLock )
 {
   //CSyncObject* sync[2] = { &m_evPortInitComplete, &m_evCancel };
   DWORD dwRes = mLock.Lock( INFINITE, FALSE );   
   if( dwRes >= WAIT_OBJECT_0 && dwRes <= WAIT_OBJECT_0 + 1 )
    dwRes -= WAIT_OBJECT_0;
   else if( dwRes >= WAIT_ABANDONED_0 && dwRes <= WAIT_ABANDONED_0 + 1 )
    dwRes = -1;
 
   if( dwRes == -1 || dwRes == 1 ) return -1;
    
   return 1;
 }

bool TPhysicalLevel::CheckBroken()
 {
   return m_bStop == true && 
	 abs(GetTickCount() - m_dwStopTime) > m_rOpt.d.m_dwTimeOutBrokenStop;
 }


int TRLSDWait::operator()()
 {      

   if( ::WaitForSingleObject(m_ev, 0) != WAIT_TIMEOUT  )
	{
      m_ev.ResetEvent();
      memset( &m_ov, 0, sizeof(OVERLAPPED) );   
      m_ov.hEvent = m_ev;   

	  m_mask = 0;
      WaitCommEvent( m_h, &m_mask, &m_ov );

      //DWORD dw;
	  //GetCommModemStatus( m_h, &dw );
	
	  //int ii = (dw & MS_RLSD_ON);
	  
	  //return !(dw & MS_RLSD_ON);
	  return 1;
	}
   return 0;
 }

void TRLSDWait::Clear()
 {
   if( m_h != INVALID_HANDLE_VALUE )
	{
	  PurgeComm( m_h, m_ab );
	  m_h = INVALID_HANDLE_VALUE;
	}
 }

TRLSDWait::~TRLSDWait()
 {
   Clear();
 }

void TRLSDWait::Init( HANDLE hn )
 {
   m_h = hn;
   m_ev.ResetEvent();
   memset( &m_ov, 0, sizeof(OVERLAPPED) );   
   m_ov.hEvent = m_ev;   

   WaitCommEvent( m_h, &m_mask, &m_ov );
 }



int TPhysicalLevel::ReadPacket( BYTE*& rpPacket, bool bDSR  )
 {
   rpPacket = NULL;

   OVERLAPPED ov; memset( &ov, 0, sizeof(OVERLAPPED) );   
   ov.hEvent = m_evOV;
   m_evOV.ResetEvent();   

   int iWaitRes = m_waitR();
   if( bDSR == true && iWaitRes ) return 4;
   int iInd = (bDSR == true) ? 4:3;
   
   /*evCOMM_RD.ResetEvent();
   OVERLAPPED ovEV; memset( &ovEV, 0, sizeof(OVERLAPPED) );   
   ovEV.hEvent = evCOMM_RD;   
   DWORD dwEvMask;
   if( bDSR == true ) WaitCommEvent( m_fFilePort, &dwEvMask, &ovEV );*/


   CSyncObject* sync[4] = { &m_evCancelConnect, &m_evCancel, &m_evOV, m_waitR.EV() };
   CMultiLock mLock( sync, iInd );
   iInd--;

   TPacket *pPck;
   BYTE *pDta;
   TAutoMemoryRc_AR<BYTE*> arc( CrePck(MAX_PACKETDATA_SIZE, pPck, pDta) );

   TSignalHold holder( &m_rIndIn, true );

   while(1)
	{
      DWORD dwRd;
	  BYTE bRead = 0;
      ReadFile( m_fFilePort, &bRead, 1, &dwRd, &ov );	  
	  

      DWORD dwRes = mLock.Lock( INFINITE, FALSE );   
      if( dwRes >= WAIT_OBJECT_0 && dwRes <= WAIT_OBJECT_0 + iInd )
        dwRes -= WAIT_OBJECT_0;
      else if( dwRes >= WAIT_ABANDONED_0 && dwRes <= WAIT_ABANDONED_0 + iInd )
        dwRes = -1;
	  
	  if( dwRes == 3 ) 
	   {
	     PurgeComm( m_fFilePort, PURGE_RXABORT );

	     //if( bDSR == false ) continue; 
		 m_waitR();

		 //DWORD dwR;
		 //GetCommModemStatus( m_fFilePort, &dwR );
		 //if( dwR & MS_RLSD_ON ) continue;
		 //CancelIo(m_fFilePort);
		 
		 return 4;
	   }
	  	  
 
      if( dwRes == -1 || dwRes == 1 ) 
	   {
	     PurgeComm( m_fFilePort, PURGE_RXABORT ); 
		 return -1;
	   }
      if( !dwRes ) 
	   {
	     PurgeComm( m_fFilePort, PURGE_RXABORT ); 
		 return 0;
	   }

	  BOOL bRDRes = GetOverlappedResult( m_fFilePort, &ov, &dwRd, FALSE );
	  if( !bRDRes ) 
	   {
	     DWORD err = ::GetLastError();
		 PurgeComm(m_fFilePort, PURGE_RXABORT); 
	     if( err == ERROR_IO_INCOMPLETE ) continue;		  

	     CFileException::ThrowOsError( err, NULL );
	   }

	  if( dwRd != 1 ) 
	   {	     
		 //PurgeComm( m_fFilePort, PURGE_RXABORT );
		 continue;
	   }
	  if( bRead != CHAR_BEGINPACKET ) 
	   {
	     if( bDSR == false ) continue;
	     //PurgeComm( m_fFilePort, PURGE_RXABORT );
	     //continue;
		 return 1;
	   }


	  memset( &ov, 0, sizeof(OVERLAPPED) );   
      ov.hEvent = m_evOV;
      m_evOV.ResetEvent();
      ReadFile( m_fFilePort, arc(), sizeof(TPacket)-1, &dwRd, &ov );	  
      

	  dwRes = mLock.Lock( INFINITE, FALSE );   
      if( dwRes >= WAIT_OBJECT_0 && dwRes <= WAIT_OBJECT_0 + iInd )
        dwRes -= WAIT_OBJECT_0;
      else if( dwRes >= WAIT_ABANDONED_0 && dwRes <= WAIT_ABANDONED_0 + iInd )
        dwRes = -1;
	  
	  if( dwRes == 3 ) 
	   {
	     PurgeComm( m_fFilePort, PURGE_RXABORT );

	     //if( bDSR == false ) continue; 
		 m_waitR();

		 //DWORD dwR;
		 //GetCommModemStatus( m_fFilePort, &dwR );
		 //if( dwR & MS_RLSD_ON ) continue;
		 //CancelIo(m_fFilePort);
		 
		 return 4;
	   }

	  //if( CheckBroken() == true ) return 4;	  

	  if( dwRes == -1 || dwRes == 1 ) 
	   {
	     PurgeComm( m_fFilePort, PURGE_RXABORT ); 
		 return -1;
	   }
      if( !dwRes ) 
	   {
	     PurgeComm( m_fFilePort, PURGE_RXABORT ); 
		 return 0;
	   }

	  bRDRes = GetOverlappedResult( m_fFilePort, &ov, &dwRd, FALSE );
	  if( !bRDRes ) 
	   {
	     DWORD err = ::GetLastError();
		 PurgeComm(m_fFilePort, PURGE_RXABORT); 
	     if( err == ERROR_IO_INCOMPLETE ) continue;		  

	     CFileException::ThrowOsError( err, NULL );
	   }

      if( dwRd != sizeof(TPacket)-1 ) 
	   {
	     PurgeComm( m_fFilePort, PURGE_RXABORT );
	     continue;
	   }

	  BYTE bCRCOld;
      BYTE bCRC = CalcCRCHdr( pPck, bCRCOld );
	  if( bCRC != bCRCOld ) 
	   {
	     PurgeComm( m_fFilePort, PURGE_RXABORT );
	     return 1;
	   }

      if( !pPck->m_Size ) 
	   {
         rpPacket = (BYTE*)pPck; arc.ClearRc();
		 return 3;
	   }

	  memset( &ov, 0, sizeof(OVERLAPPED) );   
      ov.hEvent = m_evOV;
      m_evOV.ResetEvent();
      ReadFile( m_fFilePort, pDta, pPck->m_Size, &dwRd, &ov );	  
      

	  dwRes = mLock.Lock( INFINITE, FALSE );   
      if( dwRes >= WAIT_OBJECT_0 && dwRes <= WAIT_OBJECT_0 + iInd )
        dwRes -= WAIT_OBJECT_0;
      else if( dwRes >= WAIT_ABANDONED_0 && dwRes <= WAIT_ABANDONED_0 + iInd )
        dwRes = -1;
	  
	  if( dwRes == 3 ) 
	   {
	     PurgeComm( m_fFilePort, PURGE_RXABORT );

	     //if( bDSR == false ) continue; 
		 m_waitR();

		 //DWORD dwR;
		 //GetCommModemStatus( m_fFilePort, &dwR );
		 //if( dwR & MS_RLSD_ON ) continue;
		 //CancelIo(m_fFilePort);
		 
		 return 4;
	   }

	  //if( CheckBroken() == true ) return 4;
	  
	  if( dwRes == -1 || dwRes == 1 ) 
	   {
	     PurgeComm( m_fFilePort, PURGE_RXABORT ); 
		 return -1;
	   }
      if( !dwRes ) 
	   {
	     PurgeComm( m_fFilePort, PURGE_RXABORT ); 
		 return 0;
	   }

	  bRDRes = GetOverlappedResult( m_fFilePort, &ov, &dwRd, FALSE );
	  if( !bRDRes ) 
	   {
	     DWORD err = ::GetLastError();
		 PurgeComm(m_fFilePort, PURGE_RXABORT); 
	     if( err == ERROR_IO_INCOMPLETE ) continue;		  

	     CFileException::ThrowOsError( err, NULL );
	   }
	  
	  if( !dwRd ) 
	   {
	     PurgeComm( m_fFilePort, PURGE_RXABORT );
	     continue;
	   }
      if( dwRd != pPck->m_Size ) 
	   {
	     PurgeComm( m_fFilePort, PURGE_RXABORT );
	     return 2;
	   }

      bCRC = CalcCRCAll( pPck, bCRCOld, SZ_P(pPck) );
	  if( bCRC != bCRCOld ) 
	   {
	     PurgeComm( m_fFilePort, PURGE_RXABORT );
	     return 2;
	   }

	  if( m_rOpt.d.m_iUsePoly && DecodePacket(pPck) == false ) return 2;
	   

	  rpPacket = (BYTE*)pPck; arc.ClearRc();


      /*TPacket* pPckTST = (TPacket*)new BYTE[ SZ_P((TPacket*)rpPacket) ];
      memcpy( pPckTST, rpPacket, SZ_P((TPacket*)rpPacket) );
      m_lst_TESTIn.push_back( pPckTST ); 	  */

	  return 3;
	}
 }

int TPhysicalLevel::WaitCnctPackets()
 {                  //DumpTEST( m_lst_TESTIn, "F:\\dump.in" );  
   int iReq = 0;
   int iOK = 0;
   int iOK2 = 0;

   while( 1 )
	{   
	  TPacket *pPck;
      int iRes; 
	  try {
	    iRes = ReadPacket( (BYTE*&)pPck, false );	  
	   }
	  catch( CException* pE )
	   {
	     MyReportErr( pE );
         pE->Delete();      
		 m_evEnableConnect.ResetEvent();
		 m_evCancelConnect.SetEvent();
		 m_evPortInitComplete.ResetEvent();
	     return -1;
	   }
	  TAutoMemoryRc_AR<BYTE*> arc( (BYTE*)pPck );
	  if( !iRes ) return 0;
	  if( iRes < 3 ) continue;

	  bool b1, b2, b3;
	  if( (b1=pPck->m_Type == TT_ConnectReqvest) ||
	      (b2=pPck->m_Type == TT_ConnectOK) ||
		  (b3=pPck->m_Type == TT_ConnectOK2)
        )
	   {	              
         CSingleLock lock( &m_mtxInQ, TRUE );

		 /*if( m_lstInQueue.size() ) 
		  {
            int b = 0;
			if( b == 1 )
			 {
               //list<TPacket*> m_lst_TESTIn, m_lst_TESTOut;  
               DumpTEST( m_lstInQueue, "F:\\in.in" );
			 }
			continue;
		  }*/
		 if( m_lstInQueue.size() ) 
		  {
		    BYTE bT = (*m_lstInQueue.begin())->m_Type;

			if( bT == TT_ConnectOK2 ) b3 = true;
			else if( bT == TT_ConnectOK ) b2 = true;
			else b1 = true;//TT_ConnectReqvest                      
		  }
		 else
		  {
		    arc.ClearRc();
		    m_lstInQueue.push_back( (TPacket*)pPck );
		  }

		 char cBuf[100];
		 if( b1 == true ) iReq++; 
		 else if( b2 == true ) iOK++; 
		 else if( b3 == true ) iOK2++;

		 sprintf(cBuf, "Req:%d OK:%d OK2:%d", iReq, iOK, iOK2 );
		 //m_rStatus.SetPaneText( 2, cBuf );         
		 DWORD dw;
		 ::SendMessageTimeout( m_rStatus.GetSafeHwnd(), WM_SETTEXT, 
		   0, (LPARAM)cBuf,
	       SMTO_NORMAL, 500, &dw );	        

		 if( b1 == true ) m_evConnectReqvest.SetEvent();		 
		 else if( b2 == true ) 
		  {
		    m_iCnctIndex = 1;
		    
			//PurgeComm( m_fFilePort, PURGE_RXCLEAR );
			m_evConnectOKInt.SetEvent();
			//Sleep( 1000 );
			//return 1; 
			LtsActive();
			return 1;
		  }
		 else if( b3 == true ) 
		  {
            m_iCnctIndex = 2;
		    
			//PurgeComm( m_fFilePort, PURGE_RXCLEAR );
			m_evConnectOK2.SetEvent();
			//Sleep( 500 );
			LtsActive();
			return 1;
		  }
	   }	  
	}
 }

void TPhysicalLevel::LtsActive()
 {
   TPacket *pPck;
   TConnectReq *pReq;
   TAutoMemoryRc_AR<BYTE*> arc( CrePck(sizeof(TConnectReq), pPck, (BYTE*&)pReq) );

   pPck->m_NumbChannel = NUMB_CHANNEL_SYS;
   pPck->m_Type = TT_LetsActive;   
   pPck->m_CountSend = 0;   
   
   try {
     SendPacket( pPck, false );   
	}
   catch( CException* pE )
	{ 
	  pE->Delete();
	}
 }

int TPhysicalLevel::WaitActive()
 {
   DWORD dwTim = GetTickCount();

   LtsActive();
   //LtsActive();
   //LtsActive();

   while(1)
	{

    TPacket *pPck;
    int iRes; 
    try {		    
	 iRes = ReadPacket( (BYTE*&)pPck, false );	  
	 }
	catch( CException* pE )
	 {		    
	   MyReportErr( pE );
       pE->Delete();      

	   m_evEnableConnect.ResetEvent();
	   m_evCancelConnect.SetEvent();
	   m_evPortInitComplete.ResetEvent();
	   m_evConnectOK.ResetEvent();
	   m_evConnectBroken.SetEvent();	  
	   return -1;
	 }
		 
	TAutoMemoryRc_AR<BYTE*> arc( (BYTE*)pPck );

    if( !iRes ) return 0;
	if( iRes < 3 ) continue;

	if( iRes == 4 || pPck->m_Type != TT_LetsActive )
	 {
	   //if( pPck->m_NumbChannel == NUMB_CHANNEL_SYS ) return 0;

	   if( abs(dwTim - GetTickCount()) < m_rOpt.d.dwTOWaitLA ) 
		{
		  LtsActive();
		  continue;
		}
	   return 0;
	 }	
	else 
	 {
	   LtsActive();
       m_evLetsActive.SetEvent();
	   return 1;
	 }

	}
 }

void TPhysicalLevel::ResetEvents()
 {
   m_evEnableConnect.ResetEvent();
   m_evCancelConnect.SetEvent();
   m_evPortInitComplete.ResetEvent();
   m_evConnectOK.ResetEvent();
   m_evConnectBroken.SetEvent();	  
 }

void TPhysicalLevel::OffLamps()
 {
   m_rIndCnct.SetStOn( false, true );
   m_rIndIn.SetStOn( false, true );
   m_rIndOut.SetStOn( false, true );
 }

void TPhysicalLevel::WorkCycleIn()
 {
   CSyncObject* sync[2] = { &m_evPortInitComplete, &m_evCancel };
   CMultiLock mLock( sync, 2 );

   while(1)
	{     
	  m_evLetsActive.ResetEvent();
	  m_evConnectOK.ResetEvent();	  
	  m_evConnectBroken.SetEvent();	  
	  

	  CSingleLock lockIn( &m_mtxInQ, TRUE );
 	  ClearInQ();
	  lockIn.Unlock();

	  OffLamps();	  

      int iRes1 = WaitReady( mLock );
	  if( iRes1 == -1 ) return;

      iRes1 = WaitCnctPackets();
	  if( iRes1 == -1 ) return;
	  if( !iRes1 ) continue;

      //CSingleLock lockIn2( &m_mtxInQ, TRUE );
	  //ClearInQSys();
	  //lockIn2.Unlock();

	  m_evConnectOK.SetEvent();
	  m_evConnectBroken.ResetEvent();

	  m_bInNumber = 0;
	  m_bStop = false;
	  m_dwStopTime = 0;	  

	  m_dwErrCount = 0;
	  int iWRes = WaitActive();

	  if( iWRes == -1 ) return;
	  if( !iWRes ) continue;


	  while(1)
	   {
         TPacket *pPck;
         int iRes; 
	     try {		    
	       iRes = ReadPacket( (BYTE*&)pPck, true );	  
		  }
	     catch( CException* pE )
		  {		    
	        MyReportErr( pE );
            pE->Delete();      

		    m_evEnableConnect.ResetEvent();
		    m_evCancelConnect.SetEvent();
		    m_evPortInitComplete.ResetEvent();
		    m_evConnectOK.ResetEvent();
	        m_evConnectBroken.SetEvent();	  
	        return;
		  }
		 

	     TAutoMemoryRc_AR<BYTE*> arc( (BYTE*)pPck );
	     if( !iRes ) break;
	     if( iRes < 3 && m_bStop == false ) RepeatReqvest( m_bInNumber );
		 else if( iRes == 3 )
		  {
		    if(  IsSysStartPck((TPacket*)arc()) == true ) 
			 {
			   //if( bSYSDisabled == true ) continue;
			   m_evPortInitComplete.ResetEvent();
			   break;
			 }

			if( ((TPacket*)arc())->m_Type == TT_LetsActive ) continue;
			
		    if( m_bStop == false || ((TPacket*)arc())->m_Number == m_bInNumber + 1 )
			 {
			   m_bStop = false;
               BYTE* p = arc(); arc.ClearRc();
			   m_bInNumber = ((TPacket*)p)->m_Number;
			   QueuedPck( p );
			 }
		    else if( ((TPacket*)arc())->m_Type == TT_QSendAgain ) 
			 {
		       BYTE* p = arc(); arc.ClearRc();
		       MakeSendAgain( p );
			 }			
		  }
		 else
		 if( iRes == 4 )
		  {
		    MessaBoxBroken( BR_Break );
			m_evPortInitComplete.ResetEvent();
            break;
		  }
		 else 
		  { 
		    if( CheckBroken() == true )
			 {
               MessaBoxBroken( BR_TimOut );
			   m_evPortInitComplete.ResetEvent();
			   break;
			 }
		    m_dwErrCount++; RefreshErrInd(); 
		  }
	   }
	}
 }

void TPhysicalLevel::RefreshErrInd()
 {
   if( m_rOpt.d.m_bShowErr == true )
     m_pMFWnd->DispBCE( m_iBaud, m_dwCPS, m_dwErrCount );	
 }

bool TPhysicalLevel::IsSysStartPck( TPacket* pPck )
 {
     return pPck->m_NumbChannel == NUMB_CHANNEL_SYS && 
	        (pPck->m_Type == TT_ConnectReqvest || 
	         pPck->m_Type == TT_ConnectOK ||
             pPck->m_Type == TT_ConnectOK2);
 }

void TPhysicalLevel::RepeatReqvest( BYTE bInNumber )
 {
   m_dwErrCount++;
   RefreshErrInd();

   TPacket *pPck;
   TSendAgain* pData;	  
   TAutoMemoryRc_AR<BYTE*> arc( 
	 CrePck(sizeof(TSendAgain), pPck, (BYTE*&)pData) );

   m_bStop = true;
   m_dwStopTime = GetTickCount();

   pPck->m_NumbChannel = NUMB_CHANNEL_SYS;
   pPck->m_Type = TT_QSendAgain;
   pData->m_NumbPacket = bInNumber + 1;

   arc.ClearRc();
   DWORD dw;
   TAutoMemoryRc<TPackedFrame*> arcF(
	  new TPackedFrame(pPck, dw) );

   CSingleLock lock( &m_mtxOutQ, TRUE );

   TPackedFrame *pF = arcF(); arcF.ClearRc();
   AddOutPacket( pF, true );	  
 }

void TPhysicalLevel::MakeSendAgain_Internal( BYTE ind )
 {                          
   if( m_vecQCycleOut[ind] )
	{
      TPacket *pPckAg;
      BYTE* pData;
	  int iSz = SZ_P( m_vecQCycleOut[ind] );
      TAutoMemoryRc_AR<BYTE*> arc( CrePck(iSz, pPckAg, pData) );
	  memcpy( arc(), m_vecQCycleOut[ind], iSz );

	  arc.ClearRc();
	  DWORD dw;
	  TAutoMemoryRc<TPackedFrame*> arcF(
	    new TPackedFrame(pPckAg, dw) );
	  
	  TPackedFrame *pF = arcF(); arcF.ClearRc();
	  AddOutPacket( pF, true );	  
	}
 }

void TPhysicalLevel::MakeSendAgain( BYTE* p )
 {                                           
   CSingleLock lockTr( &m_mtxTransmit, TRUE );

   TPacket *pPck = (TPacket*)p;
   CSingleLock lock( &m_mtxCycleOut, TRUE );
   CSingleLock lock2( &m_mtxOutQ, TRUE );

   TSendAgain *pAgain = (TSendAgain *)pPck->m_Data;
   BYTE bSended = m_bOutNumber + 1;
   
   BYTE bSnd = pAgain->m_NumbPacket;
   for( ;bSnd != bSended; bSnd++ )
     MakeSendAgain_Internal( bSnd );
 }

void TPhysicalLevel::QueuedPck( BYTE* p )
 {
   TAutoMemoryRc_AR<BYTE*> arc( p );
   CSingleLock lock( &m_mtxInQ, TRUE );
   CSingleLock lock2( &m_mtxLstChannels, TRUE );   

   TPacket* pPck = (TPacket*)p; arc.ClearRc();

   if( pPck->m_Type == TT_QSendAgain ) MakeSendAgain( p );	
   else
	{
      m_lstInQueue.push_back( pPck );
   
      list<TChannel*>::iterator itSta( m_lstChannels.begin() );
      list<TChannel*>::iterator itEnd( m_lstChannels.end() );
                                          
      for( ; itSta != itEnd; ++itSta )
        if( (*itSta)->GetNumb() == pPck->m_NumbChannel )
		 {
           (*itSta)->m_evInQueueHavePacket.SetEvent();
		   break;
		 }
	}

   //TPacket* pPckTST = (TPacket*)new BYTE[ SZ_P(pPck) ];
   //memcpy( pPckTST, pPck, SZ_P(pPck) );
   //m_lst_TESTIn.push_back( pPckTST );
 }

void TPhysicalLevel::OpenCOMM()
 {
   if( m_fFilePort != INVALID_HANDLE_VALUE ) CloseCOMM(); 

   char cBuf[10];
   sprintf( cBuf, "COM%d", (int)(m_rOpt.d.m_iCOMPort) );
   m_fFilePort = CreateFile( cBuf,
	 GENERIC_READ | GENERIC_WRITE,
	 0,
	 NULL,
	 OPEN_EXISTING,
	 FILE_FLAG_OVERLAPPED,
	 NULL );

   if( m_fFilePort == INVALID_HANDLE_VALUE )
	 CFileException::ThrowOsError( ::GetLastError(), cBuf );	

   m_arcTimKey = (COMMTIMEOUTS*)new BYTE[ sizeof(COMMTIMEOUTS) ];
   if( !GetCommTimeouts(m_fFilePort, m_arcTimKey()) )
     m_arcTimKey.FreeRc();
		 		 
   dwCfgSz = 0;		 
		 
   if( GetCommConfig(m_fFilePort, NULL, &dwCfgSz) )
	{
      m_arcCfgKey = (COMMCONFIG*)new BYTE[ dwCfgSz ];
	  m_arcCfgKey()->dwSize = dwCfgSz;
	  if( !GetCommConfig(m_fFilePort, m_arcCfgKey(), &dwCfgSz) )
	    m_arcCfgKey.FreeRc();
	  else
	   {
         TAutoMemoryRc_AR<COMMCONFIG*> arcT = (COMMCONFIG*)new BYTE[ dwCfgSz ];
         memcpy( arcT(), m_arcCfgKey(), dwCfgSz );

		 DCB &rDCB = arcT()->dcb;

		 rDCB.BaudRate = TranslateBaud( m_rOpt.d.m_iBaud );
		 rDCB.fParity = FALSE;
		 rDCB.fOutxCtsFlow = TRUE;
		 //rDCB.fOutxCtsFlow = FALSE;
		 rDCB.fOutxDsrFlow = FALSE;
		 //rDCB.fOutxDsrFlow = TRUE;
		 rDCB.fDtrControl = DTR_CONTROL_ENABLE;		 
		 rDCB.fDsrSensitivity = FALSE;
		 //rDCB.fDsrSensitivity = TRUE;

		 rDCB.fTXContinueOnXoff = FALSE;
		 rDCB.fOutX = FALSE;
		 rDCB.fInX = FALSE;
		 rDCB.fErrorChar = FALSE;
		 rDCB.fNull = FALSE;

		 rDCB.fRtsControl = RTS_CONTROL_TOGGLE;
		 //rDCB.fRtsControl = RTS_CONTROL_HANDSHAKE;

		 rDCB.fAbortOnError = FALSE;
		 rDCB.XonLim = 65535;
		 rDCB.XoffLim = 65535;
		 rDCB.ByteSize = 8;
		 rDCB.Parity = NOPARITY;
		 //rDCB.StopBits = ONESTOPBIT;
		 rDCB.StopBits = TranslateStop( m_rOpt.d.m_iStops );

		 rDCB.XonChar = 17;
		 rDCB.XoffChar = 19;
		 rDCB.ErrorChar = 0;
		 rDCB.EofChar = 0;

		 rDCB.EvtChar = 0;		  

		 BOOL bCRes = SetCommConfig( m_fFilePort, arcT(), dwCfgSz );

         SetupTimeouts( m_fFilePort, rDCB.BaudRate );

		 /*TAutoMemoryRc_AR<COMMTIMEOUTS*> arcT2 = (COMMTIMEOUTS*)new BYTE[ sizeof(COMMTIMEOUTS) ];
		 //memcpy( arcT2(), m_arcTimKey(), sizeof(COMMTIMEOUTS) );
		 memset( arcT2(), 0, sizeof(COMMTIMEOUTS) );
		 //arcT2()->ReadIntervalTimeout = 6;
		 BOOL bFl = SetCommTimeouts( m_fFilePort, arcT2() );*/

		 //SetupComm( m_fFilePort, 10*(MAX_PACKET_SIZE + 50), 5*(MAX_PACKET_SIZE + 50) );
		 SetupComm( m_fFilePort, 20*(MAX_PACKET_SIZE + 50), 0 ); //////!!!!!!!!!!!!!!!
	   }
	}

   GetCommMask( m_fFilePort, &m_dwKeepMask );
   SetCommMask( m_fFilePort, EV_RLSD );

   m_waitR.Init( m_fFilePort );
   //m_waitW.Init( m_fFilePort );

   //PurgeComm( m_fFilePort, PURGE_RXCLEAR );
 }

void TPhysicalLevel::CloseCOMM()
 {
   m_waitR.Clear();
   m_waitW.Clear();

   if( m_fFilePort != INVALID_HANDLE_VALUE )
	{
      m_evPortInitComplete.ResetEvent();

      SetCommMask( m_fFilePort, m_dwKeepMask );
	  PurgeComm( m_fFilePort, PURGE_RXABORT|PURGE_TXABORT );	  

      if( m_arcCfgKey() ) SetCommConfig( m_fFilePort, m_arcCfgKey(), dwCfgSz );
	  if( m_arcTimKey() ) SetCommTimeouts( m_fFilePort, m_arcTimKey() );

      CloseHandle( m_fFilePort );
	  m_fFilePort = INVALID_HANDLE_VALUE;	  
	}
 }

int TPhysicalLevel::GetCBRVal( int BaudRate )
 {
   const int iNumbBPS = 20;
   static const TBaudData2 iBauds[iNumbBPS] =
	{
	 {110,     CBR_110},

	  {110,    CBR_110},
	  {110,  CBR_110},

	  {300,    CBR_300},
	  {300,    CBR_300},
	  {600,    CBR_600},
	  {1200,   CBR_1200},
	  {1200,   CBR_1200},
	  {2400,   CBR_2400},
	  {4800,   CBR_4800},
	  {9600,   CBR_9600},
	  {9600,   CBR_9600},
	  {14400,  CBR_14400},

	  {19200,  CBR_19200},
	  {38400,  CBR_38400},
	  {56000,    CBR_56000},
	  {57600,  CBR_57600},

	  {115200, CBR_115200},
	  {128000,   CBR_128000},
	  {256000,   CBR_256000},
	};

   int iVal = 110;
   for( int i = 0; i < iNumbBPS; ++i )
	 if( iBauds[i].m_iManemonic2 == BaudRate ) 
	  {
        iVal = iBauds[i].m_iManemonic;
		break;
	  }

	return iVal;
 }

void TPhysicalLevel::SetupTimeouts( HANDLE fFilePort, int BaudRate )
 {   
   double dTime1 = ceil( 14 / double(GetCBRVal(BaudRate)) * 1000.0 );

   COMMTIMEOUTS cms; 
   memset( &cms, 0, sizeof(COMMTIMEOUTS) );
  // dTime1 *= 2;

   /*cms.ReadIntervalTimeout = 0;
   cms.ReadTotalTimeoutMultiplier = dTime1;
   cms.ReadTotalTimeoutConstant = 10*dTime1;

   cms.WriteTotalTimeoutMultiplier = dTime1;
   cms.WriteTotalTimeoutConstant = 10*dTime1;*/

   SetCommTimeouts( fFilePort, &cms );
 }
