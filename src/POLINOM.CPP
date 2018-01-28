
#include "phus.hpp"
#include "polinom.h"

#include<windows.h>

 
//int WINAPI WinMain(   HINSTANCE  hInstance,   HINSTANCE  hPrevInstance,    LPSTR  lpCmdLine,    int  nShowCmd   )
int Test()
{
	WORD res;  BYTE b; BYTE flag;	WORD C=0;
	for(BYTE i=0;C<256;i++,C++)
	 {
		  /* Нормальная декодировка */
		res=CodeByteToWord(i);
		b=CodeWordToByte(res,flag);
		if( flag || b!=i )
		   MessageBox(NULL,"Ошибка в программе","",MB_OK);

		 /* Цикл на все одиночные ошибки */
		for(BYTE j=0;j<8;j++)
		  {
		     _asm
			 {
			   xor ah,ah    
			   mov al, j
			   btc res,ax
			 }
	   	     b=CodeWordToByte(res,flag);
			 if(!flag)
				 MessageBox(NULL,"Ошибка в программе","",MB_OK);
		     _asm
			 {
			   xor ah,ah
			   mov al,j
			   btc res,ax
			 }
		}

		 /* Цикл на все двухкратные ошибки */
	 for( j=0;j<8-1;j++)
		{
		      _asm
			      {
			          xor ah,ah    
			          mov al, j
			          btc res,ax
			      }
			  for(BYTE m=j+1;m<8;m++)
			    {
		            _asm
			            {
			               xor ah,ah    
			               mov al,m
			               btc res,ax
			            }

	   	            b=CodeWordToByte(res,flag);
			        if(!flag)
				        MessageBox(NULL,"Ошибка в программе","",MB_OK);
		            _asm
			            {
			               xor ah,ah    
			               mov al,m
			               btc res,ax
			            }
			    }
		     _asm
			     {
			        xor ah,ah
			        mov al,j
			        btc res,ax
			     }
		}
 	}
	
	return TRUE;
}
 
//------------------------------------------------------------------------
/* Эта функция кодирует байт  в слово */
WORD CodeByteToWord(BYTE data)
{
	WORD res;
    _asm
	{	    
		xor ax,ax
		mov al,data
		and al,00001111b
		push ax
		call CodeFourBits
		mov bl,al

		xor ax,ax
		mov al,data
		and al,11110000b
		shr ax,4
		push ax
		call CodeFourBits
		mov bh,al
		
		mov res,bx
	
	}
	return res;	
}

//----------------------------------------------------------------

/* Эта функция  декодирует слово в байт данных 
    в случае ошибки устанавливает flag в 1 иначе 0
*/


BYTE CodeWordToByte(WORD data,BYTE& flag)
{
	BYTE res; 
	flag=1;
    _asm
	{
		mov ax,data
		xor ah,ah
		push ax
		call DeCodeFourBits	
		cmp al,0ffh
		//je error_occur
		je end

		mov bl,al

		mov ax,data
		shr ax,8
		push ax
		call DeCodeFourBits	
		cmp al,0ffh
		//je error_occur
		je end

		mov bh,al
		shl bh,4
		or bl,bh
		jmp all_right

//error_occur:
//		jmp end

all_right:
		mov res,bl
	}
	flag=0; 
end:    

	return res;	
}

//------------------------------------------------------------------------
BYTE CodeFourBits(BYTE data)
{	
	BYTE rtn;
	_asm
	{
		xor ax,ax
		mov al,data
		shl   ax,8
		mov bx,3 
next:
		bsr dx,ax
		jz   exitprg
		cmp dx,8
		jb exitprg
		cmp bx,0 
		je exitprg

		mov cx,11
		sub cx,dx
		cmp bx,cx 
		jb exitprg 
		sub bx,cx 
		shl ax,cl
		xor ah,00001101b

		jmp next

exitprg:
		mov cx,8
		sub cx,bx  
		shr ax,cl
		xor bx,bx
		mov bl, data
		shl bx,3
		or ax,bx
		mov rtn,al 
	}
	return rtn;
}

//-----------------------------------------------------------

BYTE DeCodeFourBits(BYTE data)
{	
	BYTE err;
	_asm
	{
		xor ax,ax
        mov al,data
		test al,10000000b
		jnz exitprg 
	
		shl   ax,5       // Выдвинуть 4 бита в ah для деления
		mov bx,3      // Сколько бит еще можно сносить
next:
		bsr dx,ax      // Пишет в DX порядковый ноиер левого 1-го бита 
		jz   exitprg
		cmp dx,8
		jb exitprg
		cmp bx,0
		je exitprg

		mov cx,11
		sub cx,dx
		cmp bl,cl    //нужен сдвиг больше чем остаток
		jb	exitprg
		shl ax,cl
		sub bx,cx  

	    xor ah,00001101b
		jmp next

exitprg:
		cmp ax,0
		je good
		mov err,-1

		jmp rtnrn 
good:
		mov al,data
		shr al,3
		mov err,al
rtnrn:
	}
	return err;
}

//----------------------------------------------------------------------


WORD MyBW( BYTE b )
 {
   return MAKEWORD( b, b );
 }

BYTE MyWB( WORD w, BYTE& )
 {
   return LOBYTE( w );
 }

void CodePacket( TPacket* pPck )
 { 
   char cBuf[ MAX_PACKETDATA_SIZE + 10 ];

   LPWORD lpB = (LPWORD)cBuf;
   LPBYTE lpP = pPck->m_Data;
   for( UINT i = pPck->m_Size; i > 0; --i, lpB++, lpP++ )
	{
	  *lpB = CodeByteToWord( *lpP );	
	//*lpB = MyBW( *lpP );
	}   

   pPck->m_Size *= 2;
   memcpy( pPck->m_Data, cBuf, pPck->m_Size );
 }

bool DecodePacket( TPacket* pPck )
 {   
   LPWORD lpB = (LPWORD)pPck->m_Data;
   LPBYTE lpP = pPck->m_Data;
   pPck->m_Size /= 2;
   for( UINT i = pPck->m_Size; i > 0; --i, lpB++, lpP++ )
	{
	  BYTE bRes;
	  *lpP = CodeWordToByte( *lpB, bRes );	  
	  //*lpP = MyWB( *lpB, bRes );
	  if( bRes ) return false;
	}

   return true;
 }
