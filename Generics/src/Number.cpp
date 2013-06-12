#include <math.h>

#include "Number.h"

namespace Solutions { namespace Generics
{

Fractional::Fractional() :
	m_Integer(0),
	m_Remainder(0)
{
}

Fractional::Fractional(const sint32& integer, const uint32& remainder) :
	m_Integer(integer),
	m_Remainder(remainder)
{
}

Fractional::Fractional(const Fractional& copy) :
	m_Integer(copy.m_Integer),
	m_Remainder(copy.m_Remainder)
{
}

/* virtual */ Fractional::~Fractional()
{
}

Fractional& Fractional::operator= (const Fractional& RHS)
{
	m_Integer = RHS.m_Integer;
	m_Remainder = RHS.m_Remainder;

	return (*this);
}

String Fractional::Text (const uint8 decimalPlaces) const
{
	String result (NumberType<sint32,true>(m_Remainder).Text() + _T("0000000000000000000000"));

	return (NumberType<sint32,true>(m_Integer).Text() + '.' + result.substr(0, decimalPlaces));
}

bool Fractional::operator == (const Fractional& RHS) const
{
	return ((m_Integer == RHS.m_Integer) && (m_Remainder == RHS.m_Remainder));
}

bool Fractional::operator != (const Fractional& RHS) const
{
	return (!operator==(RHS));
}

bool Fractional::operator >= (const Fractional& RHS) const
{
	return (operator==(RHS) || (Composit<float>() > RHS.Composit<float>()));
}

bool Fractional::operator <= (const Fractional& RHS) const
{
	return (operator==(RHS) || (Composit<float>() < RHS.Composit<float>()));
}

bool Fractional::operator > (const Fractional& RHS) const
{
	return (Composit<float>() > RHS.Composit<float>());
}

bool Fractional::operator < (const Fractional& RHS) const
{
	return (Composit<float>() < RHS.Composit<float>());
}

} } // namespace Solutions::Generics
