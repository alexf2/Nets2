//#pragma option -v-

#include "autorc.hpp"

void TAutoCursorRc::FreeRc()
 {
	if( tRc ) { SetCursor( tRc ); tRc = NULL; }
 }

void TAutoMemoryRc_Win::FreeRc()
 {
   if( tRc ) 
	{ 
	  while( m_iLock > 0 ) Unlock();
	  GlobalFree( tRc ); tRc = NULL; 
	}
 }

int TAutoMemoryRc_Win::Unlock()
 {
   if( m_iLock > 0 )
	{
      m_iLock--;
	  GlobalUnlock( tRc );
	}

   return m_iLock;
 }

TAutoMemoryRc_Win::~TAutoMemoryRc_Win()
 {
   if( !tRc ) return;

   while( m_iLock > 0 ) Unlock();
   GlobalFree( tRc ); tRc = NULL;
 }
