#ifndef __UNIVECTOR_H
#define __UNIVECTOR_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Generics
{
	// This class is NOT thread safe!!!!
	// Thhis
	template<typename ELEMENT>
	class UniVectorType
	{
		private:
			UniVectorType();

		public:
			UniVectorType(const uint32 startSize) :
				m_Elements(reinterpret_cast<ELEMENT*>(::malloc(((sizeof(ELEMENT)+7)&0xFFFFFFF8)*startSize))),
				m_MaxValue(startSize),
				m_Filled(0)
			{
				ASSERT (m_Elements != NULL);
			}
			UniVectorType (const UniVectorType<ELEMENT>& copy) :
				m_Elements(reinterpret_cast<ELEMENT*>(::malloc(((sizeof(ELEMENT)+7)&0xFFFFFFF8)*copy.m_MaxValue))),
				m_MaxValue(copy.m_MaxValue),
				m_Filled(copy.m_Filled)
			{
				size_t copySize = (reinterpret_cast<uint8*> (&(copy.m_Elements[copy.m_Filled])) - reinterpret_cast<uint8*>(copy.m_Elements));

				// Now copy there must be enough space..
				::memcpy(m_Elements, copy.m_Elements, copySize);
			}
			~UniVectorType()
			{
				::free(m_Elements);
			}

			// No "destructor" is called for the elements present in this line...
			inline void Clear()
			{
				m_Filled = 0;
			}

			inline uint32 Size() const
			{
				return (m_Filled);
			}

			void Add (const ELEMENT& newItem)
			{
				// Can we still add  a new entry
				if (m_Filled == m_MaxValue)
				{
					m_MaxValue = m_MaxValue * 2;
					m_Elements = reinterpret_cast<ELEMENT*>(::realloc(m_Elements, (((sizeof(ELEMENT)+7)&0xFFFFFFF8)*m_MaxValue)));
				}

				// Make a copy, the copy constructor is NOT called for this object.
				::memcpy(&(m_Elements[m_Filled]), &newItem, sizeof(ELEMENT));

				// We added a new one.
				m_Filled++;
			}

			void Add (const UniVectorType<ELEMENT>& newItems)
			{
				if (newItems.Size() > 0)
				{
					if ((m_Filled + newItems.Size()) > m_MaxValue)
					{
						do
						{
							m_MaxValue = m_MaxValue * 2;

						} while (m_MaxValue < (m_Filled + newItems.Size()));

						m_Elements = reinterpret_cast<ELEMENT*>(::realloc(m_Elements, (((sizeof(ELEMENT)+7)&0xFFFFFFF8)*m_MaxValue)));
					}

					size_t copySize = (reinterpret_cast<uint8*> (&(newItems.m_Elements[newItems.m_Filled])) - reinterpret_cast<uint8*>(newItems.m_Elements));

					// Make a copy, the copy constructor is NOT called for this object.
					::memcpy(&(m_Elements[m_Filled]), newItems.m_Elements, copySize);

					m_Filled += newItems.Size();
				}
			}

			UniVectorType<ELEMENT>& operator= (const UniVectorType<ELEMENT>& origin)
			{
				if (origin.m_Filled > m_MaxValue)
				{
					if (m_Elements != NULL)
					{
						::free (m_Elements);
					}

					// Time to resize..
					m_MaxValue = origin.m_MaxValue;
					m_Elements = reinterpret_cast<ELEMENT*>(::malloc(((sizeof(ELEMENT)+7)&0xFFFFFFF8)*m_MaxValue));
				}

				size_t copySize = (reinterpret_cast<uint8*> (&(origin.m_Elements[origin.m_Filled])) - reinterpret_cast<uint8*>(origin.m_Elements));

				// Now copy there must be enough space..
				::memcpy(m_Elements, origin.m_Elements, copySize);

				m_Filled = origin.m_Filled;

				return (*this);
			}

			ELEMENT& operator[] (const uint32 index)
			{
				ASSERT (index < m_Filled);
				return (m_Elements[index]);
			}

			const ELEMENT& operator[] (const uint32 index) const
			{
				ASSERT (index < m_Filled);
				return (m_Elements[index]);
			}

		private:
			ELEMENT*		m_Elements;
			uint32			m_MaxValue;
			uint32			m_Filled;
	};

} } // namespace Solutions::Generics

#endif // __UNIVECTOR_H