#pragma once

#include <vector>
#include <cstdint>

class BitStream
{
public:
	typedef uint8_t dataType;
private:
	dataType m_lastByte;
	uint8_t m_lastBitCount;
	size_t m_pos;
	int32_t m_lastBytePos;
	std::vector<uint8_t> m_buf;
	size_t m_bufpos;
public:
	BitStream(std::vector<uint8_t> buf) : m_lastBitCount(0), m_lastByte(0), m_pos(0), m_lastBytePos(-1), m_bufpos(0), m_buf(buf) {}

	uint8_t readOne()
	{
		uint8_t ret = m_buf[m_bufpos];
		m_bufpos ++;
		return ret;
	}

	size_t ReadBits(size_t cnt)
	{
		int origcnt = cnt;
		int bytepos = m_pos / (sizeof(dataType) * 8);
		int bitpos = m_pos % (sizeof(dataType) * 8);

		int ret = 0;
		while(cnt > 0) 
		{
			if(m_lastBytePos != bytepos)
			{
				m_lastByte = readOne();
				m_lastBytePos = bytepos;
			}
			if(sizeof(dataType) * 8 - bitpos <= cnt) //need to read more bytes.
			{
				ret |= (m_lastByte >> bitpos) << (cnt - (sizeof(dataType) * 8 - bitpos));
				cnt -= sizeof(dataType) * 8 - bitpos;
				bitpos = 0;
				bytepos ++;
			}
			else
			{
				ret |= ((dataType)(m_lastByte << (sizeof(dataType) * 8 - bitpos - cnt))) >> (sizeof(dataType) * 8 - cnt);
				cnt = 0;
			}
		}

		m_pos += origcnt;

		return ret;
	}

	void Align()
	{
		if(m_pos % (sizeof(dataType) * 8))
			m_pos = (m_pos / (sizeof(dataType) * 8) + 1) * (sizeof(dataType) * 8);
	}
};