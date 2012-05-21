#pragma once

#include <cstdint>

void BNETHookOnHostFind(const char *host, uint32_t ip);
void BNETHookOnClose(int s);
void BNETHookOnConnect(int s, uint32_t ip);
void BNETHookSetEncryptionKey(uint8_t *buffer, int length);
void BNETHookOnSend(int s, uint8_t *data, int size);
void BNETHookOnRecv(int s, uint8_t *data, int size);