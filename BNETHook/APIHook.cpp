#include "APIHook.h"

#define INCL_WINSOCK_API_TYPEDEFS 1

#include <winsock2.h>
#include <windows.h>
#include <cstdint>

#include <map>
#include <utility>

#include "../libMinHook/MinHook.h"

#include "BNETHook.h"
#include "BNETProxy.h"

typedef FARPROC (WINAPI *GETPROCADDRESS)(HMODULE, LPCSTR);

LPFN_CONNECT oldconnect = nullptr;
LPFN_SEND oldsend = nullptr;
LPFN_RECV oldrecv = nullptr;
LPFN_CLOSESOCKET oldclosesocket = nullptr;
LPFN_GETHOSTBYNAME oldgethostbyname = nullptr;
LPFN_WSARECV oldwsarecv = nullptr;
LPFN_WSASEND oldwsasend = nullptr;
LPFN_WSAGETOVERLAPPEDRESULT oldwsagetoverlappedresult = nullptr;
GETPROCADDRESS oldGetProcAddress = nullptr;

struct hostent FAR *
	WSAAPI
	newgethostbyname(
	__in const char FAR * name
	)
{
	if(name)
	{
		hostent *host = oldgethostbyname(name);

		BNETHookOnHostFind(name, *((uint32_t *)(host->h_addr_list[0])));
		return host;
	}
	return oldgethostbyname(name);
}

int
	WSAAPI
	newclosesocket(
	__in SOCKET s
	)
{
	BNETHookOnClose((int)s);
	return oldclosesocket(s);
}

int WSAAPI newconnect(
	__in SOCKET s,
	__in_bcount(namelen) const struct sockaddr FAR * name,
	__in int namelen
	)
{
	BNETHookOnConnect((int)s, (unsigned int)((sockaddr_in *)name)->sin_addr.S_un.S_addr);

	return oldconnect(s, name, namelen);
}

std::map<LPOVERLAPPED, std::pair<LPWSABUF, DWORD> > g_overlappedResults;

int WSAAPI newWSAGetOverlappedResult( __in SOCKET s, __in LPWSAOVERLAPPED lpOverlapped, __out LPDWORD lpcbTransfer, __in BOOL fWait, __out LPDWORD lpdwFlags )
{
	if(g_overlappedResults.find(lpOverlapped) != g_overlappedResults.end())
	{
		if(g_overlappedResults[lpOverlapped].second == 1)
			BNETHookOnRecv((int)s, (uint8_t *)g_overlappedResults[lpOverlapped].first->buf, g_overlappedResults[lpOverlapped].first->len);
		else
			DebugBreak();
		g_overlappedResults.erase(lpOverlapped);
	}
	return oldwsagetoverlappedresult(s, lpOverlapped, lpcbTransfer, fWait, lpdwFlags);
}

int WSAAPI
	newWSARecv(
	__in SOCKET s,
	__in_ecount(dwBufferCount) __out_data_source(NETWORK) LPWSABUF lpBuffers,
	__in DWORD dwBufferCount,
	__out_opt LPDWORD lpNumberOfBytesRecvd,
	__inout LPDWORD lpFlags,
	__inout_opt LPWSAOVERLAPPED lpOverlapped,
	__in_opt LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
	)
{
	int ret = oldwsarecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine);
	if(lpNumberOfBytesRecvd && *lpNumberOfBytesRecvd)
	{
		if(dwBufferCount == 1)
			BNETHookOnRecv((int)s, (uint8_t *)lpBuffers[0].buf, lpBuffers[0].len);
		else
			DebugBreak();
	}
	else if(lpOverlapped)
	{
		g_overlappedResults[lpOverlapped] = std::make_pair(lpBuffers, dwBufferCount);
	}
	return ret;
}

int WSAAPI newWSASend(
	__in SOCKET s,
	__in_ecount(dwBufferCount) LPWSABUF lpBuffers,
	__in DWORD dwBufferCount,
	__out_opt LPDWORD lpNumberOfBytesSent,
	__in DWORD dwFlags,
	__inout_opt LPWSAOVERLAPPED lpOverlapped,
	__in_opt LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
	)
{
	if(dwBufferCount == 1)
		BNETHookOnSend(s, (uint8_t *)lpBuffers[0].buf, lpBuffers[0].len);
	else
		DebugBreak();
	return oldwsasend((int)s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);
}

int WSAAPI newrecv(
	__in SOCKET s,
	__out_bcount_part(len, return) __out_data_source(NETWORK) char FAR * buf,
	__in int len,
	__in int flags
	)
{
	int ret = oldrecv(s, buf, len, flags);
	
	if(ret > 0)
		BNETHookOnRecv((int)s, (uint8_t *)buf, ret);

	return ret;
}

int WSAAPI newsend(
	__in SOCKET s,
	__in_bcount(len) const char FAR * buf,
	__in int len,
	__in int flags
	)
{
	if(len > 0)
		BNETHookOnSend((int)s, (uint8_t *)buf, len);
	return oldsend(s, buf, len, flags);
}

FARPROC WINAPI newGetProcAddress(HMODULE mod, LPCSTR str)
{
	if((long)str > 0x1000 && lstrcmpiA(str, "CreateModule") == 0)
	{
		realCreateModule = (CREATEMODULE)oldGetProcAddress(mod, str);
		return (FARPROC)&ProxyCreateModule;
	}

	return oldGetProcAddress(mod, str);
}

void InitializeHook()
{
	MH_Initialize();

	MH_CreateHook(&GetProcAddress, &newGetProcAddress, (void **)&oldGetProcAddress);
	MH_EnableHook(&GetProcAddress);

	MH_CreateHook(&connect, &newconnect, (void **)&oldconnect);
	MH_EnableHook(&connect);

	MH_CreateHook(&recv, &newrecv, (void **)&oldrecv);
	MH_EnableHook(&recv);

	MH_CreateHook(&WSARecv, &newWSARecv, (void **)&oldwsarecv);
	MH_EnableHook(&WSARecv);

	MH_CreateHook(&WSAGetOverlappedResult, &newWSAGetOverlappedResult, (void **)&oldwsagetoverlappedresult);
	MH_EnableHook(&WSAGetOverlappedResult);

	MH_CreateHook(&WSASend, &newWSASend, (void **)&oldwsasend);
	MH_EnableHook(&WSASend);

	MH_CreateHook(&send, &newsend, (void **)&oldsend);
	MH_EnableHook(&send);

	MH_CreateHook(&closesocket, &newclosesocket, (void **)&oldclosesocket);
	MH_EnableHook(&closesocket);

	MH_CreateHook(&gethostbyname, &newgethostbyname, (void **)&oldgethostbyname);
	MH_EnableHook(&gethostbyname);
}

void UninitializeHook()
{
	MH_Uninitialize();
}
