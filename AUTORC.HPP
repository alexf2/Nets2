#if !defined(TAUTORC)
#define TAUTORC

#include "stdafx.h"
#include <lzexpand.h>
  
template<class T> class   TAutoRc
 {
public:

  TAutoRc() {};
  TAutoRc( T tRc_ ) { tRc = tRc_; }
  virtual ~TAutoRc() {};

  TAutoRc& operator=( TAutoRc& rRc )
	{
	  FreeRc();
	  tRc = CopyRc( rRc.tRc );
	  return *this;
	}

  virtual void FreeRc() = 0;
  int operator!() { return !tRc; }
  T operator()() const { return tRc; }
  
  virtual T CopyRc( T& rT ) const
   {
     return rT;
	}

protected:
  T tRc;
 };


template<class T> class TAutoGDIRc: public TAutoRc<T>
 {
public:
  TAutoGDIRc() { tRc = NULL; }
  TAutoGDIRc( T tRc_ ): TAutoRc<T>( tRc_ ) {}

  virtual ~TAutoGDIRc()
   {
     if( !tRc ) return;
     DeleteObject( tRc ); tRc = NULL;
   }

   TAutoGDIRc& operator=( T tRc_ )
   {
     FreeRc();
     tRc = tRc_;
     return *this;
   }

  virtual void FreeRc();
 };

template<class T> void TAutoGDIRc<T>::FreeRc()
 {
   if( tRc ) { DeleteObject( tRc ); tRc = NULL; }
 }


class TAutoCursorRc: public TAutoRc<HCURSOR>
 {
public:
  TAutoCursorRc() { tRc = NULL; }
  TAutoCursorRc( HCURSOR tRc_ ): TAutoRc<HCURSOR>( SetCursor(tRc_) ) {}

  virtual ~TAutoCursorRc()
   {
     if( !tRc ) return;
     SetCursor( tRc ); tRc = NULL;
   }

   TAutoCursorRc& operator=( HCURSOR tRc_ )
	{
     FreeRc();
     tRc = SetCursor( tRc_ );
     return *this;
   }

  virtual void FreeRc();
 };


template<class T> class TAutoMemoryRc: public TAutoRc<T>
 {
public:
  TAutoMemoryRc() { tRc = NULL; }
  TAutoMemoryRc( T tRc_ ): TAutoRc<T>( tRc_ ) {}

  virtual ~TAutoMemoryRc()
   {
     if( !tRc ) return;
     delete tRc; tRc = NULL;
   }

  TAutoMemoryRc& operator=( TAutoMemoryRc& rRc )
	{
     FreeRc();
     tRc = CopyRc( rRc.tRc );
     return *this;
   }


  TAutoMemoryRc& operator=( T tRc_ )
   {
     FreeRc();
     tRc = tRc_;
     return *this;
   }

  virtual void FreeRc();

  void ClearRc() { tRc = NULL; }
 };

template<class T> void TAutoMemoryRc<T>::FreeRc()
 {
   if( tRc ) { delete tRc; tRc = NULL; }
 }


template<class T> class TAutoMemoryRc_AR: public TAutoRc<T>
 {
public:
  TAutoMemoryRc_AR() { tRc = NULL; }
  TAutoMemoryRc_AR( T tRc_ ): TAutoRc<T>( tRc_ ) {}

  virtual ~TAutoMemoryRc_AR()
   {
     if( !tRc ) return;
     delete[] tRc; tRc = NULL;
   }

  TAutoMemoryRc_AR& operator=( TAutoMemoryRc_AR& rRc )
	{
     FreeRc();
     tRc = CopyRc( rRc.tRc );
     return *this;
   }


  TAutoMemoryRc_AR& operator=( T tRc_ )
   {
     FreeRc();
     tRc = tRc_;
     return *this;
   }

  virtual void FreeRc();

  void ClearRc() { tRc = NULL; }
 };

template<class T> void TAutoMemoryRc_AR<T>::FreeRc()
 {
   if( tRc ) { delete[] tRc; tRc = NULL; }
 }


class TAutoMemoryRc_Win: public TAutoRc<HGLOBAL>
 {
public:
  TAutoMemoryRc_Win() { tRc = NULL; m_iLock = 0; }
  TAutoMemoryRc_Win( HGLOBAL tRc_ ): TAutoRc<HGLOBAL>( tRc_ ) 
   { m_iLock = 0; }

  virtual ~TAutoMemoryRc_Win();  

  TAutoMemoryRc_Win& operator=( TAutoMemoryRc_Win& rRc )
   {
     FreeRc();
     tRc = CopyRc( rRc.tRc );
	 m_iLock = rRc.m_iLock;

     return *this;
   }


  TAutoMemoryRc_Win& operator=( HGLOBAL tRc_ )
   {
     FreeRc();
     tRc = tRc_;
	 m_iLock = 0;

     return *this;
   }

  virtual void FreeRc();
  void ClearRc() { tRc = NULL; m_iLock = 0; }

  LPVOID Lock() { m_iLock++; return GlobalLock(tRc); } 
  int Unlock();

private:
  int m_iLock;
 };


template<class T> class TAutoFileRc: public TAutoRc<T>
 {
public:
  TAutoFileRc() { tRc = 0; }
  TAutoFileRc( T tRc_ ): TAutoRc<T>( tRc_ ) {}

  virtual ~TAutoFileRc()
	{
	  if( tRc == HFILE_ERROR || !tRc ) return;
	  _lclose( tRc ); tRc = 0;
	}

  TAutoFileRc& operator=( T tRc_ )
	{
	  FreeRc();
	  tRc = tRc_;
	  return *this;
	}

	virtual void FreeRc();
 };

template<class T> void TAutoFileRc<T>::FreeRc()
 {
	if( tRc ) { _lclose( tRc ); tRc = 0; }
 }


template<class T> class TAutoFileZRc: public TAutoRc<T>
 {
public:
  TAutoFileZRc() { tRc = 0; }
  TAutoFileZRc( T tRc_ ): TAutoRc<T>( tRc_ ) {}

  virtual ~TAutoFileZRc()
   {
     if( tRc == HFILE_ERROR || !tRc ) return;
     LZClose( tRc ); tRc = 0;
   }

  TAutoFileZRc& operator=( T tRc_ )
   {
     FreeRc();
     tRc = tRc_;
     return *this;
   }


   virtual void FreeRc();
 };

template<class T> void TAutoFileZRc<T>::FreeRc()
 {
   if( tRc ) { LZClose( tRc ); tRc = 0; }
 }

#endif
