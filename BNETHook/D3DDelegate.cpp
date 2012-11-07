#include <windows.h>

#include "D3DDelegate.h"

extern "C" __declspec(dllexport) void WINAPI D3DPERF_SetOptions(DWORD dwOptions);
extern "C" __declspec(dllexport) int WINAPI D3DPERF_BeginEvent(DWORD color, LPCWSTR name);
extern "C" __declspec(dllexport) int WINAPI D3DPERF_EndEvent();

struct IDirect3D9;
struct IDirect3D9Ex;
extern "C" __declspec(dllexport) IDirect3D9* WINAPI Direct3DCreate9(UINT SDKVersion);
extern "C" __declspec(dllexport) HRESULT WINAPI Direct3DCreate9Ex(__in   UINT SDKVersion, __out  IDirect3D9Ex **ppD3D);

HMODULE reald3d9 = nullptr;

typedef int (WINAPI *D3DPERF_BEGINEVENT)(DWORD, LPCWSTR);
D3DPERF_BEGINEVENT _D3D_PREF_BEGINEVENT;

typedef int (WINAPI *D3DPERF_ENDEVENT)();
D3DPERF_ENDEVENT _D3D_PREF_ENDEVENT;

typedef void (WINAPI *D3DPERF_SETOPTIONS)(DWORD);
D3DPERF_SETOPTIONS _D3DPERF_SetOptions;

typedef IDirect3D9* (WINAPI *DIRECT3DCREATE9)(UINT);
DIRECT3DCREATE9 _Direct3DCreate9;

typedef HRESULT (WINAPI *DIRECT3DCREATE9EX)(UINT, IDirect3D9Ex**);
DIRECT3DCREATE9EX _Direct3DCreate9Ex;

//d3d delegate
int WINAPI D3DPERF_BeginEvent(DWORD color, LPCWSTR name)
{
	return _D3D_PREF_BEGINEVENT(color, name);
}
int WINAPI D3DPERF_EndEvent()
{
	return _D3D_PREF_ENDEVENT();
}

void WINAPI D3DPERF_SetOptions(DWORD dwOptions)
{
	_D3DPERF_SetOptions(dwOptions);
}

IDirect3D9 *WINAPI Direct3DCreate9(UINT SDKVersion)
{
	return _Direct3DCreate9(SDKVersion);
}

HRESULT WINAPI Direct3DCreate9Ex(__in   UINT SDKVersion, __out  IDirect3D9Ex **ppD3D)
{
	return _Direct3DCreate9Ex(SDKVersion, ppD3D);
}

void InitializeDelegate()
{
	reald3d9 = LoadLibrary(L"C:\\windows\\system32\\d3d9.dll");
	_D3DPERF_SetOptions = (D3DPERF_SETOPTIONS)GetProcAddress(reald3d9, "D3DPERF_SetOptions");
	_D3D_PREF_BEGINEVENT = (D3DPERF_BEGINEVENT)GetProcAddress(reald3d9, "D3DPERF_BeginEvent");
	_D3D_PREF_ENDEVENT = (D3DPERF_ENDEVENT)GetProcAddress(reald3d9, "D3DPERF_EndEvent");
	_Direct3DCreate9 = (DIRECT3DCREATE9)GetProcAddress(reald3d9, "Direct3DCreate9");
	_Direct3DCreate9Ex = (DIRECT3DCREATE9EX)GetProcAddress(reald3d9, "Direct3DCreate9Ex");
}

void UninitializeDelegate()
{
	FreeLibrary(reald3d9);
}