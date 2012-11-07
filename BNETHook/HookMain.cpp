#include "HookMain.h"
#include "BigNumber.h"
#include "APIHook.h"
#include "D3DDelegate.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libeay32.lib")

#ifdef _DEBUG
#pragma comment(lib, "../Debug/libMinHook.lib")
#else
#pragma comment(lib, "../Release/libMinHook.lib")
#endif

int APIENTRY DllMain(_In_ void * _HDllHandle, _In_ unsigned _Reason, _In_opt_ void * _Reserved)
{
	switch(_Reason)
	{
	case DLL_PROCESS_ATTACH:
		InitializeDelegate();
		InitializeHook();
		return TRUE;
	case DLL_PROCESS_DETACH:
		UninitializeHook();
		UninitializeDelegate();
		return TRUE;
	}

	return TRUE;
}
