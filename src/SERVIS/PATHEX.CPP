#define TPATHEX_DLL
#include "pathex.hpp"

void CreateShortName( LPSTR LpFullName, int iLen,
			          char *pOut )
 {
	char Path[_MAX_PATH];
	int Len;

	//if( LpFileNameShort ) delete []LpFileNameShort; LpFileNameShort = NULL;

	strcpy( Path, LpFullName );
	if( (Len = strlen(LpFullName)) > iLen )
	 {
		LPSTR LpName;
		int   LenName;
		const char *StrIns = "\\...\\";

		for( LpName = LpFullName + Len - 1, LenName = 0;
			  LenName < Len-1 && *(LpName-1) != '\\';
			  LenName++, LpName-- );

		int SizeNeed = iLen - LenName - strlen(StrIns);
		LPSTR LpPrev, LpCur;

		for( LpPrev = LpCur = Path, LenName = 0;
			  LenName <= SizeNeed && *LpCur;
			  LenName++, LpCur++ )
		  if( *LpCur == '\\' ) LpPrev = LpCur;
		*LpPrev = 0;
		strcat( Path, StrIns ); strcat( Path, LpName );
	 }

	//if( (LpFileNameShort = new char[ _fstrlen(Path) + 1 ]) )
	  //_fstrcpy( LpFileNameShort, Path );
	//else { Status = EM_OUTOFMEMORY; return; }
	strcpy( pOut, Path );
 }

void CompressPath( CString& rStr, CDC& rDC, 
				   CRect& rT )
 {
	int iLen;
	CSize szExt( rDC.GetTextExtent(rStr) );
	if( szExt.cx <= rT.Width() ) return;

	float fMult = float(rT.Width()) / float(szExt.cx);
	CString str;
	LPSTR lp = str.GetBuffer( 1024 );	
	iLen = rStr.GetLength();

    if( rStr.Find( '\\' ) == -1 )
	  strcpy( lp, (LPSTR)(LPCSTR)rStr );
	else
	 {
	   while( szExt.cx > rT.Width() )
		{
		  *lp = 0;
		  iLen = float(iLen) * fMult;
		  if( iLen < 1 ) break;
		  CreateShortName( (LPSTR)(LPCSTR)rStr, iLen, lp );
		  szExt = rDC.GetTextExtent( lp, (iLen=strlen(lp)) );
          fMult *= 0.9;
		}
	 }

    str.ReleaseBuffer();

	
	bool bFl = str.GetLength() < 2;
	szExt = rDC.GetTextExtent( lp, (iLen=strlen(lp)) );

	if( szExt.cx > rT.Width() || bFl )
	 {
	   int iN = rStr.GetLength();
	   szExt.cx = INT_MAX;
       while( szExt.cx > rT.Width() && iN > 1 )
		{
		  iN = float(iN) * 0.8;
	      if( bFl == true )
	        str = rStr.Left( iN > 0 ? iN:0 ) + "...";
	      else
		    str = "..." + rStr.Right( iN > 0 ? iN:0 );
		  szExt = rDC.GetTextExtent( str );
		}
	   if( iN < 2 ) str = rStr;
	 }
	
	rStr = str; 
 }
