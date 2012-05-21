#include "hmacsha256.h"

HMACSHA256::HMACSHA256(std::vector<uint8_t> initVector)
{
	HMAC_CTX_init(&m_ctx);
	HMAC_Init_ex(&m_ctx, &initVector[0], initVector.size(), EVP_sha256(), NULL);
}

HMACSHA256::~HMACSHA256()
{
	HMAC_CTX_cleanup(&m_ctx);
}

std::vector<uint8_t> HMACSHA256::ComputeHash(std::vector<uint8_t> key)
{
	HMAC_Update(&m_ctx, &key[0], key.size());
	uint32_t len = 0;
	HMAC_Final(&m_ctx, m_digest, &len);

	return std::vector<uint8_t>(m_digest, m_digest + len);
}