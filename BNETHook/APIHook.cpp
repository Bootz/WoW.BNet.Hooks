#include "APIHook.h"

#define INCL_WINSOCK_API_TYPEDEFS 1
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <cstdint>

#include <map>
#include <utility>

#include "../libMinHook/MinHook.h"

#include "BNETHook.h"
#include "BNETProxy.h"

typedef FARPROC (WINAPI *GETPROCADDRESS)(HMODULE, LPCSTR);

typedef
	INT
	(WSAAPI * LPFN_GETADDRINFO)(
	__in_opt        PCSTR               pNodeName,
	__in_opt        PCSTR               pServiceName,
	__in_opt        const ADDRINFOA *   pHints,
	__deref_out     PADDRINFOA *        ppResult
	);

typedef int (WINAPI *GETQUEUEDCOMPLETTIONSTATUS)(HANDLE, LPDWORD, PULONG_PTR, LPOVERLAPPED *, DWORD);
typedef HANDLE (WINAPI *CREATEIOCOMPLETIONPORT)(HANDLE, HANDLE, ULONG_PTR, DWORD);

LPFN_CONNECT oldconnect = nullptr;
LPFN_SEND oldsend = nullptr;
LPFN_RECV oldrecv = nullptr;
LPFN_CLOSESOCKET oldclosesocket = nullptr;
LPFN_GETHOSTBYNAME oldgethostbyname = nullptr;
LPFN_WSARECV oldwsarecv = nullptr;
LPFN_WSASEND oldwsasend = nullptr;
LPFN_WSAGETOVERLAPPEDRESULT oldwsagetoverlappedresult = nullptr;
LPFN_GETADDRINFO oldgetaddrinfo = nullptr;
GETPROCADDRESS oldGetProcAddress = nullptr;
GETQUEUEDCOMPLETTIONSTATUS oldgetqueuedcompletionstatus = nullptr;
CREATEIOCOMPLETIONPORT oldcreateiocompletionport = nullptr;

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

std::map<LPOVERLAPPED, std::pair<WSABUF, DWORD> > g_overlappedResults;

int WSAAPI newWSAGetOverlappedResult( __in SOCKET s, __in LPWSAOVERLAPPED lpOverlapped, __out LPDWORD lpcbTransfer, __in BOOL fWait, __out LPDWORD lpdwFlags )
{
	int ret = oldwsagetoverlappedresult(s, lpOverlapped, lpcbTransfer, fWait, lpdwFlags);
	if(g_overlappedResults.find(lpOverlapped) != g_overlappedResults.end())
	{
		if(g_overlappedResults[lpOverlapped].second == 1)
			BNETHookOnRecv((int)s, (uint8_t *)g_overlappedResults[lpOverlapped].first.buf, *lpcbTransfer);
		else
			DebugBreak();
		g_overlappedResults.erase(lpOverlapped);
	}
	return ret;
}

std::map<HANDLE, std::map<DWORD, HANDLE> > g_iocphandles;

HANDLE WINAPI newcreateiocompletionport(
	__in      HANDLE FileHandle,
	__in_opt  HANDLE ExistingCompletionPort,
	__in      ULONG_PTR CompletionKey,
	__in      DWORD NumberOfConcurrentThreads
	)
{
	HANDLE ret = oldcreateiocompletionport(FileHandle, ExistingCompletionPort, CompletionKey, NumberOfConcurrentThreads);
	if(ExistingCompletionPort != NULL && FileHandle != INVALID_HANDLE_VALUE)
		g_iocphandles[ExistingCompletionPort][CompletionKey] = FileHandle;
	else if(ExistingCompletionPort == NULL && FileHandle != INVALID_HANDLE_VALUE)
		g_iocphandles[ret][CompletionKey] = FileHandle;

	return ret;
}

int WINAPI newgetqueuedcompletionstatus( __in HANDLE CompletionPort, __out LPDWORD lpNumberOfBytesTransferred, __out PULONG_PTR lpCompletionKey, __out LPOVERLAPPED *lpOverlapped, __in DWORD dwMilliseconds)
{
	int ret = oldgetqueuedcompletionstatus(CompletionPort, lpNumberOfBytesTransferred, lpCompletionKey, lpOverlapped, dwMilliseconds);
	if(g_overlappedResults.find(*lpOverlapped) != g_overlappedResults.end())
	{
		if(g_overlappedResults[*lpOverlapped].second == 1)
			BNETHookOnRecv((int)g_iocphandles[CompletionPort][*lpCompletionKey], (uint8_t *)g_overlappedResults[*lpOverlapped].first.buf, *lpNumberOfBytesTransferred);
		else
			DebugBreak();
		g_overlappedResults.erase(*lpOverlapped);
	}
	return ret;
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
			BNETHookOnRecv((int)s, (uint8_t *)lpBuffers[0].buf, *lpNumberOfBytesRecvd);
		else
			DebugBreak();
	}
	else if(lpOverlapped)
	{
		if(dwBufferCount == 1)
			g_overlappedResults[lpOverlapped] = std::make_pair(lpBuffers[0], dwBufferCount);
		else
			DebugBreak();
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

int WSAAPI newgetaddrinfo( __in_opt PCSTR pNodeName, __in_opt PCSTR pServiceName, __in_opt const ADDRINFOA * pHints, __deref_out PADDRINFOA * ppResult )
{
	int ret = oldgetaddrinfo(pNodeName, pServiceName, pHints, ppResult);
	if(pNodeName && ppResult && ppResult[0])
		BNETHookOnHostFind(pNodeName, ((sockaddr_in *)ppResult[0]->ai_addr)->sin_addr.S_un.S_addr);

	return ret;
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

	MH_CreateHook(&getaddrinfo, &newgetaddrinfo, (void **)&oldgetaddrinfo);
	MH_EnableHook(&getaddrinfo);

	MH_CreateHook(&GetQueuedCompletionStatus, &newgetqueuedcompletionstatus, (void **)&oldgetqueuedcompletionstatus);
	MH_EnableHook(&GetQueuedCompletionStatus);

	MH_CreateHook(&CreateIoCompletionPort, &newcreateiocompletionport, (void **)&oldcreateiocompletionport);
	MH_EnableHook(&CreateIoCompletionPort);

	BNETHookInitialize();
}

void UninitializeHook()
{
	MH_Uninitialize();
}
