#ifndef __MPEGTABLE_H
#define __MPEGTABLE_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "MPEGSection.h"
#include "MPEGAssembler.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----

namespace Solutions { namespace MPEG
{
	class EXTERNAL Table
	{
		public:
			class Iterator
			{
				public:
					inline Iterator() : m_Info(), m_Offset(NUMBER_MAX_UNSIGNED(uint32)), m_Size(0)
					{
					}
					inline Iterator(const Generics::DataElement& info) : m_Info(info), m_Offset(NUMBER_MAX_UNSIGNED(uint32)), m_Size(0)
					{
					}
					inline Iterator(const Iterator& copy) : m_Info(copy.m_Info), m_Offset(copy.m_Offset), m_Size(copy.m_Size)
					{
					}
					inline ~Iterator()
					{
					}

					Iterator& operator= (const Iterator& RHS)
					{
						m_Info = RHS.m_Info;
						m_Offset = RHS.m_Offset;
						m_Size = RHS.m_Size;

						return (*this);
					}

				public:
					inline bool IsValid () const
					{
						return ( (m_Offset != NUMBER_MAX_UNSIGNED(uint32)) && (m_Offset < m_Info.Size()) );
					}
					inline void Reset ()
					{
						m_Offset = NUMBER_MAX_UNSIGNED(uint32);
					}
					inline bool Next ()
					{
						if (m_Offset == NUMBER_MAX_UNSIGNED(uint32))
						{
							m_Offset = 0;
							m_Size = Section(Generics::DataElement(m_Info, m_Offset)).Length();
						}
						else if (m_Offset < m_Info.Size())
						{
							m_Offset += m_Size;
							m_Size = Section(Generics::DataElement(m_Info, m_Offset)).Length();
						}

						return (IsValid());
					}
					inline uint32 Count() const
					{
						uint32 count = 0;
						uint32 offset = 0;

						// Run through and count
						while (offset < m_Info.Size())
						{
							offset += Section(Generics::DataElement(m_Info, offset)).Length();
							count++;
						}

						return (count);
					}
					inline Section Element ()
					{
						ASSERT(IsValid());

						return (Section(Generics::DataElement(m_Info, m_Offset, m_Size)));
					}
					template <typename SPECIFICTABLE>
					inline SPECIFICTABLE Element ()
					{
						ASSERT(IsValid());

						return (SPECIFICTABLE(ExtendedSection(Generics::DataElement(m_Info, m_Offset, m_Size))));
					}

				private:
					Generics::DataElement	m_Info;
					uint32					m_Offset;
					uint32					m_Size;
			};

		public:
			Table () :
				m_Version(0),
				m_Current(true),
				m_RequiredSections(NUMBER_MAX_UNSIGNED(uint16)),
				m_Sections(),
				m_LoadedSections(0)
			{
			}
			Table (const Generics::DataElement& storageSpace, bool current = true) :
				m_Version(0),
				m_Current(current),
				m_RequiredSections(NUMBER_MAX_UNSIGNED(uint16)),
				m_Sections(storageSpace),
				m_LoadedSections(0)
			{
			}
			Table (const Table& copy) :
				m_Version(copy.m_Version),
				m_Current(copy.m_Current),
				m_RequiredSections(copy.m_RequiredSections),
				m_Sections (copy.m_Sections),
				m_LoadedSections (copy.m_LoadedSections)
			{
			}
			~Table()
			{
			}

			Table& operator= (const Table& RHS)
			{
				m_Version = RHS.m_Version;
				m_Current = RHS.m_Current;
				m_Sections = RHS.m_Sections;
				m_RequiredSections = RHS.m_RequiredSections;
				m_LoadedSections = RHS.m_LoadedSections;

				return (*this);
			}

		public:
			bool AddSection (const ExtendedSection& section);

			inline bool IsValid() const
			{
				return ((m_RequiredSections != NUMBER_MAX_UNSIGNED(uint16)) && (m_LoadedSections == m_RequiredSections));
			}
			inline uint8 Version () const
			{
				return (m_Version);
			}
			inline bool IsCurrent () const
			{
				return (m_Current);
			}
			inline void Reset ()
			{
				m_RequiredSections = NUMBER_MAX_UNSIGNED(uint16);
			}
			inline uint32 Length () const
			{
				return (static_cast<uint32>(m_Sections.Size()));
			}
			inline Iterator Elements ()
			{
				Iterator result (m_Sections);
				return (result);
			}
			inline operator const Generics::DataElement& () const
			{
				return (m_Sections);
			}

		private:
			uint8						m_Version;
			bool						m_Current;
			uint16						m_LoadedSections;
			uint16						m_RequiredSections;
			Generics::DataElement		m_Sections;
	};

	template <typename SECTIONTYPE, typename POLICYCLASS>
	class TableAssemblerType
	{
		private:
			template <typename SECTIONTYPE, typename POLICYCLASS>
			class SectionAssembler : public SectionAssemblerType<SECTIONTYPE,POLICYCLASS>
			{
				private:
					SectionAssembler(const SectionAssembler<SECTIONTYPE, POLICYCLASS>&);
					SectionAssembler<SECTIONTYPE, POLICYCLASS>& operator= (const SectionAssembler<SECTIONTYPE,POLICYCLASS>&);

				public:
					inline SectionAssembler(const uint16 pid, TableAssemblerType<SECTIONTYPE,POLICYCLASS>& parent) : SectionAssemblerType<SECTIONTYPE,POLICYCLASS>(pid), m_Parent(parent)
					{
					}
					inline ~SectionAssembler()
					{
					}

				public:
					virtual void Assembled (const SECTIONTYPE& data)
					{
						m_Parent.NewSection (data);
					}

				private:
					TableAssemblerType<SECTIONTYPE,POLICYCLASS>&	m_Parent;
			};

		private:
			TableAssemblerType(const TableAssemblerType<SECTIONTYPE,POLICYCLASS>& copy);
			TableAssemblerType<SECTIONTYPE,POLICYCLASS>& operator= (const TableAssemblerType<SECTIONTYPE,POLICYCLASS>&);

		public:
			inline TableAssemblerType(const uint16 pid = NUMBER_MAX_UNSIGNED(uint16)) : 
				m_SectionAssembler(pid, *this),
				m_CurrentTable(Generics::DataElement(Generics::DataStorage::Create(1024)), true),
				m_NextTable(Generics::DataElement(Generics::DataStorage::Create(1024)), false)
			{
			}
			inline ~TableAssemblerType()
			{
			}

		public:
			inline uint16 PID () const
			{
				return (m_SectionAssembler.PID());
			}
			inline void Reset (const uint16 pid = PID())
			{
				return (m_SectionAssembler.Reset(pid));
			}
			bool Assemble (const TransportPackage& package)
			{
				return (m_SectionAssembler.Assemble(package));
			}
			inline Table& CurrentTable ()
			{
				return (m_CurrentTable);
			}
			inline Table& NextTable ()
			{
				return (m_NextTable);
			}
			virtual void Assembled (const Table& /*data */)
			{
			}

		private:
			void NewSection(const SECTIONTYPE& newSection)
			{
				if (newSection.IsValid ())
				{
					if (newSection.IsCurrent())
					{
						m_CurrentTable.AddSection(newSection);

						if (m_CurrentTable.IsValid())
						{
							// We have a full table, tell the people..
							Assembled(m_CurrentTable);
						}
					}
					else
					{
						m_NextTable.AddSection(newSection);

						if (m_NextTable.IsValid())
						{
							// We have a full table, tell the people..
							Assembled(m_NextTable);
						}
					}
				}
			}

		private:
			SectionAssembler<SECTIONTYPE,POLICYCLASS>	m_SectionAssembler;
			Table										m_CurrentTable;
			Table										m_NextTable;
	};

	class EXTERNAL NIT : public ExtendedSection
	{
		public:
			typedef TableAssemblerType<ExtendedSection,SectionAssemblerPolicy<1024>>	Assembler;

		public:
			NIT ()
			{
			}
			NIT (const ExtendedSection& data) : ExtendedSection(data)
			{
			}
			NIT(const NIT& copy) : ExtendedSection(copy)
			{
			}
			~NIT()
			{
			}

			NIT& operator= (const NIT& RHS)
			{
				ExtendedSection::operator=(RHS);
				return (*this);
			}

		public:
			inline DescriptorIterator NetworkDescriptors () const
			{
				uint16 length = (((static_cast<ExtendedSection>(*this)[0] & 0x0F) << 8) | static_cast<ExtendedSection>(*this)[1]);

				return (DescriptorIterator  (Generics::DataElement(MPEG::ExtendedSection::Data(2, length))));
			}
	};

	class EXTERNAL PAT : public ExtendedSection
	{
		public:
			typedef TableAssemblerType<ExtendedSection,SectionAssemblerPolicy<1024>>	Assembler;

		public:
			class Iterator
			{
				private:
					Iterator();

				public:
					Iterator(const Generics::DataElement& data) : m_Index(NUMBER_MAX_UNSIGNED(uint16)), m_Info(data, 0, data.Size()-4)
					{
					}
					Iterator(const Iterator& copy) : m_Index(copy.m_Index), m_Info(copy.m_Info)
					{
					}
					~Iterator()
					{
					}

					Iterator& operator= (const Iterator& RHS)
					{
						m_Info = RHS.m_Info;
						m_Index = RHS.m_Index;
						return (*this);
					}

				public:
					bool IsValid () const
					{
						return ((m_Index != NUMBER_MAX_UNSIGNED(uint16)) && (m_Index < (m_Info.Size() >> 2)));
					}
					void Reset ()
					{
						m_Index = NUMBER_MAX_UNSIGNED(uint16);
					}

					bool Next ()
					{
						if (m_Index == NUMBER_MAX_UNSIGNED(uint16))
						{
							m_Index = 0;
						}
						else if (m_Index < (m_Info.Size() >> 2))
						{
							m_Index++;
						}

						return (IsValid());
					}

					uint16 Pid () const
					{
						return (m_Info.GetNumber<uint16, Generics::ENDIAN_BIG>((m_Index << 2) + 2) & 0x1FFF);
					}

					uint16 ProgramNumber () const
					{
						return (m_Info.GetNumber<uint16, Generics::ENDIAN_BIG>(m_Index << 2));
					}

					uint16 Count() const
					{
						return (static_cast<uint16>(m_Info.Size()) >> 2);
					}

				private:
					uint32					m_Index;
					Generics::DataElement	m_Info;
			};

		public:
			PAT ()
			{
			}
			PAT (const ExtendedSection& data) : ExtendedSection(data)
			{
			}
			PAT(const PAT& copy) : ExtendedSection(copy)
			{
			}
			~PAT()
			{
			}

			PAT& operator= (const PAT& RHS)
			{
				ExtendedSection::operator=(RHS);
				return (*this);
			}

		public:
			static uint8 TableId ()
			{
				return (0x00);
			}
			inline bool IsValid () const
			{
				return (ExtendedSection::IsValid() && (ExtendedSection::TableId() == PAT::TableId()));
			}
			inline Iterator Associations () const
			{
				return (Iterator(ExtendedSection::Data(0))); 
			}
	};

	class EXTERNAL PMT : public ExtendedSection
	{
		public:
			typedef TableAssemblerType<ExtendedSection,SectionAssemblerPolicy<1024>>	Assembler;

		public:
			class Iterator
			{
				public:
					Iterator(): m_Index(NUMBER_MAX_UNSIGNED(uint16)), m_Info(), m_Count(0)
					{
					}
					Iterator(const Generics::DataElement& data) : m_Index(NUMBER_MAX_UNSIGNED(uint16)), m_Info(data, 0, data.Size()-4), m_Count(0)
					{
						while (Next())
						{
							m_Count++;
						}
						Reset();
					}
					Iterator(const Iterator& copy) : m_Index(copy.m_Index), m_Info(copy.m_Info), m_Count(copy.m_Count)
					{
					}
					~Iterator()
					{
					}

					Iterator& operator= (const Iterator& RHS)
					{
						m_Info = RHS.m_Info;
						m_Index = RHS.m_Index;
						m_Count = RHS.m_Count;

						return (*this);
					}

				public:
					bool IsValid () const
					{
						return ((m_Index != NUMBER_MAX_UNSIGNED(uint16)) && (m_Index < m_Info.Size()));
					}

					void Reset ()
					{
						m_Index = NUMBER_MAX_UNSIGNED(uint16);
					}

					bool Next ()
					{
						if (m_Index == NUMBER_MAX_UNSIGNED(uint16))
						{
							m_Index = 0;
						}
						else if (m_Index < m_Info.Size())
						{
 							m_Index += (5 + (((m_Info[m_Index + 3] & 0x03) << 8) | (m_Info[m_Index + 4])));

							// It should always fit the size !!
							ASSERT (m_Index <= m_Info.Size());
						}

						return (IsValid());
					}

					uint8 StreamType () const
					{
						return (m_Info[m_Index]);
					}

					uint16 Pid () const
					{
						return (m_Info.GetNumber<uint16, Generics::ENDIAN_BIG>(m_Index + 1) & 0x1FFF);
					}

					DescriptorIterator Descriptors () const
					{
						uint32 size = (((m_Info[m_Index + 3] & 0x03) << 8) | (m_Info[m_Index + 4]));

						return (DescriptorIterator(Generics::DataElement(m_Info, m_Index + 5, size)));
					}

					uint16 Count() const
					{
						return (m_Count);
					}

				private:
					uint16					m_Count;
					uint32					m_Index;
					Generics::DataElement	m_Info;
			};

		public:
			PMT ()
			{
			}
			PMT (const ExtendedSection& data) : ExtendedSection(data)
			{
			}
			PMT(const PMT& copy) : ExtendedSection(copy)
			{
			}
			~PMT()
			{
			}

			PMT& operator= (const PMT& RHS)
			{
				ExtendedSection::operator=(RHS);
				return (*this);
			}

		public:
			DescriptorIterator Descriptors () const
			{
				uint32 size = (((static_cast<ExtendedSection>(*this)[2] & 0x03) << 8) | (static_cast<ExtendedSection>(*this)[3]));

				return (DescriptorIterator(ExtendedSection::Data(4, size)));
			}
			Iterator Streams () const
			{
				uint32 offset = (((static_cast<ExtendedSection>(*this)[2] & 0x03) << 8) | (static_cast<ExtendedSection>(*this)[3]));

				return (Iterator(ExtendedSection::Data(4 + offset)));
			}
	};

} } // namespace Solutions::DVB

#endif // __MPEGTABLE_H