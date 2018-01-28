// dibapi.h
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1997 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.


#ifndef _INC_DIBAPI
#define _INC_DIBAPI

#if defined(TDIBAPI_DLL)
  #define EXPORT_TDIBAPI_ __declspec(dllexport)
#else 
  #define EXPORT_TDIBAPI_ __declspec(dllimport)
#endif


/* Handle to a DIB */
DECLARE_HANDLE(HDIB);

/* DIB constants */
#define PALVERSION   0x300

/* DIB Macros*/

#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))
#define RECTWIDTH(lpRect)     ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)    ((lpRect)->bottom - (lpRect)->top)

// WIDTHBYTES performs DWORD-aligning of DIB scanlines.  The "bits"
// parameter is the bit count for the scanline (biWidth * biBitCount),
// and this macro returns the number of DWORD-aligned bytes needed
// to hold those bits.

#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

/* Function prototypes */
BOOL      EXPORT_TDIBAPI_  PaintDIB (HDC, LPRECT, HDIB, LPRECT, CPalette* pPal);
BOOL      EXPORT_TDIBAPI_  CreateDIBPalette(HDIB hDIB, CPalette* cPal);
LPSTR     EXPORT_TDIBAPI_  FindDIBBits (LPSTR lpbi);
DWORD     EXPORT_TDIBAPI_  DIBWidth (LPSTR lpDIB);
DWORD     EXPORT_TDIBAPI_  DIBHeight (LPSTR lpDIB);
WORD      EXPORT_TDIBAPI_  PaletteSize (LPSTR lpbi);
WORD      EXPORT_TDIBAPI_  DIBNumColors (LPSTR lpbi);
HGLOBAL   EXPORT_TDIBAPI_  CopyHandle (HGLOBAL h);

BOOL      EXPORT_TDIBAPI_  SaveDIB (HDIB hDib, CFile& file);
HDIB      EXPORT_TDIBAPI_  ReadDIBFile(CFile& file);

#endif //!_INC_DIBAPI
