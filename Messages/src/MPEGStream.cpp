#include "MPEGStream.h"
#include "MPEGAssembler.h"

namespace Solutions { namespace MPEG
{

//--------------------------------------------------------------------------------------------
// Class: MPEGStream
//--------------------------------------------------------------------------------------------

MPEGStream::MPEGStream () :
	m_StreamId(0),
	m_Buffer(),
	m_Offset(0),
	m_Current(),
	m_Pid(0),
	m_RefClock(0),
	m_WallClock(0)

{
}

MPEGStream::MPEGStream (const  MPEGStream& copy) :
	m_StreamId(copy.m_StreamId),
	m_Buffer(copy.m_Buffer),
	m_Offset(copy.m_Offset),
	m_Current(copy.m_Current),
	m_Pid(copy.m_Pid),
	m_RefClock(copy.m_RefClock),
	m_WallClock(copy.m_WallClock)
{
}

MPEGStream::MPEGStream (const Generics::DataElement& buffer, const uint64 offset, const uint64 referenceClock, const uint64 wallClock) :
	m_StreamId(0),
	m_Buffer(buffer, offset),
	m_Offset(0),
	m_Pid(0),
	m_Current(buffer),
	m_RefClock(referenceClock),
	m_WallClock(wallClock)
{
	Reset();
}

MPEGStream::MPEGStream (const Generics::DataElement& buffer, const uint64 offset, const uint16 pid, const uint8 frameSize) :
	m_StreamId(frameSize),
	m_Buffer(buffer, offset),
	m_Offset(0),
	m_Pid(pid),
	m_Current(),
	m_RefClock(0),
	m_WallClock(0)
{
	Reset();
}

// This is the constructor for files. The Source is the full file, containing packed chunks of MPEG elements
MPEGStream::MPEGStream (const Generics::DataElement& source, const uint64 offset, const uint8 streamId) :
	m_StreamId(streamId),
	m_Buffer(source, offset),
	m_Offset(0),
	m_Pid(0),
	m_Current(),
	m_RefClock(0),
	m_WallClock(0)
{
	Reset();
}

MPEGStream& MPEGStream::operator=  (const MPEGStream& RHS)
{
	m_StreamId	= RHS.m_StreamId;
	m_Buffer	= RHS.m_Buffer;
	m_Offset	= RHS.m_Offset;
	m_Pid		= RHS.m_Pid;
	m_Current   = RHS.m_Current;
	m_RefClock	= RHS.m_RefClock;
	m_WallClock	= RHS.m_WallClock;

	return (*this);
}

void MPEGStream::Reset ()
{
	m_Offset = 0;

	if ( (m_StreamId != 0) || (m_Pid != 0) )
	{
		m_RefClock  = 0;
		m_WallClock = 0;
	}
}

bool MPEGStream::Next () 
{
	bool result = false;

	if ( (m_StreamId == 0) || (m_Pid == 0) )
	{
		if  (m_Offset == 0)
		{
			m_Offset = 1;
			result = true;
		}
	}
	else
	{
		result = (m_Pid != 0 ? FindNextElementaryStreamFromStream() : FindNextElementaryStreamFromBuffer ());
	}

	return (result);
}


bool MPEGStream::FindNextElementaryStreamFromStream ()
{
	ElementaryStreamHeader result;

	// Assemble the Elementary stream...
	MPEG::AssemblerType<false,StreamAssemblerPolicy<1024>> collector (m_Pid);

	while ( (result.IsValid() == false) && (m_Offset < m_Buffer.Size()) )
	{
		if (m_Buffer[static_cast<uint32>(m_Offset)] != TransportPackage::ID())
		{
			// We need to search, we did not stop on a marker, for some reason.
			m_Offset = m_Buffer.SearchNumber<uint8, Generics::ENDIAN_LITTLE>(m_Offset, TransportPackage::ID());
		}

		if (m_Offset < m_Buffer.Size())	
		{
			TransportPackage frame (Generics::DataElement(m_Buffer, m_Offset, m_StreamId));

			if (frame.IsValid() == false)
			{
				m_Offset++;
			}
			else
			{
				// This frams is done, step over it.
				m_Offset += m_StreamId;	/* StreamId with PID <> 0 is actually a frame size ;-) */

				if (collector.Assemble(frame) == true)
				{
					Generics::OptionalType<Generics::DataElement> current = collector.CurrentFrame(true);

					// And are we done and do we have a frame..
					if (current.IsSet() == true)
					{
						result     = ElementaryStreamHeader (current.Value(), 0);
						m_Current  = result.Frame();
						m_Offset  += result.Size();
					}
				}
			}
		}
	}

	return (result.IsValid());
}


bool MPEGStream::FindNextElementaryStreamFromBuffer ()
{
	ElementaryStreamHeader result;

	// Find the fiurst tag after the current block handled
	m_Offset = MPEGHeader::FindTag (m_Buffer, m_Offset);

	// We need to walk over all enties is timing information is in here.
	while ( (result.IsValid() == false) && (m_Offset < m_Buffer.Size()) )
	{
		if (ElementaryStreamHeader::IsValid(m_Buffer, m_Offset))
		{
			result    = ElementaryStreamHeader (m_Buffer, m_Offset);
			m_Current = result.Frame();
			m_Offset += result.Size();

		}
		else if (PackStreamHeader::IsValid(m_Buffer, m_Offset))
		{
			PackStreamHeader info (m_Buffer, m_Offset);

			// Seems like it is not a transport stream but an already assembled file stream.
			m_Offset = MPEGHeader::FindTag (m_Buffer, m_Offset + info.Data().Size());
		}
		else
		{
			// oops what is this, raise an error but step over it..
			m_Offset = MPEGHeader::FindTag (m_Buffer, m_Offset+3);
		}
	}

	return (result.IsValid());
}

} } // namespace Solutions::MPEG
