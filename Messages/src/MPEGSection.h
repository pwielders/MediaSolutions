#ifndef __MPEGSECTION_H
#define __MPEGSECTION_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "MPEGAssembler.h"
#include "MPEGTransportPackage.h"
#include "MPEGDescriptors.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----

namespace Solutions { namespace MPEG
{
	class EXTERNAL Section
	{
		public:
			inline Section () :
				m_Section ()
			{
			}
			inline Section (const Generics::DataElement& data) :
				m_Section (data)
			{
			}
			inline Section (const Section& copy) :
				m_Section (copy.m_Section)
			{
			}
			inline ~Section ()
			{
			}

			inline Section& operator= (const Section& RHS)
			{
				m_Section = RHS.m_Section;

				return (*this);
			}

		public:
			inline bool IsValid () const
			{
				return ((m_Section.Size() >= 3) && (m_Section.Size() == Length()));
			}
			inline uint8 TableId () const // 8 Bits
			{
				return (m_Section[0]);
			}
			bool SectionSyntax () const
			{
				return ((m_Section[1] & 0x80) != 0);
			}
			inline uint16 Length () const
			{
				return (((m_Section[1] & 0x0F) << 8) | (m_Section[2])) + 3; // NIT-BAT-SDT
			}
			inline uint8& operator[] (const uint32 index)
			{
				return (m_Section[3+index]);
			}
			inline const uint8& operator[] (const uint32 index) const
			{
				return (m_Section[3+index]);
			}
			inline Generics::DataElement Data (const uint32 offset, const uint32 size = 0)
			{
				return (Generics::DataElement(m_Section, 3+offset, size));
			}
			inline const Generics::DataElement Data (const uint32 offset, const uint32 size = 0) const
			{
				return (Generics::DataElement(m_Section, 3+offset, size));
			}
			inline operator const Generics::DataElement& () const
			{
				return (m_Section);
			}

		protected:
			bool ValidCRC () const
			{
				uint32 counterCRC = m_Section.GetNumber<uint32, Generics::ENDIAN_BIG>(m_Section.Size() - 4);
				return (m_Section.CRC32(0, m_Section.Size() - 4) == counterCRC);
			}
		private:
			Generics::DataElement	m_Section;
	};

	class EXTERNAL ExtendedSection : public Section
	{
		public:
			inline ExtendedSection ()
			{
			}
			inline ExtendedSection (const Generics::DataElement& data) :
				Section (data)
			{

			}
			inline ExtendedSection (const ExtendedSection& copy) :
				Section (copy)
			{
			}
			inline ~ExtendedSection ()
			{
			}

			inline ExtendedSection& operator= (const ExtendedSection& RHS)
			{
				Section::operator=(RHS);

				return (*this);
			}

		public:
			inline bool IsValid () const
			{
				return (Section::IsValid() && (ValidCRC()));
			}
			inline uint16 Id () const
			{
				return ((Section::operator[](0) << 8) | (Section::operator[](1)));
			}
			inline uint8 Version () const
			{
				return ((Section::operator[](2) & 0x3F) >>  1);
			}
			inline bool IsCurrent () const
			{
				return ((Section::operator[](2) & 0x01) != 0);
			}
			bool IsNext () const
			{
				return ((Section::operator[](2) & 0x01) == 0);
			}
			inline uint8 SectionNumber () const
			{
				return (Section::operator[](3));
			}
			inline uint8 LastSectionNumber () const
			{
				return (Section::operator[](4));
			}
			inline uint8& operator[] (const uint32 index)
			{
				return (Section::operator[](5+index));
			}
			inline const uint8& operator[] (const uint32 index) const
			{
				return (Section::operator[](5+index));
			}
			inline Generics::DataElement Data (const uint32 offset, const uint32 size = 0)
			{
				return (Section::Data(5+offset, size));
			}
			inline const Generics::DataElement Data (const uint32 offset, const uint32 size = 0) const
			{
				return (Section::Data(5+offset, size));
			}
			inline uint32 Hash () const
			{
				// Id(16)/TableId(8)/Version(5)/CurNext(1)/SectionIndex(1)
				return ((Id() << 16) | (TableId() << 8) | (Version() << 3) | (IsCurrent() ? 0x04 : 0x00) | (SectionSyntax() ? 0x02 : 0x00)); 
			}
	};

	template<typename SECTIONTYPE, typename POLICYCLASS>
	class SectionAssemblerType
	{
		private:
			template<typename SECTIONTYPE, const typename POLICYCLASS>
			class ForwardClass : public AssemblerType<true, POLICYCLASS>
			{
				private:
					ForwardClass();
					ForwardClass(const ForwardClass<SECTIONTYPE,POLICYCLASS>& copy);
					ForwardClass<SECTIONTYPE,POLICYCLASS>& operator= (const ForwardClass<SECTIONTYPE,POLICYCLASS>&);

				public:
					ForwardClass (SectionAssemblerType<SECTIONTYPE, POLICYCLASS>& parent, const uint16 pid) : 
						AssemblerType<true,POLICYCLASS>(pid),
						m_Parent(parent)
					{
					}
					~ForwardClass()
					{
					}

				public:
					virtual void Assembled (const Generics::DataElement& data)
					{
						m_Parent.Assembled(SECTIONTYPE(data));
					}

					inline Generics::OptionalType<SECTIONTYPE> LastSection () const
					{
						Generics::OptionalType<Generics::DataElement>	result (m_Assembler.LastDataElement());

						return (result.IsSet() ? Generics::OptionalType<SECTIONTYPE>(SECTIONTYPE(result.Value())) : Generics::OptionalType<SECTIONTYPE> ()); 
					}

					inline Generics::OptionalType<SECTIONTYPE> CurrentSection () const
					{
						Generics::OptionalType<Generics::DataElement>	result (m_Assembler.CurrentDataElement());

						return (result.IsSet() ? Generics::OptionalType<SECTIONTYPE>(SECTIONTYPE(result.Value())) : Generics::OptionalType<SECTIONTYPE> ()); 
					}

			private:
					SectionAssemblerType<SECTIONTYPE, POLICYCLASS>&	m_Parent;
			};

		private:
			SectionAssemblerType();
			SectionAssemblerType(const SectionAssemblerType<SECTIONTYPE,POLICYCLASS>& copy);
			SectionAssemblerType<SECTIONTYPE,POLICYCLASS>& operator= (const SectionAssemblerType<SECTIONTYPE,POLICYCLASS>&);

		public:
			SectionAssemblerType(const uint16 pid) : m_Assembler(*this, pid)
			{
			}
			~SectionAssemblerType()
			{
			}

		public:
			inline uint16 PID () const
			{
				return (m_Assembler.PID());
			}
			inline void Reset (const uint16 pid = PID())
			{
				return (m_Assembler.Reset(pid));
			}
			inline bool Assemble (const TransportPackage& package)
			{
				return (m_Assembler.Assemble(package));
			}
			inline Generics::OptionalType<SECTIONTYPE> LastSection () const
			{
				return (m_Assembler.LastSection()); 
			}
			inline Generics::OptionalType<SECTIONTYPE> CurrentSection () const
			{
				return (m_Assembler.CurrenttSection()); 
			}
			virtual void Assembled (const SECTIONTYPE& /* data */)
			{
			}

		private:
			ForwardClass<SECTIONTYPE, POLICYCLASS>	m_Assembler;
	};

} } // namespace Solutions::MPEG

#endif // __MPEGSECTION_H