#include "SHA1Hash.h"
#pragma comment(lib, "libeay32.lib")

SHA1Hash::SHA1Hash()
{
	SHA1_Init(&m_ctx);
}

SHA1Hash::~SHA1Hash()
{

}

void SHA1Hash::update(std::vector<uint8_t> data)
{
	SHA1_Update(&m_ctx, (const void *)&data[0], data.size());
}

void SHA1Hash::update(const uint8_t *data, size_t size)
{
	SHA1_Update(&m_ctx, (const void *)data, size);
}

std::vector<uint8_t> SHA1Hash::compute()
{
	std::vector<uint8_t> out;
	out.resize(SHA_DIGEST_LENGTH);

	SHA1_Final((unsigned char *)&out[0], &m_ctx);

	return out;
}