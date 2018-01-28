#if !defined(_PHYSICAL_)
#define _PHYSICAL_

#include "stdafx.h"
#include "ind.hpp"
#include "cfg.hpp"


#include <afxmt.h>
#include <vector>
#include <list>
using namespace std;
//#include <afxcmn.h>


#define NUMB_CHANNEL_SYS        0
#define NUMB_CHANNEL_SRVLOCAL   1 
#define NUMB_CHANNEL_SRVREMOTE  2

enum T_TypesPacket { TT_ConnectReqvest = 0, TT_ConnectOK = 1, 
                     TT_ConnectOK2 = 2, TT_LetsActive = 3,
                     TT_QSendAgain = 4, TT_DataSta = 13, 
					 TT_Data = 14, TT_DataEnd = 15 };

#define MAX_PACKETDATA_SIZE 249
#define MAX_PACKETDATA_SIZE_POLY 122
#define MAX_PACKET_SIZE 256

#define CHAR_BEGINPACKET 99
struct TPacket
 {
   BYTE m_Number;
   BYTE m_NumbChannel:4;
   BYTE m_Type:4;
   BYTE m_Size;
   BYTE m_CRCHdr;
   BYTE m_CRCAll;
   BYTE m_CountSend;
   BYTE m_Data[1];
 };
BYTE* CrePck( BYTE bSizeData, TPacket*& pPck, BYTE*& pData );
inline BYTE SZ_P( TPacket* pPck )
 {
   return pPck->m_Size + (sizeof(TPacket) - 1);
 }

class TEvHold
 {
public:
  TEvHold( CEvent* pEv, bool bFl );  
  ~TEvHold();   

private:
  bool m_bFl;
  CEvent* m_pEv;  
 };


class TPackedFrame
 {
public:
  TPackedFrame( TPacket* pPacked, DWORD& dwRes, CEvent *pevSended = NULL );    

  virtual ~TPackedFrame();   

  TPacket* m_pPacked; //not owned ?
  bool m_FlOwnPack;

  CEvent *m_pevSended; //not owned
  DWORD& m_dwRes;

  DWORD m_dwTimeStart; //only for last block
  DWORD m_dwSizeBlock; //in bytes
 };

struct TConnectReq
 {
   int m_iBoud;
   int m_iStops;
 };

struct TSendAgain
 {
   BYTE m_NumbPacket;
 };



class TPhysicalLevel;

struct TGenericBlock
 {
   DWORD m_64Size;
 };

class TChannel
 {

friend class TPhysicalLevel;

public:
  TChannel( BYTE bNumb, TPhysicalLevel& rPh, CEvent& rConnectOK );

  virtual ~TChannel();

  void SavetyKill();
  void StartWorkCycle();
  void WorkCycle();

  CMutex* GetReadLock() { return &m_mtxReadUnLocked; }
  CEvent* GetPckEv() { return &m_evInQueueHavePacket; }
  CEvent* GetBlockEv() { return &m_evHaveBlocks; }
  CEvent* GetBrokenEv();
  CEvent* GetEvLetsActive(); 

  LPBYTE ReadBlock( DWORD& dwSize, DWORD& dwRes );
  void WriteBlock( LPBYTE lp, DWORD dwSize, CEvent* pEvEnd, DWORD& dwRes );

  BYTE GetNumb() const { return m_bNumb; }

  int GetCnctIndex() const;
                   //owned        
protected:
  CEvent  m_evInQueueHavePacket;
  void GetPacket();

private:
  TPhysicalLevel& m_rPh;
  TAutoMemoryRc<CWinThread*> m_arcThChannel;
  BYTE     m_bNumb:4;  

  CEvent& m_evConnectOK;

  CMutex m_mtxReadUnLocked;

  CMutex m_mtxIOBusy;  
  list<TGenericBlock*> m_lstBlocks;
  list<TPacket*> m_lstRetrieved;
  CEvent m_evHaveBlocks;
  void FreeRetrList();
  void FreeBlocksList();
  TGenericBlock* PopBlock();

  CEvent m_evCancel;
 };

class TMainFrame;

class TRLSDWait
 {
public:
 TRLSDWait( DWORD dwAb ): m_ev( FALSE, TRUE )
  {
    m_h = INVALID_HANDLE_VALUE;
	m_ab = dwAb;
  }

  ~TRLSDWait();

  void Init( HANDLE );
  void Clear();
  
  int operator()();

  CEvent* EV() { return &m_ev; }

private:
  CEvent m_ev;
  HANDLE  m_h;
  OVERLAPPED m_ov;
  DWORD m_mask;

  DWORD m_ab;
 };


class TPhysicalLevel
 {
public:
  TPhysicalLevel( TMFOptions& rOpt, 
                  CEvent& rEvCnctOK, CEvent& rEvEable, 
				  CEvent& rEvCancelCnct,
				  CEvent& evConnectBroken,
                  TIndicator& rIndCnct, TIndicator& rIndIn, 
				  TIndicator& rIndOut,
				  TStatusWithLamp& rStatus );

  virtual ~TPhysicalLevel();

  void SavetyKill();  
  void StartWorkCycleOut();
  void StartWorkCycleIn();  
  void WorkCycleOut();
  void WorkCycleIn();


  CMutex& GetOutQueueLock()
   {
     return m_mtxOutQ; //!!!
   }                                 //external lock 
  void AddOutPacket( TPackedFrame*, bool bFlFront = false );//need Lock before use !!!
  

  TPacket* GetInPacket( TChannel* );//for channel

  TPhysicalLevel& operator+( TChannel& rCh );
  TPhysicalLevel& operator-( TChannel& rCh );

  CEvent& GetEvCnctOK() { return m_evConnectOK; }
  CEvent& GetEvCnctBroken() { return m_evConnectBroken; }

  CEvent& GetEvLetsActive() { return m_evLetsActive; }

  BYTE PckDataSize() const
   {
     return m_rOpt.d.m_iUsePoly ? MAX_PACKETDATA_SIZE_POLY:MAX_PACKETDATA_SIZE;
   }
  BYTE* CrePck( BYTE bSizeData, TPacket*& pPck, BYTE*& pData );
  
  void OffLamps();
  void ResetEvents();

  TMFOptions& m_rOpt;
  DWORD dwTORead, dwTOWrite;

  int m_iCnctIndex;

private:
  TAutoMemoryRc<CWinThread*> m_arcThIn;
  CEvent m_evPortInitComplete;
  TAutoMemoryRc<CWinThread*> m_arcThOut;

  TAutoMemoryRc_AR<COMMCONFIG*> m_arcCfgKey;
  DWORD dwCfgSz;
  TAutoMemoryRc_AR<COMMTIMEOUTS*> m_arcTimKey;
  HANDLE m_fFilePort;
  void OpenCOMM();
  void CloseCOMM();

  TIndicator &m_rIndCnct, &m_rIndIn, &m_rIndOut;
  TStatusWithLamp& m_rStatus;

  CEvent& m_evEnableConnect;
  CEvent& m_evCancelConnect;
  CEvent& m_evConnectOK;
  CEvent& m_evConnectBroken;
  

  CEvent m_evConnectOKInt;
  CEvent m_evConnectOK2;
  CEvent m_evConnectReqvest;  
  

  CEvent m_evOV;//used by readf
  CEvent m_evOVw;//used by writef
  
  int m_iBaud;
  int m_iStops;
  DWORD m_dwCPS;
  TMainFrame *m_pMFWnd;
  DWORD m_dwErrCount;

  BYTE m_bInNumber;
  BYTE m_bOutNumber;

  __int64 m_64InCount, m_64OutCount, m_64InBytes, m_64OutBytes;

  __int64 m_64BytesOutInSession;
  DWORD m_dwTicksStartOut; //In empty out queue add packet
  DWORD m_dwTicksEndOut;   //End of write and queue is empty

  vector<TPacket*> m_vecQCycleOut; //assign 256 pck: 0-255; - owned TPacked
  list<TPacket*> m_lstInQueue; //- owned TPacked
  list<TPackedFrame*> m_lstOutQueue;
  CEvent m_evHasOutpack;

  void ClearPackets();
  void ClearInQ();
  void ClearOutQ();
  void ClearInQSys();

  CMutex m_mtxInQ, m_mtxOutQ, m_mtxCycleOut;
  CMutex m_mtxTransmit;
  bool CheckPackets( TChannel* );

  CMutex m_mtxLstChannels;
  list<TChannel*> m_lstChannels; //not owned
  void StartChannels();

  CEvent m_evCancel;

  bool m_bStop;
  DWORD m_dwStopTime;
  bool CheckBroken();

  int WaitEnableCnct( CMultiLock& mLock );
  void SendReqvestPacket();
  void SendOKPacket( bool bOK2 );
  int WaitReqvestCnct( CMultiLock& mLock );

  int WaitReady( CMultiLock& mLock );
  int WaitCnctPackets();

  int ReadPacket( BYTE*& rpPacket, bool bDSR = true );
  int SendPacket( TPacket* pPck, bool bDSR = true );
  int TransmitQPacket();

  void SetupTimeouts( HANDLE fFilePort, int BaudRate );
  int GetCBRVal( int BaudRate );

  void MakeSendAgain_Internal( BYTE ind );
  void RepeatReqvest( BYTE bInNumber );
  void QueuedPck( BYTE* p );
  void MakeSendAgain( BYTE* p );

  bool IsSysStartPck( TPacket* pPck );

  
  list<TPacket*> m_lst_TESTIn, m_lst_TESTOut;  
  void DumpTEST( list<TPacket*>& rLst, LPSTR lp );

  void RefreshErrInd();

  void LtsActive();
  int WaitActive();
  CEvent m_evLetsActive;

  //CEvent evCOMM_RD, evCOMM_WR;
  DWORD m_dwKeepMask;
  //OVERLAPPED ovEVR;
  //OVERLAPPED ovEVW;

  TRLSDWait m_waitR, m_waitW;

  TPackedFrame* GetPrioretyPck();
  int m_iNChannelSeqvence;
  
 };


#endif
