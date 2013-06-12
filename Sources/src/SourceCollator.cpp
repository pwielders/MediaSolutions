#include "SourceCollator.h"

namespace Solutions { namespace Source
{

Collator::Collator (const uint32 type, ISource* realStream) :
	m_DataStream(realStream),
	m_BitRate(0),
	m_CollatedFrames(0),
	m_Type(type),
	m_Fraction(),
	m_FractionIndex(0),
	m_FractionContainer(1,1,NUMBER_MAX_UNSIGNED(uint64))
{
	ASSERT (realStream != NULL);
}

/* virtual */ Collator::~Collator ()
{
}

/* virtual */ uint32 Collator::NextPackage (Generics::DataElementContainer& frame)
{
	const uint64 maxPackageSize = frame.Space();

	// Assume we are at the end of the road!!
	uint32 result = NUMBER_MAX_UNSIGNED(uint32);
	bool packageCompleted;

	// Reset the bitrate calculation, New Next drop...
	m_CollatedFrames = 0;
	m_BitRate = 0;

	// If there is nothing left from the last time, we can load a new one...
	if (m_FractionContainer.IsEmpty() == true)
	{
		packageCompleted = !LoadNewPackage(maxPackageSize);
	}
	else
	{
		packageCompleted = false;
	}

	// Is there still something left to process ?
	while ((packageCompleted == false) && (m_FractionIndex < m_FractionContainer.Elements()))
	{
		// Insert a fragmented frame
		uint64 leftSpace = frame.Space();

		if (m_Fraction.Size() > leftSpace)
		{
			packageCompleted = true;
			result += static_cast<uint32> (leftSpace);
			frame.Back (Generics::DataElement(m_Fraction, 0, leftSpace));
			m_Fraction = Generics::DataElement(m_Fraction, leftSpace);
		}
		else
		{
			result += static_cast<uint32> (m_Fraction.Size());
			frame.Back (m_Fraction);
			m_FractionIndex++;			

			// If we did not reach the end, continue...
			if (m_FractionIndex != m_FractionContainer.Elements())
			{
				m_Fraction = m_FractionContainer[m_FractionIndex];
			}
			else
			{
				// Get ourselves a new frame, if it is allowed..
				if ((m_Type & MULTIPLE_FRAMES) == 0)
				{
					packageCompleted = true;
				}
				else
				{
					packageCompleted = !LoadNewPackage(maxPackageSize);

					if (packageCompleted == false)
					{
						if (m_FractionContainer.Size() <= frame.Space())
						{
							packageCompleted = false;
						}
						else
						{
							packageCompleted = !((m_Type & FRAGMENTED_FRAMES) == FRAGMENTED_FRAMES);
						}
					}
				}
			}
		}
	}

	return (result);
}

/* virtual */ uint64 Collator::PresentationTime () const
{
	return (m_DataStream->PresentationTime());
}

/* virtual */ void Collator::PresentationTime (const uint64 tickLocation)
{
	m_DataStream->PresentationTime(tickLocation);
}

/* virtual */ const IProperties* Collator::Properties () const
{
	return (m_DataStream->Properties());
}

bool Collator::LoadNewPackage(const uint64 maxPackageSize)
{
	do
	{
		// Clear the buffer and load the next available frame..
		m_FractionContainer.Clear();

		// Load a next frame as there is nothing left to load !!
		m_DataStream->NextPackage(m_FractionContainer);

		// As long as this frames is too big, and we are allowed to skip it, we skip it...
	} while ((m_FractionContainer.Size() > maxPackageSize) && ((m_Type & REJECT_OVERSIZED_FRAMES) == REJECT_OVERSIZED_FRAMES));

	if (m_FractionContainer.IsEmpty() == false)
	{
		// We loaded new data, start at the beginning..
		m_FractionIndex = 0;
		m_Fraction = m_FractionContainer[m_FractionIndex];
		m_BitRate += m_DataStream->Properties()->BitRate();
		m_CollatedFrames++;
	}

	return (m_FractionContainer.IsEmpty() == false);
}

} } // namespace Solutions::Source

