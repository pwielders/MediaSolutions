#include "AVIMediaIterator.h"

namespace Solutions { namespace AVI
{

PackageIterator::LegacyIterator::LegacyIterator () :
    m_Type(static_cast<uint32>(0)),
	m_Index(),
	m_Offset(NUMBER_MAX_UNSIGNED(uint32)),
	m_Storage()
{
}

PackageIterator::LegacyIterator::LegacyIterator(const RIFF::ChunkHeader& index, const Generics::DataElement& storage, const StreamType streamType, const uint8 channel) :
	m_Type(DataChunk::Code(streamType,channel)),
	m_Index(index),
	m_Offset(NUMBER_MAX_UNSIGNED(uint32)),
	m_Storage(storage)
{
}

PackageIterator::LegacyIterator::LegacyIterator(const LegacyIterator& copy) :
    m_Type(copy.m_Type),
	m_Index(copy.m_Index),
	m_Offset(copy.m_Offset),
	m_Storage(copy.m_Storage)
{
}
PackageIterator::LegacyIterator::~LegacyIterator ()
{
}

PackageIterator::LegacyIterator& PackageIterator::LegacyIterator::operator= (const PackageIterator::LegacyIterator& RHS)
{
	m_Type = RHS.m_Type;
	m_Index = RHS.m_Index;
	m_Offset = RHS.m_Offset;
	m_Storage = RHS.m_Storage;

	return (*this);
}

void PackageIterator::LegacyIterator::Reset()
{
	m_Offset = NUMBER_MAX_UNSIGNED(uint32);
}

bool PackageIterator::LegacyIterator::Next()
{
	// MOve on to the new index..
	if (m_Offset == NUMBER_MAX_UNSIGNED(uint32))
	{
		m_Offset = 0;
	}
	else if (m_Offset < m_Index.Size())
	{
		m_Offset += IndexChunkSize();
	}

	// Make sure we are on the right type for this index...
	while ((m_Offset < m_Index.Size()) && ((IsValidChunk(m_Offset) == false) || (IsStartChunk(m_Offset) == false)))
	{
		m_Offset += IndexChunkSize();
	}

	return (IsValid());
}

uint32 PackageIterator::LegacyIterator::Size () const
{
	uint32 size = 0;

	if (IsValid())
	{
		uint32 index = m_Offset;

		// Make sure we are on the right type for this index...
		do
		{
			size += m_Index.GetNumber<uint32, Generics::ENDIAN_LITTLE>(index+12);

			do
			{
				index += IndexChunkSize();

			} while ((index < m_Index.Size()) && (IsValidChunk(index) == false));

		} while ((index < m_Index.Size()) && (IsStartChunk(index) == false)) ;
	}

	return (size);
}

uint32 PackageIterator::LegacyIterator::Package (Generics::DataElementContainer& dataFrame)
{
	uint32 result = 0;

	if (IsValid())
	{
		uint32 index = m_Offset;

		// Make sure we are on the right type for this index...
		do
		{
			uint32 offset = m_Index.GetNumber<uint32, Generics::ENDIAN_LITTLE>(index+8);
			uint32 size   = m_Index.GetNumber<uint32, Generics::ENDIAN_LITTLE>(index+12);
			result += size;

			// Add this frame to the container, it is a valid one.
			dataFrame.Back (Generics::DataElement (m_Storage, offset, size));

			do
			{
				index += IndexChunkSize();

			} while ((index < m_Index.Size()) && (IsValidChunk(index) == false));

		} while ((index < m_Index.Size()) && (IsStartChunk(index) == false)) ;
	}

	return (result);
}

PackageIterator::PackageIterator() :
	m_IteratorType(ILLEGAL),
	m_StreamType(UNKNOWN),
	m_Channel(0),
	m_StandardIndex(),
	m_FieldIndex(),
	m_SuperIndex(),
	m_IndexChunk(),
	m_Storage()
{
}

PackageIterator::PackageIterator(const PackageIterator& copy) :
	m_IteratorType(copy.m_IteratorType),
	m_StreamType(copy.m_StreamType),
	m_Channel(copy.m_Channel),
	m_StandardIndex(copy.m_StandardIndex),
	m_FieldIndex(copy.m_FieldIndex),
	m_SuperIndex(copy.m_SuperIndex),
	m_IndexChunk(copy.m_IndexChunk),
	m_Storage(copy.m_Storage)
{
}

PackageIterator::PackageIterator(const Generics::DataElement& storage, const RIFF::ChunkHeader& index, const StreamType streamType, const uint8 channel) :
	m_IteratorType(ILLEGAL),
	m_StreamType(streamType),
	m_Channel(channel),
	m_StandardIndex(),
	m_FieldIndex(),
	m_SuperIndex(),
	m_IndexChunk(index),
	m_Storage()
{
	if (m_IndexChunk.IsValid() == false)
	{
		// If we did not get an index, it is not in the stream reference so it is an 
		// old type AVI, it is then at the end of the storage.
		// Also make sure you find the actual data.
		RIFF::LISTHeader::Iterator indexer (RIFF::RIFFHeader(storage).Data());

		while (indexer.Next() == true)
		{
			if (indexer.ID() == RIFF::FourCC("idx1"))
			{
				// There should be only 1 idx1 block
				ASSERT (m_IteratorType == ILLEGAL);
				ASSERT (m_Storage.IsValid() == true);

				m_IteratorType = LEGACY;
				m_LegacyIndex = LegacyIterator(indexer.Chunk(), m_Storage, streamType, channel);
			}
			else if (indexer.ID() == RIFF::FourCC("movi"))
			{
				// There should be only 1 movi block
				ASSERT (m_Storage.IsValid() == false);

				// The data is "after" the movi, so skip the FOURCC code.
				m_Storage = Generics::DataElement(indexer.Chunk().Data(), RIFF::FourCC::Size());
			}
		}
	}
	else if (m_IndexChunk.Size() >= g_HeaderSize)
	{
		uint16 longsPerEntry = m_IndexChunk.GetNumber<uint16, Generics::ENDIAN_LITTLE>(0);
		uint8 subType = m_IndexChunk.GetNumber<uint8, Generics::ENDIAN_LITTLE>(2);
		uint8 type = m_IndexChunk.GetNumber<uint8, Generics::ENDIAN_LITTLE>(3);
		uint32 entriesInUse = m_IndexChunk.GetNumber<uint32, Generics::ENDIAN_LITTLE>(4);
	
		if (IsValid() && (longsPerEntry == 2) && (type == AVI_INDEX_OF_CHUNKS) && (subType == 0))
		{
			m_IteratorType = STANDARD;
			m_StandardIndex = StandardIterator(m_IndexChunk.Data(), entriesInUse);
		}
		else if (IsValid() && (longsPerEntry == 3) && (type == AVI_INDEX_OF_CHUNKS) && (subType == AVI_INDEX_2FIELD))
		{
			m_IteratorType = FIELD;
			m_FieldIndex = FieldIterator(m_IndexChunk.Data(), entriesInUse);
		}
		else if (IsValid() && (longsPerEntry == 4) && (type == AVI_INDEX_OF_INDEXES) && (subType == 0))
		{
			m_IteratorType = SUPER_CHUNK_INDEX;
			m_SuperIndex = SuperIterator(m_IndexChunk.Data(), entriesInUse*longsPerEntry);
		}
		else if (IsValid() && (longsPerEntry == 4) && (type == AVI_INDEX_OF_INDEXES) && (subType == AVI_INDEX_2FIELD))
		{
			m_IteratorType = SUPER_INDEX_INDEX;
			m_SuperIndex = SuperIterator(m_IndexChunk.Data(), entriesInUse);
			m_FieldIndex = FieldIterator(m_IndexChunk.Data(), entriesInUse);
		}
	}
}

PackageIterator::~PackageIterator()
{
}

PackageIterator& PackageIterator::operator= (const PackageIterator& RHS)
{
	m_StreamType = RHS.m_StreamType;
	m_Channel = RHS.m_Channel;
	m_IteratorType = RHS.m_IteratorType;
	m_StandardIndex = RHS.m_StandardIndex;
	m_SuperIndex = RHS.m_SuperIndex;
	m_FieldIndex = RHS.m_FieldIndex;

	return (*this);
}

bool PackageIterator::IsValid() const
{
	switch (m_IteratorType)
	{
	case LEGACY:
		 return (m_LegacyIndex.IsValid());
		 break;
	case STANDARD:
		 return (m_StandardIndex.IsValid());
		 break;
	case FIELD:
		 return (m_FieldIndex.IsValid());
		 break;
	case SUPER_CHUNK_INDEX:
		 return (m_SuperIndex.IsValid());
		 break;
	case SUPER_INDEX_INDEX:
		 return (m_SuperIndex.IsValid());
		 break;
	default:
		ASSERT(false);
	}

	return (false);
}

void PackageIterator::Reset()
{
	switch (m_IteratorType)
	{
	case LEGACY:
		 m_LegacyIndex.Reset();
		 break;
	case STANDARD:
		 m_StandardIndex.Reset();
		 break;
	case FIELD:
		 m_FieldIndex.Reset();
		 break;
	case SUPER_CHUNK_INDEX:
		 m_SuperIndex.Reset();
		 break;
	case SUPER_INDEX_INDEX:
		 m_SuperIndex.Reset();
		 break;
	default:
		ASSERT(false);
	}
}

bool PackageIterator::Next()
{
	switch (m_IteratorType)
	{
	case LEGACY:
		 return (m_LegacyIndex.Next());
		 break;
	case STANDARD:
		 return (m_StandardIndex.Next());
		 break;
	case FIELD:
		 return (m_FieldIndex.Next());
		 break;
	case SUPER_CHUNK_INDEX:
		 return (m_SuperIndex.Next());
		 break;
	case SUPER_INDEX_INDEX:
		 return (m_SuperIndex.IsValid());
		 break;

	default:
		ASSERT(false);
	}
	return (false);
}

uint32 PackageIterator::Package(Generics::DataElementContainer& container)
{
	uint32 result = 0;

	switch (m_IteratorType)
	{
	case LEGACY:
		 result = m_LegacyIndex.Package(container);
		 break;
	case STANDARD:
		 result = m_StandardIndex.Package(container);
		 break;
	case FIELD:
		 result = m_FieldIndex.Package(container);
		 break;
	case SUPER_CHUNK_INDEX:
		 result = m_SuperIndex.Package(container);
		 break;
	case SUPER_INDEX_INDEX:
		 result = m_SuperIndex.Package(container);
		 break;
	default:
		ASSERT(false);
	}

	return (result);
}

uint32 PackageIterator::Size() const
{
	switch (m_IteratorType)
	{
	case LEGACY:
		 return(m_LegacyIndex.Size());
		 break;
	case STANDARD:
		 return(m_StandardIndex.Size());
		 break;
	case FIELD:
		 return(m_FieldIndex.Size());
		 break;
	case SUPER_CHUNK_INDEX:
		 return(m_SuperIndex.Size());
		 break;
	case SUPER_INDEX_INDEX:
		 return(m_SuperIndex.Size());
		 break;
	default:
		ASSERT(false);
	}

	return (0);
}

} } // namespace Solutions::AVI