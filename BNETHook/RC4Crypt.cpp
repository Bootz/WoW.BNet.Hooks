#include "RC4Crypt.h"

RC4Crypt::RC4Crypt(std::vector<uint8_t> key)
{
	EVP_CIPHER_CTX_init(&m_ctx);
	EVP_EncryptInit_ex(&m_ctx, EVP_rc4(), NULL, NULL, NULL);
	EVP_CIPHER_CTX_set_key_length(&m_ctx, key.size());
	EVP_EncryptInit_ex(&m_ctx, NULL, NULL, &key[0], NULL);
}

RC4Crypt::~RC4Crypt()
{
	EVP_CIPHER_CTX_cleanup(&m_ctx);
}

std::vector<uint8_t> RC4Crypt::Process(std::vector<uint8_t> data)
{
	std::vector<uint8_t> out(data);
	int outlen = 0;
	EVP_EncryptUpdate(&m_ctx, &out[0], &outlen, &out[0], out.size());
	EVP_EncryptFinal_ex(&m_ctx, &out[0], &outlen);

	return out;
}