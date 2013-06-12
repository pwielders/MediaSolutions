#ifndef __SLOTTEDSTORAGEITERATOR_H
#define __SLOTTEDSTORAGEITERATOR_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "SlottedStorage.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Generics
{
	template<typename POLICY, typename SLOTTEDOBJECT, uint16 LENGTH>
	class SlottedStorageIterator
	{
		private:
			typedef SlottedStorage<SLOTTEDOBJECT,LENGTH> Container;

		private:
			SlottedStorageIterator (const SlottedStorageIterator<POLICY,SLOTTEDOBJECT,LENGTH>&);
			SlottedStorageIterator<POLICY,SLOTTEDOBJECT,LENGTH>& operator= (const SlottedStorageIterator<POLICY,SLOTTEDOBJECT,LENGTH>&);

		public:
			inline SlottedStorageIterator(POLICY& policy, Container& container) :
				m_CurrentIndex (0),
				m_CurrentBlock (NULL),
				m_Policy (policy),
				m_Container (container)
			{
			}
			virtual ~SlottedStorageIterator()
			{
			}

			inline bool IsValid () const
			{
				return (m_CurrentBlock != NULL);
			}

			inline uint64 BaseTime () const
			{
				return (m_BaseTime);
			}

			inline void Reset ()
			{
				m_CurrentBlock = NULL;
			}

			bool Next ()
			{
				if (m_CurrentBlock == NULL) 
				{
					// Record the base time, we start again !!
					m_BaseTime = Time::Now().Ticks();

					m_Container.Lock ();

					if (m_Container.HasData () == false)
					{
						SlotInfoType<SLOTTEDOBJECT> newItem;

						// Seems we have a completely empty domain. Get the first item..
						if (m_Policy.First(newItem) == true)
						{
							m_Container.AddSlot (newItem);
							m_CurrentBlock = &(m_Container.Root());
							m_CurrentIndex = 0;
						}
					}
					else
					{
						m_CurrentBlock = &(m_Container.Root());
						m_CurrentIndex = 0;
					}

					m_Container.Unlock ();
				}
				else if (m_CurrentBlock->Length () > (m_CurrentIndex +1))
				{
					// Space enough, we just increment.
					m_CurrentIndex++;
				}
				else if (m_CurrentBlock->NextSlot() != NULL)
				{
					// We have a currentblock and we are the end of this block, move on to the 
					// next block..
					m_CurrentBlock = m_CurrentBlock->NextSlot();
					m_CurrentIndex = 0;
				}
				else
				{
					// Oops we are out of slots. Can we create one ?
					SlotInfoType<SLOTTEDOBJECT> newItem;

					m_Container.Lock ();

					// Seems we have a completely empty domain. Get the first item..
					if (m_Policy.Next(newItem) == true)
					{
						m_Container.AddSlot (newItem);

						if (m_CurrentBlock->Length() > (m_CurrentIndex +1))
						{
							m_CurrentIndex++;
						}
						else
						{
							m_CurrentBlock = m_CurrentBlock->NextSlot();
							m_CurrentIndex = 0;
						}
					}
					else
					{
						// We are at the end of the list !!!
						m_CurrentBlock = NULL;
					}

					m_Container.Unlock ();
				}

				return (m_CurrentBlock != NULL);
			}

			bool SetToSlot (const uint64 slotHash)
			{
				if ((slotHash < BeginSlot ()) || (slotHash > EndSlot()))
				{
					m_CurrentIndex = m_Container.FindIndex(slotHash, m_CurrentBlock);
				}

				if (m_CurrentBlock != NULL)
				{
					// Do not forget to adjust the base time..
					m_BaseTime = (Time::Now().Ticks() - BeginSlot());
				}

				ASSERT ((m_CurrentBlock != NULL) ^ (m_CurrentIndex == ~0));

				return (m_CurrentBlock != NULL);
			}

			inline SLOTTEDOBJECT& Data ()
			{
				SLOTTEDOBJECT* result = (*m_CurrentBlock)[m_CurrentIndex];

				ASSERT (result != NULL);

				return (*result);
			}

			inline const SLOTTEDOBJECT& Data () const
			{
				const SLOTTEDOBJECT* result = (*m_CurrentBlock)[m_CurrentIndex];

				ASSERT (result != NULL);

				return (*result);
			}

			inline uint64 BeginSlot () const
			{
				return (m_CurrentBlock->BeginSlot(m_CurrentIndex));
			}

			inline uint64 EndSlot () const
			{
				return (m_CurrentBlock->EndSlot(m_CurrentIndex));
			}

			POLICY& Policy ()
			{
				return (m_Policy);
			}

			const POLICY& Policy () const
			{
				return (m_Policy);
			}

		private:
			uint64														m_BaseTime;
			unsigned int												m_CurrentIndex;
			typename Container::SlotInfoArray<SLOTTEDOBJECT,LENGTH>*	m_CurrentBlock;
			POLICY&														m_Policy;
			Container&													m_Container;
	};

} } // namespace Solutions::Generics

#endif // __SLOTTEDSTORAGEITERATOR_H