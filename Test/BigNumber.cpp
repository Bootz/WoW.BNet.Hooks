#include "BigNumber.h"

#pragma comment(lib, "libeay32.lib")

BigNumber::BigNumber()
{
	m_bn = BN_new();
}

BigNumber::BigNumber(uint32_t data)
{
	m_bn = BN_new();
	BN_set_word(m_bn, data);
}

BigNumber::BigNumber(const BigNumber &bn)
{
	m_bn = BN_dup(bn.m_bn);
}

BigNumber::BigNumber(const std::vector<uint8_t> &data)
{
	m_bn = BN_new();
	size_t sz = data.size();
	uint8_t *t = new uint8_t[sz];
	for (int i = 0; i < data.size(); i++)
		t[i] = data[data.size() - 1 - i];
	BN_bin2bn(t, data.size(), m_bn);

	delete[] t;
}

BigNumber::BigNumber(const uint8_t *data, size_t size)
{
	m_bn = BN_new();
	uint8_t *t = new uint8_t[size];
	for (int i = 0; i < size; i++)
		t[i] = data[size - 1 - i];
	BN_bin2bn(t, size, m_bn);

	delete[] t;
}

BigNumber::~BigNumber()
{
	BN_free(m_bn);
}

const char *BigNumber::hexStr()
{
	return BN_bn2hex(m_bn);
}

void BigNumber::setRand(int numbits)
{
	BN_rand(m_bn, numbits, 0, 1);
}

BigNumber BigNumber::operator=(const BigNumber &bn)
{
	BN_copy(m_bn, bn.m_bn);
	return *this;
}

BigNumber BigNumber::operator+=(const BigNumber &bn)
{
	BN_add(m_bn, m_bn, bn.m_bn);
	return *this;
}

BigNumber BigNumber::operator-=(const BigNumber &bn)
{
	BN_sub(m_bn, m_bn, bn.m_bn);
	return *this;
}

BigNumber BigNumber::operator*=(const BigNumber &bn)
{
	BN_CTX *bnctx;

	bnctx = BN_CTX_new();
	BN_mul(m_bn, m_bn, bn.m_bn, bnctx);
	BN_CTX_free(bnctx);

	return *this;
}

BigNumber BigNumber::operator/=(const BigNumber &bn)
{
	BN_CTX *bnctx;

	bnctx = BN_CTX_new();
	BN_div(m_bn, NULL, m_bn, bn.m_bn, bnctx);
	BN_CTX_free(bnctx);

	return *this;
}

BigNumber BigNumber::operator%=(const BigNumber &bn)
{
	BN_CTX *bnctx;

	bnctx = BN_CTX_new();
	BN_mod(m_bn, m_bn, bn.m_bn, bnctx);
	BN_CTX_free(bnctx);

	return *this;
}

BigNumber BigNumber::exp(const BigNumber &bn)
{
	BigNumber ret;
	BN_CTX *bnctx;

	bnctx = BN_CTX_new();
	BN_exp(ret.m_bn, m_bn, bn.m_bn, bnctx);
	BN_CTX_free(bnctx);

	return ret;
}

BigNumber BigNumber::modExp(const BigNumber &bn1, const BigNumber &bn2)
{
	BigNumber ret;
	BN_CTX *bnctx;

	bnctx = BN_CTX_new();
	BN_mod_exp(ret.m_bn, m_bn, bn1.m_bn, bn2.m_bn, bnctx);
	BN_CTX_free(bnctx);

	return ret;
}

std::vector<uint8_t> BigNumber::asByteArray()
{
	std::vector<uint8_t> out;
	out.resize(BN_num_bytes(m_bn));
	BN_bn2bin(m_bn, (unsigned char *)&out[0]);

	std::reverse(out.begin(), out.end());

	return out;
}
