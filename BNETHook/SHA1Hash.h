#pragma once

#include <openssl/sha.h>
#include <cstdint>
#include <vector>

class SHA1Hash
{
private:
	SHA_CTX m_ctx;
public:
	SHA1Hash();
	~SHA1Hash();

	void update(std::vector<uint8_t> data);
	void update(const uint8_t *data, size_t size);

	template<typename T>
	void update(const T *data, size_t size)
	{
		update((const uint8_t *)data, size);
	}
	std::vector<uint8_t> compute();
};