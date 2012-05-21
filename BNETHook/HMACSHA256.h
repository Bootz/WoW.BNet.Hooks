#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <cstdint>
#include <vector>

class HMACSHA256
{
private:
	HMAC_CTX m_ctx;
	uint8_t m_digest[SHA256_DIGEST_LENGTH];
public:
	HMACSHA256(std::vector<uint8_t> initVector);
	~HMACSHA256();
	std::vector<uint8_t> ComputeHash(std::vector<uint8_t> key);
};