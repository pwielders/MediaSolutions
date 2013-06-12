#ifndef __DATAELEMENTCONTAINER_H
#define __DATAELEMENTCONTAINER_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "UniVector.h"
#include "IMedia.h"
#include "DataElement.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Generics
{
	// This class is NOT thread safe!!!!
	class DataElementContainer
	{
		public:
			DataElementContainer() :
				m_FrontQueue(5),
				m_BackQueue(30),
				m_Length(0),
				m_MaxLength(NUMBER_MAX_UNSIGNED(uint64))
			{
			}
			DataElementContainer(const uint64 maxLength) :
				m_FrontQueue(5),
				m_BackQueue(30),
				m_Length(0),
				m_MaxLength(maxLength)
			{
			}
			DataElementContainer(const uint32 backSize, const uint32 frontSize, const uint64 maxLength) :
				m_FrontQueue(frontSize),
				m_BackQueue(backSize),
				m_Length(0),
				m_MaxLength(maxLength)
			{
			}
			DataElementContainer(const DataElementContainer& copy) :
				m_FrontQueue(copy.m_FrontQueue),
				m_BackQueue(copy.m_BackQueue),
				m_Length(copy.m_Length),
				m_MaxLength(copy.m_MaxLength)
			{
			}
			~DataElementContainer()
			{
			}

			DataElementContainer& operator= (const DataElementContainer& RHS)
			{
				m_FrontQueue = RHS.m_FrontQueue;
				m_BackQueue = RHS.m_BackQueue;
				m_Length = RHS.m_Length;
				m_MaxLength = RHS.m_MaxLength;

				return (*this);
			}


		public:
			void Front (const DataElement& element)
			{
				if ((m_Length + element.Size()) <= m_MaxLength)
				{
					m_FrontQueue.Add(element);
					m_Length += element.Size();
				}
			}

			void Back (const DataElement& element)
			{
				if ((m_Length + element.Size()) <= m_MaxLength)
				{
					m_BackQueue.Add(element);
					m_Length += element.Size();
				}
			}

			void Back (const DataElementContainer& frames)
			{
				if ((m_Length + frames.Size()) <= m_MaxLength)
				{
					// First add all elements from the front queue
					for (uint32 teller = frames.m_FrontQueue.Size(); teller != 0; --teller)
					{
						const DataElement& frame = frames.m_FrontQueue[teller-1];

						m_BackQueue.Add(frame);
					}

					m_BackQueue.Add(frames.m_BackQueue);

					m_Length += frames.Size();
				}
			}

			inline bool IsEmpty() const
			{
				return ((m_BackQueue.Size() == 0) && (m_FrontQueue.Size() == 0));
			}

			inline uint32 Elements () const
			{
				return (m_FrontQueue.Size() + m_BackQueue.Size());
			}

			inline uint64 Size () const
			{
				return (m_Length);
			}

			inline uint64 Space () const
			{
				return (m_MaxLength - m_Length);
			}

			inline void Clear () 
			{
				m_Length = 0;
				m_FrontQueue.Clear();
				m_BackQueue.Clear();
			}

			inline DataElement& operator[] (const uint32 index)
			{
				ASSERT (index < (m_FrontQueue.Size() + m_BackQueue.Size()));

				if (index  >= m_FrontQueue.Size())
				{
					return (m_BackQueue[index - m_FrontQueue.Size()]);
				}

				return (m_FrontQueue[m_FrontQueue.Size() - 1 - index]);
			}

			inline const DataElement& operator[] (const uint32 index) const
			{
				ASSERT (index < (m_FrontQueue.Size() + m_BackQueue.Size()));

				if (index >= m_FrontQueue.Size())
				{
					return (m_BackQueue[index - m_FrontQueue.Size()]);
				}

				return (m_FrontQueue[m_FrontQueue.Size() - 1 - index]);
			}

		private:
			uint64							m_Length;
			uint64							m_MaxLength;
			UniVectorType<DataElement>		m_FrontQueue;
			UniVectorType<DataElement>		m_BackQueue;
	};

} } // namespace Solutions::Generics

#endif // __DATAELEMENTCONTAINER_H