#pragma once

#include <openssl/bn.h>
#include <vector>
#include <cstdint>

class BigNumber
{
private:
	BIGNUM *m_bn;
public:
	BigNumber();
	BigNumber(uint32_t data);
	BigNumber(const std::vector<uint8_t> &data);
	BigNumber(const uint8_t *data, size_t size);
	BigNumber::BigNumber(const BigNumber &bn);
	~BigNumber();
	const char *hexStr();

	void setRand(int numbits);

	BigNumber operator=(const BigNumber &bn);

	BigNumber operator+=(const BigNumber &bn);
	BigNumber operator+(const BigNumber &bn)
	{
		BigNumber t(*this);
		return t += bn;
	}
	BigNumber operator-=(const BigNumber &bn);
	BigNumber operator-(const BigNumber &bn)
	{
		BigNumber t(*this);
		return t -= bn;
	}
	BigNumber operator*=(const BigNumber &bn);
	BigNumber operator*(const BigNumber &bn)
	{
		BigNumber t(*this);
		return t *= bn;
	}
	BigNumber operator/=(const BigNumber &bn);
	BigNumber operator/(const BigNumber &bn)
	{
		BigNumber t(*this);
		return t /= bn;
	}
	BigNumber operator%=(const BigNumber &bn);
	BigNumber operator%(const BigNumber &bn)
	{
		BigNumber t(*this);
		return t %= bn;
	}

	BigNumber modExp(const BigNumber &bn1, const BigNumber &bn2);
	BigNumber exp(const BigNumber &);

	std::vector<uint8_t> asByteArray();
};