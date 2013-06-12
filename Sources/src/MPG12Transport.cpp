#include "Source.h"
#include "MPG12Transport.h"

namespace Solutions { namespace Source
{

//--------------------------------------------------------------------------------------------
// Class: MPG12Source
//--------------------------------------------------------------------------------------------
MPG12Transport::MPG12Transport (const Generics::DataElement& dataObject) : 
	m_Duration(0),
	m_MPEGSource(dataObject),
	m_ChannelIterator(m_MPEGSource.Streams())
{
}

MPG12Transport::~MPG12Transport()
{
}

} } // namespace Solutions::Source
