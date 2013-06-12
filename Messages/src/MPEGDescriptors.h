#ifndef __MPEGDESCRIPTORS_H
#define __MPEGDESCRIPTORS_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "MPEGTransportPackage.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----

namespace Solutions { namespace MPEG
{
	class EXTERNAL Descriptor
	{
		public:
			inline Descriptor () :
				m_Descriptor()
			{
			}
			inline Descriptor (const Generics::DataElement& data) :
				m_Descriptor (data)
			{
			}
			inline Descriptor (const Descriptor& copy) :
				m_Descriptor(copy.m_Descriptor)
			{
			}
			inline ~Descriptor()
			{
			}

			inline Descriptor& operator= (const Descriptor& RHS)
			{
				m_Descriptor = RHS.m_Descriptor;
				
				return (*this);
			}

		public:
			inline uint8 Tag () const
			{
				return (m_Descriptor[0]);
			}
			inline uint8 Length() const
			{
				return (m_Descriptor[1] + 2);
			}

		private:
			Generics::DataElement	m_Descriptor;
	};

	class EXTERNAL DescriptorIterator
	{
		public:
			inline DescriptorIterator () :
				m_Descriptors(),
				m_Index(NUMBER_MAX_UNSIGNED(uint32))
			{
			}
			inline DescriptorIterator (const Generics::DataElement& data) :
				m_Descriptors (data),
				m_Index(NUMBER_MAX_UNSIGNED(uint32))
			{
			}
			inline DescriptorIterator (const DescriptorIterator& copy) :
				m_Descriptors(copy.m_Descriptors),
				m_Index(copy.m_Index)
			{
			}
			inline ~DescriptorIterator()
			{
			}

			inline DescriptorIterator& operator= (const DescriptorIterator& RHS)
			{
				m_Descriptors = RHS.m_Descriptors;
				m_Index = RHS.m_Index;
				
				return (*this);
			}
		public:
			inline bool IsValid () const
			{
				return (m_Index < m_Descriptors.Size());
			}
			inline void Reset ()
			{
				m_Index = NUMBER_MAX_UNSIGNED(uint32);
			}
			bool Next ()
			{
				if (m_Index == NUMBER_MAX_UNSIGNED(uint32))
				{
					m_Index = 0;
				}
				else if (m_Index < m_Descriptors.Size())
				{
					m_Index += Descriptor(Generics::DataElement(m_Descriptors, m_Index)).Length();
				}
				
				// See if we have a valid descriptor, Does it fit the block we have ?
				if ((m_Index + Descriptor(Generics::DataElement(m_Descriptors, m_Index)).Length()) > m_Descriptors.Size())
				{
					// It's too big, Jump to the end..
					m_Index = static_cast<uint32>(m_Descriptors.Size());
				}

				return (IsValid());
			}
			inline Descriptor Current ()
			{
				return (Descriptor (Generics::DataElement(m_Descriptors, m_Index)));
			}
			inline const Descriptor Current () const
			{
				return (Descriptor (Generics::DataElement(m_Descriptors, m_Index)));
			}

			uint32 Count () const
			{
				uint32 count = 0;
				uint32 offset = 0;
				while (offset < m_Descriptors.Size())
				{
					count++;
					offset += Descriptor(Generics::DataElement(m_Descriptors, offset)).Length();
				}

				if (offset > m_Descriptors.Size())
				{
					// reduce the count by one, the last one is toooooooo big
					count--;
				}

				return (count);
			}

		private:
			Generics::DataElement	m_Descriptors;
			uint32					m_Index;
	};

} } //  // namespace Solutions::MPEG

#endif //__MPEGDESCRIPTORS_H