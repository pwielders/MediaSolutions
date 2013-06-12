#include "RIFFHeader.h"

namespace Solutions { namespace RIFF
{

FourCC::FourCC(const uint8 a, const uint8 b, const uint8 c, const uint8 d)
{
	m_Code = ntohl ((a <<24) | (b << 16) | (c << 8) | (d));
}

FourCC::FourCC(const TCHAR code[])
{
	m_Code = ntohl ((code[0] <<24) | (code[1]  << 16) | (code[2]  << 8) | (code[3]));
}

FourCC::FourCC(const uint32 code)
{
	m_Code = code;
}

FourCC& FourCC::operator= (const FourCC& RHS)
{
	m_Code = RHS.m_Code;

	return(*this);
}

} } // namespace Solutions::RIFF