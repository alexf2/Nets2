#if !defined(_LOCAL_USER_)
#define _LOCAL_USER_

#include "fsrv.hpp"

int CmpRecords( TFileRecord *p1, TFileRecord *p2 );

class TVFileLocal: public TVirtualFile
 {
public:
  TVFileLocal( LPCSTR lpName, UINT uiAtr ):  
	TVirtualFile( lpName, uiAtr )
	 {
	 }
  virtual ~TVFileLocal();

  virtual void MakeOpen();
  virtual DWORD GetSize();
  virtual DWORD Read( BYTE*, DWORD, TNotifyInstance* pNotify );
  virtual void Write( BYTE*, DWORD, TNotifyInstance* pNotify );
  virtual void Close();
  virtual bool CheckExist();

  virtual void Erase( LPCSTR );
  
private:
  CFile m_File;
 };

class TVSearchLocal: public TVirtualSearch
 {
public:
  TVSearchLocal( LPCSTR lp ): TVirtualSearch( lp )
   {
     lHandl = -1;
   }

  virtual ~TVSearchLocal();

  virtual _finddata_t *GetFirst();
  virtual _finddata_t *Next();

private:
  long lHandl;
 };

class TLocalDrive: public TDrive
 {
public:
  TLocalDrive(): TDrive() {};
  virtual ~TLocalDrive();

  virtual void Init( int iDr, UINT uiType );
  virtual bool IsRemote() const;
  
  virtual void Directory( TUserOfDrive&, vector<TFileRecord*>& );
  virtual void MkDir( LPCSTR lp );
  virtual void RmEmptyDir( LPCSTR, bool bWarn );

  virtual TSecurity* GetSecurityAndAtr( LPCSTR );
  virtual void SetSecurityAndAtr( LPCSTR, TSecurity* );

  virtual DWORD GetBuffSize() const;

  /*virtual void Copy( TUserOfDrive& src, TUserOfDrive& dst, 
                     vector<TFileRecord*>*, 
					 bool bSubDir, bool bMove, TNotifyInstance* pNotify );
  virtual void Erase( TUserOfDrive& src, vector<TFileRecord*>*, 
                      bool bSubDir, TNotifyInstance* pNotify );*/

  void ClearFList( vector<TFileRecord*>& rV );
  void FindFillTmp( LPSTR path_buffer );

private:
  vector<TFileRecord*> m_rvecTmp;

  virtual TVirtualFile* OpenFile( LPCSTR, UINT );
  virtual TVirtualSearch* OpenSearch( LPCSTR );
 };


#endif
