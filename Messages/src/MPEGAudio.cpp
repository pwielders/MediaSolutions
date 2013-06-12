#include "MPEGAudio.h"
#include "MPEGSection.h"
#include "MPEGAssembler.h"
#include "MPEGTable.h"

namespace Solutions { namespace MPEG
{

//--------------------------------------------------------------------------------------------
// Class: MPEGAudio::MPEGElement
//--------------------------------------------------------------------------------------------

MPEGAudio::MPEGElement::MPEGElement (const Generics::DataElement& source, const uint64 offset) :
	m_Buffer(),
	m_Index(NUMBER_MAX_UNSIGNED(uint64)),
	m_PlayTime(0)
{
	// Now let's find the first header..
	uint64 startOffset = AudioHeader::FindTag(source, offset);

	// See if there is still space to move forward...
	if (startOffset < source.Size())
	{
		m_Buffer = Generics::DataElement(source, startOffset);
		m_Frame = AudioHeader(m_Buffer, 0);
	}
}

bool MPEGAudio::MPEGElement::Next()
{
	bool valid = false;

	// See if we are prior to the buffer
	if (m_Index == NUMBER_MAX_UNSIGNED(uint64))
	{
		m_Index = 0;
		valid = m_Frame.IsValid();
	}
	else
	{
		// Move on to the next Package...
		m_Index = AudioHeader::FindTag(m_Buffer, m_Index + m_Frame.Data().Size());

		// See if there is still space to move forward...
		if (m_Index < m_Buffer.Size())
		{
			m_Frame = AudioHeader(m_Buffer, m_Index);
			m_PlayTime += m_Frame.FrameDuration();
			valid = true;
		}
	}

	return (valid);
}

//--------------------------------------------------------------------------------------------
// Class: MPEGAudio
//--------------------------------------------------------------------------------------------

MPEGAudio::MPEGAudio () :
	m_Index(),
	m_Source(),
	m_BaseTime(0)
{
}

MPEGAudio::MPEGAudio (const  MPEGAudio& copy) :
	m_Index(copy.m_Index),
	m_Source(copy.m_Source),
	m_BaseTime(copy.m_BaseTime)
{
}

MPEGAudio::MPEGAudio (const MPEGStream& stream) :
	m_Index(),
	m_Source(stream),
	m_BaseTime(0)
{
	Reset();
}

MPEGAudio& MPEGAudio::operator=  (const MPEGAudio& RHS)
{
	m_Index		  = RHS.m_Index;
	m_Source      = RHS.m_Source;
	m_BaseTime    = RHS.m_BaseTime;

	return (*this);
}

bool MPEGAudio::Next()
{
	bool valid = m_Index.Next();

	while ( (valid == false) &&  (m_Source.Next() != false) )
	{
		m_Index = MPEGAudio::MPEGElement (m_Source.Section(), 0);

		valid = m_Index.Next();
	}

	return (valid);
}

} } // namespace Solutions::MPEG
