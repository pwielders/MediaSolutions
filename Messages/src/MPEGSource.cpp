#include "MPEGSource.h"
#include "Tracing/include/Tracing.h"

namespace Solutions { namespace MPEG
{

//--------------------------------------------------------------------------------------------
// Class: MPEGSource
//--------------------------------------------------------------------------------------------
MPEGSource::MPEGSource (const Generics::DataElement& dataObject) : 
	m_Data()
{
	PackStreamHeader lastTimeStamp;
	bool readyForIteration = false;
	uint32 offset = 0;

	// Find the first Header
	MPEGHeader startPoint(dataObject, offset);

	while ((readyForIteration == false) && (startPoint.IsValid()))
	{
		// By definitian, if we find an incorrect header size, we would like 
		// to move forward. We are on a valid startpoint, so it is save to 
		// assume we can progress by 3 (00:00:01)
		uint32 headerSize = 3;

		// Display the section that we are processing !!!
		TRACE(Trace::TraceInformationText, (startPoint.TagText()));

		// -------------------------------------------------------
		// Outside of the stream...
		// -------------------------------------------------------
		if (SystemStreamHeader::IsValid(startPoint.TagId()))
		{
			SystemStreamHeader systemStreamHeader (startPoint);
			if (systemStreamHeader.IsValid() == false)
			{
				// If we find it here, it means it is a simple strem, 1 in a
				readyForIteration = true;

				// It is potentially a multi stream object. Lets define all streams and find them
				SystemStreamHeader::Iterator index = systemStreamHeader.Streams();

				while (index.Next())
				{
					StreamInfo newStream;
					newStream.id = index.ID();
					newStream.type = (index.IsAudio() ? AUDIO : (index.IsVideo() ? VIDEO : UNKNOWN));
					newStream.offset = offset;
					m_ChannelsOffset.push_back(newStream);
				}

				headerSize = systemStreamHeader.Size();
			}
		}
		else if (PaddingStreamHeader::IsValid(startPoint.TagId()))
		{
			// Just skip, nothing interesting in here
			PaddingStreamHeader header (startPoint);
			if (header.IsValid())
			{
				headerSize = header.Size();
			}
		}
		else if (PackStreamHeader::IsValid(startPoint.TagId()))
		{
			// Extract the clock from this, this identifies the actual time of 
			// playout with reference to the start in 90kHz.
			lastTimeStamp =  PackStreamHeader (startPoint);
			if (lastTimeStamp.IsValid())
			{
				headerSize = lastTimeStamp.Size();
			}
		}
		// -------------------------------------------------------
		// Inside the stream
		// -------------------------------------------------------
		else if (SequenceHeader::IsValid(startPoint.TagId()))
		{
			// If we find it here, it means it is a simple stream, 1 stream in it.
			readyForIteration = true;

			StreamInfo newStream;
			newStream.id = 0;
			newStream.type = VIDEO;
			newStream.offset = offset;
			m_ChannelsOffset.push_back(newStream);

			headerSize = 0;
		}
		else
		{
			// ExtendedSequenceHeader::ID:
			// UserDataHeader::ID:
			// GroupOfPicturesHeader::ID:
			// PictureHeader::ID:
			// Log a warning, this is strange, we did not expect this...
		}

		// ---------------------------------------------------------------------------------------------

		// This one is handled, Jump over it.
		offset += headerSize;

		if (readyForIteration == false)
		{
			// Are we on a marker ?
			if (MPEGHeader::IsValid(dataObject, offset) == false)
			{
				// Move forward to a marker.
				offset = static_cast<uint32>(MPEGHeader::FindTag(dataObject, offset));
			}

			// Find the Next Header
			startPoint = MPEGHeader(dataObject, offset);
		}
	}

	if (readyForIteration == true)
	{
		m_Data = Generics::DataElement (dataObject, offset);
	}
}

MPEGSource::~MPEGSource()
{
}

} } // namespace Solutions::MPEG
