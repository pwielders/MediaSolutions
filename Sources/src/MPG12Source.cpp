#include "Source.h"
#include "MPG12Source.h"

namespace Solutions { namespace Source
{

//--------------------------------------------------------------------------------------------
// Class: MPG12Source
//--------------------------------------------------------------------------------------------
MPG12Source::MPG12Source (const Generics::DataElement& dataObject) : 
	m_Duration(0),
	m_MPEGSource(dataObject),
	m_ChannelIterator()
{
	if (m_MPEGSource.IsValid())
	{
		m_ChannelIterator = m_MPEGSource.Streams();
	}
}

MPG12Source::~MPG12Source()
{
}

} } // namespace Solutions::Source
