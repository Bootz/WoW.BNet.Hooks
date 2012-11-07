#include "BNETHook.h"

#include "RC4Crypt.h"
#include "HMACSHA256.h"

#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <varargs.h>
#include <Windows.h>

struct BNETConnectionInfo
{
	RC4Crypt *encryption;
	RC4Crypt *decryption;
	std::vector<uint8_t> sessionKey;
	bool useCrypt;

	BNETConnectionInfo() : useCrypt(false), encryption(nullptr), decryption(nullptr) {}
};

int g_masterSock;
uint32_t g_masterIP;
std::map<int, BNETConnectionInfo> g_bnetSockets;

void logv(const wchar_t *format, va_list val)
{
	wchar_t dest[16384];
	int len = wvsprintf(dest, format, val) * 2;
	DWORD temp;

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

void log(const wchar_t *format, ...)
{
	va_list val;
	va_start(val, format);
	logv(format, val);
	va_end(val);
}

std::wstring stringify(std::vector<uint8_t> data)
{
	std::wstringstream str;
	for(auto it = data.begin(), end = data.end(); it != end; it ++)
		str << std::setw(2) << std::setfill(L'0') << std::hex << *it << ", ";

	return str.str();
}

void BNETHookLog(const wchar_t *format, ...)
{
	va_list val;
	va_start(val, format);
	logv(format, val);
	va_end(val);
}

void BNETHookInitialize()
{
	log(L"Initialize");
}

void BNETHookSetSessionKey(uint8_t *buffer, int length)
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

	log(L"Session key: %s", stringify(std::vector<uint8_t>(buffer, buffer + length)).c_str());
}

void BNETHookOnHostFind(const char *host, uint32_t ip)
{
	if(strstr(host, "logon.battle.net") || strstr(host, "actual.battle.net"))
	{
		g_masterIP = ip;
		log(L"Got IP %S => %x.", host, ip);
	}
}

void BNETHookOnClose(int s)
{
	if(g_bnetSockets.find(s) != g_bnetSockets.end())
	{
		g_bnetSockets.erase(s);
		log(L"Battle.net 2.0 connection Disconnected");
	}
}

void BNETHookOnConnect(int s, uint32_t ip)
{
	if(ip == g_masterIP)
	{
		g_masterSock = s;
		g_bnetSockets[s] = BNETConnectionInfo();
		log(L"Battle.net 2.0 connection start");
	}
}

void BNETHookOnSend(int s, uint8_t *data, int size)
{
	if(g_bnetSockets.find(s) != g_bnetSockets.end() && size != 0)
	{
		std::wstring dataStr;
		std::vector<uint8_t> realData;
		if(g_bnetSockets[s].useCrypt)
			realData = g_bnetSockets[s].encryption->Process(std::vector<uint8_t>(data, data + size));
		else
		{
			realData = std::vector<uint8_t>(data, data + size);
			if(g_bnetSockets[s].encryption != nullptr)
				g_bnetSockets[s].useCrypt = true; //Encrypted stream starts after seeding key and sending 2-byte packet(45 01)
		}
		log(L"Send: %s", stringify(realData).c_str());
	}
}

void BNETHookOnRecv(int s, uint8_t *data, int size)
{
	if(g_bnetSockets.find(s) != g_bnetSockets.end() && size != 0)
	{
		std::wstring dataStr;
		std::vector<uint8_t> realData;
		if(g_bnetSockets[s].useCrypt)
			realData = g_bnetSockets[s].decryption->Process(std::vector<uint8_t>(data, data + size));
		else
			realData = std::vector<uint8_t>(data, data + size);
		log(L"Recv: %s", stringify(realData).c_str());
	}
}
