#if !defined(_SERVIS_)
#define _SERVIS_

#include "stdafx.h"

#if defined(SERVIS_DLL)
  #define EXPORT_SERVIS_ __declspec(dllexport)
#else 
  #define EXPORT_SERVIS_ __declspec(dllimport)
#endif


extern "C" EXPORT_SERVIS_ void InitServisDLL();
extern "C" EXPORT_SERVIS_ HMODULE GetDLL_BWCC_ServisModule();

#endif
