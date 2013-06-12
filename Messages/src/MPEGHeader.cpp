#include "MPEGHeader.h"

namespace Solutions { namespace MPEG
{

// -------------------------------------------------------------------------------------------------------------
// MPEG: tooling/conveniance methods
// -------------------------------------------------------------------------------------------------------------
uint64 FindHeaderOrMacro (const Generics::DataElement& data, const uint64 offset)
{
	static const uint8 SearchKey[] = { 0x00, 0x00 };
	uint64 index = offset;

	while ( (index + sizeof(SearchKey) < data.Size()) &&
		    ((index = data.Search (offset, SearchKey, sizeof(SearchKey))) <= data.Size()) && 
		    (data[static_cast<uint32>(index+2)] != 0x00) && (data[static_cast<uint32>(index+2)] != 0x01) )
	{
		// Intentionally left empty !!!
	}

	return (index + sizeof(SearchKey) < data.Size() ? index : data.Size ());
}

uint64 FindMacro (const Generics::DataElement& data, const uint64 offset)
{
	static const uint8 SearchKey[] = { 0x00, 0x00, 0x00 };

	return ((offset + sizeof(SearchKey) > data.Size()) ? data.Size() : data.Search (offset, SearchKey, sizeof(SearchKey)));
}

uint64 FindHeader (const Generics::DataElement& data, const uint64 offset)
{
	static const uint8 SearchKey[] = { 0x00, 0x00, 0x01 };

	return ((offset + sizeof(SearchKey) > data.Size()) ? data.Size() : data.Search (offset, SearchKey, sizeof(SearchKey)));
}

uint64 FindHeader (const Generics::DataElement& data, const uint64 offset, const uint8 marker)
{
	uint64 result = offset;

	while ( ((result = FindHeader (data, result)) < data.Size()) && (data[static_cast<uint32>(result)+3] != marker) )
	{
		// Move over the header, this is not the correct one
		result += 3;
	}

	return (result);
}

//uint64 TimeStamp2(const Generics::DataElement& data, const uint64 offset)
//{
//	return (((data[static_cast<uint32>(offset)]   & 0x18) >> 3) << 30) | 
//		    ((data[static_cast<uint32>(offset)]   & 0x03) << 28)       | 
//		     (data[static_cast<uint32>(offset)+1] << 20)               | 
//		    ((data[static_cast<uint32>(offset)+2] & 0xF8) << 12)       | 
//		    ((data[static_cast<uint32>(offset)+2] & 0x03) << 13)       | 
//			 (data[static_cast<uint32>(offset)+3] << 5)                | 
//			 (data[static_cast<uint32>(offset)+4] >> 3);
//
//	//uint16 clock =  ((data[static_cast<uint32>(offset)+4] & 0x03) << 7)        | 
//	//	             (data[static_cast<uint32>(offset)+5] >> 1);
//
//	//if (clock == 0)
//	//{
//	//	return (static_cast<double>((((data[static_cast<uint32>(offset)] >> 5) & 0x01) ? 0x100000000L : 0) + stamp) / 
//	//			static_cast<double>(90000));
//	//}
//
//	//// this is what I understood... CHECK
//	//// if not zero, then we have a 27 MHz accuracy with a max of 27 MHz
//	//// so clockfreq might well be 27MHz / clock
//	////  nonsense: TS /= (double)(27000000 / clock);
//	//return (static_cast<double>((((data[static_cast<uint32>(offset)] >> 5) & 0x01) ? 0x100000000L : 0) + stamp) / 
//	//		static_cast<double>(90000));
//}
//

// -------------------------------------------------------------------------------------------------------------
// MPEG CLASS: AudioHeader
// -------------------------------------------------------------------------------------------------------------
/* static */ uint64 AudioHeader::FindTag (const Generics::DataElement& data, const uint64 offset)
{
	static const uint8 HEADER = 0xFF;

	bool   found = false;
	uint64 location = offset;

	while ((found == false) && (location < data.Size()) && ((location = data.Search (location, &HEADER, sizeof(HEADER))) < data.Size()))
	{
		// we found the 011111111b pattern. See if the next two bit also contain 11
		if ((data.Buffer()[location + 1] & 0xE0) == 0xE0)
		{
			// Check if this location contains a valid header
			MPEG::AudioHeader info = MPEG::AudioHeader (data, location);

			if (info.IsValid())
			{
				// Do an additional check, see if after this frame (which should be correct) we
				// find another valid header...
				found = ( (info.BlockSize() + location) <= data.Size() );
			}

			if (found == false)
			{
				// The current location could be 0xFF, keep this in mind, only progress 1.
				location += 1;
			}
		}
		else
		{
			// If not all bits in 0xE0 are set, it is definitely not a 0xFF, skip this E byte as wel..
			location += 2;
		}
	}

	return (location);
}

AudioHeader::AudioHeader(const Generics::DataElement& value, const uint64 offset) :
	m_Value (value, offset)
{
	if ( (IsValid() != true) || (m_Value.Size() < BlockSize()) )
	{
		m_Value = Generics::DataElement();
	}
	else
	{
		m_Value.Size(BlockSize());
	}
}

uint16 AudioHeader::BitRate () const
{
	static uint16 table[2][3][16] = {
	   // MPEG 1
	   { {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,0},   // layer 1
		 {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,0},   // layer 2
		 {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,0} }, // layer 3

	   // MPEG 2
	   { {0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,0},       // layer 1
		 {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0},            // layer 2
		 {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0} }           // layer 3
	};
	
	return (table[IsMPEG2() ? 1 : 0][Layer()-1][(m_Value[2]>>4)]);
}

uint16 AudioHeader::SamplingFrequency () const
{
	uint16 result = 0;

	if (IsMPEG25 ())
	{
		static uint16 table[3] = { 11025, 12000, 8000 }; // mpeg 2.5

		result = table[((m_Value[2] >> 2) & 0x03)];
	}
	else if (IsMPEG2())
	{
		static uint16 table[3] = { 22050, 24000, 16000 }; // mpeg 2

		result = table[((m_Value[2] >> 2) & 0x03)];
	}
	else
	{
		static uint16 table[3] = { 44100, 48000, 32000 }; // mpeg 1

		result = table[((m_Value[2] >> 2) & 0x03)];
	}

	return (result);
}

uint32 AudioHeader::BlockSize () const
{
	uint32 framesize = BitRate() * ((Layer() == 1) ? 12000*4 : 144000);

	framesize /= (SamplingFrequency() << (IsMPEG2() ? 1 : 0));

	return (framesize + ((m_Value[2] >> 1) & 0x1) + (HasCRC() ? 2 : 0));
}

uint32 AudioHeader::FrameDuration () const
{
	static unsigned samplesByLayer[4] = { 0, 384000, 1152000, 1152000 };

	uint32 samples = samplesByLayer[Layer()];
	uint32 frequency = (SamplingFrequency() << (IsMPEG2() ? 1 : 0));

	// result is Number of Samples / Frequency
  	return (samples / frequency); // rounds to nearest integer (ms)
}

// -------------------------------------------------------------------------------------------------------------
// MPEG CLASS: MPEGHeader
// -------------------------------------------------------------------------------------------------------------
MPEGHeader::MPEGHeader(const Generics::DataElement& data, const uint64 offset) : 
	m_Value(),
	m_TagId (NUMBER_MAX_UNSIGNED(uint16))
{
	// Find the right MPEG header..
	uint64 startIndex = FindHeader(data, offset);

	if (startIndex <= data.Size())
	{
		// Seems like we found an MPEG header before reaching the end..
		m_Value = Generics::DataElement(data, startIndex);
	}
}

MPEGHeader::MPEGHeader(const uint8 tagId, const Generics::DataElement& data, const uint64 offset) : 
	m_Value(),
	m_TagId (tagId)
{
	// Find the right MPEG header..
	uint64 startIndex = FindHeader(data, offset, tagId);

	if (startIndex <= data.Size())
	{
		// Seems like we found an MPEG header before reaching the end..
		m_Value = Generics::DataElement(data, startIndex);
	}
}

uint64 MPEGHeader::FindTag (const Generics::DataElement& data, const uint64 offset)
{
	return (FindHeader(data, offset));
}

const TCHAR* MPEGHeader::TagText () const
{
	static TCHAR unknownTag[40];

	switch (TagId())
	{
	case PICTURE_MARKER: 
		return (_T("Picture"));
		break;
	case SEQUENCE_MARKER:
		return (_T("Sequence"));
		break;
	case SEQUENCE_EXTENDED_MARKER:
		return (_T("Extended Sequence"));
		break;
	case GROUP_OF_PICTURES_MARKER:
		return (_T("Group Of Pictures"));
		break;
	case USER_DATA_MARKER:
		return (_T("User Data"));
		break;
	case PADDING_STREAM_MARKER:
		return (_T("Padding Stream"));
		break;
	case SYSTEM_STREAM_MARKER:
		return (_T("System Stream"));
		break;
	case PACK_STREAM_MARKER:
		return (_T("Pack Stream"));
		break;
	default:
		if (SliceHeader::IsValid(*this))
		{
			return (_T("Slice"));
		}
		else if (ElementaryStreamHeader::IsValid(*this))
		{
			return (_T("Elementary Stream"));
		}
		else
		{
			sprintf_s (unknownTag, sizeof (unknownTag), "Unknown tag <%d>", TagId());
		}
		break;
	}

	return (unknownTag);
}

void MPEGHeader::ExpectedLength (const uint32 length)
{
	if (length <= m_Value.Size())
	{
		// The derived class has indictaed to us, a "calculated length", lets adjust the buffer
		m_Value = Generics::DataElement(m_Value, 0, length);
	}
	else
	{
		// Seems that the derived class thinks that more data should be present, assume it is an incorrect header
		m_Value = Generics::DataElement();
	}
}

// -------------------------------------------------------------------------------------------------------------
// MPEG CLASS: PictureHeader
// -------------------------------------------------------------------------------------------------------------
PictureHeader::PictureHeader(const MPEGHeader& data) :
	MPEGHeader(data, PictureHeader::ID)
{
	ExpectedLength(CalculateLength());
}

PictureHeader::PictureHeader(const Generics::DataElement& data, const uint64 offset) :
	MPEGHeader (PictureHeader::ID, data, offset)
{
	ExpectedLength(CalculateLength());
}

uint32 PictureHeader::CalculateLength() const
{
	uint32 result = 0;

	if (MPEGHeader::IsValid())
	{
		result = static_cast<uint32>(FindHeader (Data(), 4));
	}

	return (result);
}

// -------------------------------------------------------------------------------------------------------------
// MPEG CLASS: SliceHeader
// -------------------------------------------------------------------------------------------------------------
SliceHeader::SliceHeader(const MPEGHeader& data) :
	MPEGHeader(data, data.TagId())
{
	ExpectedLength(CalculateLength());
}

SliceHeader::SliceHeader(const Generics::DataElement& data, const uint64 offset) :
	MPEGHeader (data, offset)
{
	ExpectedLength(CalculateLength());
}

uint32 SliceHeader::CalculateLength() const
{
	uint32 result = 0;

	if (MPEGHeader::IsValid())
	{
		// Officiale, there are first Passing bytes but they start with a high bit,
		// as we will be looking for a new header after the start position, and we
		// know that this header starts looking for two '0', by definition, it will
		// not stop in the padding, so do not search the padding bytes, its useless
		// for the parser ;-)
		// result = 7;

		//while ((*this)[result] & 0x80)
		//{
		//	result++;
		//}

		// Time to find the next header
		result = static_cast<uint32>(FindHeader(Data(), 4));
	}

	return (result);
}

// -------------------------------------------------------------------------------------------------------------
// MPEG CLASS: UserDataHeader
// -------------------------------------------------------------------------------------------------------------
UserDataHeader::UserDataHeader(const MPEGHeader& data) :
	MPEGHeader(data, UserDataHeader::ID),
	m_UserText()
{
	ExtractUserData();
}

UserDataHeader::UserDataHeader(const Generics::DataElement& data, const uint64 offset) :
	MPEGHeader(UserDataHeader::ID, data, offset),
	m_UserText()
{
	ExtractUserData();
}

void UserDataHeader::ExtractUserData()
{
	if (MPEGHeader::IsValid ())
	{
		// Find the next Marker fo the end..
		// Find the right MPEG header..
		uint64 length = FindHeader(MPEGHeader::Data(), 0);

		if (length >= NUMBER_MAX_UNSIGNED(uint16))
		{
			// That's to big, this is invalid...
			MPEGHeader::ExpectedLength (0);
		}
		else
		{
			// Seems like we actually have user data, translate it to the right string..
			String userData;
			uint16 bytesToHandle = static_cast<uint16>(length);
			uint16 index = 0;

			// Set the current length..
			MPEGHeader::ExpectedLength (bytesToHandle);

			// we have enough space by definition..
			userData.reserve(bytesToHandle+1);

			while (index != bytesToHandle)
			{
				if (isprint ((*this)[index]))
				{
					// Add the byte to the string it is redable..
					userData.push_back(static_cast<TCHAR>((*this)[index]));
				}
				index++;
			}

			// Close this sequence with a 0
			userData.push_back(static_cast<TCHAR>(0));

			m_UserText = Generics::TextFragment(userData);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------
// MPEG CLASS: SequenceHeader
// -------------------------------------------------------------------------------------------------------------
SequenceHeader::SequenceHeader(const MPEGHeader& data) :
	MPEGHeader(data, SequenceHeader::ID)
{
	ExpectedLength(CalculateLength());
}

SequenceHeader::SequenceHeader(const Generics::DataElement& data, const uint64 offset) :
	MPEGHeader (SequenceHeader::ID, data, offset)
{
	ExpectedLength(CalculateLength());
}

uint32 SequenceHeader::CalculateLength() const
{
	uint32 result = 0;

	if (MPEGHeader::IsValid())
	{
		result = ((((*this)[11] & 0x03) != 0) ? (12 + 64) : 12);
	
		ASSERT (MPEGHeader::IsValid(MPEGHeader::Data(), result) == true);
	}

	return (result);
}

// -------------------------------------------------------------------------------------------------------------
// MPEG CLASS: ExtendedSequenceHeader
// -------------------------------------------------------------------------------------------------------------
ExtendedSequenceHeader::ExtendedSequenceHeader(const MPEGHeader& data) :
	MPEGHeader (data, ExtendedSequenceHeader::ID)
{
	ExpectedLength(CalculateLength());
}

ExtendedSequenceHeader::ExtendedSequenceHeader(const Generics::DataElement& data, const uint64 offset) :
	MPEGHeader (ExtendedSequenceHeader::ID, data, offset)
{
	ExpectedLength(CalculateLength());
}

uint32 ExtendedSequenceHeader::CalculateLength() const
{
	uint32 result = 0;

	if (MPEGHeader::IsValid())
	{
		if (IsSequenceExtension())
		{
			result = 10;
		}
		else if (IsDisplayExtension())
		{
			result = (HasColorDescription() ? 12 :9);
		}
		else if (IsPictureExtension())
		{
			result = (HasCompositeDisplay() ? 11 : 9);
		}
	}
	return (result);
}

// -------------------------------------------------------------------------------------------------------------
// MPEG CLASS: SystemStreamHeader
// -------------------------------------------------------------------------------------------------------------
SystemStreamHeader::SystemStreamHeader(const MPEGHeader& data) :
	MPEGHeader (data, SystemStreamHeader::ID)
{
	ExpectedLength(CalculateLength());
}

SystemStreamHeader::SystemStreamHeader(const Generics::DataElement& data, const uint64 offset) :
	MPEGHeader (SystemStreamHeader::ID, data, offset)
{
	ExpectedLength(CalculateLength());
}

uint32 SystemStreamHeader::CalculateLength() const
{
	uint32 result =0;

	if (MPEGHeader::IsValid())
	{
		m_Streams = Iterator (MPEGHeader::Data(), 12);

		result = (12 + m_Streams.Count() * 3);
	}

	return (result);
}
// -------------------------------------------------------------------------------------------------------------
// MPEG CLASS: GroupOfPicturesHeader
// -------------------------------------------------------------------------------------------------------------
GroupOfPicturesHeader::GroupOfPicturesHeader(const MPEGHeader& data) :
	MPEGHeader (data, GroupOfPicturesHeader::ID)
{
	ExpectedLength(CalculateLength());
}

GroupOfPicturesHeader::GroupOfPicturesHeader(const Generics::DataElement& data, const uint64 offset) :
	MPEGHeader (GroupOfPicturesHeader::ID, data, offset)
{
	ExpectedLength(CalculateLength());
}

// -------------------------------------------------------------------------------------------------------------
// MPEG CLASS: PaddingStreamHeader
// -------------------------------------------------------------------------------------------------------------
PaddingStreamHeader::PaddingStreamHeader(const MPEGHeader& data) :
	MPEGHeader (data, PaddingStreamHeader::ID)
{
	ExpectedLength(CalculateLength());
}

PaddingStreamHeader::PaddingStreamHeader(const Generics::DataElement& data, const uint64 offset) :
	MPEGHeader (PaddingStreamHeader::ID, data, offset)
{
	ExpectedLength(CalculateLength());
}

uint32 PaddingStreamHeader::CalculateLength() const
{
	uint32 result = 0;

	if (PaddingStreamHeader::IsValid())
	{
		result = (*this).GetNumber<uint16, Generics::ENDIAN_BIG>(4) + 6;

		// Search up till the next "start point, we might find some additional data...
		result =  static_cast<uint32>(MPEGHeader::FindTag(result));
	}

	return (result);
}

// -------------------------------------------------------------------------------------------------------------
// MPEG CLASS: ElementaryStreamHeader
// -------------------------------------------------------------------------------------------------------------
ElementaryStreamHeader::ElementaryStreamHeader(const MPEGHeader& data) :
	MPEGHeader (data, data.TagId())
{
	if (MPEGHeader::IsValid())
	{
		ExpectedLength(CalculateLength());
	}
}

ElementaryStreamHeader::ElementaryStreamHeader(const Generics::DataElement& data, const uint64 offset) :
	MPEGHeader (data, offset)
{
	if (MPEGHeader::IsValid())
	{
		ExpectedLength(CalculateLength());
	}
}

ElementaryStreamHeader::ElementaryStreamHeader(const Generics::DataElement& data, const uint64 offset, const uint8 streamId) :
	MPEGHeader (streamId, data, offset)
{
	ASSERT ((streamId >= 0xBD) && (streamId <= 0xEF));

	if (MPEGHeader::IsValid())
	{
		ExpectedLength(CalculateLength());
	}
}

uint32 ElementaryStreamHeader::CalculateLength() const
{
	uint32 result = 0;

	if (ElementaryStreamHeader::IsValid())
	{
		result = (*this).GetNumber<uint16, Generics::ENDIAN_BIG>(4) + 6;

		// Search up till the next "start point, we might find some additional data...
		result =  static_cast<uint32>(MPEGHeader::FindTag(result));
	}

	return (result);
}

uint32  ElementaryStreamHeader::CalculatedHeaderSize() const
{
	uint32 index = 6;

	if (HasExtension())
	{
		index = ExtendedExtensionOffset() + ExtendedExtensionSize();

		while ((*this)[index] == 0xFF)
		{
			++index;
		}
	}

	return (index);
}

// -------------------------------------------------------------------------------------------------------------
// MPEG CLASS: PackStreamHeader
// -------------------------------------------------------------------------------------------------------------
PackStreamHeader::PackStreamHeader(const MPEGHeader& data) :
	MPEGHeader (data, PackStreamHeader::ID)
{
	if (MPEGHeader::IsValid())
	{
		ExpectedLength(CalculateLength());
	}
}

PackStreamHeader::PackStreamHeader(const Generics::DataElement& data, const uint64 offset) :
	MPEGHeader (PackStreamHeader::ID, data, offset)
{
	if (MPEGHeader::IsValid())
	{
		ExpectedLength(CalculateLength());
	}
}

uint32 PackStreamHeader::CalculateLength() const
{
	uint32 index = 0;
	
	if (MPEGHeader::IsValid())
	{
		if (IsPacked ())
		{
			index = 12;
		}
		else
		{
			//It seems that it is possible that the Packstream header does not caontain 
			// a mux rate. This can be seen by the sticky markers "1".
			index = 14 + ((*this)[13] & 0x7);

			// all bytes that equal 0xFF are stuffing bytes, include these with this header...
			while ((*this)[index] == 0xFF)
			{
				++index;
			}
		}
	}

	return (index);
}

} } // namespace Solutions::MPEG