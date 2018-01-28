#if !defined(_MYDIB_)
#define _MYDIB_

#include "stdafx.h"

#if defined(MYDIB_DLL)
  #define EXPORT_MYDIB_ __declspec(dllexport)
#else 
  #define EXPORT_MYDIB_ __declspec(dllimport)
#endif


extern "C" EXPORT_MYDIB_ void InitMyDIBDLL();
extern "C" EXPORT_MYDIB_ HMODULE GetDLL_MyDIB_ServisModule();

#endif
