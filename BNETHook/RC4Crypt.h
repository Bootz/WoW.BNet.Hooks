#include <openssl/evp.h>
#include <vector>
#include <cstdint>

class RC4Crypt
{
private:
	EVP_CIPHER_CTX m_ctx;
public:
	RC4Crypt(std::vector<uint8_t> key);
	~RC4Crypt();

	std::vector<uint8_t> Process(std::vector<uint8_t> data);
};