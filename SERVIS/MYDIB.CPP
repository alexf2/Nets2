#define MYDIB_DLL
#include "mydib.hpp"

#include <afxdllx.h>

static AFX_EXTENSION_MODULE extensionServisDLL = { NULL, NULL };

extern "C" int APIENTRY DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved )
 {
   if( dwReason == DLL_PROCESS_ATTACH )
	{	
	  if( !AfxInitExtensionModule(extensionServisDLL, hInstance) )
	    return 0;
	}  
   else if( dwReason == DLL_PROCESS_DETACH )
    {    
      AfxTermExtensionModule( extensionServisDLL );    
    }
   return 1;   
 }

// Exported DLL initialization is run in context of running application
extern "C" __declspec(dllexport) void InitMyDIBDLL()
 {
	// create a new CDynLinkLibrary for this app
   new CDynLinkLibrary( extensionServisDLL );	
 }

extern "C" __declspec(dllexport) HMODULE GetDLL_MyDIB_ServisModule()
 {
   return extensionServisDLL.hModule;
 }
