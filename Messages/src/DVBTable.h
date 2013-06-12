#ifndef __DVBTABLE_H
#define __DVBTABLE_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "MPEGSection.h"
#include "MPEGTable.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----

namespace Solutions { namespace DVB
{
	class EXTERNAL SDT : public MPEG::ExtendedSection
	{
		public:
			typedef MPEG::TableAssemblerType<ExtendedSection,MPEG::SectionAssemblerPolicy<1024>>	Assembler;

		public:
			SDT ()
			{
			}
			SDT (const Generics::DataElement& data) : MPEG::ExtendedSection(data)
			{
			}
			SDT(const SDT& copy) : MPEG::ExtendedSection(copy)
			{
			}
			~SDT()
			{
			}

			SDT& operator= (const SDT& RHS)
			{
				MPEG::ExtendedSection::operator=(RHS);
				return (*this);
			}

		public:
			inline uint16 OriginalNetworkId () const
			{
				return ((static_cast<MPEG::ExtendedSection>(*this)[0] << 8) | static_cast<MPEG::ExtendedSection>(*this)[1]);
			}
	};

	class EXTERNAL BAT : public MPEG::ExtendedSection
	{
		public:
			typedef MPEG::TableAssemblerType<ExtendedSection,MPEG::SectionAssemblerPolicy<1024>>	Assembler;

		public:
			BAT ()
			{
			}
			BAT (const Generics::DataElement& data) : MPEG::ExtendedSection(data)
			{
			}
			BAT(const BAT& copy) : MPEG::ExtendedSection(copy)
			{
			}
			~BAT()
			{
			}

			BAT& operator= (const BAT& RHS)
			{
				MPEG::ExtendedSection::operator=(RHS);
				return (*this);
			}

		public:
			inline MPEG::DescriptorIterator BouquetDescriptors () const
			{
				uint16 length = (((static_cast<MPEG::ExtendedSection>(*this)[0] & 0x0F) << 8) | (static_cast<MPEG::ExtendedSection>(*this)[1]));

				return (MPEG::DescriptorIterator  (Generics::DataElement(MPEG::ExtendedSection::Data(2, length))));
			}
	};

	class EXTERNAL EIT : public MPEG::ExtendedSection
	{
		public:
			typedef MPEG::TableAssemblerType<ExtendedSection,MPEG::SectionAssemblerPolicy<4096>>	Assembler;

		public:
			EIT ()
			{
			}
			EIT (const Generics::DataElement& data) : MPEG::ExtendedSection(data)
			{
			}
			EIT(const EIT& copy) : MPEG::ExtendedSection(copy)
			{
			}
			~EIT()
			{
			}

			EIT& operator= (const EIT& RHS)
			{
				MPEG::ExtendedSection::operator=(RHS);
				return (*this);
			}

		public:
			inline uint16 TransportStreamId () const
			{
				return ((static_cast<MPEG::ExtendedSection>(*this)[0] << 8) | static_cast<MPEG::ExtendedSection>(*this)[1]);
			}
			inline uint16 OriginalNetworkId () const
			{
				return ((static_cast<MPEG::ExtendedSection>(*this)[2] << 8) | static_cast<MPEG::ExtendedSection>(*this)[3]);
			}
			inline uint8 SegmentLastSectionNumber () const
			{
				return (static_cast<MPEG::ExtendedSection>(*this)[4]);
			}
			inline uint8 LastTableId () const
			{
				return (static_cast<MPEG::ExtendedSection>(*this)[5]);
			}
	};

	class EXTERNAL TOT : public MPEG::Section
	{
		public:
			typedef MPEG::SectionAssemblerType<MPEG::Section,MPEG::SectionAssemblerPolicy<1024>>	Assembler;

		public:
			TOT ()
			{
			}
			TOT (const Generics::DataElement& data) : MPEG::Section(data)
			{
			}
			TOT(const TOT& copy) : MPEG::Section(copy)
			{
			}
			~TOT()
			{
			}

			TOT& operator= (const TOT& RHS)
			{
				MPEG::Section::operator=(RHS);
				return (*this);
			}

		public:
			inline uint64 UTCTime () const
			{
				return ((static_cast<MPEG::Section>(*this)[0] << 32) | (static_cast<MPEG::Section>(*this)[1] << 24) | (static_cast<MPEG::Section>(*this)[2] << 16) | (static_cast<MPEG::Section>(*this)[3] << 8) | static_cast<MPEG::Section>(*this)[4]);
			}
			inline MPEG::DescriptorIterator Descriptors () const
			{
				uint16 length = (((static_cast<MPEG::Section>(*this)[5] & 0x0F) << 8) | static_cast<MPEG::Section>(*this)[6]);

				return (MPEG::DescriptorIterator (Generics::DataElement(MPEG::Section::Data(7, length))));
			}
	};

	class EXTERNAL TDTSection : public MPEG::Section
	{
		public:
			typedef MPEG::SectionAssemblerType<MPEG::Section,MPEG::SectionAssemblerPolicy<1024>>	Assembler;

		public:
			TDTSection ()
			{
			}
			TDTSection (const Generics::DataElement& data) : MPEG::Section(data)
			{
			}
			TDTSection(const TDTSection& copy) : MPEG::Section(copy)
			{
			}
			~TDTSection()
			{
			}

			TDTSection& operator= (const TDTSection& RHS)
			{
				MPEG::Section::operator=(RHS);
				return (*this);
			}

		public:
			inline uint64 UTCTime () const
			{
				return ((static_cast<MPEG::Section>(*this)[0] << 32) | (static_cast<MPEG::Section>(*this)[1] << 24) | (static_cast<MPEG::Section>(*this)[2] << 16) | (static_cast<MPEG::Section>(*this)[3] << 8) | static_cast<MPEG::Section>(*this)[4]);
			}
	};

} } // namespace Solutions::DVB

#endif // __DVBSECTION_H