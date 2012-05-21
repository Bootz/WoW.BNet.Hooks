#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <cstdint>
#include <vector>

class HMACSHA1
{
private:
	HMAC_CTX m_ctx;
	uint8_t m_digest[SHA_DIGEST_LENGTH];
public:
	HMACSHA1(std::vector<uint8_t> initVector);
	~HMACSHA1();
	void Update(std::vector<uint8_t> data);
	std::vector<uint8_t> ComputeHash(std::vector<uint8_t> key);
};