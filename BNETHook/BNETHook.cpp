#include "BNETHook.h"

#include "RC4Crypt.h"
#include "HMACSHA256.h"

#include <map>
#include <varargs.h>
#include <Windows.h>

struct BNETConnectionInfo
{
	RC4Crypt *encryption;
	RC4Crypt *decryption;
	std::vector<uint8_t> sessionKey;
};

int g_masterSock;
uint32_t g_masterIP;
std::map<int, BNETConnectionInfo> g_bnetSockets;

void log(const wchar_t *format, ...)
{
	va_list val;
	va_start(val, format);
	wchar_t dest[16384];
	int len = wvsprintf(dest, format, val) * 2;
	DWORD temp;
	va_end(val);

	static HANDLE outfile = INVALID_HANDLE_VALUE;
	if(outfile == INVALID_HANDLE_VALUE)
	{
		outfile = CreateFile(L"bnet.log", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);
		WriteFile(outfile, "\xFF\xFE", 2, &temp, NULL);
	}

	WriteFile(outfile, dest, len, &temp, NULL);
	WriteFile(outfile, L"\r\n", 4, &temp, NULL);
	FlushFileBuffers(outfile);
}

void stringify(wchar_t *out, uint8_t *buf, int size)
{
	for(int i = 0; i < size; i ++)
		wsprintf(out + i * 4, L"%02x, ", buf[i]);
	out[size * 4 - 2] = 0;
}

void BNETHookSetEncryptionKey(uint8_t *buffer, int length)
{
	uint8_t DecryptionKey[] =  { 0x68, 0xE0, 0xC7, 0x2E, 0xDD, 0xD6, 0xD2, 0xF3, 0x1E, 0x5A, 0xB1, 0x55, 0xB1, 0x8B, 0x63, 0x1E };
	uint8_t EncryptionKey[] = { 0xDE, 0xA9, 0x65, 0xAE, 0x54, 0x3A, 0x1E, 0x93, 0x9E, 0x69, 0x0C, 0xAA, 0x68, 0xDE, 0x78, 0x39 };
	HMACSHA256 decryptionhash(std::vector<uint8_t>(buffer, buffer + length));
	HMACSHA256 encryptionhash(std::vector<uint8_t>(buffer, buffer + length));
	std::vector<uint8_t> calculatedDecryptKey = decryptionhash.ComputeHash(std::vector<uint8_t>(DecryptionKey, DecryptionKey + 16));
	std::vector<uint8_t> calculatedEncryptKey = encryptionhash.ComputeHash(std::vector<uint8_t>(EncryptionKey, EncryptionKey + 16));

	g_bnetSockets[g_masterSock].encryption = new RC4Crypt(calculatedEncryptKey);
	g_bnetSockets[g_masterSock].decryption = new RC4Crypt(calculatedDecryptKey);
	g_bnetSockets[g_masterSock].sessionKey.assign(buffer, buffer + length);
}

void BNETHookOnHostFind(const char *host, uint32_t ip)
{
	if(strstr(host, "battle.net"))
		g_masterIP = ip;
}

void BNETHookOnClose(int s)
{
	if(g_bnetSockets.find(s) != g_bnetSockets.end())
		g_bnetSockets.erase(s);
}

void BNETHookOnConnect(int s, uint32_t ip)
{
	if(ip == g_masterIP)
	{
		g_masterSock = s;
		g_bnetSockets[s] = BNETConnectionInfo();
		log(L"New connection to %x", ip);
	}
}

void BNETHookOnSend(int s, uint8_t *data, int size)
{
	if(g_bnetSockets.find(s) != g_bnetSockets.end())
	{
		wchar_t *temp = new wchar_t[size * 4 + 4];
		stringify(temp, data, size);
		log(L"Send: %s", temp);
		delete [] temp;
	}
}

void BNETHookOnRecv(int s, uint8_t *data, int size)
{
	if(g_bnetSockets.find(s) != g_bnetSockets.end())
	{
		wchar_t *temp = new wchar_t[size * 4 + 4];
		stringify(temp, data, size);
		log(L"Recv: %s", temp);
		delete [] temp;
	}
}
