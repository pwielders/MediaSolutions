#include "MPEGTable.h"

namespace Solutions { namespace MPEG
{

bool Table::AddSection (const ExtendedSection& section)
{
	bool addedSection = false;

	if ( (m_RequiredSections == NUMBER_MAX_UNSIGNED(uint16)) || (section.Version() != m_Version) )
	{
		// Time to start building a new thing..
		m_Version = section.Version();
		m_Current = section.IsCurrent();
		m_RequiredSections = section.LastSectionNumber() + 1;
		m_LoadedSections = 1;

		if (m_Sections.Size(section.Length()) == true)
		{
			m_Sections.Copy(section);
			addedSection = true;
		}
	}
	else if (m_Current == section.IsCurrent())
	{
		if (section.LastSectionNumber() > m_RequiredSections)
		{
			m_RequiredSections = section.LastSectionNumber();
		}

		uint32 offset = 0;

		// Run through and count
		while ( (addedSection == false) && (offset < m_Sections.Size()) )
		{
			ExtendedSection current (Generics::DataElement(m_Sections, offset));

			if (current.SectionNumber() == section.SectionNumber())
			{
				// Check the size of the current one
				if (current.Length() < section.Length())
				{
					m_Sections.Expand(offset + current.Length(), section.Length() - current.Length());
				}
				else if (current.Length() > section.Length())
				{
					// Oops, rearrange..
					m_Sections.Shrink(offset, current.Length() - section.Length());
				}

				// Space oke, copy it in..
				m_Sections.Copy(section, offset);

				// It's a replace 
				addedSection = true;
			}
			else if (current.SectionNumber() > section.SectionNumber())
			{
				// Increase the space for this buffer
				m_Sections.Expand(offset, section.Length());

				// Space oke, copy it in..
				m_Sections.Copy(section, offset);

				// Shift it in front of this one...
				addedSection = true;
			}
		}

		if (addedSection == false)
		{
			// Add it to the end...
			if (m_Sections.Size(m_Sections.Size() + section.Length()) == true)
			{
				// We created some additional space
				m_Sections.Copy (section, offset);
			}

		}
	}

	return (addedSection);
}

} } // namespace Solutions::MPEG