#include "HMACSHA1.h"

#pragma comment(lib, "libeay32.lib")

HMACSHA1::HMACSHA1(std::vector<uint8_t> initVector)
{
	HMAC_CTX_init(&m_ctx);
	HMAC_Init_ex(&m_ctx, &initVector[0], initVector.size(), EVP_sha1(), NULL);
}

HMACSHA1::~HMACSHA1()
{
	HMAC_CTX_cleanup(&m_ctx);
}

void HMACSHA1::Update(std::vector<uint8_t> data)
{
	HMAC_Update(&m_ctx, &data[0], data.size());
}

std::vector<uint8_t> HMACSHA1::ComputeHash(std::vector<uint8_t> key)
{
	HMAC_Update(&m_ctx, &key[0], key.size());
	uint32_t len = 0;
	HMAC_Final(&m_ctx, m_digest, &len);

	return std::vector<uint8_t>(m_digest, m_digest + len);
}