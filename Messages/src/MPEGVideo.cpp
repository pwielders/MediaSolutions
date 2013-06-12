#include "MPEGVideo.h"
#include "MPEGSection.h"
#include "MPEGAssembler.h"
#include "MPEGTable.h"

namespace Solutions { namespace MPEG
{

//--------------------------------------------------------------------------------------------
// Class: MPEGVideo::MPEGElement
//--------------------------------------------------------------------------------------------

MPEGVideo::MPEGElement::MPEGElement (InfoLoader& loadedInfo) :
	m_Buffer(),
	m_Index(0),
	m_Info(loadedInfo),
	m_PlayingTime(0)
{
}

MPEGVideo::MPEGElement::MPEGElement (const MPEGElement& copy) :
	m_Buffer(copy.m_Buffer),
	m_Index(copy.m_Index),
	m_Info(copy.m_Info),
	m_PlayingTime(copy.m_PlayingTime)
{
}

MPEGVideo::MPEGElement::MPEGElement (const Generics::DataElement& source, const uint64 offset, InfoLoader& loadedInfo) :
	m_Buffer(source, offset),
	m_Index(NUMBER_MAX_UNSIGNED(uint64)),
	m_Info(loadedInfo),
	m_PlayingTime(0)
{
	// All right time to load starting stuff if it is present...
	if (m_Info.State() != MPEGVideo::InfoLoader::NO_LOADING)
	{
		// We already loaded something, see if there is data before we find the first marker..
		uint64 marker = Solutions::MPEG::MPEGHeader::FindTag (m_Buffer, 0);

		if (marker != 0)
		{
			if (m_Info.State() == MPEGVideo::InfoLoader::SLICE_LOADING)
			{
				m_Info.SliceLoading(Generics::DataElement(m_Buffer, 0, marker));
			}
			else if (m_Info.State() == MPEGVideo::InfoLoader::PICTURE_LOADING)
			{
				m_Info.PictureLoading(Generics::DataElement(m_Buffer, 0, marker));
			}
			else if (m_Info.State() == MPEGVideo::InfoLoader::GROUP_LOADING)
			{
				m_Info.GroupOfPicturesLoading(Generics::DataElement(m_Buffer, 0, marker));
			}
			else
			{
				ASSERT (false);
			}
		}

		if (marker < m_Buffer.Size())
		{
			m_Info.Loaded();
		}

		m_Buffer = Generics::DataElement (m_Buffer, marker);
	}
}

MPEGVideo::MPEGElement& MPEGVideo::MPEGElement::operator=  (const MPEGVideo::MPEGElement& RHS)
{
	m_Buffer = RHS.m_Buffer;
	m_Index = RHS.m_Index;
	m_Info = RHS.m_Info;
	m_PlayingTime = RHS.m_PlayingTime;

	return (*this);
}

bool MPEGVideo::MPEGElement::NextGOP()
{
	if ( (m_Index < m_Buffer.Size()) || (m_Index == NUMBER_MAX_UNSIGNED(uint64)) )
	{
		if (m_Index == NUMBER_MAX_UNSIGNED(uint64))
		{
			m_Index = 0;
		}

		m_Index = MPEGHeader::FindTag(m_Buffer, m_Index);

		while ((m_Index < m_Buffer.Size()) && (GroupOfPicturesHeader::IsValid(m_Buffer, m_Index) == false))
		{
			if ( (SliceHeader::IsValid  (m_Buffer, m_Index) == true) ||
				 (PictureHeader::IsValid(m_Buffer, m_Index) == true) )
			{
				// Move on to the next tag !
				m_Index = MPEGHeader::FindTag(m_Buffer, (m_Index+4));
			}
			else if (MetaDataSection(m_Buffer, m_Index) == false)
			{
				// That is unexpected, log some trace line
				TRACE_L1 ("unexpected MPEG element <%X>", MPEGHeader::TagId(m_Buffer, m_IndexGOP));

				// Move on to the next tag !
				m_Index = MPEGHeader::FindTag(m_Buffer, (m_Index+3));
			}

		}

		// Did we end-up on a correct GOP header ?
		if (m_Index < m_Buffer.Size())
		{
			MPEG::GroupOfPicturesHeader loadedFrame = MPEG::GroupOfPicturesHeader(m_Buffer, m_Index);

			// We got this one, jump over it, and prepare for the next ... 
			m_Index += loadedFrame.Size ();

			// See if it is the last frame, if so we need to get the next frame as well
			if (m_Index == m_Buffer.Size())
			{
				// Oops, last frame, continue loading in the next elemntary stream.
				m_Info.GroupOfPicturesLoading(loadedFrame.Data());
			}
			else
			{
				// There is another frame following this one, so this one is closed !!
				m_Info.GroupOfPicturesLoading(loadedFrame);
			}
		}
	}

	return (m_Index < m_Buffer.Size());
}

bool MPEGVideo::MPEGElement::NextPicture()
{
	m_NewGroupOfPicturesHeader = false;

	if ((m_Index < m_Buffer.Size()) || (m_Index == NUMBER_MAX_UNSIGNED(uint64)))
	{
		if (m_Index == NUMBER_MAX_UNSIGNED(uint64))
		{
			m_Index = 0;
		}

		m_Index = MPEGHeader::FindTag(m_Buffer, m_Index);

		while ((m_Index < m_Buffer.Size()) && (PictureHeader::IsValid(m_Buffer, m_Index) == false))
		{
			if (GroupOfPicturesHeader::IsValid(m_Buffer, m_Index))
			{
				if  (m_NewGroupOfPicturesHeader == true)
				{
					// That is unexpected, log some trace line
					TRACE_L1 ("Another GOP header without a Picture header contained", 0);
				}

				// Oops moving into a new GOP, record it..
				MPEG::GroupOfPicturesHeader loadedFrame = MPEG::GroupOfPicturesHeader(m_Buffer, m_Index);

				// Handled it, add it to the index..
				m_Index += loadedFrame.Size();

				// See if it is the last frame, if so we need to get the next frame as well
				if (m_Index == m_Buffer.Size())
				{
					// Oops, last frame, continue loading in the next elemntary stream.
					m_Info.GroupOfPicturesLoading(loadedFrame.Data());
				}
				else
				{
					// There is another frame following this one, so this one is closed !!
					m_Info.GroupOfPicturesLoading(loadedFrame);
				}
				m_NewGroupOfPicturesHeader = true;
			}
			else if  (SliceHeader::IsValid(m_Buffer, m_Index) == true)
			{
				// Move on to the next tag !
				m_Index = MPEGHeader::FindTag(m_Buffer, (m_Index+4));
			}
			else if (MetaDataSection(m_Buffer, m_Index) == false)
			{
				// That is unexpected, log some trace line
				TRACE_L1 ("unexpected MPEG element <%X>", MPEGHeader::TagId(m_Buffer, m_Index));

				// Move on to the next tag !
				m_Index = MPEGHeader::FindTag(m_Buffer, (m_Index+3));
			}
		}

		// Did we end-up on a correct Picture header ?
		if (m_Index < m_Buffer.Size())
		{
			// Oops moving into a new Picture, record it..
			m_PlayingTime += PictureFrameTime();

			MPEG::PictureHeader loadedFrame = MPEG::PictureHeader(m_Buffer, m_Index);

			// We got this one, jump over it, and prepare for the next ... 
			m_Index += loadedFrame.Size ();

			// See if it is the last frame, if so we need to get the next frame as well
			if (m_Index == m_Buffer.Size())
			{
				// Oops, last frame, continue loading in the next elemntary stream.
				m_Info.PictureLoading(loadedFrame.Data());
			}
			else
			{
				// There is another frame following this one, so this one is closed !!
				m_Info.PictureLoading(loadedFrame);
			}
		}
	}

	return ( (m_Index < m_Buffer.Size()) && (m_Info.GroupOfPicturesHeader().IsValid()) );
}

bool MPEGVideo::MPEGElement::NextSlice()
{
	m_NewGroupOfPicturesHeader = false;
	m_NewPictureHeader = false;

	if ((m_Index < m_Buffer.Size()) || (m_Index == NUMBER_MAX_UNSIGNED(uint64)))
	{
		if (m_Index == NUMBER_MAX_UNSIGNED(uint64))
		{
			m_Index = 0;
		}

		m_Index = MPEGHeader::FindTag(m_Buffer, m_Index);

		while ((m_Index < m_Buffer.Size()) && (SliceHeader::IsValid(m_Buffer, m_Index) == false))
		{
			if (GroupOfPicturesHeader::IsValid(m_Buffer, m_Index))
			{
				if  (m_NewGroupOfPicturesHeader == true)
				{
					// That is unexpected, log some trace line
					TRACE_L1 ("Another GOP header without a Picture header contained", 0);
				}

				// Oops moving into a new GOP, record it..
				MPEG::GroupOfPicturesHeader loadedFrame = MPEG::GroupOfPicturesHeader(m_Buffer, m_Index);

				// Handled it, add it to the index..
				m_Index += loadedFrame.Size();

				// See if it is the last frame, if so we need to get the next frame as well
				if (m_Index == m_Buffer.Size())
				{
					// Oops, last frame, continue loading in the next elemntary stream.
					m_Info.GroupOfPicturesLoading(loadedFrame.Data());
				}
				else
				{
					// There is another frame following this one, so this one is closed !!
					m_Info.GroupOfPicturesLoading(loadedFrame);
				}
				m_NewGroupOfPicturesHeader = true;
			}
			else if (PictureHeader::IsValid(m_Buffer, m_Index))
			{
				if  (m_NewPictureHeader == true)
				{
					// That is unexpected, log some trace line
					TRACE_L1 ("Another GOP header without a Picture header contained", 0);
				}

				// Oops moving into a new GOP, record it..
				MPEG::PictureHeader loadedFrame = MPEG::PictureHeader(m_Buffer, m_Index);

				// Handled it, add it to the index..
				m_Index += loadedFrame.Size();

				// See if it is the last frame, if so we need to get the next frame as well
				if (m_Index == m_Buffer.Size())
				{
					// Oops, last frame, continue loading in the next elemntary stream.
					m_Info.PictureLoading(loadedFrame.Data());
				}
				else
				{
					// There is another frame following this one, so this one is closed !!
					m_Info.PictureLoading(loadedFrame);
				}
				m_NewPictureHeader = true;
			}
			else  if (MetaDataSection(m_Buffer, m_Index) == false)
			{
				// That is unexpected, log some trace line
				TRACE_L1 ("unexpected MPEG element <%X>", MPEGHeader::TagId(m_Buffer, m_Index));

				// Move on to the next tag !
				m_Index = MPEGHeader::FindTag(m_Buffer, (m_Index+3));
			}
		}
	}

	// Did we end-up on a correct Picture header ?
	if (m_Index < m_Buffer.Size())
	{
		MPEG::SliceHeader loadedFrame = MPEG::SliceHeader(m_Buffer, m_Index);

		// We got this one, jump over it, and prepare for the next ... 
		m_Index += loadedFrame.Size ();

		// See if it is the last frame, if so we need to get the next frame as well
		if (m_Index == m_Buffer.Size())
		{
			// Oops, last frame, continue loading in the next elemntary stream.
			m_Info.SliceLoading(loadedFrame.Data());
		}
		else
		{
			// There is another frame following this one, so this one is closed !!
			m_Info.SliceLoading(loadedFrame);
		}
	}

	return ( m_Info.IsValid() );
}

uint32 MPEGVideo::MPEGElement::PictureFrameTime() const
{
	uint32 frameDuration = 0;

	if (m_Info.PictureHeader().IsValid())
	{
		// Extract the duration of this frame from it...
		frameDuration = m_Info.PictureHeader().Delay();
	}
	return (frameDuration);
}

bool MPEGVideo::MPEGElement::MetaDataSection(const Generics::DataElement& buffer, const uint64 offset)
{
	bool result = false;

	// Check for Sequence, ExtendedSequence or UserData
	if (ExtendedSequenceHeader::IsValid(buffer, offset) == true)
	{
		ExtendedSequenceHeader info (buffer, offset);

		result = true;
	}
	else if (SequenceHeader::IsValid(buffer, offset) == true)
	{
		SequenceHeader info (buffer,offset);

		result = true;
	}
	else if (UserDataHeader::IsValid(buffer, offset) == true)
	{
		UserDataHeader info (buffer,offset);

		result = true;
	}

	return (result);
}

//--------------------------------------------------------------------------------------------
// Class: MPEGVideo
//--------------------------------------------------------------------------------------------

MPEGVideo::MPEGVideo () :
	m_LoadedInfo(),
	m_Index(m_LoadedInfo),
	m_Source(),
	m_BaseTime(0)
{
}

MPEGVideo::MPEGVideo (const  MPEGVideo& copy) :
	m_LoadedInfo(copy.m_LoadedInfo),
	m_Index(copy.m_Index),
	m_Source(copy.m_Source),
	m_BaseTime(copy.m_BaseTime)
{
}

MPEGVideo::MPEGVideo (const MPEGStream& stream) :
	m_LoadedInfo(),
	m_Index(m_LoadedInfo),
	m_Source(stream),
	m_BaseTime(0)
{
	Reset();
}

MPEGVideo& MPEGVideo::operator=  (const MPEGVideo& RHS)
{
	m_Index		 = RHS.m_Index;
	m_Source	 = RHS.m_Source;
	m_BaseTime	 = RHS.m_BaseTime;
	m_LoadedInfo = RHS.m_LoadedInfo;

	return (*this);
}

bool MPEGVideo::NextGOP()
{
	bool valid = m_Index.NextGOP();

	while ( (valid == false) &&  (m_Source.Next() != false) )
	{
		m_Index = MPEGVideo::MPEGElement (m_Source.Section(), 0, m_LoadedInfo);

		valid = m_Index.NextGOP();
	}

	return (valid);
}

bool MPEGVideo::NextPicture()
{
	bool valid = m_Index.NextPicture();

	while ( (valid == false) &&  (m_Source.Next() != false) )
	{
		m_Index = MPEGVideo::MPEGElement (m_Source.Section(), 0, m_LoadedInfo);

		valid = m_Index.NextPicture();
	}

	return (valid);
}

bool MPEGVideo::NextSlice()
{
	bool valid = m_Index.NextSlice();

	while ( (valid == false) &&  (m_Source.Next() != false) )
	{
		m_Index = MPEGVideo::MPEGElement (m_Source.Section(), 0, m_LoadedInfo);

		valid = m_Index.NextSlice();
	}

	return (valid);
}


} } // namespace Solutions::MPEG
