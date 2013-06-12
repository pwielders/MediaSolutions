#include "RTCPMessage.h"

namespace Solutions { namespace RTCP
{

// RTCP packet types:
const unsigned char RTCP_PT_SR = 200;
const unsigned char RTCP_PT_RR = 201;
const unsigned char RTCP_PT_SDES = 202;
const unsigned char RTCP_PT_BYE = 203;
const unsigned char RTCP_PT_APP = 204;

// SDES tags:
const unsigned char RTCP_SDES_END = 0;
const unsigned char RTCP_SDES_CNAME = 1;
const unsigned char RTCP_SDES_NAME = 2;
const unsigned char RTCP_SDES_EMAIL = 3;
const unsigned char RTCP_SDES_PHONE = 4;
const unsigned char RTCP_SDES_LOC = 5;
const unsigned char RTCP_SDES_TOOL = 6;
const unsigned char RTCP_SDES_NOTE = 7;
const unsigned char RTCP_SDES_PRIV = 8;

//-------------------------------------------------------------------------------------
//-- Message::ReportBlock
//-------------------------------------------------------------------------------------
Message::ReportBlock::ReportBlock() : 
	m_Buffer()
{
}

Message::ReportBlock::ReportBlock(const Generics::DataElement& buffer) : 
	m_Buffer(Generics::DataStorage::Create(MAX_REPORTBLOCK_SIZE))
{
	ASSERT (buffer.Size() == MAX_REPORTBLOCK_SIZE);

	m_Buffer.Copy(buffer);
}

Message::ReportBlock::ReportBlock(const Message::ReportBlock& copy) : 
	m_Buffer(copy.m_Buffer)
{
}

Message::ReportBlock::~ReportBlock()
{
}

Message::ReportBlock& Message::ReportBlock::operator= (const ReportBlock& RHS) 
{
	m_Buffer = RHS.m_Buffer;

	return (*this);
}

Message::ReportBlock& Message::ReportBlock::operator= (const Generics::DataElement& RHS)
{
	ASSERT (RHS.Size() == MAX_REPORTBLOCK_SIZE);

	// Seems like we are preparing a block ?
	m_Buffer.Copy(RHS);

	return (*this);
}

uint32 Message::ReportBlock::SynchronisationSource () const
{
	return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_BIG>(0));
}

void Message::ReportBlock::SynchronisationSource (const uint32 value)
{
	m_Buffer.SetNumber<uint32, Generics::ENDIAN_BIG>(0, value);
}

uint8 Message::ReportBlock::FractionLost () const
{
	return (m_Buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(4));
}

void Message::ReportBlock::FractionLost (const uint8 value)
{
	m_Buffer.SetNumber<uint8, Generics::ENDIAN_BIG>(4, value);
}

uint32 Message::ReportBlock::TotalNumberOfPackagesLost () const
{
	return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_BIG>(4) & 0x00FFFFFF);
}

void Message::ReportBlock::TotalNumberOfPackagesLost (const uint32 value)
{
	uint32 result = (static_cast<uint32>(m_Buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(4)) << 24) | (value & 0x00FFFFFF); 
	m_Buffer.SetNumber<uint32, Generics::ENDIAN_BIG>(4, result);
}

uint32 Message::ReportBlock::HighestSequenceNumber () const
{
	return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_BIG>(8));
}

void Message::ReportBlock::HighestSequenceNumber (const uint32 value)
{
	m_Buffer.SetNumber<uint32, Generics::ENDIAN_BIG>(8, value);
}

uint32 Message::ReportBlock::Jitter () const
{
	return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_BIG>(12));
}

void Message::ReportBlock::Jitter (const uint32 value)
{
	m_Buffer.SetNumber<uint32, Generics::ENDIAN_BIG>(12, value);
}

uint32 Message::ReportBlock::LastTimeStamp () const
{
	return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_BIG>(16));
}

void Message::ReportBlock::LastTimeStamp (const uint32 value)
{
	m_Buffer.SetNumber<uint32, Generics::ENDIAN_BIG>(16, value);
}

uint32 Message::ReportBlock::DeltaDelay () const
{
	return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_BIG>(20));
}

void Message::ReportBlock::DeltaDelay (const uint32 value)
{
	m_Buffer.SetNumber<uint32, Generics::ENDIAN_BIG>(20, value);
}

const Generics::DataElement Message::ReportBlock::Data() const
{
	return (m_Buffer);
}

//-------------------------------------------------------------------------------------
//-- Message::ReportBlock::Iterator
//-------------------------------------------------------------------------------------
Message::ReportBlock::Iterator::Iterator() : 
    m_PreStart(true),
	m_Current(NUMBER_MAX_UNSIGNED(uint32)),
	m_Buffer()
{
}

Message::ReportBlock::Iterator::Iterator(const Generics::DataElement& buffer) :
    m_PreStart(true),
	m_Current(NUMBER_MAX_UNSIGNED(uint32)),
	m_Buffer(buffer)
{
}

Message::ReportBlock::Iterator::Iterator(const Message::ReportBlock::Iterator& copy) :
    m_PreStart(true),
	m_Current(NUMBER_MAX_UNSIGNED(uint32)),
	m_Buffer(copy.m_Buffer)
{
}

Message::ReportBlock::Iterator::~Iterator()
{
}

Message::ReportBlock::Iterator& Message::ReportBlock::Iterator::operator= (const Message::ReportBlock::Iterator& copy)
{
    m_PreStart = copy.m_PreStart;
	m_Current = copy.m_Current;
	m_Buffer = copy.m_Buffer;

	return (*this);
}

uint8 Message::ReportBlock::Iterator::Count () const
{
	return (m_Buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(0) & 0x1F);
}

bool Message::ReportBlock::Iterator::IsValid() const
{
	return (m_Current != ~0);
}

void Message::ReportBlock::Iterator::Reset()
{
	m_PreStart = true;
	m_Current = NUMBER_MAX_UNSIGNED(uint32);
}

bool Message::ReportBlock::Iterator::Next()
{
	if (m_PreStart == true)
	{
		m_PreStart = false;

		if (Count() > 0)
		{
			m_Current = 0;
		}
	}
	else if (m_Current < static_cast<uint8>(m_Buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(0) & 0x1F))
	{
		++m_Current;
	}
	else
	{
		m_Current = NUMBER_MAX_UNSIGNED(uint32);
	}

	return (m_Current != ~0);
}

const Message::ReportBlock Message::ReportBlock::Iterator::Value() const
{
	return (ReportBlock(Generics::DataElement(m_Buffer, 1 + (m_Current * MAX_REPORTBLOCK_SIZE), MAX_REPORTBLOCK_SIZE)));
}

//-------------------------------------------------------------------------------------
//-- Message::ChunkItem::Item
//-------------------------------------------------------------------------------------
Message::ChunkItem::Item::Item(const Generics::DataElement& buffer) :
	m_Buffer(Generics::ToString(
				reinterpret_cast<const char*> (&(buffer.Buffer()[2])), 
				buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(1))),
	m_Type(buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(0))
{
}

Message::ChunkItem::Item::Item(const uint8 type, const Generics::TextFragment& text) :
	m_Buffer(text),
	m_Type(type)
{
}

Message::ChunkItem::Item::Item(const Item& copy) :
	m_Buffer(copy.m_Buffer),
	m_Type(copy.m_Type)
{
}

Message::ChunkItem::Item::~Item()
{
}

uint8 Message::ChunkItem::Item::Type () const
{
	return (m_Type);
}

const Generics::TextFragment Message::ChunkItem::Item::Text() const
{
	return (m_Buffer);
}

//-------------------------------------------------------------------------------------
//-- Message::ChunkItem::Item::Iterator
//-------------------------------------------------------------------------------------
Message::ChunkItem::Item::Iterator::Iterator() :
	m_PreStart(true),
	m_Current(NUMBER_MAX_UNSIGNED(uint32)),
	m_Buffer()
{
}

Message::ChunkItem::Item::Iterator::Iterator(const Generics::DataElement& buffer) :
	m_PreStart(true),
	m_Current(NUMBER_MAX_UNSIGNED(uint32)),
	m_Buffer(buffer)
{
}

Message::ChunkItem::Item::Iterator::Iterator(const Iterator& copy) :
	m_PreStart(copy.m_PreStart),
	m_Current(copy.m_Current),
	m_Buffer(copy.m_Buffer)
{
}

Message::ChunkItem::Item::Iterator::~Iterator()
{
}

Message::ChunkItem::Item::Iterator& Message::ChunkItem::Item::Iterator::operator= (const Message::ChunkItem::Item::Iterator& copy)
{
	m_PreStart = copy.m_PreStart;
	m_Current = copy.m_Current;
	m_Buffer = copy.m_Buffer;

	return (*this);
}

bool Message::ChunkItem::Item::Iterator::IsValid() const
{
	return (m_Current != ~0);
}

uint16 Message::ChunkItem::Item::Iterator::Length() const
{
	return (2 + m_Buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(m_Current + 1)); 
}

void Message::ChunkItem::Item::Iterator::Reset()
{
	m_PreStart = true;
	m_Current = NUMBER_MAX_UNSIGNED(uint32);
}

bool Message::ChunkItem::Item::Iterator::Next()
{
	if (m_PreStart == true)
	{
		m_PreStart = false;

		if (m_Buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(m_Current) != 0)
		{
			m_Current = 0;
		}
	}
	else if (m_Current != ~0) 
	{
		if (m_Buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(m_Current + Length()) != 0)
		{
			m_Current += Length();
		}
		else
		{
			m_Current = NUMBER_MAX_UNSIGNED(uint32);
		}
	}

	return (m_Current != NUMBER_MAX_UNSIGNED(uint32));
}

const Message::ChunkItem::Item Message::ChunkItem::Item::Iterator::Value() const
{
	return (Message::ChunkItem::Item(Generics::DataElement(m_Buffer, m_Current, Length())));
}

//-------------------------------------------------------------------------------------
//-- Message::ChunkItem::Iterator
//-------------------------------------------------------------------------------------
Message::ChunkItem::Iterator::Iterator() : 
	m_Index(NUMBER_MAX_UNSIGNED(uint32)),
	m_Current(NUMBER_MAX_UNSIGNED(uint32)),
	m_Buffer()
{
}

Message::ChunkItem::Iterator::Iterator(const Generics::DataElement& buffer) :
	m_Index(NUMBER_MAX_UNSIGNED(uint32)),
	m_Current(NUMBER_MAX_UNSIGNED(uint32)),
	m_Buffer(buffer)
{
}

Message::ChunkItem::Iterator::Iterator(const Iterator& copy) :
	m_Index(NUMBER_MAX_UNSIGNED(uint32)),
	m_Current(NUMBER_MAX_UNSIGNED(uint32)),
	m_Buffer(copy.m_Buffer)
{
}

Message::ChunkItem::Iterator::~Iterator()
{
}

Message::ChunkItem::Iterator& Message::ChunkItem::Iterator::operator= (const Message::ChunkItem::Iterator& copy)
{
	m_Index = copy.m_Index;
	m_Current = copy.m_Current;
	m_Buffer = copy.m_Buffer;

	return (*this);
}

bool Message::ChunkItem::Iterator::IsValid() const
{
	return (m_Current != ~0);
}

uint16  Message::ChunkItem::Iterator::Length() const
{
	uint16 length = 4 + 1 + 3; // By default, we have the SSRC, 
							   // 1 Closing byte and we must 
							   // align on a 32 bit boundary..

	// Calculate the variable length of this chunk..
	Generics::DataElement currentChunkItem (m_Buffer, m_Current);
	ChunkItem::Item::Iterator index (ChunkItem(currentChunkItem).Items());

	while (index.Next())
	{
		length += index.Length();
	}

	return (length & 0xFFFC); // Make sure we are aligned...
}

void Message::ChunkItem::Iterator::Reset()
{
	m_Index = NUMBER_MAX_UNSIGNED(uint32);
	m_Current = NUMBER_MAX_UNSIGNED(uint32);
}

bool Message::ChunkItem::Iterator::Next()
{
	if (m_Index == NUMBER_MAX_UNSIGNED(uint32))
	{
		m_Index = 0;

		if (m_Index < static_cast<uint8>(m_Buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(0) & 0x1F))
		{
			// It seems hat we are on a valid spot.
			m_Current = 0;
		}
	}
	else if (m_Current != ~0) 
	{
		if (m_Index < static_cast<uint8>(m_Buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(0) & 0x1F))
		{
			// See how long the current entry is
			m_Current += Length();
			m_Index++;
		}
		else
		{
			m_Current = NUMBER_MAX_UNSIGNED(uint32);
		}
	}

	return (m_Current != NUMBER_MAX_UNSIGNED(uint32));
}

const Message::ChunkItem Message::ChunkItem::Iterator::Value() const
{
	return (ChunkItem(Generics::DataElement(m_Buffer, m_Current)));
}

//-------------------------------------------------------------------------------------
//-- Message::ChunkItem
//-------------------------------------------------------------------------------------
Message::ChunkItem::ChunkItem(const uint32 syncSource) :
	m_Buffer(Generics::DataStorage::Create(1024))
{
	m_Buffer.Size(4 + 4);

	m_Buffer.SetNumber<uint32, Generics::ENDIAN_BIG>(0, syncSource);
	m_Buffer.SetNumber<uint32, Generics::ENDIAN_BIG>(4, 0); // closing 0 and follow up (32 bit boundary);
}

Message::ChunkItem::ChunkItem(const Generics::DataElement& buffer) :
	m_Buffer(Generics::DataStorage::Create(static_cast<uint32>(buffer.Size()) + 1024))
{
	m_Buffer.Copy(buffer);
}

Message::ChunkItem::ChunkItem(const ChunkItem& copy) :
	m_Buffer(copy.m_Buffer)
{
}

Message::ChunkItem::~ChunkItem()
{
}

Message::ChunkItem& Message::ChunkItem::operator= (const Message::ChunkItem& RHS)
{
	m_Buffer = RHS.m_Buffer;

	return (*this);
}

uint32 Message::ChunkItem::SyncSourceId() const
{
	return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_BIG>(0));
}

void Message::ChunkItem::SyncSourceId(const uint32 syncSource)
{
	m_Buffer.SetNumber<uint32, Generics::ENDIAN_BIG>(0, syncSource);
}

Message::ChunkItem::Item::Iterator Message::ChunkItem::Items () const
{
	return (Message::ChunkItem::Item::Iterator(Generics::DataElement(m_Buffer, 4)));
}

void Message::ChunkItem::AddItem (const Item& text)
{
	std::string realData;

	Generics::ToString(text.Text().Data(), realData);

	Item::Iterator index (Generics::DataElement(m_Buffer, 4));

	uint32 offset = 4 /* SSRC */;

	// We are not allowed to extend this, firts creae a copy..
	while (index.Next())
	{
		offset += index.Length();
	}

	uint32 boundSize = ((offset + 
				     	1 /* Closing type */ +
						1 /* New type to add */ + 
						1 /* New length to add */ +
						realData.length() /* New text to add */) + 3) & 0xFFFFFFFC;

	m_Buffer.Size(boundSize);

	// Now we should definitely have enough space, add the new item.
	m_Buffer.SetNumber<uint8, Generics::ENDIAN_BIG>(offset, text.Type());
	m_Buffer.SetNumber<uint8, Generics::ENDIAN_BIG>(offset+1, static_cast<uint8>(realData.length()));
	m_Buffer.SetBuffer(offset+2, realData.length(), reinterpret_cast<const uint8*>(realData.c_str()));

	// Make sure we put the length at the end of the items..
	offset += 1 /* New type to add */ + 
			  1 /* New length to add */ +
			  realData.length() /* New text to add */;

	// Add, at least 1, '0' characters at the end..
	m_Buffer.SetNumber<uint8, Generics::ENDIAN_BIG>(offset, 0);
	offset++;

	// And fill it uo unt ill we reach a 32 bit boundary
	while (offset % 4)
	{
		m_Buffer.SetNumber<uint8, Generics::ENDIAN_BIG>(offset, 0);
		offset++;
	}
}

const Generics::DataElement Message::ChunkItem::Data() const
{
	return (m_Buffer);
}

//-------------------------------------------------------------------------------------
//-- Message::Message
//-------------------------------------------------------------------------------------
Message::Message() :
	m_Buffer(Generics::DataStorage::Create(1024))
{
	m_Buffer.Size(4);
	m_Buffer.SetNumber<uint8, Generics::ENDIAN_BIG>(0, 0x80); // version 2, padding off, RC == 0;
}

Message::Message(const Generics::DataElement& input) :
	m_Buffer(Generics::DataStorage::Create(static_cast<uint32>(input.Size())))
{
	m_Buffer.Copy(input);
}

bool Message::IsBye() const
{
	return (m_Buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(1) == RTCP_PT_BYE ? true : false);	
}

bool Message::IsSDES() const
{
	return (m_Buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(1) == RTCP_PT_SDES ? true : false);	
}

bool Message::IsSenderReport() const 
{ 
	return (m_Buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(1) == RTCP_PT_SR  ? true : false);	
}

bool Message::IsReceiverReport() const 
{ 
	return (m_Buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(1) == RTCP_PT_RR ? true : false);	
}

uint32 Message::SynchronisationSource() const
{ 
	// This can only be set if this is a sender report.
	ASSERT (IsSenderReport() || IsReceiverReport());

	return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_BIG>(4));	
}

uint64 Message::NTPTimeStamp () const 
{ 
	// This can only be read if this is a sender report.
	ASSERT (IsSenderReport());

	return (m_Buffer.GetNumber<uint64, Generics::ENDIAN_BIG>(8));	
}

uint32 Message::RTPTimeStamp() const 
{ 
	// This can only be read if this is a sender report.
	ASSERT (IsSenderReport());

	return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_BIG>(16));	
}

uint32 Message::PackageCount() const 
{
	// This can only be read if this is a sender report.
	ASSERT (IsSenderReport());

	return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_BIG>(20));	
}

uint32 Message::OctetCount() const 
{
	// This can only be read if this is a sender report.
	ASSERT (IsSenderReport());

	return (m_Buffer.GetNumber<uint32, Generics::ENDIAN_BIG>(24));	
}

Message::ReportBlock::Iterator Message::Reports () const
{
	// This can only be read if this is a sender or reciever report.
	ASSERT (IsSenderReport() || IsReceiverReport());

	return (ReportBlock::Iterator(Generics::DataElement(m_Buffer, (IsSenderReport() ? 28 : 8))));
}

Message::ChunkItem::Iterator Message::Chunks () const
{
	ASSERT (IsSDES());

	return (ChunkItem::Iterator(Generics::DataElement(m_Buffer, 8)));
}

void Message::IsSDES (const bool value)
{
	// And do we need to change ?
	if (IsSDES() ^ value)
	{
		m_Buffer.SetNumber<uint8, Generics::ENDIAN_BIG>(1, RTCP_PT_SDES);

		// Remove any reports or data attached to this message
		m_Buffer.SetNumber<uint8, Generics::ENDIAN_BIG>(0, ((m_Buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(0)) & 0xE0));	
		m_Buffer.SetNumber<uint16, Generics::ENDIAN_BIG>(2, 1);	

		// Make sure we have enough spece for an SDES..
		m_Buffer.Size (8);
	}
}

void Message::IsBye (const bool value)
{
	// And do we need to change ?
	if (IsBye() ^ value)
	{
		m_Buffer.SetNumber<uint8, Generics::ENDIAN_BIG>(1, RTCP_PT_BYE);

		// Remove any reports attached to this message
		m_Buffer.SetNumber<uint8, Generics::ENDIAN_BIG>(0, ((m_Buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(0)) & 0xE0));	
		m_Buffer.SetNumber<uint16, Generics::ENDIAN_BIG>(2, 1);	

		// Make sure we have enough spece for a bye..
		m_Buffer.Size (8);
	}
}

void Message::IsSenderReport (const bool value)
{
	// And do we need to change ?
	if (IsSenderReport() ^ value)
	{
		m_Buffer.SetNumber<uint8, Generics::ENDIAN_BIG>(1, RTCP_PT_SR);

		// Remove any reports attached to this message
		m_Buffer.SetNumber<uint8, Generics::ENDIAN_BIG>(0, (m_Buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(0) & 0xE0));	
		m_Buffer.SetNumber<uint16, Generics::ENDIAN_BIG>(2, 6);

		// Make sure we have enough spece for a sender report..
		m_Buffer.Size (28);
	}
}

void Message::IsReceiverReport (const bool value)
{
	// And do we need to change ?
	if (IsReceiverReport() ^ value)
	{
		m_Buffer.SetNumber<uint8, Generics::ENDIAN_BIG>(1, RTCP_PT_RR);

		// Remove any reports attached to this message
		m_Buffer.SetNumber<uint8, Generics::ENDIAN_BIG>(0, ((m_Buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(0)) & 0xE0));	
		m_Buffer.SetNumber<uint16, Generics::ENDIAN_BIG>(2, 1);	

		// Make sure we have enough spece for a Receiver report..
		m_Buffer.Size (8);
	}
}

void Message::SynchronisationSource (const uint32 value)
{
	// This can only be set if this is a sender report.
	ASSERT (IsSenderReport() || IsReceiverReport());

	m_Buffer.SetNumber<uint32, Generics::ENDIAN_BIG>(4, value);
}

// Microseconds since 
void Message::NTPTimeStamp (const uint64 value)
{
	// This can only be set if this is a sender report.
	ASSERT (IsSenderReport());

	m_Buffer.SetNumber<uint64, Generics::ENDIAN_BIG>(8, value);
}

void Message::RTPTimeStamp (const uint32 value)
{
	// This can only be set if this is a sender report.
	ASSERT (IsSenderReport());

	m_Buffer.SetNumber<uint32, Generics::ENDIAN_BIG>(16, value);
}

void Message::PackageCount (const uint32 value)
{
	// This can only be set if this is a sender report.
	ASSERT (IsSenderReport());

	m_Buffer.SetNumber<uint32, Generics::ENDIAN_BIG>(20, value);
}

void Message::OctetCount (const uint32 value)
{
	// This can only be set if this is a sender report.
	ASSERT (IsSenderReport());

	m_Buffer.SetNumber<uint32, Generics::ENDIAN_BIG>(24, value);
}

void Message::AddReport(const ReportBlock& report)
{
	const Generics::DataElement data (report.Data());

	// This can only be set if this is a sender or Receiver report.
	ASSERT (IsSenderReport() || IsReceiverReport());

	uint32 offset = static_cast<uint32>(m_Buffer.Size());

	// Make sure we have enough space, get the current size
	m_Buffer.Size (offset + static_cast<uint32>(data.Size()));

	// Make sure this data is added to the end of the buffer and 
	// update the sizes accordingly.
	m_Buffer.Copy(data, offset);

	m_Buffer.SetNumber<uint8, Generics::ENDIAN_BIG>(0,  (m_Buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(0) + 1));	
	m_Buffer.SetNumber<uint16, Generics::ENDIAN_BIG>(2, static_cast<uint16>((m_Buffer.Size() - 4) / 4));	
}

void Message::AddChunk(const ChunkItem& chunk)
{
	const Generics::DataElement data (chunk.Data());

	// This can only be set if this is a SDES report.
	ASSERT (IsSDES());

	uint32 offset = static_cast<uint32>(m_Buffer.Size());

	// Make sure we have enough space, get the current size
	m_Buffer.Size (offset + static_cast<uint32>(data.Size()));

	// Make sure this data is added to the end of the buffer and 
	// update the sizes accordingly.
	m_Buffer.Copy(data, offset);

	m_Buffer.SetNumber<uint8, Generics::ENDIAN_BIG>(0,  (m_Buffer.GetNumber<uint8, Generics::ENDIAN_BIG>(0) + 1));	
	m_Buffer.SetNumber<uint16, Generics::ENDIAN_BIG>(2, static_cast<uint16>((m_Buffer.Size() - 4) / 4));	
}

const Generics::DataElement Message::Data() const
{
	return (m_Buffer);
}

} } // namespace Solutions::RTCP