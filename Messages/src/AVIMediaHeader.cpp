#include "AVIMediaHeader.h"

namespace Solutions { namespace AVI
{

Stream::Stream(const RIFF::LISTHeader::Iterator& indexer, const Generics::DataElement& storage, const uint8 channel) :
	m_Channel(channel),
	m_Storage(storage),
	m_Buffer(),
	m_Format(),
	m_Index(),
	m_Name()
{
	RIFF::LISTHeader::Iterator iterator(indexer);

	// Extract Header buffer and Format buffer
	while (iterator.Next() == true)
	{
		if (iterator.ID() == RIFF::FourCC(_T("strh")))
		{
			// There should be only 1 strh
			ASSERT (m_Buffer.IsValid() == false);

			m_Buffer = iterator.Chunk();
		}
		else if (iterator.ID() == RIFF::FourCC(_T("strf")))
		{
			// There should be only 1 strf
			ASSERT (m_Format.IsValid() == false);

			m_Format = iterator.Chunk();
		}
		else if (iterator.ID() == RIFF::FourCC(_T("strn")))
		{
			// There should be only 1 strf
			ASSERT (m_Name.empty() == true);

			m_Name = Generics::ToString(reinterpret_cast<const char*> (iterator.Chunk().Data().Buffer()));
		}
		else if (iterator.ID() == RIFF::FourCC(_T("indx")))
		{
			// There should be only 1 strf
			ASSERT (m_Name.empty() == true);

			m_Index = iterator.Chunk();
		}
	}
}

Main::Main() :
	m_Buffer (),
	m_ODMLList(),
	m_HeaderIterator(),
	m_Storage()
{
}

Main::Main(const Generics::DataElement& buffer) :
	m_Buffer (),
	m_ODMLList(),
	m_HeaderIterator(),
	m_Storage()
{
	if (buffer.IsValid ())
	{
		// Seems like we have an open file. Analyse it.
		uint64 offset = buffer.Search (0,  RIFF::RIFFHeader::FCC().Code(),  RIFF::FourCC::Size());

		if (offset < buffer.Size())
		{
			// We need this to take all the other information from it, like packages and indexes....
			m_Storage = Generics::DataElement(buffer, offset);

			// But first make something to get the metadata..
			RIFF::RIFFHeader header (m_Storage);

			if ( (header.IsValid() == true) && (header.FileType() == RIFF::FourCC(_T("AVI "))))
			{
				// Now get the elements in the list
				RIFF::LISTHeader::Iterator outerLoop (header.Data());

				while (outerLoop.Next() == true)
				{
					if ((outerLoop.IsList()) && (outerLoop.ID () == RIFF::FourCC(_T("hdrl"))))
					{
						m_HeaderIterator = outerLoop.List().Elements();

						while (m_HeaderIterator.Next())
						{
							if (m_HeaderIterator.ID() == RIFF::FourCC(_T("avih")))
							{
								// There should be only 1 avih
								ASSERT (m_Buffer.IsValid() == false);
	
								m_Buffer = m_HeaderIterator.Chunk();
							}
							else if ((m_HeaderIterator.IsList() == true) && (m_HeaderIterator.ID() == RIFF::FourCC(_T("odml"))))
							{
								// There should be only 1 odml
								ASSERT (m_ODMLList.IsValid() == false);
	
								m_ODMLList = m_HeaderIterator.List().Elements();
							}
						}
					}
				}

				// Jump over the list and see if we find an index (old AVI representation)
				Generics::DataElement data (buffer, offset + header.Allocated());
			}
		}
	}
}

Main::Main(const Main& copy) :
	m_Buffer (copy.m_Buffer),
	m_ODMLList(copy.m_ODMLList),
	m_HeaderIterator(copy.m_HeaderIterator),
	m_Storage(copy.m_Storage)

{
}

Main::~Main()
{
}


Main& Main::operator= (const Main& RHS)
{
	m_Buffer = RHS.m_Buffer;
	m_ODMLList = RHS.m_ODMLList;
	m_HeaderIterator = RHS.m_HeaderIterator;
	m_Storage = RHS.m_Storage;

	return (*this);
}

} } // namespace Solutions::AVI