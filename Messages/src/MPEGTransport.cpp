#include "MPEGTransport.h"
#include "MPEGTransportPackage.h"
#include "MPEGAssembler.h"

namespace Solutions { namespace MPEG
{

//--------------------------------------------------------------------------------------------
// Class: MPEGSource
//--------------------------------------------------------------------------------------------

MPEGTransport::MPEGTransport (const Generics::DataElement& dataObject, const uint64 offset) : 
	m_Data(dataObject),
	m_Space(),
	m_PMTTable(),	// PMT will not be bigger than 1Kb
	m_FrameSize(0)
{
	// Find the right MPEG header..
	uint64 startIndex = offset;
	Generics::ScopedStorage<256>	PATSpace;

	// Start looking for the PATSection
	PAT::Assembler associationTable (0);
	PMT::Assembler mappingTable;

	while ((m_PMTTable.IsValid() == false) && ((startIndex = m_Data.SearchNumber<uint8, Generics::ENDIAN_LITTLE>(startIndex, TransportPackage::ID())) < m_Data.Size()))
	{
		Generics::DataElement package(Generics::DataElement(m_Data, startIndex, m_FrameSize));

		if (m_FrameSize == 0)
		{
			if ((m_FrameSize = DeterminePackageSize(package)) == 0)
			{
				startIndex++;
			}
		}

		if (m_FrameSize != 0)
		{
			TransportPackage frame (package);

			if (frame.IsValid() == false)
			{
				startIndex++;
			}
			else
			{
				if (associationTable.Assemble(frame))
				{
					if (associationTable.CurrentTable().IsValid())
					{
						Table::Iterator	sectionEnumerator (associationTable.CurrentTable().Elements());

						ASSERT (sectionEnumerator.Count() == 1);

						// Move on to the only available section in the table.
						sectionEnumerator.Next();

						PAT::Iterator programEnumerator (sectionEnumerator.Element<PAT>().Associations());

						// As this is a file, We only expect 1 TS in the file...
						ASSERT (programEnumerator.Count() == 1);

						// Move on to the only available programm in the table.
						programEnumerator.Next();

						mappingTable.Reset(programEnumerator.Pid());
					}
				}
				else if (mappingTable.Assemble(frame))
				{
					if (mappingTable.CurrentTable().IsValid())
					{
						m_PMTTable = mappingTable.CurrentTable();
					}
				}

				startIndex += m_FrameSize;	
			}
		}
	}
}

MPEGTransport::~MPEGTransport()
{
}

uint8 MPEGTransport::DeterminePackageSize (Generics::DataElement& package)
{
	// Determine the frameSize, 188, 204, 208
	if (package[188] == TransportPackage::ID())
	{
		package.Size(188);
		return(188);
	}
	else if (package[204] == TransportPackage::ID())
	{
		package.Size(204);
		return(204);
	}
	else if (package[208] == TransportPackage::ID())
	{
		package.Size(208);
		return(208);
	}

	return (0);
}

} } // namespace Solutions::MPEG
