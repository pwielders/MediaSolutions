#include "RTPMessage.h"

namespace Solutions { namespace RTP
{

Message::Message(const uint8 version) :
	m_Buffer(Generics::DataStorage::Create(RTP_MINIMUM_HEADER_SIZE + (16 * 4) + RTP_HEADER_EXTENSION_SIZE))
{
	m_ExtensionOffset = 0;
	m_ExtensionSize = 0;

	// Start off by default everthing off, seems to be most common ;-)
	m_Buffer[0] = 0;
	m_Buffer[1] = 0;

	Version(version);

	//TODO: Qualify this to less, using the PayLoadType !!!
	m_MaximumHeaderSize = (RTP_MINIMUM_HEADER_SIZE + RTP_HEADER_EXTENSION_SIZE);
}

Message::~Message ()
{
}

uint8 Message::Version () const
{
	return ((m_Buffer[0] & 0xC0) >> 6);
}

void Message::Version (const uint8 version)
{
	ASSERT (version <= 0x03);

	m_Buffer[0] = (m_Buffer[0] & 0x3F) | ((version & 0x03) << 6);
}

bool Message::HasPadding () const
{
	return ((m_Buffer[0] & 0x20) != 0);
}

void Message::HasPadding (const bool padding)
{
	m_Buffer[0] = (m_Buffer[0] & 0xDF) | (padding ? 0x20 : 0x00);
}

bool Message::HasExtension () const
{
	return ((m_Buffer[0] & 0x10) != 0);
}

void Message::HasExtension (const bool extension)
{
	m_Buffer[0] = (m_Buffer[0] & 0xEF) | (extension ? 0x10 : 0x00);
}

uint8 Message::ContributingSourceCount () const
{
	return (m_Buffer[0] & 0x0F);
}

bool Message::HasMarker () const
{
	return ((m_Buffer[1] & 0x80) != 0);
}

void Message::HasMarker (const bool marker)
{
	m_Buffer[1] = (m_Buffer[1] & 0x7F) | (marker ? 0x80 : 0x00);
}

uint8 Message::PayLoadType () const
{
	return (m_Buffer[1] & 0x7F);
}

void Message::PayLoadType (const uint8 type)
{
	ASSERT (type <= 0x7F);

	m_Buffer[1] = (m_Buffer[1] & 0x80) | (type & 0x7F);
}

uint32 Message::Size () const
{
	return (RTP_MINIMUM_HEADER_SIZE + (ContributingSourceCount() * 4) + m_ExtensionSize);
}

const Generics::DataElement Message::Data() const
{
	return (Generics::DataElement (m_Buffer, 0, Size()));
}

uint16 Message::Sequence () const
{
	return ((m_Buffer[2] << 8) | (m_Buffer[3]));
}

void Message::Sequence (const uint16 sequenceNumber)
{
	m_Buffer[2] = static_cast<uint8>((sequenceNumber & 0xFF00) >> 8);
	m_Buffer[3] = static_cast<uint8> (sequenceNumber & 0x00FF);
}

uint32 Message::TimeStamp () const
{
	return ((m_Buffer[4] << 24) | (m_Buffer[5] << 16) |(m_Buffer[6] << 8) | (m_Buffer[7]));
}

void Message::TimeStamp (const uint32 timeStamp)
{
	m_Buffer[4] = static_cast<uint8>((timeStamp & 0xFF000000) >> 24);
	m_Buffer[5] = static_cast<uint8>((timeStamp & 0x00FF0000) >> 16);
	m_Buffer[6] = static_cast<uint8>((timeStamp & 0x0000FF00) >>  8);
	m_Buffer[7] = static_cast<uint8> (timeStamp & 0x000000FF);
}

uint32 Message::SessionSource () const
{
	return ((m_Buffer[8] << 24) | (m_Buffer[9] << 16) |(m_Buffer[10] << 8) | (m_Buffer[11]));
}

void Message::SessionSource (const uint32 sourceId)
{
	m_Buffer[8]  = static_cast<uint8>((sourceId & 0xFF000000) >> 24);
	m_Buffer[9]  = static_cast<uint8>((sourceId & 0x00FF0000) >> 16);
	m_Buffer[10] = static_cast<uint8>((sourceId & 0x0000FF00) >>  8);
	m_Buffer[11] = static_cast<uint8> (sourceId & 0x000000FF);
}

uint32 Message::ContributingSourceEntry (const uint8 index) const
{
	ASSERT (index <= 0x0F);

	return ((m_Buffer[RTP_MINIMUM_HEADER_SIZE +     (index*4)] << 24) | 
		    (m_Buffer[RTP_MINIMUM_HEADER_SIZE + 1 + (index*4)] << 16) |
			(m_Buffer[RTP_MINIMUM_HEADER_SIZE + 2 + (index*4)] << 8)  | 
			(m_Buffer[RTP_MINIMUM_HEADER_SIZE + 3 + (index*4)]));
}

void Message::AddContributingSourceEntry (const uint32 contributingSource)
{
	uint8 index = (m_Buffer[0] & 0x0F);

	ASSERT (index < 0x0F);

	m_Buffer[0] = (m_Buffer[0] & 0xF0) | (index + 1);

	// Check if we need to move the extension!!
	if (m_ExtensionOffset != 0)
	{
		const uint8* source      = &m_Buffer[m_ExtensionOffset + m_ExtensionSize];
		uint8*       destination = &m_Buffer[m_ExtensionOffset + m_ExtensionSize + 4];
		uint32       count       = m_ExtensionSize;

		while (count != 0)
		{
			count--;
			*destination-- = *source--;
		}
		m_ExtensionOffset += 4;
	}

	m_Buffer[RTP_MINIMUM_HEADER_SIZE + 0 + (index*4)] = static_cast<uint8>((contributingSource & 0xFF000000) >> 24);
	m_Buffer[RTP_MINIMUM_HEADER_SIZE + 1 + (index*4)] = static_cast<uint8>((contributingSource & 0x00FF0000) >> 16);
	m_Buffer[RTP_MINIMUM_HEADER_SIZE + 2 + (index*4)] = static_cast<uint8>((contributingSource & 0x0000FF00) >>  8);
	m_Buffer[RTP_MINIMUM_HEADER_SIZE + 3 + (index*4)] = static_cast<uint8> (contributingSource & 0x000000FF);
}

const Generics::DataElement Message::Extension () const
{
	return (Generics::DataElement(m_Buffer, m_ExtensionOffset, m_ExtensionSize));
}

// The insertion of the extension should be the last step as it is not taken into account
// the calculation of the RTP Header. The Size (of the package) will however be adjusted.
// After adding an extension, do NOT modify the header anymore !!!
void Message::Extension (const Generics::DataElement& data)
{
	ASSERT (data.Size() < RTP_HEADER_EXTENSION_SIZE);

	m_ExtensionOffset = RTP_MINIMUM_HEADER_SIZE + (ContributingSourceCount() * 4);
	m_ExtensionSize = static_cast<uint32>(data.Size());

	// Insert the extension 
	::memcpy (&m_Buffer[m_ExtensionOffset], data.Buffer(), m_ExtensionSize);
}

} } // namespace Solutions::RTP